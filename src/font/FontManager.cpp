#include "PCH.h"

#include "font/FontManager.h"
#include "os/FileManager.h"
#include "global/Utils.h"
#include "gfx/RenderInterface.h"

namespace cs
{

	Font::Font(FontLock& lock, const std::string& name, const FontAtlasSize& d)
		: fileName(name)
		, dimm(d)
		, doubleSpace(1.0f)
	{
		
	}

	bool Font::init(FT_Library& ft)
	{
		std::string filePath;
		if (FileManager::getInstance()->getPathToFile(fileName, filePath))
		{
			if (FT_New_Face(ft, filePath.c_str(), 0, &this->face))
			{
				log::print(LogError, "Could not open font\n");
				return false;
			}
		}
		else
		{
			log::print(LogError, "Could not find font\n");
		}

		cs::log::print(cs::LogInfo, "Font: ", fileName, " initialized");
		
		return true;
	}

	TextureHandlePtr Font::getTexture(uint16 sz)
	{ 
		FontAtlasPtr atlas = this->getAtlas(sz);
		if (atlas.get())
		{
			return atlas->getAtlasTexture();
		}
		return TextureHandlePtr();
	}

	TextureHandlePtr Font::getCharacterHandle(const std::string& charStr, int32 sz)
	{
		assert(charStr.length() > 0);
		return this->getCharacterHandle(charStr[0], sz);
	}

    TextureHandlePtr Font::getCharacterHandle(char character, int32 sz)
    {
        uint16 adjusted_size = (uint16) (sz * RenderInterface::getInstance()->getContentScale());
        FontAtlasPtr atlas = this->getAtlas(adjusted_size, false);
        if (atlas.get())
        {
            CharacterPair pair;
            pair.first = character;
            pair.second = (uint16) adjusted_size;
            
            std::string str = FontAtlas::getCharPairAsString(pair);
            if (str.length() > 0)
            {
                return atlas->getSourceTextureHandle(str);
            }
        }
        return TextureHandlePtr();
    }

	FontAtlasPtr Font::getAtlas(uint16 sz, bool canCreate)
	{
    
        FontTextureSizeMap::iterator it = this->fontTexture.find(sz);
        if (it != this->fontTexture.end())
            return it->second;

        if (canCreate)
        {
            uint32 atlasSize = this->dimm.fontLarge;
            if (sz < 50)
            {
                atlasSize = this->dimm.fontMedium;
            }
            if (sz < 30)
            {
                atlasSize = this->dimm.fontSmall;
            }
            FontAtlasPtr atlas = CREATE_CLASS(FontAtlas, this->fileName, atlasSize);
            this->fontTexture[sz] = atlas;
            return atlas;
        }
        else
        {
            return FontAtlasPtr();
        }
    }

	void Font::getInfo(std::string& str)
	{
		std::stringstream infoStr;
		size_t charCount = 0;
		for (auto& atlas : this->fontTexture)
		{
			charCount += atlas.second->getNumCharacters();
		}
		infoStr << "Font Name: " << this->fileName << " Atlases: " << this->fontTexture.size() << " Characters: " << charCount << std::endl;
		str += infoStr.str();
	}

	const CharacterData* Font::findChar(const CharacterPair& pair)
	{
		if (!this->face)
		{
			log::error("Cannot load character - no face defined!");
			return nullptr;
		}

		FontAtlasPtr fontAtlas = this->getAtlas(pair.second);
		if (!fontAtlas)
		{
			log::error("Error - no atlas found!");
			return nullptr;
		}

		CharacterData* find_data = fontAtlas->getChar(pair);
		if (find_data)
			return find_data;

		if (!fontAtlas->shouldAddChar(pair))
		{
			return nullptr;
		}

		static CharacterData kDefaultCharacterTex;

		FT_Set_Pixel_Sizes(this->face, 0, pair.second);
		if (FT_Load_Char(this->face, pair.first, FT_LOAD_RENDER))
		{
			log::print(LogError, "Could not load character ", pair.first, ", ", pair.second);
			return &kDefaultCharacterTex;
		}

		FT_GlyphSlot g = this->face->glyph;
		CharacterData* data = new CharacterData;
		
		data->left = g->bitmap_left;
		data->top = g->bitmap_top;
		data->x_advance = uint16(g->advance.x >> 6);
		data->y_advance = uint16(g->advance.y >> 6);

		TextureAtlasData* textureData = new TextureAtlasData();
		textureData->width = g->bitmap.width;
		textureData->height = g->bitmap.rows;
		textureData->bytelen = g->bitmap.width * g->bitmap.rows;
		textureData->bytes = new uchar[textureData->bytelen];
        textureData->channels = TextureAlpha;
        
		memcpy(textureData->bytes, g->bitmap.buffer, textureData->bytelen);

		data->data = textureData;

		fontAtlas->addChar(pair, data);
		return nullptr;
	}

