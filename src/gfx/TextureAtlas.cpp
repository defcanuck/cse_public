#include "PCH.h"

#include "gfx/TextureAtlas.h"
#include "gfx/RenderInterface.h"
#include "gfx/RenderTexture.h"
#include "global/Utils.h"

// #define WRITE_SOLID_TEXTURES 1
namespace cs
{

    const bool kTestingClear = false;
    
	TextureAtlasData::TextureAtlasData()
		: width(0)
		, height(0)
		, texture(nullptr)
		, bytes(nullptr)
		, bytelen(0)
        , channels(TextureAlpha)
	{

	}

	TextureAtlasData::~TextureAtlasData()
	{
		delete[] this->bytes;
	}

	TextureAtlasData::TextureAtlasData(const TextureAtlasData& rhs)
	{
		this->width = rhs.width;
		this->height = rhs.height;
		this->texture = rhs.texture;
		this->bytelen = rhs.bytelen;
		this->bytes = new uchar[bytelen];
        this->channels = rhs.channels;
        
		memcpy(this->bytes, rhs.bytes, this->bytelen);
		this->callback = rhs.callback;
	}

	void TextureAtlasData::operator=(const TextureAtlasData& rhs)
	{
		this->width = rhs.width;
		this->height = rhs.height;
		this->texture = rhs.texture;
		this->bytelen = rhs.bytelen;
		this->bytes = new uchar[bytelen];
		memcpy(this->bytes, rhs.bytes, this->bytelen);
		this->callback = rhs.callback;
	}

	TextureAtlas::TextureAtlas(const std::string& atlasName, uint32 dimm, TextureChannels c)
		: name(atlasName)
        , channels(c)
		, dimensions(dimm)
		, cleared(false)
	{

		Dimensions rttDimm(dimm, dimm);
		Dimensions depthDimm(dimm, dimm);

		RectF fullRect(0.0f, 0.0f, float32(dimm), float32(dimm));
		emptyAreas.push_back(fullRect);

		this->rtt = CREATE_CLASS(RenderTexture, "FontAtlas", rttDimm, this->channels);
		TextureResourcePtr texRes = this->rtt->getTextureResource();
		TexturePtr atlasTexture = CREATE_CLASS(Texture, "FontAtlas", texRes);

		RectF boxRect(0.0f, 0.0f, 1.0f, 1.0f);
		this->atlasTexture = CREATE_CLASS(TextureHandle, atlasTexture, boxRect);
	}

	TextureAtlas::~TextureAtlas()
	{

	}

	TextureHandlePtr TextureAtlas::getTextureHandle(const std::string& name)
	{
		AtlasMap::iterator it;
		if ((it = this->map.find(name)) != this->map.end())
		{
			return it->second;
		}
		return TextureHandlePtr();
	}
    
    TextureHandlePtr TextureAtlas::getSourceTextureHandle(const std::string& name)
    {
        AtlasAddMap::iterator it = this->addedMap.find(name);
        if (it != this->addedMap.end())
        {
            TextureAtlasData* data = it->second;
            assert(data);
            return data->texture;
        }
        return TextureHandlePtr();
    }

	bool TextureAtlas::getRect(const std::string& str, RectF& rect)
	{
		AtlasMap::iterator it;
		if ((it = this->map.find(str)) != this->map.end())
		{
			rect = it->second->getUVRect();
            
#if defined(CS_METAL)
            rect.pos.y = 1.0f - rect.pos.y;
            rect.size.h = -rect.size.h;
#endif
			return true;
		}

		static RectF kEmptyCharacterRect;

		rect = kEmptyCharacterRect;
		return false;
	}

