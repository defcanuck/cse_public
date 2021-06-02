#include "PCH.h"

#include "ui/UITextElement.h"
#include "global/ResourceFactory.h"

namespace cs
{
	const std::string kDefaultFontName = "FreeSansBold.ttf";
	static std::string gDebugTextBreakHere = "break_here";

    namespace UITextElementUtils
    {
        inline vec2 fixPlatformUVs(const vec2& uv)
        {
#if defined(CS_METAL)
            vec2 flippedUv = uv;
            flippedUv.y = 1.0f - flippedUv.y;
            return flippedUv;
#else
            return uv;
#endif
        }
    }

	UITextElement::UITextElement(const std::string& name)
		: UIElement(name)
		, textHAlign(HAlignCenter)
		, textVAlign(VAlignCenter)
		, textHPadding(UIDimension())
		, textVPadding(UIDimension())
		, fontColor(255, 255, 255, 255)
		, fontSize(kDefaultFontSize)
		, fontShader(CREATE_CLASS(ShaderHandle, (defaultFontShaderResource ? defaultFontShaderResource : RenderInterface::kDefaultFontShader)))
		, fontShadowShader(CREATE_CLASS(ShaderHandle, (defaultFontShaderResource ? defaultFontShaderResource : RenderInterface::kDefaultFontShader)))
		, text("")
		, textScale(1.0f)
		, textDirty(false)
		, fontOutlineWidth(0.0f)
		, fontOutlineColor(ColorB::Black)
		, shadowAnim(1.0f)
		, textAngle(0.0f)
		, textWrap(false)
		, forceTextDirty(false)
	{
		this->init();
	}

	UITextElement::UITextElement(const std::string& name, const std::string& txt)
		: UIElement(name)
		, textHAlign(HAlignCenter)
		, textVAlign(VAlignCenter)
		, textHPadding(UIDimension())
		, textVPadding(UIDimension())
		, fontColor(255, 255, 255, 255)
		, fontSize(kDefaultFontSize)
		, fontShader(CREATE_CLASS(ShaderHandle, (defaultFontShaderResource ? defaultFontShaderResource : RenderInterface::kDefaultFontShader)))
		, fontShadowShader(CREATE_CLASS(ShaderHandle, (defaultFontShaderResource ? defaultFontShaderResource : RenderInterface::kDefaultFontShader)))
		, text(txt)
		, textScale(1.0f)
		, textDirty(false)
		, fontOutlineWidth(0.0f)
		, fontOutlineColor(ColorB::Black)
		, shadowAnim(1.0f)
		, textAngle(0.0f)
		, textWrap(false)
		, forceTextDirty(false)
	{
		this->init();
	}

	void UITextElement::setTextHPadding(float32 value, SpanMeasure measure)
	{
		this->textHPadding.value = value;
		if (measure != SpanNone)
		{
			this->textHPadding.measure = measure;
			this->setTextDirty();
		}
	}

	void UITextElement::setTextVPadding(float32 value, SpanMeasure measure)
	{
		this->textVPadding.value = value;
		if (measure != SpanNone)
		{
			this->textVPadding.measure = measure;
			this->setTextDirty();
		}
	}

	uint32 UITextElement::getScaledFontSize()
	{
		uint32 rawSize = uint32(this->fontSize * RenderInterface::getInstance()->getContentScale());
#if defined(CS_WINDOWS) || defined(CS_IOS)
		// rawSize = uint32(float(rawSize) * avg);
#endif
		return rawSize;
	}

	void UITextElement::preload()
	{
		TextOptions options;
		options.size = this->getScaledFontSize();
		
		if (this->font.get())
		{
			this->font->preload(this->text, options);
		}
		BASECLASS::preload();
	}
    
    void UITextElement::setFontShadowShader(const std::string& shaderName)
    {
        // defaultDebug
        ShaderResourcePtr shaderResource = std::static_pointer_cast<ShaderResource>(
            ResourceFactory::getInstance()->loadResource<ShaderResource>(shaderName));
        if (shaderResource.get())
        {
            ShaderHandlePtr shaderHandle = CREATE_CLASS(ShaderHandle, shaderResource);
            this->fontShadowShader = shaderHandle;
        }
        else
        {
            log::error("Cannot find shader with name ", shaderName);
        }
    }
    
