#pragma once

#include "ClassDef.h"

#include "spine/spine.h"

#include "global/Resource.h"
#include "global/Event.h"
#include "gfx/Texture.h"
#include "animation/Animator.h"

#include <map>

namespace spine
{
	typedef spSkeletonData SkeletonData;
	typedef spSkeleton Skeleton;
	typedef spSkeletonJson SkeletonJson;
	typedef spAnimationState AnimationState;
	typedef spAnimationStateData AnimationStateData;
	typedef spAnimation Animation;
	typedef spAtlas Atlas;
	typedef spEvent Event;
	typedef spRegionAttachment RegionAttachment;
	typedef spSlot Slot;
	typedef spAtlasPage AtlasPage;
	typedef spMixPose MixPose;
	typedef spMixDirection MixDirection;
	typedef spMeshAttachment MeshAttachment;
	typedef spSkin Skin;
	typedef spColor Color;
};

namespace cs
{
	struct SkeletonDataWrapper
	{
		SkeletonDataWrapper(spine::SkeletonData* data) : value(data) { }
		~SkeletonDataWrapper()
		{
			spSkeletonData_dispose(this->value);
		}
	
		spine::SkeletonData* value;
	};

	struct SkeletonWrapper
	{
		SkeletonWrapper(spine::Skeleton* skl) : value(skl) { }
		~SkeletonWrapper()
		{
			spSkeleton_dispose(this->value);
		}

		spine::Skeleton* value;
	};


	struct SpineAnimationInstance
	{
		SpineAnimationInstance()
			: skeletonData(nullptr)
			, skeleton(nullptr)
			, animationState(nullptr)
			, animationStateData(nullptr)
			, currentAnimation(nullptr)
			, currentAnimName("")
			, currentTime(0.0f)
			, endTime(1.0f)
			, active(false)
		{ }

		SpineAnimationInstance(const SpineAnimationInstance& rhs)
			: skeletonData(rhs.skeletonData)
			, skeleton(rhs.skeleton)
			, animationState(rhs.animationState)
			, animationStateData(rhs.animationStateData)
		{ }

		void operator=(const SpineAnimationInstance& rhs)
		{
			this->skeletonData = rhs.skeletonData;
			this->skeleton = rhs.skeleton;
			this->animationState = rhs.animationState;
			this->animationStateData = rhs.animationStateData;
		}

		~SpineAnimationInstance()
		{
			delete this->animationState;
			delete this->animationStateData;
		}

		void playAnimation(const std::string& anim_name, float32 speed = 1.0f, AnimationType type = AnimationTypeLoop);
		void applyAnimation(float32 dt);
		size_t getNumSprites();

		bool getGeometryInfo(int32& num_vertices, int32& num_indices, int32& num_active_sprites);
		
		std::string name;
		std::shared_ptr<SkeletonDataWrapper> skeletonData;
		std::shared_ptr<SkeletonWrapper> skeleton;
		spine::AnimationState* animationState;
		spine::AnimationStateData* animationStateData;

		StringList animationNames;
		std::string currentAnimName;
		
		AnimationType animationType;
		float32 animationSpeed;

		spine::Animation* currentAnimation;

		float32 currentTime;
		float32 endTime;
		bool active;

		std::vector<std::string> skinNames;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(SpineTextureAtlas, Resource)
	public:
		SpineTextureAtlas(const std::string& fileName);

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		size_t getTextures(std::vector<TexturePtr>& textures);
		const spine::Atlas* getAtlas() const { return this->atlas; }

	private:

		SpineTextureAtlas()
			: atlas(nullptr)
		{ }

		void initAtlas();

		spine::Atlas* atlas;

	};

	typedef std::shared_ptr<SkeletonDataWrapper> SkeletonDataPtr;

	CLASS_DEFINITION_DERIVED_REFLECT(SpineSkeletonData, Resource)
	public:

		SpineSkeletonData(const std::string& fileName);
		
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		bool getAnimationInstance(SpineTextureAtlasPtr& atlas, SpineAnimationInstance& instance);

		static void populate(SpineAnimationInstance& instance, SkeletonDataPtr& data);

	private:

		SpineSkeletonData() {  }

		typedef std::map<const spine::Atlas*, SkeletonDataPtr> SkeletonMap;
		SkeletonMap skeletons;
		
	};

	template <>
    std::string SerializableHandle<SpineTextureAtlas>::getExtension();

	template <>
    std::string SerializableHandle<SpineTextureAtlas>::getDescription();

    template <>
    std::string SerializableHandle<SpineSkeletonData>::getExtension();

    template <>
    std::string SerializableHandle<SpineSkeletonData>::getDescription();

	CLASS_DEFINITION_REFLECT(SpineSkeletonHandle)
	public:
		SpineSkeletonHandle()
			: skel(nullptr) { }
		SpineSkeletonHandle(SpineSkeletonDataPtr& ptr)
			: skel(ptr) { }
		SpineSkeletonHandle(const std::string& fileName);

		Event onChanged;
		void onSpineSkeletonChanged();

		SpineSkeletonDataPtr& getSpineSkeletonData() { return this->skel; }

		void clear()
		{
			this->skel = nullptr;
			this->onChanged.invoke();
		}

	private:

		SpineSkeletonDataPtr skel;

	};

	CLASS_DEFINITION_REFLECT(SpineAtlasHandle)
	public:
		SpineAtlasHandle()
			: atlas(nullptr) { }
		SpineAtlasHandle(SpineTextureAtlasPtr& ptr)
			: atlas(ptr) { }
		SpineAtlasHandle(const std::string& fileName);

		Event onChanged;
		void onSpineAtlasChanged();

		SpineTextureAtlasPtr& getSpineAtlas() { return this->atlas; }

		void clear()
		{
			this->atlas = nullptr;
			this->onChanged.invoke();
		}

	private:

		SpineTextureAtlasPtr atlas;

	};

}
