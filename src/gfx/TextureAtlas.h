#pragma once

#include "ClassDef.h"
#include "gfx/TextureHandle.h"
#include "gfx/RenderTexture.h"
#include "global/Singleton.h"
#include "global/Event.h"

#include <map>

namespace cs
{

	struct TextureAtlasData
	{

		typedef std::function<void(TextureHandlePtr&)> OnTextureAtlased;

		TextureAtlasData();
		TextureAtlasData(const TextureAtlasData& rhs);
		
		virtual ~TextureAtlasData();
		void operator=(const TextureAtlasData& rhs);

		uint16 width;
		uint16 height;
		uchar* bytes;
		uint16 bytelen;
		TextureHandlePtr texture;
        TextureChannels channels;

		OnTextureAtlased callback;
		Event onAtlased;
	};

	CLASS_DEFINITION(TextureAtlas)
	public:
		TextureAtlas(const std::string& atlasName, uint32 dimm, TextureChannels c);
		virtual ~TextureAtlas();

		const TextureHandlePtr& getAtlasTexture() const { return this->atlasTexture; }
		bool getRect(const std::string& str, RectF& rect);
		TextureHandlePtr getTextureHandle(const std::string& name);
        TextureHandlePtr getSourceTextureHandle(const std::string& name);
    
		void addTexture(const std::string& name, TextureAtlasData* data);

		virtual ShaderHandlePtr& getShader() const;
		virtual bool flipY() const { return true; }

		bool canFitRect(uint32 width, uint32 height);
		void update();
		virtual void cleanup();

	protected:

		std::string name;
		bool cleared;

		RenderTexturePtr rtt;
		TextureHandlePtr atlasTexture;
        TextureChannels channels;
    
		uint32 dimensions;
		typedef std::vector<RectF> EmptyAreas;
		EmptyAreas emptyAreas;

		typedef std::map<std::string, TextureHandlePtr> AtlasMap;
		AtlasMap map;

		typedef std::map<std::string, TextureAtlasData*> AtlasAddMap;
		AtlasAddMap toAddMap;
        AtlasAddMap addedMap;
    
	private:

		void createTextures();
		void drawToAtlas(const std::string& texName, TextureAtlasData& data);
		bool getFitRect(TextureAtlasData& data, RectF& fitRect);
		void drawTexture(const std::string& name, RectF& atlasRect, TextureHandlePtr& srcTex);
		virtual void onTextureUpdated(const std::string& name, TextureAtlasData* data) { }

	};

	struct TextureAtlasReference
	{
		TextureAtlasReference()
			: data(nullptr)
		{ }

		void onTextureHandleAvailable(TextureHandlePtr result)
		{
			this->resultHandle = result;
		}

		TextureAtlasData* data;
		TextureHandlePtr resultHandle;
		TextureAtlasPtr atlas;
	};

	typedef std::shared_ptr<TextureAtlasReference> TextureAtlasReferencePtr;

	class TextureAtlasManager : public Singleton<TextureAtlasManager>
	{
	public:

		TextureHandlePtr getTextureHandle(const std::string& name);
		TextureAtlasReferencePtr getTextureAtlas(const TextureHandlePtr& sourceTexture);
		void updateAtlases();

	private:

		TextureAtlasPtr getAtlas(const TextureHandlePtr& handleToAdd);
		void pushAtlas();

		std::vector<TextureAtlasPtr> atlases;
		
		typedef std::map<std::string, TextureAtlasReferencePtr> TexNameToAtlas;
		TexNameToAtlas nameToAtlas;

	};
}
