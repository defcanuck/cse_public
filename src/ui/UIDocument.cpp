#include "PCH.h"

#include "ui/UIDocument.h"
#include "gfx/RenderInterface.h"
#include "global/Utils.h"
#include "global/ResourceFactory.h"
#include "global/Stats.h"

// #define DEBUG_BUFFER_FILL 1

namespace cs
{

	UIDocument::UIDocument(const std::string& name)
		: documentName(name)
		, sortOrder(0)
		, locked(false)
		, pendingRemoval(false)
		, animMode(false)
		, consumeClicks(false)
		, tint(ColorB::White)
	{
		this->root = CREATE_CLASS(UIElement, "root");
		this->root->setVertexColor(ColorB::Clear);
		this->root->setEnabled(true);

		EngineStats::incrementStat(StatTypeUIDocument);
	}

	UIDocument::~UIDocument()
	{
		this->fx.removeAll();
		this->root->removeAllChildren();

		EngineStats::decrementStat(StatTypeUIDocument);
	}

	void UIDocument::init(const RectI& size)
	{

		this->setDocumentSize(RectF(
			PointF(float32(size.pos.x), float32(size.pos.y)),
			PointF(float32(size.size.w), float32(size.size.h))));

		GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);

        data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, data->decl.getStride() });
		data->decl.addAttrib(AttributeType::AttribTexCoord1, { AttributeType::AttribTexCoord1, TypeFloat, 2, data->decl.getStride() });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, data->decl.getStride() });

		data->vertexSize = kInitNumElements * 4;
		data->indexSize = kInitNumElements * 6;
		data->storage = BufferStorageDynamic;

		for (int32 i = 0; i < UIBatchPassMAX; ++i)
		{
			this->batch[i] = CREATE_CLASS(BatchDraw, data);
		}

		this->luaPopulate();
		this->onStartRendering();

	}

	void UIDocument::onStartRendering()
	{
		if (!this->root.get())
		{
			return;
		}

		struct local
		{
			static void onUIStart(UIElement* element)
			{
				assert(element);
				element->onFirstFrame();
			}
		};

		this->root->traverse(&local::onUIStart);

	}

	void UIDocument::draw(const RectI& screen_rect, UIBatchPass pass)
	{
        RenderInterface* rend = RenderInterface::getInstance();
        rend->pushDebugScope(this->getName());
        
		// Update VB (double buffered)
		this->batch[pass]->update(); 

		mat4 projection = glm::ortho(
			float32(screen_rect.pos.x), float32(screen_rect.pos.x + screen_rect.size.w),
			float32(screen_rect.pos.y), float32(screen_rect.pos.y + screen_rect.size.h),
			-10.0f, 10.0f);

		mat4 mvp = projection;

		cs::UniformPtr matrix = SharedUniform::getInstance().getUniform("mvp");
		assert(matrix);
		matrix->setValue(mvp);

        rend->pushDebugScope("elements");
		this->batch[pass]->draw();
        rend->popDebugScope();
        
        rend->pushDebugScope("fx");
		fx.draw();
        rend->popDebugScope();
        
        rend->popDebugScope();
        
	}

	void UIDocument::process(float32 dt, float32& depth, RectI& screenSize, UIBatchPass pass)
	{

		if (pass == UIBatchPassMain)
		{
			this->luaProcess(dt);
		}

		this->batch[pass]->clear();

		if (pass == UIBatchPassMain)
		{
			if (this->animations.animList.size() > 0)
			{
				float32 maxDt = clamp(0.0f, 1.0f / 3.0f, dt);
				this->animations.process(maxDt);
			}

			fx.process(dt);
		}
		
		UIBatchProcessInfo info;
		UIBatchProcessData data;
		data.delta = dt;
		data.depth = depth;
		data.animating = this->animMode;
		data.tint = this->tint;
		data.screen = this->root->getScreenRect(data.bounds);
		data.pass = pass;

		float32 baseDepth = depth;
		this->root->batch(this->batch[pass]->drawData, this->batch[pass]->numVertices, this->batch[pass]->numIndices, data, info);
		if (info.getMaxDepth() > baseDepth)
		{
			depth = baseDepth + (info.getMaxDepth() - baseDepth);
		}

		if (pass == UIBatchPassMain)
		{
			bool ret = this->root->prepareGUI();
			if (ret)
			{
				// log::info("Document consumed click: ", this->getName());
			}
		}
	}

	UIDocumentPtr UIDocumentCache::createNewDocument(const std::string& documentName)
	{
		UIDocumentPtr ptr = this->getDocument(documentName);
		if (ptr)
		{
			log::info("Duplicate Document found for ", documentName);
			return ptr;
		}
			
		UIDocumentPtr doc = CREATE_CLASS(UIDocument, documentName);

		this->addDocument(doc);
		return doc;
	}

	void UIDocumentCache::addDocument(UIDocumentPtr& doc)
	{
		this->cache[doc->getName()] = doc;
	}

	void UIDocumentCache::removeDocument(UIDocumentPtr& doc)
	{
		if (this->cache.at(doc->getName()))
			this->cache.erase(doc->getName());
	}

	UIDocumentPtr& UIDocumentCache::getDocument(const std::string& name)
	{
		UIDocumentCacheMap::iterator it = this->cache.find(name);
		if (it != this->cache.end())
			return it->second;

		static UIDocumentPtr kEmptyUIDocument;
		return kEmptyUIDocument;
	}

	bool UIDocument::onCursor(ClickInput input, ClickParams& params, UIClickResults& results)
	{
		this->luaOnCursor(input, params.state, params.position);
		bool ret = this->root->onCursor(input, params, results);
		return ret || this->consumeClicks;
	}

	void UIDocument::onCursorMove(const vec2& pos)
	{
		this->luaOnCursorMove(pos);
	}

	void UIDocument::setDocumentSize(const RectF& view)
	{
		assert(this->root);
		this->root->setPosition(view.pos);
		this->root->setWidth(view.size.w, SpanPixels);
		this->root->setHeight(view.size.h, SpanPixels);
		this->root->updateLayout();
	}

	void UIDocument::setDocumentColor(const ColorB& color)
	{
		assert(this->root);
		this->root->setVertexColor(color);
	}

	void UIDocument::addEmitter(UIElementPtr& element, const std::string& effectName, const vec3& pos, bool preDraw)
	{
		ParticleEmitter::GetWorldPositionFunc func = [element, pos]()
		{
			return vec3(pos.x, pos.y, pos.z + element->getLastDepth());
		};

		this->addEmitterInternal(element, effectName, func, preDraw);
	}

	void UIDocument::addEmitter(UIElementPtr& element, const std::string& effectName, float32 zOffset, bool preDraw)
	{

		ParticleEmitter::GetWorldPositionFunc func = [element, zOffset]()
		{
			RectF rect = element->getScreenRect();
			PointF center = rect.getCenter();
			return vec3(center.x, center.y, zOffset + element->getLastDepth());
		};

		this->addEmitterInternal(element, effectName, func, preDraw);
	}

	void UIDocument::addEmitterInternal(UIElementPtr& element, const std::string& effectName, ParticleEmitter::GetWorldPositionFunc& func, bool preDraw)
	{
		const std::string elementName = element->getName();
		if (this->fx.exists(elementName))
		{
			this->fx.remove(elementName);
		}

		ResourcePtr resource = ResourceFactory::getInstance()->loadResource<ParticleEffect>(effectName);
		if (!resource.get())
		{
			log::error("UIDocument: unable to find effect ", effectName);
			return;
		}

		ParticleEmitter::SpawnParams params;
		params.worldFunc = func;
		params.contentScale = RenderInterface::getInstance()->getContentScale();
		params.renderManually = true;

		struct local
		{
			static bool drawHeaps(const UIElementPtr& element, std::shared_ptr<ParticleHeapCollection> heapsToDraw)
			{
				static std::map<uintptr_t, int32> gElementToHeapCounter;

				bool drewHeaps = false;
				for (auto& heapIt : heapsToDraw->buffers[RenderTraversalMain])
				{
					ParticleHeapPtr& heap = heapIt.second;
					if (heap->getNumParticles() == 0 && heap->hasGracePeriodExpired())
						continue;

					if (heap->getLinks() > 0 || !heap->isExpired())
					{
						heap->draw();
						drewHeaps = true;
					}
				}
				return drewHeaps;
			}
		};

		ParticleEmitterPtr emitter = this->fx.addEmitter(elementName, effectName, params, RenderTraversalMain);
		if (emitter.get())
		{
			std::shared_ptr<ParticleHeapCollection> heapToUse = this->fx.getHeap(elementName);
			if (preDraw)
			{
				DrawCall::PreDraw callback = [heapToUse, element]()
				{
					if (!local::drawHeaps(element, heapToUse))
					{
						element->setPreCallback(nullptr);
					}
				};
				element->setPreCallback(callback);
			}
			else
			{
				DrawCall::PostDraw callback = [heapToUse, element]()
				{
					if (!local::drawHeaps(element, heapToUse))
					{
						element->setPostCallback(nullptr);
					}
				};
				element->setPostCallback(callback);
			}
		}
	}

	void UIDocument::removeEmitter(UIElementPtr& element)
	{
		const std::string elementName = element->getName();
		//element->setPostCallback(nullptr); 
		this->fx.remove(elementName);
	}

	void UIDocument::removeAllEmitters()
	{
		this->fx.removeAll();
	}

	void UIDocument::setAnimationMode(bool anim) 
	{ 
		this->animMode = anim; 
	}

	void UIDocument::preload()
	{
		if (this->root.get())
		{
			this->root->preload();
		}
	}
}
