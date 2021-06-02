#pragma once

#include "ClassDef.h"

#include "global/Singleton.h"
#include "font/FontAtlas.h"

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftoutln.h>
#include <fttrigon.h>

#include <string>
#include <map>

#include "gfx/TextureHandle.h"

namespace cs
{

	// "FreeSans.ttf"

	class FontManager;

	struct TextVertex
	{
		vec3 pos;
		vec2 uv;
	};

	typedef std::vector<TextVertex> TextVertices;
	typedef std::vector<TextVertices> TextLines;

	struct TextOptions
	{
		uint16 size;
	};

    struct FontAtlasSize
    {
        uint32 fontSmall;
        uint32 fontMedium;
        uint32 fontLarge;
    };

	CLASS_DEFINITION(Font)

		struct FontLock { };

	public:
	
		Font(FontLock& lock, const std::string& name, const FontAtlasSize& atlasSize);
		virtual ~Font() { }

		bool generateVertices(const std::string& str, TextOptions& options, TextLines& lines, RectF& bounds, std::vector<RectF>& lineBounds, float32* maxWidth, float32 textScale = 1.0f);

		TextureHandlePtr getTexture(uint16 sz);
        TextureHandlePtr getCharacterHandle(char character, int32 sz);
		TextureHandlePtr getCharacterHandle(const std::string& charStr, int32 sz);
		FontAtlasPtr getAtlas(uint16 sz, bool canCreate = true);

		CharacterData* getChar(const CharacterPair& pair);

		void update();
		void cleanup();

		void getInfo(std::string& str);
		void preload(const std::string& str, TextOptions& options);
		void setDoubleSpace(float32 space) { this->doubleSpace = space; }
		float32 getDoubleSpace() const { return this->doubleSpace; }

	private:

		friend class FontManager;

		const CharacterData* findChar(const CharacterPair& pair);
		bool init(FT_Library& fl);
		
		std::string fileName;
		FT_Face face;

		typedef std::map<uint16, FontAtlasPtr> FontTextureSizeMap;
		FontTextureSizeMap fontTexture;
		FontAtlasSize dimm;
		float32 doubleSpace;

	};

	class FontManager : public Singleton<FontManager>
	{
	public:
		FontManager();

		FontPtr& getFont(const std::string& fileName);

		void updateFonts();
		void cleanup();
		void preload(const std::string& fileName, uint16 sz);
        void preloadWithString(const std::string& fileName, uint16 sz, const char* str);
        
		void setDoubleSpace(const std::string& fontName, float32 doubleSpace);

		std::string getDiagnostics();

	private:
		
		static FT_Library ft;

		typedef std::map<std::string, FontPtr> FontMap;
		FontMap fonts;
	};
}
