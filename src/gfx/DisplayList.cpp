#include "PCH.h"

#include "gfx/DisplayList.h"
#include "gfx/RenderInterface.h"

namespace cs
{

	BEGIN_META_CLASS(RenderTraversalBase) END_META()
	BEGIN_META_CLASS(RenderTraversalMainPass) END_META()
	BEGIN_META_CLASS(RenderTraversalShadowPass) END_META()


	RenderTraversalBasePtr RenderTraversalBase::getTypePtr(RenderTraversal traversal)
	{
		switch (traversal)
		{
			case RenderTraversalMain: return CREATE_CLASS(RenderTraversalMainPass);
			case RenderTraversalShadow: return CREATE_CLASS(RenderTraversalShadowPass);
			default: return CREATE_CLASS(RenderTraversalBase);
		}
	}

	std::string gBreakOnTag = "";

	UniformPtr DisplayListNode::objectToWorldMatrix;
	UniformPtr DisplayListNode::modelViewProjectionMatrix;
	UniformPtr DisplayListNode::globalColor;

	UniformPtr DisplayListTraversal::cameraPositionUniform;
	UniformPtr DisplayListTraversal::cameraDirectionUniform;
	UniformPtr DisplayListTraversal::viewportUniform;
	UniformPtr DisplayListTraversal::viewportInverseUniform;
	
	void DisplayListNode::draw(const DisplayListNode& node, void* data)
	{

#if defined(_DEBUG)
		if (node.tag.length() > 0 && node.tag == gBreakOnTag)
		{
			log::info("HERE!");
		}
#endif
        RenderInterface::getInstance()->pushDebugScope(node.tag);
		
		objectToWorldMatrix->setValue(node.objectToWorld);
		modelViewProjectionMatrix->setValue(node.mvp);

		DisplayListUtil::DrawParams* params = reinterpret_cast<DisplayListUtil::DrawParams*>(data);
		assert(data);

		vec4 adj_color = node.color;
		ColorF global_tint = params->tint;

		if (!(node.flags & (0x1 << int32(RenderableOptionIgnoreGlobalTint))))
		{
			adj_color.r *= params->tint.r;
			adj_color.g *= params->tint.g;
			adj_color.b *= params->tint.b;
			adj_color.a *= params->tint.a;
		}
		else
		{
			// whatever man
			memcpy(&global_tint, &node.color, sizeof(vec4));
		}

		globalColor->setValue(adj_color);
		
		for (auto& it : node.geomList)
		{
			it.geom->draw(it.overrides.get(), it.drawIndex, global_tint);
		}
        
		RenderInterface::getInstance()->popDebugScope();
	}

	void DisplayListTraversal::draw(DisplayListUtil::DrawParams* params, const PassParams& passParams)
	{
		const Transform& camera_transform = this->camera->getTransform();
		const RectI viewRect = this->camera->getViewport();

		vec3 camera_position;
		vec3 camera_direciton;

		switch (camera->getProjectionType())
		{
			case ProjectionOrthographic:
				camera_position = camera_transform.getPosition();
				camera_position.x -= float32(viewRect.size.w) * 0.5f;
				camera_position.y -= float32(viewRect.size.h) * 0.5f;
				camera_position.z = this->camera->getOrthographicDepth();
				camera_direciton = kDefalutZAxis;
				break;
		}
			
		cameraPositionUniform->setValue(camera_position);
		cameraDirectionUniform->setValue(camera_direciton);

		vec2 viewport = vec2(float32(viewRect.size.w), float32(viewRect.size.h));
		viewportUniform->setValue(viewport);

		vec2 viewport_inv = vec2(1.0f / viewport.x, 1.0f / viewport.y);
		viewportInverseUniform->setValue(viewport_inv);

		this->nodes.traverse(&DisplayListNode::draw, (void*) params);
	}