	void TextureAtlas::createTextures()
	{
		for (AtlasAddMap::iterator it = this->toAddMap.begin(); it != this->toAddMap.end(); it++)
		{
			TextureAtlasData& data = *it->second;

			if (data.width > 0 && data.height > 0 && !data.texture.get())
			{
				uint32 adjW = std::max<uint32>(32, nextPow2(data.width));
				uint32 adjH = std::max<uint32>(32, nextPow2(data.height));

				uint32 tex_size = getTextureSize(data.channels);

				size_t gpu_length = adjW * adjH * tex_size;
				uchar* gpu_bytes = new uchar[gpu_length];
				memset(gpu_bytes, 255, gpu_length);
                
#if !defined(WRITE_SOLID_TEXTURES)
				for (uint32 i = 0; i < data.height; i++)
				{
					uchar* rowDst = gpu_bytes + (adjW * i * tex_size);
					uchar* rowSrc = data.bytes + (data.width * i * tex_size);
					memcpy(rowDst, rowSrc, data.width * tex_size);
				}
#else
                for (uint32 h = 0; h < data.height; h++)
                {
                    for (uint32 w = 0; w < data.width; w++)
                    {
                        uint32 idx = w + (data.width * h);
                        gpu_bytes[idx] = rand() % 255;
                    }
                }
#endif
                
				RectF uvRect(0.0f, 0.0f, data.width / float32(adjW), data.height / float32(adjH));

				TextureResourcePtr texRes = RenderInterface::getInstance()->loadTexture(
					Dimensions(adjW, adjH),
					data.channels,
					gpu_bytes);

				TexturePtr tex = CREATE_CLASS(Texture, it->first, texRes);
				data.texture = CREATE_CLASS(TextureHandle, tex, uvRect);

			}
		}
	}

	void TextureAtlas::update()
	{
		if (this->toAddMap.size() == 0)
			return;

        RenderInterface* render_interface = RenderInterface::getInstance();
        const Dimensions& dimm = this->rtt->getDimensions();

		render_interface->pushDebugScope("CharacterCreate");
		this->createTextures();
		render_interface->popDebugScope();
        
        std::vector<ClearMode> clearmode;
		this->rtt->bind(false, &clearmode);
        render_interface->pushDebugScope("FontAtlasUpdate");
        
#if !defined(CS_METAL)
		if (!this->cleared || kTestingClear)
		{
			RenderInterface::getInstance()->setClearColor(ColorF::Clear);
			std::vector<ClearMode> clearmode;
			clearmode.push_back(ClearColor);
			RenderInterface::getInstance()->clear(clearmode);
			this->cleared = true;
		}
#endif

		RectI viewport(0, 0, dimm.w, dimm.h);
        render_interface->setZ(-1.0f, 1.0f);
		render_interface->setViewport(viewport);

        render_interface->startRenderPass();
        
		cs::UniformPtr matrix = SharedUniform::getInstance().getUniform("mvp");
		assert(matrix);

		mat4 projection = glm::ortho(0.0f, float32(dimm.w), 0.0f, float32(dimm.h), -1.0f, 1.0f);
		matrix->setValue(projection);

		if (kTestingClear)
		{
			RectF fullRect(0.0f, 0.0f, float32(this->dimensions), float32(this->dimensions));
			emptyAreas.clear();
			emptyAreas.push_back(fullRect);
		}

		for (auto& it : this->toAddMap)
		{
			std::stringstream str;
			str << "Texture: " << it.first;
			std::string charStr = str.str();
			render_interface->pushDebugScope(charStr);

			this->drawToAtlas(it.first, *it.second);

			this->onTextureUpdated(it.first, it.second);

			render_interface->popDebugScope();
		}

        if (!kTestingClear)
        {
            this->cleanup();
        }
        
        this->rtt->unbind();
        
		render_interface->popDebugScope();
	}

	void TextureAtlas::drawToAtlas(const std::string& texName, TextureAtlasData& data)
	{
		RectF fitRect;
		if (this->getFitRect(data, fitRect))
		{
			if (!data.texture)
				return;

			TextureAtlas::drawTexture(texName, fitRect, data.texture);
			TexturePtr rtt_tex = CREATE_CLASS(Texture, texName + "_RTT", this->rtt->getTextureResource());

			float32 invDimm = 1.0f / float32(this->dimensions);
			fitRect.pos.x *= invDimm;
			fitRect.pos.y *= invDimm;
			fitRect.size.w *= invDimm;
			fitRect.size.h *= invDimm;

			this->map[texName] = CREATE_CLASS(TextureHandle, rtt_tex, fitRect);

			// set the result texture handle
			if (data.callback)
			{
				data.callback(this->map[texName]);
			}

			// alert listeners the TextureHandle is atlased and ready for use
			data.onAtlased.invoke();
		}
		else
		{
			log::info("Error - cannot fit texture in atlas - ", texName);
		}
	}