    void UITextElement::populateDrawData(const TextLines& data, BatchDrawDataPtr& drawData, const ColorB& fontColor, const std::vector<float>* wrapOffsets)
    {
        
        drawData->positions.clear();
        drawData->vcolors.clear();
        drawData->indices.clear();
        drawData->uvs0.clear();
        
        uint16 char_offset = 0;
        vec2 flipUV0;
        
        for (size_t i = 0; i < data.size(); i++)
        {
            const TextVertices& verts = data[i];
			vec3 linePosOffset = kZero3;
			if (wrapOffsets)
			{
				linePosOffset = vec3((*wrapOffsets)[i], 0.0f, 0.0f);
			}
            for (size_t c = 0; c < verts.size(); c++)
            {
                const TextVertex& it = verts[c];
				drawData->positions.push_back(it.pos + linePosOffset);
                drawData->uvs0.push_back(it.uv);
                drawData->vcolors.push_back(fontColor);
            }
            
            static std::vector<vec2> kCharUVs =
            {
                vec2(0.0f, 0.0f),
                vec2(0.0f, 1.0f),
                vec2(1.0f, 1.0f),
                vec2(1.0f, 0.0f)
            };
            
            size_t numChars = verts.size() / 4;
            
            // Add secondary UVs for character effects
            for (size_t c = 0; c < numChars; c++)
            {
                drawData->uvs1.insert(drawData->uvs1.end(), kCharUVs.begin(), kCharUVs.end());
            }
            
            for (uint16 idx = 0; idx < numChars; idx++)
            {
                uint16 charIdx = char_offset + (idx * 4);
                drawData->indices.push_back(charIdx + 0);
                drawData->indices.push_back(charIdx + 2);
                drawData->indices.push_back(charIdx + 1);
                
                drawData->indices.push_back(charIdx + 0);
                drawData->indices.push_back(charIdx + 3);
                drawData->indices.push_back(charIdx + 2);
            }
            char_offset += uint16(verts.size());
        }
    }
    
	void evaluatePositionMultiLineHeight(VAlign align, const float32& spanHeight, const float32& elemHeight, float32& result, const float32& padding, const float32& firstLineHeight)
	{
		switch (align)
		{
		case VAlignBottom:
			result = padding + elemHeight - firstLineHeight;
			break;
		case VAlignTop:
			result = padding - firstLineHeight + spanHeight;
			break;
		case VAlignCenter:
			result = spanHeight - elemHeight - padding + firstLineHeight;
			result = result - (spanHeight * 0.5) + (elemHeight * 0.5);
			break;
		}
	}

	void UITextElement::refreshTextVertices(const RectF& bounds, const UIBatchProcessData& data)
	{

		this->textVerts.options.size = this->getScaledFontSize();
		this->textVerts.lines.clear();
		this->textVerts.lineBounds.clear();
		this->textVerts.wrapOffsets.clear();

        this->textVerts.textRect.pos.x = 0.0f;
        this->textVerts.textRect.pos.y = 0.0f;
        this->textVerts.textRect.size.w = 0.0f;
        this->textVerts.textRect.size.h = 0.0f;

		this->textVerts.xPadding = this->textHPadding.evaluate(this->screen_rect.size.w);
		this->textVerts.yPadding = this->textVPadding.evaluate(this->screen_rect.size.h);
        
		
		float32 adjWidth = bounds.size.w - (this->textVerts.xPadding * 2.0f);
		this->textDirty = !this->font->generateVertices(this->text, this->textVerts.options, this->textVerts.lines, this->textVerts.textRect, this->textVerts.lineBounds, (this->textWrap) ? &adjWidth : nullptr, this->textScale);

		this->textVerts.xOffset = 0.0f;
		this->textVerts.yOffset = 0.0f;


		//float32 lineOffset = this->textVerts.textRect.size.h / float32(this->textVerts.lines.size());
		evaluatePosition(this->textHAlign, this->screen_rect.size.w, this->textVerts.textRect.size.w, this->textVerts.xOffset, this->textVerts.xPadding);

		this->textEntry.tint = this->fontColor;

		if (this->textVerts.lines.size() > 1)
		{
			float32 firstLineHeight = this->textVerts.lineBounds[0].size.h;
			evaluatePositionMultiLineHeight(this->textVAlign, this->screen_rect.size.h, this->textVerts.textRect.size.h, this->textVerts.yOffset, this->textVerts.yPadding, firstLineHeight);

			float32 elem_width = this->textVerts.textRect.size.w;
			for (size_t i = 0; i < this->textVerts.lines.size(); ++i)
			{
				float32 center_offset = 0.0f;
				float32 line_width = this->textVerts.lineBounds[i].size.w;
				evaluatePosition(this->textHAlign, elem_width, line_width, center_offset, 0.0f);
				this->textVerts.wrapOffsets.push_back(center_offset);
			}

			UITextElement::populateDrawData(this->textVerts.lines, this->textEntry.drawData, this->fontColor, &this->textVerts.wrapOffsets);
		}
		else
		{
			evaluatePosition(this->textVAlign, this->screen_rect.size.h, this->textVerts.textRect.size.h, this->textVerts.yOffset, this->textVerts.yPadding);

			UITextElement::populateDrawData(this->textVerts.lines, this->textEntry.drawData, this->fontColor);
		}

		this->textEntry.drawData->shader = this->fontShader;
		this->textEntry.drawData->texture[0] = font->getTexture(this->getScaledFontSize());
        //this->textEntry.drawData->texture[0] = RenderInterface::kDefaultTexture;
		this->textEntry.drawData->drawType = DrawTriangles;
	}

