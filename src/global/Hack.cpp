#include "PCH.h"

#include "global/Hack.h"

#include "physics/PhysicsModifier.h"
#include "physics/PhysicsShape.h"

#include "gfx/MeshRenderable.h"
#include "gfx/EnvironmentRenderable.h"
#include "gfx/SplineRenderable.h"

#include "geom/Volume.h"

#include "fx/ParticleEffect.h"
#include "fx/ParticleModule.h"

#include "animation/spine/SpineAnimation.h"
#include "animation/spine/SpineRenderable.h"

#include "ecs/comp/AnimationComponent.h"
#include "ecs/comp/GameComponent.h"
#include "ecs/EntitySearch.h"
#include "scene/ReferenceNode.h"

#include "audio/SoundManager.h"

namespace cs
{
	void includeHackTypes()
	{

		EntitySearchParams params;
		
		FORCE_META_REGISTER(PhysicsShape);
		FORCE_META_REGISTER(PhysicsShapeBox);
		FORCE_META_REGISTER(PhysicsShapeTriangle);
		FORCE_META_REGISTER(PhysicsShapeCircle);
		FORCE_META_REGISTER(PhysicsShapeMesh);
		FORCE_META_REGISTER(MeshRenderable);
		FORCE_META_REGISTER(EnvironmentRenderable);

		FORCE_META_REGISTER(PolygonListVolume);
		FORCE_META_REGISTER(MeshHandleVolume);

		FORCE_META_REGISTER(ParticleEffect);
		FORCE_META_REGISTER(ParticleEffectData);
		FORCE_META_REGISTER(EmissionHandler);

		FORCE_META_REGISTER(EmissionHandlerBurst);
		FORCE_META_REGISTER(EmissionHandlerTimed);
		FORCE_META_REGISTER(EmissionHandlerInfinite);
		FORCE_META_REGISTER(ParticleModule);
		FORCE_META_REGISTER(ParticleModuleSize);
		FORCE_META_REGISTER(ParticleModuleRotation);

		FORCE_META_REGISTER(SpineSkeletonData);
		FORCE_META_REGISTER(SpineTextureAtlas);
		FORCE_META_REGISTER(SpineSkeletonHandle);
		FORCE_META_REGISTER(SpineAtlasHandle);
		FORCE_META_REGISTER(SpineRenderable);
		FORCE_META_REGISTER(SplineRenderable);

		FORCE_META_REGISTER(Animator);
		FORCE_META_REGISTER(AnimatorLerpVec2);
		FORCE_META_REGISTER(AnimatorLerpVec3);
		FORCE_META_REGISTER(AnimatedValue);
		FORCE_META_REGISTER(AnimationPosition);
		FORCE_META_REGISTER(AnimationSize);
		FORCE_META_REGISTER(AnimationTextureUV);

		FORCE_META_REGISTER(AnimationMethod);
		FORCE_META_REGISTER(AnimationMethodValue);
		FORCE_META_REGISTER(AnimationMethodPath);

		FORCE_META_REGISTER(GameSelectableBehavior);
		FORCE_META_REGISTER(GameSelectableMoveBehavior);
		FORCE_META_REGISTER(GameSelectableRotateBehavior);

		FORCE_META_REGISTER(SoundEffect);
		FORCE_META_REGISTER(SoundEffectHandle);

		log::info("Hack include ", cs::SerializableHandle<cs::PropertySet>::getDescription());
		log::info("Hack include ", cs::SerializableHandle<cs::PropertySet>::getExtension());

		log::info("Hack include ", cs::SerializableHandle<cs::Scene>::getDescription());
		log::info("Hack include ", cs::SerializableHandle<cs::Scene>::getExtension());

		log::info("Hack include ", cs::SerializableHandle<cs::ReferenceNode>::getDescription());
		log::info("Hack include ", cs::SerializableHandle<cs::ReferenceNode>::getExtension());

	}
}