	bool TextureAtlas::canFitRect(uint32 width, uint32 height)
	{
		float32 kAtlasScale = 1.0f;
		PointF sz((width * kAtlasScale) + 2, (height * kAtlasScale) + 2);
		RectF a, b;
		EmptyAreas::iterator it;
		for (it = this->emptyAreas.begin(); it != this->emptyAreas.end(); it++)
		{
			RectF volume = (*it);
			if (volume.canFit(sz))
			{
				return true;
			}
		}
		return false;
	}

	bool TextureAtlas::getFitRect(TextureAtlasData& data, RectF& fitRect)
	{

		struct local
		{
			static bool compareArea(const RectF& a, const RectF& b)
			{
				return (a.size.w * a.size.h) < (b.size.w * b.size.h);
			}
		};

		std::sort(this->emptyAreas.begin(), this->emptyAreas.end(), local::compareArea);

		float32 kAtlasScale = 1.0f;
		PointF sz((data.width * kAtlasScale) + 2, (data.height * kAtlasScale) + 2);
		RectF a, b;
		EmptyAreas::iterator it;
		for (it = this->emptyAreas.begin(); it != this->emptyAreas.end(); it++)
		{
			RectF volume = (*it);
			if (volume.canFit(sz))
			{
				volume.split(sz, fitRect, a, b);
				break;
			}
		}

		if (it != this->emptyAreas.end())
		{

			fitRect.pos.x = fitRect.pos.x + 1;
			fitRect.pos.y = fitRect.pos.y + 1;

			fitRect.size.w = fitRect.size.w - 2;
			fitRect.size.h = fitRect.size.h - 2;

			this->emptyAreas.erase(it);

			if (a.size.w > 0 && a.size.h > 0)
				this->emptyAreas.push_back(a);

			if (b.size.w > 0 && b.size.h > 0)
				this->emptyAreas.push_back(b);

			return true;
		}
		return false;
	}

	ShaderHandlePtr& TextureAtlas::getShader() const
	{
		return RenderInterface::kDefaultTextureShader;
	}

	void TextureAtlas::drawTexture(const std::string& name, RectF& atlasRect, TextureHandlePtr& srcTex)
	{
		cs::GeometryDataPtr draw = CREATE_CLASS(cs::GeometryData);

		draw->indexSize = 6;
		draw->vertexSize = 4;

		draw->indexData.push_back(0);
		draw->indexData.push_back(2);
		draw->indexData.push_back(1);
		draw->indexData.push_back(0);
		draw->indexData.push_back(3);
		draw->indexData.push_back(2);

		PointF bl = atlasRect.getBL();
		PointF tl = atlasRect.getTL();
		PointF tr = atlasRect.getTR();
		PointF br = atlasRect.getBR();

		vec3 pos[] =
		{
			vec3(bl.x, bl.y, 0.0f),
			vec3(tl.x, tl.y, 0.0f),
			vec3(tr.x, tr.y, 0.0f),
			vec3(br.x, br.y, 0.0f)
		};

		const RectF& charRect = srcTex->getUVRect();
		vec2 tex[] = {
			toVec2((this->flipY()) ? charRect.getBL() : charRect.getTL()),
			toVec2((this->flipY()) ? charRect.getTL() : charRect.getBL()),
			toVec2((this->flipY()) ? charRect.getTR() : charRect.getBR()),
			toVec2((this->flipY()) ? charRect.getBR() : charRect.getTR()),
		};

		for (int i = 0; i < 4; i++)
		{
			// pos
			draw->vertexData.push_back(pos[i].x);
			draw->vertexData.push_back(pos[i].y);
			draw->vertexData.push_back(pos[i].z);

			// texture coordinate
			draw->vertexData.push_back(tex[i].x);
			draw->vertexData.push_back(tex[i].y);
		}

		draw->decl.addAttrib(AttribPosition, { AttribPosition, TypeFloat, 3, 0 });
		draw->decl.addAttrib(AttribTexCoord0, { AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });

		cs::DrawCallPtr drawCall = CREATE_CLASS(cs::DrawCall);
		drawCall->type = DrawTriangles;
		drawCall->offset = 0;
		drawCall->count = 6;
		drawCall->textures[0] = srcTex;
		drawCall->shaderHandle = CREATE_CLASS(ShaderHandle, this->getShader());
		drawCall->indexType = TypeUnsignedShort;
		drawCall->depthTest = false;
		drawCall->indices = draw->indexData;
		drawCall->tag = name;
        drawCall->cullFace = CullNone;
        
		draw->drawCalls.push_back(drawCall);

		static GeometryPtr geom;
		geom = CREATE_CLASS(cs::Geometry, draw);

		cs::UniformPtr color = SharedUniform::getInstance().getUniform("color");
		if (color)
			color->setValue(vec4(1.0, 1.0, 1.0, 1.0f));

		geom->draw(nullptr);
	}