	void UITextElement::refreshShadowVertices()
	{
		for (auto& shadow : this->shadowList)
		{

			shadow.entry.drawData->positions.clear();
			shadow.entry.drawData->vcolors.clear();
			shadow.entry.drawData->indices.clear();
			shadow.entry.drawData->uvs0.clear();

			vec3 shadow_offset(shadow.offset.x * this->shadowAnim, shadow.offset.y * this->shadowAnim, 0.0f);

			ColorB shadow_color = shadow.color;
			shadow_color.a = shadow_color.a * this->fontColor.a / 255;

			shadow.entry.tint = shadow_color;
			uint16 char_offset = 0;
			for (size_t i = 0; i < this->textVerts.lines.size(); i++)
			{
				const TextVertices& verts = this->textVerts.lines[i];
				vec3 wrapOffset = kZero3;
				if (i < this->textVerts.wrapOffsets.size())
				{
					wrapOffset = vec3(this->textVerts.wrapOffsets[i], 0.0f, 0.0f);
				}

				for (size_t c = 0; c < verts.size(); c++)
				{
					const TextVertex& it = verts[c];
					shadow.entry.drawData->positions.push_back(it.pos + shadow_offset + wrapOffset);
					shadow.entry.drawData->uvs0.push_back(it.uv);
					shadow.entry.drawData->vcolors.push_back(shadow_color);
				}

				uint16 numChars = uint16(verts.size()) / 4;
				for (uint16 idx = 0; idx < numChars; idx++)
				{
					uint16 charIdx = char_offset + (idx * 4);
					shadow.entry.drawData->indices.push_back(charIdx + 0);
					shadow.entry.drawData->indices.push_back(charIdx + 2);
					shadow.entry.drawData->indices.push_back(charIdx + 1);

					shadow.entry.drawData->indices.push_back(charIdx + 0);
					shadow.entry.drawData->indices.push_back(charIdx + 3);
					shadow.entry.drawData->indices.push_back(charIdx + 2);
				}
				char_offset += uint16(verts.size());
			}

			shadow.entry.drawData->shader = this->fontShadowShader;
			shadow.entry.drawData->texture[0] = font->getTexture(this->getScaledFontSize());
			shadow.entry.drawData->drawType = DrawTriangles;
		}
	}

