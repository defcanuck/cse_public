#include "PCH.h"

#include "font/FontAtlas.h"
#include "gfx/RenderInterface.h"
#include "gfx/RenderTexture.h"
#include "global/Utils.h"

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftoutln.h>
#include <fttrigon.h>

namespace cs
{
    const TextureChannels kFontAtlasChannels = TextureRGBA;

	CharacterData::CharacterData()
		: left(0)
		, top(0)
		, x_advance(0)
		, y_advance(0)
		, data(nullptr)
		, atlased(false)
	{ }

	CharacterData::CharacterData(const CharacterData& rhs)
	{
		this->left = rhs.left;
		this->top = rhs.top;
		this->x_advance = rhs.x_advance;
		this->y_advance = rhs.y_advance;
		this->atlased = rhs.atlased;
		if (rhs.data)
		{
			this->data = new TextureAtlasData(*rhs.data);
		}
		else
		{
			this->data = nullptr;
		}
	}

	void CharacterData::operator=(const CharacterData& rhs)
	{
		this->left = rhs.left;
		this->top = rhs.top;
		this->x_advance = rhs.x_advance;
		this->y_advance = rhs.y_advance;
		this->atlased = rhs.atlased;
		if (rhs.data)
		{
			this->data = new TextureAtlasData(*rhs.data);
		}
	}

	CharacterData::~CharacterData()
	{
		if (this->data)
		{
			delete this->data;
		}
	}

	FontAtlas::FontAtlas(const std::string& atlasName, uint32 dimm)
		: TextureAtlas(atlasName, dimm, kFontAtlasChannels)
	{ }

	FontAtlas::~FontAtlas()
	{
		for (auto& it : this->characterMap)
		{
			delete it.second;
		}
		this->characterMap.clear();
	}

	ShaderHandlePtr& FontAtlas::getShader() const
	{
		return RenderInterface::kFontAtkasShader;
	}

	bool FontAtlas::getCharacterRect(CharacterPair& pair, RectF& charRect)
	{
		std::string pairStr = FontAtlas::getCharPairAsString(pair);
		return this->getRect(pairStr, charRect);
	}
	
	bool FontAtlas::shouldAddChar(const CharacterPair& pair)
	{
		std::string str = getCharPairAsString(pair);
		return this->toAddMap.find(str) == this->toAddMap.end();
	}

	CharacterData* FontAtlas::getChar(const CharacterPair& pair)
	{
		CharacterMap::iterator it;
		if ((it = this->characterMap.find(pair)) != this->characterMap.end() && it->second->atlased)
		{
			return it->second;
		}
		return nullptr;
	}

	void FontAtlas::addChar(const CharacterPair& pair, CharacterData* data)
	{
		std::string str = getCharPairAsString(pair);
		CharacterMap::iterator it;
		if ((it = this->characterMap.find(pair)) != this->characterMap.end())
		{
			return;
		}

		this->characterMap[pair] = data;
		this->addTexture(str, this->characterMap[pair]->data);
	}

	void FontAtlas::cleanup()
	{
		BASECLASS::cleanup();
	}

	void FontAtlas::onTextureUpdated(const std::string& name, TextureAtlasData* data)
	{
		CharacterMap::iterator it;
		CharacterPair pair = FontAtlas::getCharPairForString(name);
		if ((it = this->characterMap.find(pair)) != this->characterMap.end())
		{
			it->second->atlased = true;
		}
	}
	
}
