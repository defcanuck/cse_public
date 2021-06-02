#pragma once

#include "ClassDef.h"
#include "ui/UIElement.h"

#include "math/Rect.h"
#include "gfx/TextureResource.h"
#include "gfx/TextureHandle.h"
#include "gfx/BatchDraw.h"

#include "imgui.h"

// #define IM_GUI_DISABLED 1

namespace cs
{

	struct GUIContext
	{
		static void setColors();

		static void init();
		static void destroy();

		static void refreshFontAtlas();

		static void beginPrepareGUI(float32 dt);
		static void endPrepareGUI();

		static void setDimensions(const Dimensions& dimm) { guiDimm = dimm; }
		static void draw();

		static void onInputCursor(ClickInput input, ClickParams& params);
		static void onInputZoom(ClickInput input, float32 zoom);
		static void onInputKey(uint32 key, uint32 flags, bool down);
		static void onInputText(char* text);

		static Dimensions guiDimm;
		static TextureResourcePtr guiFontAtlas;
		static TextureHandlePtr guiFontAtlasHandle;
		static intptr_t guiFontAtlasId;

		static std::vector<ImFont*> guiFonts;
		static std::vector<std::pair<int32, ColorB>> styleColors;

		static TextureHandlePtr* pushTexture(TextureHandlePtr textureHandle);

		static bool isContextValid();
		
	private:

		static void pushStyleColors();
		static void popStyleColors();
		
		const static uint32 kMaxVertices;
		const static uint32 kMaxIndices;

		static void drawLists(ImDrawData* draw_data);
		static GeometryPtr geom;

		static const size_t kMaxTextures = 32;
		static TextureHandlePtr textures[kMaxTextures];
		static size_t textureIndex;

	};

	namespace ImGuiLink
	{
		bool Button(const char* label, const ImVec2& size_arg);
	}

	class GUIContextInputClickListener : public Singleton <GUIContextInputClickListener>
	{
	public:
		GUIContextInputClickListener() : consumeInput(false) {}
		~GUIContextInputClickListener() { };
	
		bool onInputCursor(ClickInput input, ClickParams& params)
		{
			GUIContext::onInputCursor(input, params);
			return false;
		}

		bool onInputKey(uint32 key, uint32 flags, bool down)
		{
			GUIContext::onInputKey(key, flags, down);
			return false;
		}

		void onInputZoom(ClickInput input, float32 zoom)
		{
			GUIContext::onInputZoom(input, zoom);
		}

		void onInputText(char* text)
		{
			GUIContext::onInputText(text);
		}

		bool consumeInput;

	};
}