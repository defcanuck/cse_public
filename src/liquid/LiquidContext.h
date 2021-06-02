#pragma once

#include <Box2D/Particle/b2Particle.h>
#include <Box2D/Particle/b2ParticleSystem.h>

#include "ClassDef.h"

#include "gfx/Types.h"
#include "gfx/VertexDeclaration.h"
#include "gfx/DynamicGeometry.h"
#include "gfx/DrawOptions.h"

#include "liquid/LiquidGenerator.h"

#include "math/Ray.h"
#include "math/Transform.h"

#include "ecs/Entity.h"

#include "math/GLM.h"

namespace cs
{
	class SceneData;
	class LiquidGroup;

	typedef std::map<const b2ParticleHandle*, vec2> ParticlePositionMap;
	typedef std::vector<const b2ParticleHandle*> ParticleList;

	CLASS_DEFINITION_DERIVED_REFLECT(LiquidContext, Entity)
	public:

		const static int32 kMaxParticles = 5000;

		LiquidContext(const std::string& n, ECSContext* entityContext);
		virtual ~LiquidContext();

		void setMaxParticles(uint32 maxp);

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dc);
		
		bool canAddParticles() const { return this->ps->GetParticleCount() < this->maxParticles; }

		void updateParticleDelta(const vec2& offset);
		void updateParticlePositions(ParticlePositionMap& positions);

		void setRadius(float32 rad);

		DynamicGeometryPtr& getParticleGeometry() { return this->particleGeometry; }
		bool getParticlePosition(const b2ParticleHandle* handle, b2Vec2& position);

		void setShader(ShaderHandlePtr& shader);
		void setTexture(TextureHandlePtr& texture, uint32 stage = 0);

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void reset(bool active = false);
		
		virtual bool canAddComponent(const std::type_index& t);
	
	protected:
		
		friend class LiquidGroup;

		const b2ParticleHandle* addParticle(vec2 pos, LiquidGroup* group = nullptr);
		void removeParticle(const b2ParticleHandle* handle);
		
		void setParticleVelocity(const b2ParticleHandle* handle, const b2Vec2& vel);
		void setParticlePosition(const b2ParticleHandle* handle, const b2Vec2& pos);

	private:

		LiquidContext() 
			: gravityScale(1.0f)
			, radius(1.0f) 
		{ }

		LiquidContext(ECSContext* entityContext);

		void init();

		void removeAllParticles();
		void setRadiusImpl();
		void setGravityScaleImpl();
		void onMaxParticlesChanged();

		b2ParticleSystem* ps;
		float32 radius;
		float32 gravityScale;
		
		int32 maxParticles;

		DrawOptions options;

		ShaderHandlePtr shaderHandle;
		TextureHandlePtr textureHandle;
		DynamicGeometryPtr particleGeometry;

	};
}
