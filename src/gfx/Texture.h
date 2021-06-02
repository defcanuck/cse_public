#pragma once

#include "ClassDef.h"
#include "global/Resource.h"
#include "gfx/TextureResource.h"
#include "gfx/DepthBuffer.h"
#include "global/SerializableHandle.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(Texture, Resource)
	public:

		Texture() : Resource("Error"), textureResource(nullptr) { }
		Texture(const std::string& name, const TextureResourcePtr& resource);
    
		virtual void bind(uint32 stage, bool wrapU = false, bool wrapV = false);
		bool equals(const TexturePtr& rhs) const;

		const TextureResourcePtr& getTextureResource() const;
		void adjustRect(RectF& rect);

		uint32 getWidth() const;
		uint32 getHeight() const;
        TextureChannels getChannels() const;

		static bool preload(const std::string& fileName);

	protected:

		Texture(const std::string& name) : Resource(name), textureResource(nullptr) { }

	private:

		TextureResourcePtr textureResource;
		
	};

	template <>
    std::string SerializableHandle<Texture>::getExtension();

	template <>
    std::string SerializableHandle<Texture>::getDescription();
}
