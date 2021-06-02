#include "PCH.h"

#include "ui/imgui/GUIContext.h"
#include "gfx/RenderInterface.h"
#include "math/GLM.h"
#include "os/InputManager.h"

#include <SDL.h>

namespace cs
{
	Dimensions GUIContext::guiDimm;
	
	TextureResourcePtr GUIContext::guiFontAtlas = nullptr;
	TextureHandlePtr GUIContext::guiFontAtlasHandle = nullptr;

	std::vector<ImFont*> GUIContext::guiFonts;
	intptr_t GUIContext::guiFontAtlasId = -1;

	const uint32 GUIContext::kMaxVertices = 20000;
	const uint32 GUIContext::kMaxIndices = 60000;
	GeometryPtr GUIContext::geom = nullptr;

	TextureHandlePtr GUIContext::textures[GUIContext::kMaxTextures];
	size_t GUIContext::textureIndex = 0;

	std::vector<std::pair<int32, ColorB>> GUIContext::styleColors;

	static bool gPushGUIFrame = false;
	static bool gHasGUIFrame = false;
	static bool gGuiInitialized = false;

	void GUIContext::drawLists(ImDrawData* drawData)
	{

#if defined(IM_GUI_DISABLED)
		return;
#endif


		if (!gGuiInitialized || !drawData->Valid)
			return;

		
		assert(drawData->TotalVtxCount <= kMaxVertices);
		assert(drawData->TotalIdxCount <= kMaxIndices);

		BufferObjectPtr& vbo = GUIContext::geom->getVertexBuffer();
		BufferObjectPtr& ibo = GUIContext::geom->getIndexBuffer();

		GUIContext::geom->bindVertices();
		GUIContext::geom->bindIndices();

		ImDrawVert* vbo_bytes = (ImDrawVert*) vbo->lock(BufferAccessWrite);
		ImDrawIdx* ibo_bytes = (ImDrawIdx*) ibo->lock(BufferAccessWrite);

		size_t numVertices = 0;
		size_t numIndices = 0;

		std::vector<DrawCallPtr> drawCalls;

		for (int32 i = 0; i < drawData->CmdListsCount; ++i)
		{
			ImDrawList* drawListCmd = drawData->CmdLists[i];
			assert(drawListCmd);

			ImDrawVert* drawListVertex = &drawListCmd->VtxBuffer.front();
			ImDrawIdx* drawListIndex = &drawListCmd->IdxBuffer.front();

			memcpy(vbo_bytes, drawListVertex, drawListCmd->VtxBuffer.size() * sizeof(ImDrawVert));
			memcpy(ibo_bytes, drawListIndex, drawListCmd->IdxBuffer.size() * sizeof(ImDrawIdx));
			
			if (i > 0)
			{
				for (int32 idx_off = 0; idx_off < drawListCmd->IdxBuffer.size(); idx_off++)
				{
					ImDrawIdx* index = (ImDrawIdx*)(ibo_bytes) + idx_off;
					*index = *index + static_cast<unsigned short>(numVertices);
				}
			}
			
			size_t cmdIndexCount = 0;
			for (int32 cmd = 0; cmd < drawListCmd->CmdBuffer.size(); ++cmd)
			{

				const ImDrawCmd* drawCmd = &drawListCmd->CmdBuffer[cmd];
				if (drawCmd->UserCallback)
				{
					drawCmd->UserCallback(drawListCmd, drawCmd);
				}
				else
				{

					TextureHandlePtr* texId = (TextureHandlePtr*) drawCmd->TextureId;
					bool isFont = texId == &GUIContext::guiFontAtlasHandle;
					bool isTexture = !isFont && texId != nullptr;
					
					DrawCallPtr dc = CREATE_CLASS(DrawCall);
					dc->offset = uint32(numIndices + cmdIndexCount);
					dc->type = DrawTriangles;
					dc->indexType = TypeUnsignedShort;
					dc->cullFace = CullNone;

					if (isFont)
					{
						dc->shaderHandle = RenderInterface::kDefaultFontShader;
						dc->textures[0] = GUIContext::guiFontAtlasHandle;
					} 
					else if (isTexture) 
					{
						dc->shaderHandle = RenderInterface::kDefaultTextureShader;
						dc->textures[0] = *texId;
					} 
					else 
					{
						dc->shaderHandle = RenderInterface::kDefaultTextureColorShader;
						dc->textures[0] = RenderInterface::kWhiteTexture;
					}

					
					dc->depthTest = false;
					dc->color = ColorB::White;
					dc->count = drawCmd->ElemCount;

					RectI clipRect;
					clipRect.pos.x = (int32)drawCmd->ClipRect.x;
					clipRect.pos.y = (int32) (static_cast<float32>(GUIContext::guiDimm.w) - drawCmd->ClipRect.w);
					clipRect.size.w = (int32)(drawCmd->ClipRect.z - drawCmd->ClipRect.x),
					clipRect.size.h = (int32)(drawCmd->ClipRect.w - drawCmd->ClipRect.y);

					//dc->scissor = true;
					//dc->scissorRect = clipRect;

					drawCalls.push_back(dc);
					cmdIndexCount += drawCmd->ElemCount;

				}
			}

			numIndices += drawListCmd->IdxBuffer.size();
			ibo_bytes += drawListCmd->IdxBuffer.size();

			vbo_bytes += drawListCmd->VtxBuffer.size();
			numVertices += drawListCmd->VtxBuffer.size();
		}


		vbo->unlock();
		ibo->unlock();

		RenderInterface::getInstance()->clearBuffer(BufferTypeIndex);
		RenderInterface::getInstance()->clearBuffer(BufferTypeVertex);

		GUIContext::geom->bindAll();

		RectI screen_rect;
		screen_rect.size.w = GUIContext::guiDimm.w;
		screen_rect.size.h = GUIContext::guiDimm.h;

		mat4 projection = glm::ortho(
			float32(screen_rect.pos.x), float32(screen_rect.pos.x + screen_rect.size.w),
			float32(screen_rect.size.h - screen_rect.pos.y), float32(screen_rect.pos.y),
			-1.0f, 1.0f);

		mat4 mvp = projection;

		cs::UniformPtr matrix = SharedUniform::getInstance().getUniform("mvp");
		assert(matrix);
		matrix->setValue(mvp);

		GUIContext::geom->draw(drawCalls, nullptr);
	}

