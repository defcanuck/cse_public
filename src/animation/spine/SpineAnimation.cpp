#include "PCH.h"

#include "animation/spine/SpineImpl.h"
#include "animation/spine/SpineAnimation.h"
#include "global/ResourceFactory.h"

namespace cs
{
	BEGIN_META_RESOURCE(SpineSkeletonData)

	END_META()

	BEGIN_META_RESOURCE(SpineTextureAtlas)

	END_META()

	BEGIN_META_CLASS(SpineSkeletonHandle)

		ADD_MEMBER_RESOURCE(skel);
			SET_MEMBER_CALLBACK_POST(&SpineSkeletonHandle::onSpineSkeletonChanged);

	END_META();

	BEGIN_META_CLASS(SpineAtlasHandle)

		ADD_MEMBER_RESOURCE(atlas);
			SET_MEMBER_CALLBACK_POST(&SpineAtlasHandle::onSpineAtlasChanged);

	END_META();

	size_t SpineAnimationInstance::getNumSprites()
	{
		if (!this->skeleton.get())
			return 0;

		return this->skeleton->value->slotsCount;
	}


	bool SpineAnimationInstance::getGeometryInfo(int32& num_vertices, int32& num_indices, int32& num_active_sprites)
	{
		if (!this->skeleton.get())
			return false;

		num_vertices = 0;
		num_indices = 0;
		

		for (int32 i = 0; i < this->skeleton->value->slotsCount; i++)
		{
			spine::Slot* slot = this->skeleton->value->slots[i];
			if (!slot || !slot->attachment)
				continue;

			switch (slot->attachment->type)
			{
				case SP_ATTACHMENT_REGION:
				{
					spine::RegionAttachment* attachment = reinterpret_cast<spine::RegionAttachment*>(slot->attachment);
					num_vertices += 4;
					num_indices += 6;
					num_active_sprites++;
					break;
				}
				case SP_ATTACHMENT_MESH:
				{
					spine::MeshAttachment* attachment = reinterpret_cast<spine::MeshAttachment*>(slot->attachment);
					num_indices += attachment->trianglesCount;
					num_vertices += attachment->super.worldVerticesLength / 2;
					num_active_sprites++;
					break;
				}
				default:
					assert(false);
					log::error("Can't handle this type of sprite");
					break;
			}

		}

		return true;
	}

	void SpineAnimationInstance::playAnimation(const std::string& anim_name, float32 speed, AnimationType type)
	{
		assert(this->skeletonData.get());
		const spine::SkeletonData* sklData = this->skeletonData.get()->value;
		this->currentAnimation = const_cast<spine::Animation*>(spSkeletonData_findAnimation(sklData, anim_name.c_str()));
		if (this->currentAnimation)
		{
			this->currentTime = 0.0f;
			this->endTime = this->currentAnimation->duration;

			this->animationSpeed = speed;
			this->animationType = type;
			this->active = true;

		}
	}

	void SpineAnimationInstance::applyAnimation(float32 dt)
	{
		if (!this->currentAnimation || !this->active)
			return;

		assert(this->endTime > 0.0f);

		float32 lastTime = this->currentTime;
		this->currentTime += dt * this->animationSpeed;
		if (this->currentTime >= this->endTime)
		{
			switch (this->animationType)
			{
			case AnimationTypeNone:
				this->currentTime = std::min<float32>(this->currentTime, this->endTime);
				this->active = false;
				break;
			case AnimationTypeLoop:
				this->currentTime = std::min<float32>(this->currentTime - this->endTime, this->endTime);
				break;
			case AnimationTypeBounce:
				break;
			}
		}

		spine::Event** outEvents = nullptr;
		int32 eventsCount = 0;
		float alpha = 1.0f;
		spine::MixPose pose = SP_MIX_POSE_CURRENT;
		spine::MixDirection direction = SP_MIX_DIRECTION_IN;

		spAnimation_apply(this->currentAnimation, this->skeleton.get()->value, lastTime, this->currentTime, this->animationType == AnimationTypeLoop, outEvents, &eventsCount, alpha, pose, direction);
		spSkeleton_updateWorldTransform(this->skeleton.get()->value);
	}

	SpineSkeletonData::SpineSkeletonData(const std::string& fileName)
		: Resource(fileName)
	{

	}

	void SpineSkeletonData::onPostLoad(const LoadFlagMask& flags)
	{
		// todo
	}

	void SpineSkeletonData::populate(SpineAnimationInstance& instance, std::shared_ptr<SkeletonDataWrapper>& data)
	{
		instance.skeletonData = data;
		instance.skeleton = std::shared_ptr<SkeletonWrapper>(new SkeletonWrapper(spSkeleton_create(data.get()->value)));
		instance.animationStateData = spAnimationStateData_create(data.get()->value);
		instance.animationState = spAnimationState_create(instance.animationStateData);
	}

