#include "PCH.h"

#include "gfx/Renderer.h"

namespace cs
{
	namespace renderer
	{
		void draw(const mat4& mvp, const ColorB& global_color, GeometryPtr& geom)
		{

			cs::UniformPtr matrix = SharedUniform::getInstance().getUniform("mvp");
			assert(matrix);
			matrix->setValue(mvp);

			geom->draw(nullptr, -1, toColorF(global_color));
		}

		DrawCallPtr createDraw(const std::string& tag, GeometryPtr& geom, TextureHandlePtr& texture, ShaderHandlePtr& shader)
		{
			DrawCallPtr draw = CREATE_CLASS(DrawCall);

			BufferObjectPtr& indexBuffer = geom->getIndexBuffer();
			if (indexBuffer.get())
			{
				draw->count = indexBuffer->getSize() / size_t(getTypeSize(TypeUnsignedShort));
				draw->offset = 0;
				draw->indexType = TypeUnsignedShort;
			}

			draw->tag = tag;
			draw->type = DrawTriangles;
			draw->shaderHandle = CREATE_CLASS(ShaderHandle, shader);
			draw->textures[0] = CREATE_CLASS(TextureHandle, texture);
			draw->color = ColorB::White;

			return draw;
		}

		void pushRenderTarget(const std::string& name, RenderTargetCapture& capture, const RectI& viewport, RenderTargetType rtt_type)
		{
			RenderInterface* render_interface = RenderInterface::getInstance();

			std::vector<ClearMode> clearmode = { ClearDepth, ClearColor };
			const ColorF kClearColor = ColorF::Clear;

			if (!capture.rtt)
			{
				uint32 adjW;
				uint32 adjH;

				if (rtt_type == RenderTargetTypeNone)
				{
					adjW = std::max<uint32>(32, nextPow2(viewport.size.w));
					adjH = std::max<uint32>(32, nextPow2(viewport.size.h));
					capture.rtt = RenderTargetManager::getInstance()->populateSharedTarget(name, adjW, adjH, TextureRGBAFloat, RenderInterface::kDefaultDepthComponent);
				}
				else
				{
					capture.rtt = RenderTargetManager::getInstance()->getTarget(rtt_type);
					const Dimensions& dimm = capture.rtt->getDimensions();
					adjW = dimm.w;
					adjH = dimm.h;
				}

				PostProcessParams resolveToFrontParams;
				resolveToFrontParams.shader = RenderInterface::kDefaultTextureShader;
				resolveToFrontParams.viewport = viewport;
				resolveToFrontParams.inputMap.push_back(RenderTextureInputMapping(RenderTargetManager::kUseBackBuffer, TextureStageDiffuse));
				resolveToFrontParams.clearModes = { ClearColor, ClearDepth };
				resolveToFrontParams.clearColor = ColorF(1.0f, 1.0f, 0.0f, 1.0f);
				resolveToFrontParams.isDynamic = true;

				TextureHandlePtr sampleTexture = CREATE_CLASS(TextureHandle, capture.rtt->getTexture());
				//TextureHandlePtr sampleTexture = CREATE_CLASS(TextureHandle, "test_texture.png");

				capture.fillRect.pos.x = (float32)viewport.pos.x;
				capture.fillRect.pos.y = (float32)viewport.pos.y;
				capture.fillRect.size.w = (float32)viewport.size.w / (float32)adjW;
				capture.fillRect.size.h = (float32)viewport.size.h / (float32)adjH;

				sampleTexture->setUVRect(capture.fillRect);
				resolveToFrontParams.fillRect = capture.fillRect;

#if defined(CS_METAL)
				resolveToFrontParams.fillRect.flipVertical();
#endif
				capture.resolveToFront = CREATE_CLASS(PostProcess, "ResolveToFront", resolveToFrontParams);
				capture.resolveToFront->mapInputs(RenderTargetManager::getInstance()->renderTargets);
				capture.resolveToFront->setTexture(sampleTexture, TextureStageDiffuse);

				DrawCallPtr& resolveDraw = capture.resolveToFront->getResolveDraw();
				resolveDraw->dstBlend = BlendOneMinusSrcAlpha;
				resolveDraw->srcBlend = BlendSrcAlpha;
			}


			capture.rtt->bind(false, &clearmode);
			render_interface->pushDebugScope("UI");

#if !defined(CS_METAL)
			render_interface->setClearColor(kClearColor);
			render_interface->clear(clearmode);
#endif
		}

		void popRenderTarget(RenderTargetCapture& capture, const RectI& viewport)
		{
			RenderInterface* render_interface = RenderInterface::getInstance();

			capture.rtt->unbind();
			render_interface->getInstance()->popDebugScope();

			if (capture.resolveToFront.get())
			{
				RenderInterface::getInstance()->setDefaultFrameBuffer();
				capture.resolveToFront->draw(viewport);
			}
		}
	}
}
