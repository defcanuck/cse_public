#pragma once

#include "ClassDef.h"
#include "ecs/Entity.h"
#include "math/GLM.h"

#include "liquid/LiquidGenerator.h"
#include "liquid/LiquidContext.h"

#include "physics/PhysicsContact.h"
#include "physics/PhysicsLiquidCollision.h"

#include "ecs/comp/LiquidComponent.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(LiquidGroup, Entity)
	
	public:

		LiquidGroup(const std::string& n, ECSContext* entityContext);
		virtual ~LiquidGroup();

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void reset(bool active = false);
		virtual void clear();
		virtual bool intersects(const Transform& transform, const Ray& ray, EntityIntersection& hit_data);

		
		void setLiquidContext(LiquidContextPtr& cxt);
		void setGenerator(LiquidGeneratorPtr& gen);

		static void linkToContext(SceneNodePtr& parent, LiquidGroup* group);

		virtual size_t getSelectableVolume(SelectableVolumeList& volumes);

		virtual void onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);
		bool getParticlePosition(const b2ParticleHandle* handle, b2Vec2& position);

		void convertToPreset();

		virtual bool canAddComponent(const std::type_index& t);
		
	protected:

		friend PhysicsContact;
		friend LiquidComponent;
		friend PhysicsLiquidCollision;
		friend PhysicsLiquidCollisionWarp;

		struct WarpToVolume
		{
			WarpToVolume()
				: curTime(0.0f)
				, maxTime(0.0f)
				, onEndSpawn(nullptr)
				, onEndTransform(nullptr)
				, onEndVelocity(kZero2)
				, tempPosition(kZero2)
			{

			}

			b2Vec2 getSpawn() const
			{
				vec2 newSpawn = this->onEndSpawn->getRandomValue();
				newSpawn = this->onEndTransform->translate(newSpawn);
				return b2Vec2(newSpawn.x, newSpawn.y);
			}

			b2Vec2 getVelocity() const
			{
				vec2 newVelocity = this->onEndTransform->rotate(this->onEndVelocity);
				return b2Vec2(newVelocity.x, newVelocity.y);
			}

			float32 curTime;
			float32 maxTime;
			VolumePtr onEndSpawn;
			TransformPtr onEndTransform;
			vec2 onEndVelocity;
			vec2 tempPosition;
		};

		void removeAllParticles();
		void removeParticle(const b2ParticleHandle* handle);
		void updateParticles(float32 dt);

		void warpParticle(const b2ParticleHandle* handle, float32 time, const VolumePtr& onEnd, const TransformPtr& transform, const vec2& onEndVelocity = vec2(0.0f, 0.0f));

	private:
		LiquidGroup()
			: generator(nullptr)
			, context(nullptr)
		{ 
			this->setSelectable(true);
		}

		void generateParticles();
		void addParticle(const vec2 pos, const vec2 offset);

		LiquidGeneratorPtr generator;

		LiquidContextPtr context;
		
		int32 numParticles;
		int32 maxParticles;

		ParticlePositionMap particles;
		ParticleList toDelete;

		typedef std::map<const b2ParticleHandle*, WarpToVolume> ParticleWarpList;
		ParticleWarpList toWarp;
	};


}
