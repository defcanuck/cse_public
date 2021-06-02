#include "PCH.h"

#include "liquid/LiquidContext.h"
#include "scene/Scene.h"
#include "global/Utils.h"

#include "ecs/ECS.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/comp/DrawableComponent.h"
#include "ecs/comp/ScriptComponent.h"

#include "physics/PhysicsGlobals.h"

namespace cs
{
	
	BEGIN_META_CLASS(LiquidContext)

		ADD_MEMBER(radius);
			SET_MEMBER_CALLBACK_POST(&LiquidContext::setRadiusImpl);
		ADD_MEMBER(maxParticles);
			SET_MEMBER_MIN(0);
			SET_MEMBER_MAX(1000);
			SET_MEMBER_CALLBACK_POST(&LiquidContext::onMaxParticlesChanged);
		ADD_MEMBER(gravityScale);
			SET_MEMBER_DEFAULT(1.0f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2.0f);
			SET_MEMBER_CALLBACK_POST(&LiquidContext::setGravityScaleImpl);
		
		ADD_MEMBER_PTR(textureHandle);
		
		ADD_MEMBER(options);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();
	}

	LiquidContext::LiquidContext(const std::string& n, ECSContext* entityContext) 
		: Entity(n, entityContext)
		, ps(nullptr)
		, radius(1.0f)
		, gravityScale(1.0f)
		, maxParticles(kMaxParticles)
		, shaderHandle(CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultColorShader))
	{
		this->init();
	}


	LiquidContext::LiquidContext(ECSContext* entityContext)
		: Entity("context", entityContext)
		, ps(nullptr)
		, radius(1.0f)
		, gravityScale(1.0f)
		, maxParticles(kMaxParticles)
		, shaderHandle(RenderInterface::kDefaultTextureShader)
	{
		this->init();
	}

	LiquidContext::~LiquidContext()
	{
		this->removeAllParticles();
		if (this->ps)
		{
			PhysicsSystem::getInstance()->getWorld().DestroyParticleSystem(this->ps);
			this->ps = nullptr;
		}
	}

	void LiquidContext::init()
	{
		this->shaderHandle = RenderInterface::kDefaultTextureShader;
		this->color = ColorB::White;

		if (!this->ps)
		{
			const b2ParticleSystemDef particleSystemDef;
			b2World& world = PhysicsSystem::getInstance()->getWorld();
			this->ps = world.CreateParticleSystem(&particleSystemDef);

			this->setRadiusImpl();
			this->setGravityScaleImpl();
		}
	}
	
	void LiquidContext::setParticlePosition(const b2ParticleHandle* handle, const b2Vec2& pos)
	{
		b2Vec2* fx_positions = this->ps->GetPositionBuffer();
		int32 index = handle->GetIndex();
		assert(index < this->ps->GetParticleCount());
		fx_positions[index] = PhysicsConst::worldToBox2D(pos);
	}

	void LiquidContext::setParticleVelocity(const b2ParticleHandle* handle, const b2Vec2& vel)
	{
		b2Vec2* fx_velocity = this->ps->GetVelocityBuffer();
		int index = handle->GetIndex();
		assert(index < this->ps->GetParticleCount());
		fx_velocity[index] = PhysicsConst::worldToBox2D(vel);
	}

	void LiquidContext::updateParticlePositions(ParticlePositionMap& positions)
	{
		b2Vec2* fx_positions = this->ps->GetPositionBuffer();
		int32 fx_count = this->ps->GetParticleCount();
		for (auto it : positions)
		{
			const b2ParticleHandle* handle = it.first;
			int32 index = handle->GetIndex();
			if (index < 0 || index >= this->maxParticles)
				continue;

			b2Vec2& fx_pos = fx_positions[index];
			fx_pos = PhysicsConst::worldToBox2D(it.second.x, it.second.y);
		}
	}

	void LiquidContext::updateParticleDelta(const vec2& offset)
	{

	}

	void LiquidContext::reset(bool active)
	{
		this->removeAllParticles();
		BASECLASS::reset(active);
	}

	void LiquidContext::onMaxParticlesChanged()
	{
		this->reset();
	}

	void LiquidContext::removeParticle(const b2ParticleHandle* handle)
	{
		int32 index = handle->GetIndex();
		this->ps->DestroyParticle(index);
	}

	bool LiquidContext::getParticlePosition(const b2ParticleHandle* handle, b2Vec2& position)
	{
		int32 index = handle->GetIndex();
		if (index < 0 || index >= this->ps->GetParticleCount())
			return false;

		b2Vec2* fx_positions = this->ps->GetPositionBuffer();
		position = PhysicsConst::box2DToWorld(fx_positions[index]);
		return true;
	}

	void LiquidContext::removeAllParticles()
	{
		for (int32 i = 0; i < this->ps->GetParticleCount(); i++)
		{
			this->ps->DestroyParticle(i);
		}
	}

	void LiquidContext::setMaxParticles(uint32 maxp)
	{
		this->maxParticles = maxp;
	}

	void LiquidContext::onPostLoad(const LoadFlagMask& flags)
	{
		
		this->init();

		BASECLASS::onPostLoad(flags);

		ADD_COMPONENT(this, DrawableComponent);

		cs::GeometryDataPtr data = CREATE_CLASS(GeometryData);
        data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });
		data->vertexSize = this->maxParticles * 4;
		data->indexSize = this->maxParticles * 6;
		data->storage = BufferStorageDynamic;

		this->particleGeometry = CREATE_CLASS(DynamicGeometry, data);

		DynamicGeometry::VertexUpdateFunc vfunc;
		vfunc = std::bind(&LiquidContext::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->particleGeometry->setVertexUpdateFunc(vfunc);

		DynamicGeometry::IndexUpdateFunc ifunc;
		ifunc = std::bind(&LiquidContext::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->particleGeometry->setIndexUpdateFunc(ifunc);

		GeometryPtr geoptr = std::static_pointer_cast<Geometry>(this->particleGeometry);
		
		const DrawableComponentPtr& draw = this->getComponent<DrawableComponent>();
		RenderablePtr rend = CREATE_CLASS(Renderable);
		rend->addGeometry(geoptr);
		draw->setRenderable(rend);

		DynamicGeometry::AdjustDrawCallFunc dcfunc;
		dcfunc = std::bind(&LiquidContext::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->particleGeometry->setDrawCallAdjustFunc(dcfunc);

	}

	size_t LiquidContext::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		
		uint32 vertexCtr = 0;
		assert(this->ps);

		b2Vec2* fx_positions = this->ps->GetPositionBuffer();		
		
		vec2 uvs[] =
		{
			this->textureHandle->getUVByIndex(0),
			this->textureHandle->getUVByIndex(1),
			this->textureHandle->getUVByIndex(2),
			this->textureHandle->getUVByIndex(3)
		};

		float32 off = this->radius * 1.2f;
		vec3 kOffsets[] =
		{
			vec3( -off, -off, 0.0f),
			vec3( -off,  off, 0.0f),
			vec3(  off,  off, 0.0f),
			vec3(  off, -off, 0.0f)
		};

		char* pos_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);
		char* uv_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);
		size_t stride = decl.getStride();

		for (int32 i = 0; i < this->ps->GetParticleCount(); i++)
		{
			b2Vec2 box_pos = PhysicsConst::box2DToWorld(fx_positions[i]);
			
			for (size_t t = 0; t < 4; t++)
			{
				vec3* pos = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, vertexCtr * stride));
				assert(size_t(pos) - size_t(data) < bufferSize);
				*pos = vec3(box_pos.x, box_pos.y, 0.0f) + kOffsets[t];

				vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, vertexCtr * stride));
				assert(size_t(uv) - size_t(data) < bufferSize);
				*uv = uvs[t];

				vertexCtr++;
			}
		}

		return vertexCtr;
	}

	void LiquidContext::setRadius(float32 rad)
	{
		if (this->radius != rad)
		{
			this->radius = rad;
			this->setRadiusImpl();
		}
	}

	void LiquidContext::setRadiusImpl()
	{
		if (!this->ps)
			return;

		this->ps->SetRadius(PhysicsConst::worldToBox2D(this->radius));
	}


	void LiquidContext::setGravityScaleImpl()
	{
		if (!this->ps)
			return;

		this->ps->SetGravityScale(this->gravityScale);
	}

	size_t LiquidContext::updateIndices(uchar* data, size_t bufferSize)
	{
		uint32 indexCtr = 0;
		uint16* indices = reinterpret_cast<uint16*>(data);
		int32 particleCount = this->ps->GetParticleCount();
		for (int32 i = 0; i < particleCount; i++)
		{
			uint16 offset = i * 4;
			indices[indexCtr++] = offset + kStaticQuadIndices[0];
			indices[indexCtr++] = offset + kStaticQuadIndices[1];
			indices[indexCtr++] = offset + kStaticQuadIndices[2];

			indices[indexCtr++] = offset + kStaticQuadIndices[3];
			indices[indexCtr++] = offset + kStaticQuadIndices[4];
			indices[indexCtr++] = offset + kStaticQuadIndices[5];
		}
		return indexCtr;
	}

	const b2ParticleHandle* LiquidContext::addParticle(vec2 pos, LiquidGroup* group)
	{
		if (!this->ps)
		{
			log::error("Attempting to add a particle without a context initialized");
			return nullptr;
		}

		if (this->ps->GetParticleCount() >= this->maxParticles)
		{
			log::print(LogError, "Exceeded Particle Count!");
			return nullptr;
		}

		b2ParticleDef pd;
		pd.flags = b2_elasticParticle | b2_fixtureContactListenerParticle;
		pd.color.Set(255, 255, 255, 255);
		pd.position.Set(PhysicsConst::worldToBox2D(pos.x), PhysicsConst::worldToBox2D(pos.y));
		pd.userData = group;
		
		int32 index = this->ps->CreateParticle(pd);
		if (index > 0)
		{
			const b2ParticleHandle* handle = this->ps->GetParticleHandleFromIndex(index);
			return handle;
		}
		return nullptr;
	}

	void LiquidContext::setShader(ShaderHandlePtr& shader)
	{
		this->shaderHandle = shader;
	}

	void LiquidContext::setTexture(TextureHandlePtr& texture, uint32 stage)
	{
		this->textureHandle = texture;
	}

	void LiquidContext::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->tag = this->getName();
		dc->type = DrawTriangles;
		dc->indexType = TypeUnsignedShort;
		dc->count = this->ps->GetParticleCount() * 6;
		dc->offset = 0;
		dc->shaderHandle = this->shaderHandle;
		dc->textures[0] = this->textureHandle;
		dc->color = this->getColor();
		dc->srcBlend = this->options.blend.getSourceBlend();
		dc->dstBlend = this->options.blend.getDestBlend();
		dc->depthTest = true;
		dc->depthWrite = false;
		dcs.push_back(dc);
	}

	bool LiquidContext::canAddComponent(const std::type_index& t)
	{ 
		ComponentEnabledMap componentEnableMap;
		componentEnableMap[typeid(DrawableComponent)] = true;
		componentEnableMap[typeid(ScriptComponent)] = true;

		return componentEnableMap.count(t) != 0; 
	}
}