	void DisplayListPass::draw(DisplayList* display_list, DisplayListUtil::DrawParams* params)
	{
		// Bind the appropriate target
		RenderInterface* render_interface = RenderInterface::getInstance();
		RectI oldViewport = RenderInterface::getInstance()->getViewport();
		DisplayListTraversal::PassParams passParams;

		if (this->target)
		{
#if defined(CS_METAL)
            if (this->clearModes.size() > 0)
            {
                render_interface->setClearColor(this->clearColor);
                render_interface->clear(this->clearModes);
            }
#endif
 
			this->target->bind(true);
            render_interface->pushDebugScope(this->passName);
            
			passParams.targetSize = this->target->getDimensions();

            bool forceViewport = false;
#if defined(CS_METAL)
            forceViewport = true;
#endif
            render_interface->setZ(this->zNear, this->zFar);
			render_interface->setViewport(this->targetViewport, forceViewport);

#if !defined(CS_METAL)
			if (this->clearModes.size() > 0)
			{
				render_interface->setClearColor(this->clearColor);
				render_interface->clear(this->clearModes);
			}
#endif
		}
        else
        {
            render_interface->pushDebugScope(this->passName);
        }

		if (this->preCallback)
		{
			this->preCallback(params->scene);
		}

		if (!this->traversalMask.none())
		{
			for (size_t i = 0; i < RenderTraversalMAX; ++i)
			{
				if (this->traversalMask.test((RenderTraversal)i))
				{
					DisplayListTraversal& traversal = display_list->traversals[i];
					traversal.draw(params, passParams);
				}
			}
		}

		if (this->nodes.getSize() > 0)
		{
			DisplayListUtil::DrawParams dummyParams;
			this->nodes.traverse(&DisplayListNode::draw, (void*) &dummyParams);
		}

		if (this->postCallback)
		{
			this->postCallback(params->scene);
		}

		// unblind the target if necessary
		if (this->target)
		{
			this->target->unbind();
			render_interface->setViewport(oldViewport);
		}
        
        render_interface->getInstance()->popDebugScope();
	}

	DisplayList::DisplayList()
	{
		for (size_t i = 0; i < RenderTraversalMAX; ++i)
		{
			DisplayListTraversal& traversal = this->traversals[i];
			traversal.type = (RenderTraversal)i;
		}

		memset(this->traversalCount, 0, RenderTraversalMAX * sizeof(uint32));
	}

	void DisplayList::draw(DisplayListUtil::DrawParams* params)
	{
		for (auto& it : this->passes)
		{
            it->draw(this, params);
        }
	}

	DisplayListPassPtr DisplayList::addPass(const std::string& passName, RenderTraversal traversal)
	{
		const uint32 kTraversalOffset = 10;
		uint32 order = static_cast<int32>(traversal) * kTraversalOffset + this->traversalCount[traversal]++;
		return this->addPass(passName, order);
	}

	DisplayListPassPtr DisplayList::addPass(const std::string& passName, uint32 order)
	{
		DisplayListPassPtr newPass = CREATE_CLASS(DisplayListPass, passName, order);

		DisplayListPassList::iterator it = this->passes.begin();
		while (it != this->passes.end())
		{
			if (order < it->get()->traversalOrder)
			{
				this->passes.insert(it, newPass);
				return newPass;
			}
			++it;
		}
		this->passes.push_back(newPass);
		return newPass;
	}

	void DisplayList::initUniforms()
	{
		DisplayListNode::objectToWorldMatrix = 
			SharedUniform::getInstance().getUniform("object");

		DisplayListNode::modelViewProjectionMatrix = 
			SharedUniform::getInstance().getUniform("mvp");

		DisplayListNode::globalColor = 
			SharedUniform::getInstance().getUniform("color");

		DisplayListTraversal::cameraPositionUniform 
			= SharedUniform::getInstance().getUniform("camera_position");

		DisplayListTraversal::cameraDirectionUniform 
			= SharedUniform::getInstance().getUniform("camera_direction");

		DisplayListTraversal::viewportUniform 
			= SharedUniform::getInstance().getUniform("viewport");

		DisplayListTraversal::viewportInverseUniform 
			= SharedUniform::getInstance().getUniform("viewport_inv");

	}

	void DisplayList::addCallback(const std::string& name, DisplayListPass::displayCallbackFunc func, bool pre)
	{
		DisplayListPassList::iterator it = this->passes.begin();
		while (it != this->passes.end())
		{
			if (name == it->get()->passName)
			{
				if (pre)
				{
					it->get()->preCallback = func;
				}
				else
				{
					it->get()->postCallback = func;
				}
			}
			++it;
		}
	}
}
