#include "PCH.h"

#include "ui/imgui/GUIMeta.h"
#include "ui/imgui/GUIList.h"

#include "gfx/BatchRenderable.h"
#include "gfx/MeshRenderable.h"
#include "gfx/TrailRenderable.h"
#include "gfx/SplineRenderable.h"

#include "gfx/EnvironmentRenderable.h"

#include "liquid/LiquidGenerator.h"

#include "physics/PhysicsModifier.h"

#include "scripting/LuaScriptHandle.h"

#include "ecs/comp/ComponentList.h"

#include "global/PropertySet.h"

#include "fx/ParticleEffect.h"
#include "fx/ParticleEmitter.h"

#include "animation/spine/SpineAnimation.h"
#include "animation/spine/SpineRenderable.h"

#include "animation/AnimationType.h"

#include "scene/MutexReferenceNode.h"

namespace cs
{
	const float32 kMetaFieldIndent = 3.5f;

	void GUICreatorFactory::initCreators()
	{
		DEFINE_META_GUI_VALUE(GUINullVal);
		DEFINE_META_GUI_VALUE(float32);
		DEFINE_META_GUI_VALUE(int32);
		DEFINE_META_GUI_VALUE(bool);
		DEFINE_META_GUI_VALUE(std::string);
		DEFINE_META_GUI_VALUE(Transform);
		DEFINE_META_GUI_VALUE(RectF);
		DEFINE_META_GUI_VALUE(RectI);
		DEFINE_META_GUI_VALUE(SizeF);
		DEFINE_META_GUI_VALUE(SizeI);
		DEFINE_META_GUI_VALUE(vec2);
		DEFINE_META_GUI_VALUE(vec3);
		DEFINE_META_GUI_VALUE(vec4);
		DEFINE_META_GUI_VALUE(ColorB);
		DEFINE_META_GUI_VALUE(ColorF);
		
		DEFINE_META_GUI_VALUE(RenderTraversalMask);

		DEFINE_META_GUI_SERIALIZABLE(Shape);
		DEFINE_META_GUI_SERIALIZABLE(QuadShape);
		DEFINE_META_GUI_SERIALIZABLE(ParallelogramShape);
		DEFINE_META_GUI_SERIALIZABLE(GridShape);
		DEFINE_META_GUI_SERIALIZABLE(CircleShape);
		DEFINE_META_GUI_SERIALIZABLE(EllipseShape);

		DEFINE_META_GUI_SERIALIZABLE(DrawCallOverrides);
		DEFINE_META_GUI_SERIALIZABLE(DrawOptions);
		DEFINE_META_GUI_SERIALIZABLE(DrawOptionsBlend);
		DEFINE_META_GUI_SERIALIZABLE(BlendTypeBase);
		DEFINE_META_GUI_SERIALIZABLE(BlendTypeZero);
		DEFINE_META_GUI_SERIALIZABLE(BlendTypeOne);
		DEFINE_META_GUI_SERIALIZABLE(BlendTypeSrcAlpha);
		DEFINE_META_GUI_SERIALIZABLE(BlendTypeSrcAlphaInv);

		DEFINE_META_GUI_SERIALIZABLE(DrawOptionsDepth);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeBase);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeNever);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeEqual);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeLess);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeLessEqual);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeGreater);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeGreaterEqual);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeNotEqual);
		DEFINE_META_GUI_SERIALIZABLE(DepthTypeAlways);

		DEFINE_META_GUI_SERIALIZABLE(RenderTraversalBase);
		DEFINE_META_GUI_SERIALIZABLE(RenderTraversalMainPass);
		DEFINE_META_GUI_SERIALIZABLE(RenderTraversalShadowPass);

		DEFINE_META_GUI_SERIALIZABLE(TextureHandle);
		DEFINE_META_GUI_SERIALIZABLE(TextureHandleConst);
		
		DEFINE_META_GUI_SERIALIZABLE(TextureAnimation);
		DEFINE_META_GUI_SERIALIZABLE(TextureAnimationSheet);

		DEFINE_META_GUI_SERIALIZABLE(BatchRenderable);
		DEFINE_META_GUI_SERIALIZABLE(Renderable);
		
		DEFINE_META_GUI_SERIALIZABLE(EnvironmentRenderable);
		DEFINE_META_GUI_SERIALIZABLE(EnvironmentModificationTypeDrop);
		DEFINE_META_GUI_SERIALIZABLE(EnvironmentModificationTypeDropVertex);
		DEFINE_META_GUI_SERIALIZABLE(EnvironmentModificationTypeDropFace);
		DEFINE_META_GUI_SERIALIZABLE(EnvironmentModificationTypeDropTriangle);

		DEFINE_META_GUI_SERIALIZABLE(SceneReferenceHandle);

		DEFINE_META_GUI_SERIALIZABLE(AudioVolumeDistanceAdjustor);
		DEFINE_META_GUI_SERIALIZABLE(AudioVolumeAdjuster);

		DEFINE_META_GUI_SERIALIZABLE(LiquidGeneratorRandom);
		DEFINE_META_GUI_SERIALIZABLE(LiquidGeneratorPreset);
		DEFINE_META_GUI_SERIALIZABLE(LiquidGeneratorGrid);

		DEFINE_META_GUI_SERIALIZABLE(PhysicsShape);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsShapeBox);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsShapeMesh);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsShapeCircle)
		DEFINE_META_GUI_SERIALIZABLE(PhysicsShapeTriangle);
		
		DEFINE_META_GUI_SERIALIZABLE(PhysicsBody);

		DEFINE_META_GUI_SERIALIZABLE(PhysicsBodyType);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsBodyTypeDynamic);

		DEFINE_META_GUI_SERIALIZABLE(PhysicsLiquidCollision);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsLiquidCollisionKill);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsLiquidCollisionWarp);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsLiquidCollisionScript);

		DEFINE_META_GUI_SERIALIZABLE(PhysicsBodyCollision);
		DEFINE_META_GUI_SERIALIZABLE(PhysicsBodyCollisionScript);

		DEFINE_META_GUI_SERIALIZABLE(ParticleEffectHandle);
		DEFINE_META_GUI_SERIALIZABLE(ParticleEffectData);
		DEFINE_META_GUI_SERIALIZABLE(ParticleEmitter);

		DEFINE_META_GUI_SERIALIZABLE(EmissionHandler);
		DEFINE_META_GUI_SERIALIZABLE(EmissionHandlerBurst);
		DEFINE_META_GUI_SERIALIZABLE(EmissionHandlerTimed);
		DEFINE_META_GUI_SERIALIZABLE(EmissionHandlerInfinite);

		DEFINE_META_GUI_SERIALIZABLE(ParticleBaseValue);

		DEFINE_META_GUI_SERIALIZABLE(ParticleFloatValue);
		DEFINE_META_GUI_SERIALIZABLE(ParticleFloatValueConstant);
		DEFINE_META_GUI_SERIALIZABLE(ParticleFloatValueRandom);

		DEFINE_META_GUI_SERIALIZABLE(ParticleVec3Value);
		DEFINE_META_GUI_SERIALIZABLE(ParticleVec3ValueConstant);
		DEFINE_META_GUI_SERIALIZABLE(ParticleVec3ValueRandom);
		DEFINE_META_GUI_SERIALIZABLE(ParticleVec3ValueRandomDirection);

		DEFINE_META_GUI_SERIALIZABLE(ParticleVec2Value);
		DEFINE_META_GUI_SERIALIZABLE(ParticleVec2ValueConstant);
		DEFINE_META_GUI_SERIALIZABLE(ParticleVec2ValueRandom);

		DEFINE_META_GUI_SERIALIZABLE(ParticleColorValue);
		DEFINE_META_GUI_SERIALIZABLE(ParticleColorValueConstant);
		DEFINE_META_GUI_SERIALIZABLE(ParticleColorValueRandom);
		DEFINE_META_GUI_SERIALIZABLE(ParticleColorValueRandomFixed);
		DEFINE_META_GUI_SERIALIZABLE(ParticleColorValueRandomOffset);

		DEFINE_META_GUI_SERIALIZABLE_VECTOR(ParticleModuleList);

		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueSize);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueSizeLerp); 
		
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueColor);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueColorLerp);
	

		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueVelocity);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueAcceleration);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueRotation);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleValueRotationSpeed);

		DEFINE_META_GUI_SERIALIZABLE(ParticleModule);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleColor);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleSize);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleSimulation);
		DEFINE_META_GUI_SERIALIZABLE(ParticleModuleRotation);
		
		DEFINE_META_GUI_SERIALIZABLE(GameSelectableBehavior);
		DEFINE_META_GUI_SERIALIZABLE(GameSelectableMoveBehavior);
		DEFINE_META_GUI_SERIALIZABLE(GameSelectableRotateBehavior);
		
		DEFINE_META_GUI_SERIALIZABLE(MeshRenderable);
		DEFINE_META_GUI_SERIALIZABLE(MeshHandle);
		
		DEFINE_META_GUI_SERIALIZABLE(Volume);
		DEFINE_META_GUI_SERIALIZABLE(QuadVolume);
		DEFINE_META_GUI_SERIALIZABLE(PolygonListVolume);
		DEFINE_META_GUI_SERIALIZABLE(MeshHandleVolume);
		DEFINE_META_GUI_SERIALIZABLE(EllipseVolume);

		DEFINE_META_GUI_SERIALIZABLE(LuaScriptHandle);

		DEFINE_META_GUI_SERIALIZABLE(ShaderHandle);
		
		DEFINE_META_GUI_SERIALIZABLE_VECTOR(PropertySet::PropertyVector);
		DEFINE_META_GUI_SERIALIZABLE(PropertySetHandle);
		DEFINE_META_GUI_SERIALIZABLE(PropertySetHandleOverload);

		DEFINE_META_GUI_SERIALIZABLE_VECTOR(ReferenceNode::ReferenceOverloadProperty);
		DEFINE_META_GUI_SERIALIZABLE_VECTOR(ReferenceHandleChoiceList);

		DEFINE_META_GUI_SERIALIZABLE_MAP(TextureStages);
		DEFINE_META_GUI_SERIALIZABLE_VECTOR(ShaderUniformList);

		DEFINE_META_GUI_SERIALIZABLE(PropertyColor);
		DEFINE_META_GUI_SERIALIZABLE(PropertyInteger);
		DEFINE_META_GUI_SERIALIZABLE(PropertyDouble);
		DEFINE_META_GUI_SERIALIZABLE(PropertyString);
		DEFINE_META_GUI_SERIALIZABLE(PropertySet);
		DEFINE_META_GUI_SERIALIZABLE(PropertyVector3);

		DEFINE_META_GUI_SERIALIZABLE(SpineSkeletonHandle);
		DEFINE_META_GUI_SERIALIZABLE(SpineAtlasHandle);
		DEFINE_META_GUI_SERIALIZABLE_VECTOR(SpineRenderable::AtlasTextureHandleList);

		DEFINE_META_GUI_SERIALIZABLE_MAP_STRING(PropertySetOverrides);

		DEFINE_META_GUI_SERIALIZABLE(Animator);
		DEFINE_META_GUI_SERIALIZABLE(AnimatorLerpVec3);
		DEFINE_META_GUI_SERIALIZABLE(AnimatorLerpVec2);
		DEFINE_META_GUI_SERIALIZABLE(AnimatorLerpFloat);
		DEFINE_META_GUI_SERIALIZABLE(AnimatorLerpColor);

		DEFINE_META_GUI_SERIALIZABLE(AnimatedValue);
		DEFINE_META_GUI_SERIALIZABLE(AnimationPosition);
		DEFINE_META_GUI_SERIALIZABLE(AnimationRotation);
		DEFINE_META_GUI_SERIALIZABLE(AnimationColor);
		DEFINE_META_GUI_SERIALIZABLE(AnimationSize);
		DEFINE_META_GUI_SERIALIZABLE(AnimationTextureUV);

		DEFINE_META_GUI_SERIALIZABLE(AnimationScaleBase);
		DEFINE_META_GUI_SERIALIZABLE(AnimationScaleCenter);
		DEFINE_META_GUI_SERIALIZABLE(AnimationScaleLeft);

		DEFINE_META_GUI_SERIALIZABLE(BatchRenderable);
		DEFINE_META_GUI_SERIALIZABLE(SplineRenderable);
		
		DEFINE_META_GUI_SERIALIZABLE_VECTOR(AnimatedValueList);

		DEFINE_META_GUI_SERIALIZABLE(AnimationMethod);
		DEFINE_META_GUI_SERIALIZABLE(AnimationMethodValue);
		DEFINE_META_GUI_SERIALIZABLE(AnimationMethodPath);

		DEFINE_META_GUI_SERIALIZABLE(AnimationPlayBase);
		DEFINE_META_GUI_SERIALIZABLE(AnimationPlayLoop);
		DEFINE_META_GUI_SERIALIZABLE(AnimationPlayBounce);

		DEFINE_META_GUI_SERIALIZABLE(SoundEffectHandle);

		DEFINE_META_GUI_PTR(ShaderResource);
		DEFINE_META_GUI_PTR(Texture);
		DEFINE_META_GUI_PTR(SceneReference);
		
		DEFINE_META_GUI_PTR(Mesh);

		DEFINE_META_GUI_SERIALIZABLE(MeshShapeInstance);
		DEFINE_META_GUI_SERIALIZABLE_MAP_STRING(MeshShapeInstanceMap);

		//DEFINE_META_GUI_SERIALIZABLE_MAP_STRING(MeshShapeInstanceMap);

		DEFINE_META_GUI_PTR(LuaScriptFile);
		DEFINE_META_GUI_PTR(PropertySetResource);
		DEFINE_META_GUI_PTR(ParticleEffect);
		DEFINE_META_GUI_PTR(SpineSkeletonData);
		DEFINE_META_GUI_PTR(SpineTextureAtlas);
		DEFINE_META_GUI_PTR(SpineRenderable);
		DEFINE_META_GUI_PTR(SoundEffect);


	}

	void GUICreatorFactory::populate(const MetaData* meta, void* ptr, GUIFields* fields, float32 indent)
	{
		MetaData::MemberList members;
		meta->getAllMembers(members);

		for (auto it : members)
		{
			GUIValue* value = nullptr;
			const Member* member = it;
			if (member->getIgnoreGUI())
				continue;

			void *offsetData = PTR_ADD(ptr, member->getOffset());

			MetaData* metadata = nullptr;
			if (!member->getIsPointer())
				metadata = const_cast<MetaData*>(member->getMetaData());
			else
			{
				SerializablePtr* serial_ptr = reinterpret_cast<SerializablePtr*>(offsetData);
				if (serial_ptr->get())
					metadata = const_cast<MetaData*>((*serial_ptr)->getMetaData());
				else
					metadata = const_cast<MetaData*>(member->getMetaData());
			}
			
			std::type_index typeIndex = metadata->getTypeIndex();
			std::vector<std::type_index> types;
			metadata->getAllTypes(types);

			for (auto it : types)
			{
				std::type_index type = it;
				value = GUICreatorFactory::getInstance()->create(member, typeIndex, offsetData, ptr, indent + kMetaFieldIndent);
			}

			if (!value)
			{
				std::type_index type = metadata->getTypeIndex();
			}

			if (value)
				fields->add(value);
			else
				log::info("Ignoring member ", member->getName(), " in metadata ", metadata->getName(), " for object ", meta->getName());
		}

		const MetaData::MetaFunctions& functions = meta->getMetaFunctions();
		for (auto it : functions)
		{
			CallbackPtr callback = it->createCallback(ptr);
			GUIValueButton* button = new GUIValueButton(it->text, callback);
			fields->add(button);
		}
	}
}