#include "PCH.h"

#include "gfx/Texture.h"
#include "global/ResourceFactory.h"

namespace cs
{

	BEGIN_META_RESOURCE(Texture)
	
	END_META()

	Texture::Texture(const std::string& name, const TextureResourcePtr& resource) :
		Resource(name),
		textureResource(resource)
	{

	}

	void Texture::bind(uint32 stage, bool wrapU, bool wrapV)
	{
		if (this->textureResource)
			this->textureResource->bind(stage, wrapU, wrapV);
	}

	bool Texture::equals(const TexturePtr& rhs) const
	{
		return this->textureResource.get() == rhs->getTextureResource().get();
	}

	const TextureResourcePtr& Texture::getTextureResource() const 
	{
		return this->textureResource;
	}

	void Texture::adjustRect(RectF& rect)
	{
		if (this->textureResource.get())
		{
			this->textureResource->adjustRect(rect);
		}
	}

	bool Texture::preload(const std::string& fileName)
	{
		TexturePtr texture = std::static_pointer_cast<Texture>(
			ResourceFactory::getInstance()->loadResource<Texture>(fileName));
		return texture.get() != nullptr;
	}


	uint32 Texture::getWidth() const 
	{ 
		return (this->textureResource.get()) ? this->textureResource->getRealWidth() : 0; 
	}
	
	uint32 Texture::getHeight() const 
	{ 
		return (this->textureResource.get()) ? this->textureResource->getRealHeight() : 0; 
	}
    
    TextureChannels Texture::getChannels() const
    {
        return (this->textureResource.get()) ? this->textureResource->getChannels() : TextureNone; 
    }
    
    template <>
    std::string SerializableHandle<Texture>::getExtension()
    {
        return "png";
    }
    
    template <>
    std::string SerializableHandle<Texture>::getDescription()
    {
        return "CSE Texture";
    }
}