	void UITextElement::batch(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{
		BASECLASS::batch(display_list, numVertices, numIndices, data, info);
		if (this->textPassMask.test(data.pass))
		{
			this->batchText(display_list, numVertices, numIndices, data, info);
		}
	}

	void UITextElement::batchText(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{

#if defined(_DEBUG)
		if (this->text == gDebugTextBreakHere)
		{
			log::info("Break Here!");
		}
#endif

		if (!this->font)
		{
			log::error("No font to render text!");
			return;
		}
			
		if (this->text.length() < 0)
			return;

		RectF bounds = this->getScreenRect(data.bounds);
		if (this->textDirty || this->forceTextDirty)
		{
			this->refreshTextVertices(bounds, data);
			this->refreshShadowVertices();
		}

		std::vector<std::pair<std::string, BatchDrawDataPtr>> drawInstances;
		if (this->shadowList.size() > 0)
		{
			for (auto it : this->shadowList)
			{
				if (it.entry.tint.a > 0)
				{
					drawInstances.push_back({ this->getName() + "_text_shadow", it.entry.drawData });
				}
			}
		}

		if (this->textEntry.tint.a > 0)
		{
			drawInstances.push_back({ this->getName() + "_text", this->textEntry.drawData });
		}

		PointF pos_offset = bounds.pos;
		
		float32 offset_z = kDepthIncrement;

		for (auto it : drawInstances)
		{
			float32 adj_depth = data.depth + this->depthBias + offset_z;

			info.updateAndEvaluate(adj_depth);

			display_list.push_back(BatchDrawParams(it.second, numVertices));
			BatchDrawParams& params = display_list.back();

			vec3 offset(
				pos_offset.x + this->textVerts.xOffset,
				pos_offset.y + this->textVerts.yOffset,
				adj_depth);

			offset_z += 0.01f;

			params.tag = it.first;
			params.bounds = bounds;
			params.transform = glm::translate(offset) * glm::rotate(degreesToRadians(this->textAngle), kDefalutZAxis);
			params.depth = false;
			params.tint = this->elementColor * data.tint;

			numVertices += static_cast<uint32>(it.second->positions.size());
			numIndices += static_cast<uint16>(it.second->indices.size());
		}
	}

	void UITextElement::setTextDirty()
	{
		this->textDirty = true;
	}

	void UITextElement::init()
	{
		this->textPassMask.set(UIBatchPassMain);
		this->font = FontManager::getInstance()->getFont(kDefaultFontName);
	}

	void UITextElement::setText(const std::string& txt) 
	{ 

		if (this->text != txt)
		{
			this->text = txt;
			this->setTextDirty();
		}
	}

	void UITextElement::setDirty()
	{
		BASECLASS::setDirty();
		this->setTextDirty();
	}

	void UITextElement::setTextVAlign(VAlign align) 
	{ 
		if (this->textVAlign != align)
		{
			this->textVAlign = align;
			this->setTextDirty();
		}
	}

	void UITextElement::setTextHAlign(HAlign align) 
	{ 
		if (this->textHAlign != align)
		{
			this->textHAlign = align;
			this->setTextDirty();
		}
	}

	void UITextElement::setTextScale(float32 ts)
	{
		if (this->textScale != ts)
		{
			this->textScale = ts;
			this->setTextDirty();
		}
	}

	void UITextElement::setTextAngle(float32 angle)
	{
		if (this->textAngle != angle)
		{
			this->textAngle = angle;
			this->setTextDirty(); 
		}
	}

	void UITextElement::setFont(const FontPtr& ft) 
	{ 
		if (!this->font.get() || this->font.get() != ft.get())
		{
			this->font = ft;
			this->setTextDirty();
		}
	}

	void UITextElement::setFont(const std::string& str)
	{
		FontPtr f = FontManager::getInstance()->getFont(str);
		if (f.get())
		{
			this->setFont(f);
		}
	}

	void UITextElement::setFontAlpha(float32 alpha)
	{
		this->fontColor.a = static_cast<uint8>(alpha * 255.0f);
		this->setTextDirty();
	}
	
	void UITextElement::setFontColor(const ColorB& color) 
	{ 
		this->fontColor = color; 
		this->setTextDirty();
	}
	
	void UITextElement::setFontSize(uint16 fz) 
	{ 
		assert(fz < 120);
		this->fontSize = fz; 
		this->setTextDirty();
	}

	void UITextElement::setShadowAnimation(float32 anim)
	{
		this->shadowAnim = anim;
		this->setTextDirty();
	}

	void UITextElement::setFontShader(const std::string& shader)
	{
		ShaderResourcePtr shaderResource = std::static_pointer_cast<ShaderResource>(
			ResourceFactory::getInstance()->loadResource<ShaderResource>(shader));
		if (shaderResource.get())
		{
			ShaderHandlePtr shaderHandle = CREATE_CLASS(ShaderHandle, shaderResource);
			this->fontShader = shaderHandle;
		}
		
	}

	void UITextElement::setTextWrap(bool wrap)
	{
		this->textWrap = wrap;
		this->setTextDirty();
	}

	void UITextElement::addTextShadow(vec2 offset, ColorB color)
	{
		this->shadowList.push_back(TextShadow());
		TextShadow& shadow = this->shadowList.back();
        shadow.color = color;
		shadow.offset = offset;
	}

	void UITextElement::clearTextShadows()
	{
		this->shadowList.clear();
	}

	void UITextElement::setFontOutline(float32 amount)
	{
		this->fontOutlineWidth = amount;
		if (this->fontOutlineWidth > 0.0f)
		{
			this->fontShader = CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultFontOutlineShader);
		}
		else
		{
			this->fontShader = CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultFontShader);
		}
	}

	void UITextElement::setFontOutlineColor(const ColorB& color)
	{
		this->fontOutlineColor = color;
	}

}
