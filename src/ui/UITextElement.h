#pragma once

#include "ui/UIElement.h"
#include "font/FontManager.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(UITextElement, UIElement)

		const static uint16 kDefaultFontSize = 32;

	public:
		
		UITextElement(const std::string& name);
		UITextElement(const std::string& name, const std::string& txt);

		virtual void batch(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);

		void setText(const std::string& txt);
		void setTextVAlign(VAlign align);
		void setTextHAlign(HAlign align);
		void setTextScale(float32 ts);
		void setTextWrap(bool wrap);
		void setForceTextDirty(bool forceDirty) { this->forceTextDirty = forceDirty; }

		float32 getTextScale() const { return this->textScale; }

		void setTextAngle(float32 angle);
		float32 getTextAngle() const { return this->textAngle; }

		void setFont(const std::string& str);
		void setFont(const FontPtr& ft);
		void setFontColor(const ColorB& color);
		void setFontAlpha(float32 alpha);
		void setFontSize(uint16 fz);
		void setFontOutline(float32 amount);
		void setFontOutlineColor(const ColorB& color);
		
		void setFontShader(const std::string& name);
		ShaderHandlePtr& getFontShader() { return this->fontShader; }

		float32 getFontAlpha() const { return this->fontColor.a / 255.0f; }
		float32 getShadowAnimation() const { return this->shadowAnim; }

		void setTextHPadding(float32 value, SpanMeasure measure = SpanNone);
		void setTextVPadding(float32 value, SpanMeasure measure = SpanNone);

		void addTextShadow(vec2 offset, ColorB color);
		void setShadowAnimation(float32 anim);
		void clearTextShadows();
        void setFontShadowShader(const std::string& shaderName);

		void setTextRenderOnPass(UIBatchPass pass) { this->textPassMask.set(pass); }
		bool isTextPassSet(UIBatchPass pass) { return this->textPassMask.test(pass); }

		virtual void preload();
    
		static void populateDrawData(const TextLines& lines, BatchDrawDataPtr& drawData, const ColorB& fontColor, const std::vector<float32>* wrapOffsets = nullptr);

	protected:

		virtual void setDirty();

		struct GeneratedText
		{
			TextOptions options;
			TextLines lines;
			RectF textRect;
			std::vector<RectF> lineBounds;
			std::vector<float32> wrapOffsets;

			float32 xOffset;
			float32 yOffset;
			float32 xPadding;
			float32 yPadding;
		};

		struct TextEntry
		{
			TextEntry()
				: drawData(CREATE_CLASS(BatchDrawData))
			{ }
			BatchDrawDataPtr drawData;
			ColorB tint;
		};

		struct TextShadow
		{
			ColorB color;
			vec2 offset;
			TextEntry entry;
		};

		void init();

		void refreshTextVertices(const RectF& bounds, const UIBatchProcessData& data);
		void refreshShadowVertices();

		uint32 getScaledFontSize();

		void batchText(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);

		HAlign textHAlign;
		VAlign textVAlign;

		UIDimension textHPadding;
		UIDimension textVPadding;

		ColorB fontColor;

		ShaderHandlePtr fontShader;
		ShaderHandlePtr fontShadowShader;
		FontPtr font;
		uint16 fontSize;
		std::string text;
		float32 textScale;
		float32 textAngle;
		bool textWrap;

		float32 fontOutlineWidth;
		ColorB fontOutlineColor;

		UIBatchPassMask textPassMask;

	private:

		void setTextDirty();

		typedef std::vector<TextShadow> TextShadowList;
		TextShadowList shadowList;
		float32 shadowAnim;

		GeneratedText textVerts;
		TextEntry textEntry;
		bool textDirty;
		bool forceTextDirty;

	};
}