	void Font::update()
	{
		for (auto& it : this->fontTexture)
		{
			if (it.second.get())
			{
				it.second->update();
			}
		}
	}

	void Font::cleanup()
	{

		for (auto& it : this->fontTexture)
		{
			if (it.second.get())
			{
				it.second->cleanup();
			}
		}
	}

	void Font::preload(const std::string& str, TextOptions& options)
	{
		FontAtlasPtr fontAtlas = this->getAtlas(options.size);
		if (!fontAtlas)
		{
			log::error("Error - no atlas found!");
			return;
		}

		for (size_t c = 0; c < str.length(); c++)
		{
			CharacterPair charPair(str[c], options.size);
			const CharacterData* ptr = this->findChar(charPair);
		}
	}

	bool Font::generateVertices(const std::string& str, TextOptions& options, TextLines& lineData, RectF& bounds, std::vector<RectF>& lineBounds, float32* maxWidth, float32 textScale)
	{
		bool firstVert = true;
		float32 x_adv = 0.0f;
		float32 y_adv = 0.0f;

		bool found_all = true;
		
		// Use 'T' as the max height character
		CharacterPair heightPair('T', options.size);
		const CharacterData* height_ptr = this->findChar(heightPair);
		size_t use_height = 0;
		if (height_ptr)
		{
			use_height = height_ptr->data->height * 1.2;
		}

		FontAtlasPtr fontAtlas = this->getAtlas(options.size);
		if (!fontAtlas)
		{
			log::error("Error - no atlas found!");
			return false;
		}

		TextVertices vertices;
		RectF vertBounds;
		FloatExtentCalculator widthCalc;
		FloatExtentCalculator heightCalc;
		
		std::string curLine;

		for (size_t c = 0; c < str.length(); c++)
		{
			
			bool wrapAround = maxWidth && (x_adv > *maxWidth);
			if (str[c] == '\n' || wrapAround)
			{
				if (wrapAround)
				{
					while (str[--c] != ' ' && vertices.size() > 0)
					{
						vertices.erase(vertices.end() - 4, vertices.end());
					}
				}

				if (vertices.size() > 0)
				{
					vertBounds.pos.x = vertices[0].pos.x;
					vertBounds.pos.y = heightCalc.minValue;
					vertBounds.size.w = vertices[vertices.size() - 1].pos.x - vertices[0].pos.x;
					vertBounds.size.h = heightCalc.maxValue - heightCalc.minValue;
				}
				else
				{
					vertBounds.pos.x = 0.0f;
					vertBounds.pos.y = 0.0f;
					vertBounds.size.w = 0.0f;
					vertBounds.size.h = 0.0f;
				}
				
				widthCalc.reset();
				heightCalc.reset();

				lineData.push_back(vertices);
				vertices.clear();

				bounds.combine(vertBounds);

				lineBounds.push_back(vertBounds);
				
				curLine = std::string();
				x_adv = 0.0f;
				y_adv -= use_height + 1;
				continue;
			}

			// discard leading spaces
			if (str[c] == ' '  && vertices.size() == 0)
			{
				continue;
			}

			curLine.push_back(str[c]);
			float32 scale = (str[c] == ' ') ? this->getDoubleSpace() : 1.0f;

			CharacterPair charPair(str[c], options.size);
			const CharacterData* ptr = this->findChar(charPair);
			if (!ptr)
			{
				found_all = false;
				x_adv += 4.0f * scale;
				continue;
			}

			if (ptr->data->texture == nullptr)
			{
				x_adv += ptr->x_advance * scale;
				found_all = found_all && isspace(str[c]);
				continue;
			}
				
			RectF charRect;
			found_all = found_all && fontAtlas->getCharacterRect(charPair, charRect);

			RectF scaleRect(ptr->left, ptr->top - ptr->data->height, ptr->data->width, ptr->data->height);

			widthCalc.evaluate(x_adv + scaleRect.getLeft());
			widthCalc.evaluate(x_adv + scaleRect.getRight());
			heightCalc.evaluate(y_adv + scaleRect.getTop());
			heightCalc.evaluate(y_adv + scaleRect.getBottom());

			if (textScale != 1.0f)
			{
				scaleRect.scaleCenter(textScale);
			}

			{
				vertices.push_back({
					vec3(x_adv + scaleRect.getLeft(), y_adv + scaleRect.getBottom(), 0.0f), toVec2(charRect.getBL())
				});

				vertices.push_back({
					vec3(x_adv + scaleRect.getLeft(), y_adv + scaleRect.getTop(), 0.0f), toVec2(charRect.getTL())
				});

				vertices.push_back({
					vec3(x_adv + scaleRect.getRight(), y_adv + scaleRect.getTop(), 0.0f), toVec2(charRect.getTR())
				});

				vertices.push_back({
					vec3(x_adv + scaleRect.getRight(), y_adv + scaleRect.getBottom(), 0.0f), toVec2(charRect.getBR())
				});
			}

			x_adv += ptr->x_advance;			
		}

		if (vertices.size() > 0)
		{
			vertBounds.pos.x = widthCalc.minValue;
			vertBounds.pos.y = heightCalc.minValue;
			vertBounds.size.w = widthCalc.maxValue - widthCalc.minValue;
			vertBounds.size.h = heightCalc.maxValue - heightCalc.minValue;
		}
		
		bounds.combine(vertBounds);

		lineData.push_back(vertices);
		lineBounds.push_back(vertBounds);

		return found_all;
	}


