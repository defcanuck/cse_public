#include "PCH.h"

#include "gfx/DrawCall.h"
#include "gfx/RenderInterface.h"

namespace cs
{

	BEGIN_META_CLASS(DrawCallOverrides)
		ADD_MEMBER(textureOverrideHandles);
			ADD_COMBO_META_INTEGER(TextureStageDiffuse, "Diffuse");
	END_META()

	
	DrawCall::DrawCall()
		: tag()
		, type(DrawNone)
		, offset(0)
		, count(0)
		, indexType(TypeNone)
		, blend(true)
		, srcBlend(BlendSrcAlpha)
		, dstBlend(BlendOneMinusSrcAlpha)
		, depthTest(true)
		, depthFunc(DepthLess)
		, color(255, 255, 255, 255)
		, scissor(false)
		, scissorRect()
		, cullFace(CullBack)
		, frontFace(FrontFaceCCW)
		, layer(-1)
		, depthWrite(true)
        , instanceIndex(0)
    { }

	DrawCall::DrawCall(const DrawCall& rhs)
		: tag(rhs.tag)
		, type(rhs.type)
		, offset(rhs.offset)
		, count(rhs.count)
		, shaderHandle(rhs.shaderHandle)
		, textures(rhs.textures)
		, indexType(rhs.indexType)
		, indices(rhs.indices)
		, blend(rhs.blend)
		, srcBlend(rhs.srcBlend)
		, dstBlend(rhs.dstBlend)
		, depthTest(rhs.depthTest)
		, depthFunc(rhs.depthFunc)
		, color(rhs.color)
		, scissor(rhs.scissor)
		, scissorRect(rhs.scissorRect)
		, cullFace(rhs.cullFace)
		, frontFace(rhs.frontFace)
		, layer(-1)
		, depthWrite(rhs.depthWrite)
		, postCallback(rhs.postCallback)
		, preCallback(rhs.preCallback)
		, uniformCallback(rhs.uniformCallback)
        , instanceIndex(rhs.instanceIndex)
	{ }

	void DrawCall::bind(Geometry* geom, DrawCallOverrides* overrides)
	{
		ShaderBindParams bindParams;
		bindParams.geom = geom;
        bindParams.channels = RenderInterface::getInstance()->getCurrentRenderChannels();
        bindParams.depth = RenderInterface::getInstance()->getCurrentDepthChannels();
        
		if (overrides != nullptr)
		{
			ShaderResourcePtr& shaderResource = this->shaderHandle->getShader();
			if (shaderResource.get())
			{
				const std::string shaderName = shaderResource->getName();
				std::map<std::string, ShaderHandlePtr>::iterator it = overrides->shaderOverrideHandles.find(shaderName);
				
				if (it != overrides->shaderOverrideHandles.end())
				{
					it->second->bind(bindParams);
				}
				else
				{
					if (this->shaderHandle)
                    {
                        
                        this->shaderHandle->bind(bindParams);
                    }
				}
			}
		}
		else
		{
			if (this->shaderHandle)
            {
				this->shaderHandle->bind(bindParams);
            }
			else
				log::print(LogError, "No Shader Bound to Draw!");
		}

		for (auto& it : this->textures)
		{
			TextureHandlePtr& texture = it.second;
			if (texture)
			{
				uint32 stage = getPhysicalStage((TextureStage) it.first);
				texture->bind(stage);
			}
		}

		// bind any textures we've overriden from the base
		if (overrides != nullptr)
		{
			for (auto it : overrides->textureOverrideHandles)
			{
				TextureHandlePtr& texture = it.second;
				if (texture)
				{
					uint32 stage = getPhysicalStage((TextureStage)it.first);
					texture->bind(stage);
				}
			}
		}
	}

	void DrawCall::setUniforms(ColorF tint)
	{

		cs::UniformPtr color = SharedUniform::getInstance().getUniform("color");
		if (color.get())
		{
			vec4 newColor = toVec4(toColorF(this->color));
			newColor.x *= tint.r;
			newColor.y *= tint.g;
			newColor.z *= tint.b;
			newColor.w *= tint.a;

			color->setValue(newColor);
		}

		if (this->uniformCallback)
		{
			this->uniformCallback();
		}
	}

    /*
	void DrawCall::execute(std::vector<DrawCallPtr>& drawCalls, DrawCallOverrides* overrides, ColorF tint)
	{
		for (auto& it : drawCalls)
		{
			DrawCallPtr drawCall = it;
			if (drawCall->count == 0)
				continue;

			drawCall->setUniforms(tint);
			drawCall->bind(overrides);

			RenderInterface::getInstance()->draw(drawCall);
		}
	}
    */
}