	void TextureAtlas::cleanup()
	{
        for (auto& it : this->toAddMap)
        {
            if (this->addedMap.find(name) == this->addedMap.end())
            {
                this->addedMap[it.first] = it.second;
            }
        }
		this->toAddMap.clear();
	}

	void TextureAtlas::addTexture(const std::string& name, TextureAtlasData* data)
	{
		AtlasAddMap::iterator it;
		if ((it = this->toAddMap.find(name)) != this->toAddMap.end())
		{
			return;
		}

		this->toAddMap[name] = data;
	}

	TextureHandlePtr TextureAtlasManager::getTextureHandle(const std::string& name)
	{
		TexNameToAtlas::iterator it = this->nameToAtlas.find(name);
		if (it != this->nameToAtlas.end() && it->second->atlas.get())
		{
			return it->second->atlas->getTextureHandle(name);
		}

		return TextureHandlePtr();
	}

	TextureAtlasReferencePtr TextureAtlasManager::getTextureAtlas(const TextureHandlePtr& sourceTexture)
	{
		const std::string& name = sourceTexture->getTextureName();
		TexNameToAtlas::iterator it = this->nameToAtlas.find(name);
		if (it != this->nameToAtlas.end() && it->second->atlas.get())
		{
			return it->second;
		}

		TexturePtr tex = sourceTexture->getTexture();

		TextureAtlasReferencePtr atlasReference = CREATE_CLASS(TextureAtlasReference);
		atlasReference->data = new TextureAtlasData;
		atlasReference->data->width = tex->getWidth();
		atlasReference->data->height = tex->getHeight();
		atlasReference->data->texture = sourceTexture;
        atlasReference->data->channels = tex->getChannels();
        
		atlasReference->data->callback = std::bind(&TextureAtlasReference::onTextureHandleAvailable, atlasReference.get(), std::placeholders::_1);

		this->nameToAtlas[name] = atlasReference;

		atlasReference->atlas = this->getAtlas(sourceTexture);
		atlasReference->atlas->addTexture(name, atlasReference->data);

		return atlasReference;
	}


	TextureAtlasPtr TextureAtlasManager::getAtlas(const TextureHandlePtr& handleToAdd)
	{
		TextureAtlasPtr atlasToUse;
		if (this->atlases.size() == 0)
		{
			this->pushAtlas();
			atlasToUse = this->atlases.back();
		}
		else
		{
			const TexturePtr& srcTexture = handleToAdd->getTexture();
			for (auto& it : this->atlases)
			{
				if (it->canFitRect(srcTexture->getWidth(), srcTexture->getHeight()))
				{
					atlasToUse = it;
					break;
				}
			}

			if (!atlasToUse.get())
			{
				this->pushAtlas();
				atlasToUse = this->atlases.back();
			}
		}

		assert(atlasToUse.get());
		return atlasToUse;
	}

	void TextureAtlasManager::pushAtlas()
	{
		uint32 dimm = 1024;
		std::stringstream name;
		name << "Atlas_" << this->atlases.size();
		this->atlases.push_back(CREATE_CLASS(TextureAtlas, name.str(), dimm, TextureRGBA));
	}

	void TextureAtlasManager::updateAtlases()
	{
		for (auto& it : this->atlases)
		{
			it->update();
		}
	}
}