	CharacterData* Font::getChar(const CharacterPair& pair)
	{
		FontAtlasPtr fontAtlas = this->getAtlas(pair.second);
		if (!fontAtlas)
		{
			log::error("Error - no atlas found!");
			return nullptr;
		}

		if (fontAtlas.get())
			return fontAtlas->getChar(pair);

		return nullptr;
	}

	FT_Library FontManager::ft;

	FontManager::FontManager()
	{
		if (FT_Init_FreeType(&ft)) 
		{
			log::print(LogError, "Could not init freetype library");
		}
	}

    void FontManager::preloadWithString(const std::string& fileName, uint16 sz, const char* str)
    {
        FontPtr& font = this->getFont(fileName);
        if (!font.get())
        {
            log::error("Cannot find font ", fileName);
            return;
        }
        
        TextOptions options;
        options.size = sz;
        
        TextLines lines;
        RectF dummyRect;
		std::vector<RectF> dummyBounds;
        
		font->generateVertices(str, options, lines, dummyRect, dummyBounds, nullptr);
    }
    
	void FontManager::preload(const std::string& fileName, uint16 sz)
	{
        const char* kStr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!";
        this->preloadWithString(fileName, sz, kStr);
	}

	void FontManager::setDoubleSpace(const std::string& fontName, float32 doubleSpace)
	{
		FontPtr& font = this->getFont(fontName);
		if (!font.get())
		{
			log::error("Cannot find font ", fontName);
			return;
		}

		font->setDoubleSpace(doubleSpace);
	}

	FontPtr& FontManager::getFont(const std::string& fileName)
	{
		FontMap::iterator it = this->fonts.find(fileName);
		if (it != this->fonts.end())
			return it->second;

        FontAtlasSize sz;
        sz.fontSmall = 512;
        sz.fontMedium = 1024;
        sz.fontLarge = 2048;

		Font::FontLock lock;
		FontPtr font = CREATE_CLASS(Font, lock, fileName, sz);
		if (font->init(ft))
		{
			this->fonts[fileName] = font;
			return this->fonts[fileName];
		}

		static std::shared_ptr<Font> kEmptyFontPtr(nullptr);
		return kEmptyFontPtr;
	}

	void FontManager::updateFonts()
	{
		for (auto& it : this->fonts)
			it.second->update();
	}


	std::string FontManager::getDiagnostics()
	{
		std::string str;
		for (auto& it : this->fonts)
			it.second->getInfo(str);
		return str;
	}

	void FontManager::cleanup()
	{
		for (auto& it : this->fonts)
			it.second->cleanup();
	}


}
