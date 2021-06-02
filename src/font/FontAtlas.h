#pragma once

#include "gfx/TextureAtlas.h"

#include <map>

namespace cs
{
	typedef std::pair<uchar, uint16> CharacterPair;
	
	struct CharacterData
	{
		CharacterData();
		CharacterData(const CharacterData& rhs);

		virtual ~CharacterData();
		void operator=(const CharacterData& rhs);

		int16 left;
		int16 top;
		int16 x_advance;
		int16 y_advance;
		TextureAtlasData* data;
		bool atlased;
	};

	CLASS_DEFINITION_DERIVED(FontAtlas, TextureAtlas)
	public:
		FontAtlas(const std::string& atlasName, uint32 dimm);
		virtual ~FontAtlas();

		inline static std::string getCharPairAsString(const CharacterPair& pair)
		{
			std::stringstream str;
			str << uint16(pair.first) << "_" << pair.second;
			return str.str();
		}

		inline static CharacterPair getCharPairForString(const std::string& str)
		{
			StringList pairList = explode(str, '_');
			assert(pairList.size() == 2);
			CharacterPair pair;
			pair.first = static_cast<uchar>(atoi(pairList[0].c_str()));
			pair.second = static_cast<uint16>(atoi(pairList[1].c_str()));
			return pair;
		}

		bool shouldAddChar(const CharacterPair& pair);
		CharacterData* getChar(const CharacterPair& pair);
		void addChar(const CharacterPair& pair, CharacterData* data);
		bool getCharacterRect(CharacterPair& pair, RectF& charRect);

		virtual ShaderHandlePtr& getShader() const;
		virtual bool flipY() const { return false; }
		
		static void drawCharacter(const CharacterPair& pair, RectF& atlasRect, TextureHandlePtr& srcTex);

		virtual void cleanup();
		size_t getNumCharacters() const { return this->characterMap.size(); }

	private:
		
		void addCharacterToAtlas(const CharacterPair& pair, CharacterData& data);
		virtual void onTextureUpdated(const std::string& name, TextureAtlasData* data);

		typedef std::map<CharacterPair, CharacterData*> CharacterMap;
		CharacterMap characterMap;

	};
}