	void GUIContext::refreshFontAtlas()
	{

#if defined(IM_GUI_DISABLED)
	return;
#endif

		ImGuiIO& io = ImGui::GetIO();
		uchar* pixels;
		int32 w, h;
		io.Fonts->GetTexDataAsAlpha8(&pixels, &w, &h);
		Dimensions dimm(w, h);

		GUIContext::guiFontAtlas = RenderInterface::getInstance()->loadTexture(
			dimm, TextureAlpha, pixels);

		
	
		TexturePtr guiAtlasTexture = CREATE_CLASS(Texture, "GuiFontAtlas", GUIContext::guiFontAtlas);
		GUIContext::guiFontAtlasHandle = 
			CREATE_CLASS(cs::TextureHandle, guiAtlasTexture);

		GUIContext::guiFontAtlasId = GUIContext::guiFontAtlas->getTextureId();
		ImGui::GetIO().Fonts->TexID = (void *)(intptr_t) &GUIContext::guiFontAtlasHandle;
	}

	void testImGui();

	static const char* ImGui_ImplSdl_GetClipboardText()
	{
		return SDL_GetClipboardText();
	}

	static void ImGui_ImplSdl_SetClipboardText(const char* text)
	{
		SDL_SetClipboardText(text);
	}

	void GUIContext::init()
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		ImGuiIO& io = ImGui::GetIO();

		struct local
		{
			
		};
		
		io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
		io.KeyMap[ImGuiKey_A] = SDLK_a;
		io.KeyMap[ImGuiKey_C] = SDLK_c;
		io.KeyMap[ImGuiKey_V] = SDLK_v;
		io.KeyMap[ImGuiKey_X] = SDLK_x;
		io.KeyMap[ImGuiKey_Y] = SDLK_y;
		io.KeyMap[ImGuiKey_Z] = SDLK_z;

		// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
		io.RenderDrawListsFn = &GUIContext::drawLists;

		io.SetClipboardTextFn = ImGui_ImplSdl_SetClipboardText;
		io.GetClipboardTextFn = ImGui_ImplSdl_GetClipboardText;

		io.Fonts->AddFontDefault();
		GUIContext::refreshFontAtlas();

		GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);

		data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 2, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec2) });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeUnsignedByte, 4, sizeof(vec2) + sizeof(vec2) });

		data->vertexSize = kMaxVertices;
		data->indexSize = kMaxIndices;
		data->storage = BufferStorageDynamic;

		InputManager::getInstance()->addClickListener(GUIContextInputClickListener::getInstance(), &GUIContextInputClickListener::onInputCursor, 600, "GUI");
		InputManager::getInstance()->addKeyListener(GUIContextInputClickListener::getInstance(), &GUIContextInputClickListener::onInputKey, 200, "GUI");
		InputManager::getInstance()->addTextListener(GUIContextInputClickListener::getInstance(), &GUIContextInputClickListener::onInputText);

		GUIContext::geom = CREATE_CLASS(Geometry, data);

		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_Border, ColorB(0, 0, 0, 255)));

		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_Text, ColorB(255, 255, 255, 255)));
		
		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_WindowBg, ColorB(32, 32, 32, 255)));
		
		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_Button, ColorB(0, 128, 255, 255)));
		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_ButtonHovered, ColorB(0, 64, 255, 255)));
		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_ButtonActive, ColorB(0, 0, 255, 255)));

		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_TitleBg, ColorB(0, 0, 0, 255)));
		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_TitleBgActive, ColorB(0, 0, 0, 255)));
		GUIContext::styleColors.push_back(std::pair<int32, ColorB>(ImGuiCol_TitleBgCollapsed, ColorB(16, 16, 16, 255)));	

		gGuiInitialized = true;
	}

	void GUIContext::onInputKey(uint32 key, uint32 flags, bool down)
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[key] = down;
		io.KeyShift = ((flags & InputModifierShift) != 0);
		io.KeyCtrl = ((flags & InputModiferControl) != 0);
		io.KeyAlt = ((flags & InputModifierAlt) != 0);
	}

	void GUIContext::pushStyleColors()
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		struct local
		{
			static ImVec4 convert(const ColorB& col)
			{
				ImVec4 imcol;
				imcol.x = col.r / 255.0f;
				imcol.y = col.g / 255.0f;
				imcol.z = col.b / 255.0f;
				imcol.w = col.a / 255.0f;
				return imcol;
			}
		};

		for (const auto it : GUIContext::styleColors)
		{
			const std::pair<int32, ColorB>& color = it;
			ImGui::PushStyleColor((ImGuiCol) color.first, local::convert(color.second));
		}
	}

	void GUIContext::popStyleColors()
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		ImGui::PopStyleColor(int(GUIContext::styleColors.size()));
	}

	void GUIContext::destroy()
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->ClearInputData();
		io.Fonts->ClearTexData();

		ImGui::Shutdown();
	}

	bool GUIContext::isContextValid()
	{
		return gGuiInitialized && gPushGUIFrame;
	}

	void GUIContext::beginPrepareGUI(float32 dt)
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		if (!gGuiInitialized)
		{
			return;
		}

		if (dt > 1.0f)
		{
			log::info("ImGui - Ignore large delta");
			return;
		}

		gPushGUIFrame = true;

		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;
		io.DisplaySize = ImVec2((float32) guiDimm.w, (float32) guiDimm.h);

		ImGui::NewFrame();

		GUIContext::pushStyleColors();
		testImGui();
		
	}

	void GUIContext::endPrepareGUI()
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		if (!gGuiInitialized)
		{
			return;
		}

		if (!gPushGUIFrame)
			return;

		gPushGUIFrame = false;
		gHasGUIFrame = true;
		GUIContext::popStyleColors();
	}
	
	void GUIContext::draw()
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif

		if (!gGuiInitialized || !gHasGUIFrame)
		{
			return;
		}

		RectI viewport(0, 0, guiDimm.w, guiDimm.h);
		RenderInterface::getInstance()->setViewport(viewport);
		ImGui::Render();

		gHasGUIFrame = false;

		GUIContext::textureIndex = 0;
	}

	void GUIContext::onInputCursor(ClickInput input, ClickParams& params)
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif
		if (!gGuiInitialized)
		{
			return;
		}

		if (input == ClickNone)
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		float32 x = GUIContext::guiDimm.x * params.position.x;
		float32 y = GUIContext::guiDimm.y - GUIContext::guiDimm.y * params.position.y;

		// Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		io.MousePos = ImVec2(x, y);
		
		//log::print(LogInfo, io.MousePos.x, " ", io.MousePos.y);
		
		io.MouseDown[(uint32)input] = params.state == TouchDown || params.state == TouchPressed;
		io.MouseClicked[(uint32)input] = params.state == TouchPressed;
				
	}

	void GUIContext::onInputZoom(ClickInput input, float32 zoom)
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif
		if (!gGuiInitialized)
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel = zoom;
	}

	void GUIContext::onInputText(char* text)
	{
#if defined(IM_GUI_DISABLED)
		return;
#endif
		if (!gGuiInitialized)
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharactersUTF8(text);
	}

	TextureHandlePtr* GUIContext::pushTexture(TextureHandlePtr textureHandle)
	{
#if defined(IM_GUI_DISABLED)
		return nullptr;
#endif

		assert(GUIContext::textureIndex < GUIContext::kMaxTextures);
		GUIContext::textures[GUIContext::textureIndex++] = textureHandle;
		return &GUIContext::textures[GUIContext::textureIndex - 1];
	}

	void testImGui()
	{

		return;

		bool showTestWindow = false;
		static bool showAnotherWindow = true;
		static float f = 0.0f;
		{
			/*
			ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Debug");

			ColorF color(0.25f, 0.25f, 0.25f, 0.5f);
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&color);

			if (ImGui::Button("Test Window"))
				showTestWindow ^= 1;
			if (ImGui::Button("Another Window"))
				showAnotherWindow ^= 1;

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
			*/
		}

		// 2. Show another simple window, this time using an explicit Begin/End pair
		
		if (showAnotherWindow)
		{
		
			ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);
			bool collapsed = ImGui::Begin("Another Window", &showAnotherWindow, ImGuiWindowFlags_NoResize);

			ImGui::Text("Hello, world!");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			if (ImGui::Button("Test Window"))
				log::print(LogInfo, "HELLO BUTTON!");
			ImGui::End();

			if (!collapsed)
				log::print(LogError, "Collapsed!");

			if (!showAnotherWindow)
				log::print(LogError, "Close!");
		}

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		if (showTestWindow)
		{
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
			ImGui::ShowTestWindow(&showAnotherWindow);
		}
	}

	namespace ImGuiLink
	{
		bool Button(const char* label, const ImVec2& size_arg)
		{
			if (ImGui::Button(label, size_arg))
			{
				GUIContextInputClickListener::getInstance()->consumeInput = true;
				return true;
			}

			return false;
		}
	}


}