	bool SpineSkeletonData::getAnimationInstance(SpineTextureAtlasPtr& atlas, SpineAnimationInstance& instance)
	{
		if (!atlas.get() || !atlas->getAtlas())
		{
			log::error("No atlas defined!");
			return false;
		}

		SkeletonMap::iterator it = this->skeletons.find(atlas->getAtlas());
		if (it != this->skeletons.end())
		{
			SkeletonDataPtr& data = it->second;
			
			instance.name = this->getName();
			this->populate(instance, data);
			return true;
		}

		std::string filePath;
		if (!FileManager::getInstance()->getPathToFile(this->getName(), filePath))
		{
			log::error("Cannot find Spine Animation Data!");
			return false;
		}

		const spine::Atlas* spine_atlas = atlas->getAtlas();
		spine::SkeletonJson* json_data = spSkeletonJson_create(const_cast<spine::Atlas*>(spine_atlas));
		spine::SkeletonData* skel_data = spSkeletonJson_readSkeletonDataFile(json_data, filePath.c_str());
		if (!skel_data)
		{
			log::error("Error loading skeleton data!");
			log::error(json_data->error);
			return false;
		}

		SkeletonDataPtr data_ptr = std::shared_ptr<SkeletonDataWrapper>(new SkeletonDataWrapper(skel_data));

		instance.name = this->getName();
		this->populate(instance, data_ptr);
		this->skeletons[atlas->getAtlas()] = data_ptr;
		return true;
	}

	SpineTextureAtlas::SpineTextureAtlas(const std::string& fileName)
		: Resource(fileName)
		, atlas(nullptr)
	{
		this->initAtlas();
	}

	void SpineTextureAtlas::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->getName().length() > 0)
			this->initAtlas();
	}

	void SpineTextureAtlas::initAtlas()
	{
		AutoRelease<FileStream> stream;
		if (!FileManager::getInstance()->openStreamFile(this->getName(), (DataStream**)&stream.value))
		{
			log::error("Cannot find ", this->getName());
			return;
		}

		std::string buffer;
		std::istream& ifs = stream->getStream();

		ifs.seekg(0, std::ios::end);
		buffer.reserve((uint32)ifs.tellg());
		ifs.seekg(0, std::ios::beg);

		buffer.assign((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());

		std::vector<char> writable(buffer.begin(), buffer.end());
		writable.push_back('\0');

		void* data_ptr = nullptr;
		this->atlas = spAtlas_create((char*)&buffer[0], static_cast<int32>(buffer.length()), stream.value->filePath.c_str(), data_ptr);
		if (!this->atlas)
		{
			log::error("Failed to load spine atlas ", this->getName());
		}
	}

	size_t SpineTextureAtlas::getTextures(std::vector<TexturePtr>& textures)
	{
		if (!this->atlas)
		{
			return 0;
		}

		spine::AtlasPage* page = this->atlas->pages;
		while (page != nullptr)
		{
			
			SpineTexture* spine_tex = (SpineTexture*) page->rendererObject;
			textures.push_back(spine_tex->texture);
			page = page->next;

		}
		return textures.size();
	}

	SpineSkeletonHandle::SpineSkeletonHandle(const std::string& fileName)
	{
		this->skel = std::static_pointer_cast<SpineSkeletonData>(
			ResourceFactory::getInstance()->loadResource<SpineSkeletonData>(fileName));
	}

	void SpineSkeletonHandle::onSpineSkeletonChanged()
	{
		this->onChanged.invoke();
	}

	SpineAtlasHandle::SpineAtlasHandle(const std::string& fileName)
	{
		this->atlas = std::static_pointer_cast<SpineTextureAtlas>(
			ResourceFactory::getInstance()->loadResource<SpineTextureAtlas>(fileName));
	}

	void SpineAtlasHandle::onSpineAtlasChanged()
	{
		this->onChanged.invoke();
	}
    
    template <>
    std::string SerializableHandle<SpineTextureAtlas>::getExtension()
    {
        return "atlas";
    }
    
    template <>
    std::string SerializableHandle<SpineTextureAtlas>::getDescription()
    {
        return "CSE SpineTextureAtlas";
    }
    
    template <>
    std::string SerializableHandle<SpineSkeletonData>::getExtension()
    {
        return "json";
    }
    
    template <>
    std::string SerializableHandle<SpineSkeletonData>::getDescription()
    {
        return "CSE SpineSkeletonData";
    }
}
