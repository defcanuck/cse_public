#include "PCH.h"

#include "ui/UIGlobal.h"

#if defined(CS_WINDOWS)
    #include "ui/imgui/GUIMeta.h"
#endif

namespace cs
{
	ShaderHandlePtr defaultShaderResource = nullptr;
	ShaderHandlePtr defaultFontShaderResource = nullptr;
	ShaderHandlePtr defaultMaskResource = nullptr;

	float gResolutionWidthScale = 1.0f;
	float gResolutionHeightScale = 1.0f;

	void initUIGlobals()
	{
        
        if (!ShaderCompile::shouldCompileShaders(ShaderFlagsUI))
            return;
        
		ShaderParams paramsUI;
		paramsUI.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
			"attribute highp vec4 pos;\n"
			"attribute highp vec2 tex0;\n"
            "attribute highp vec2 tex1;\n"
			"attribute highp vec4 col;\n"
			"varying highp vec4 vtex0;\n"
            "varying highp vec4 vtex1;\n"
			"varying highp vec4 vcol;\n"
			"uniform highp mat4 mvp;\n"
			
			"void main(void)\n"
			"{\n"
			"    gl_Position = mvp * pos;\n"
			"    vtex0 = vec4(tex0, 1.0, 1.0);\n"
            "    vtex1 = vec4(tex1, 1.0, 1.0);\n"
			"	 vcol = col;\n"
			"}\n"));

		paramsUI.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
			"uniform sampler2D texture0;\n"
			"varying highp vec4 vtex0;\n"
            "varying highp vec4 vcol;\n"
            "uniform highp vec4 color;\n"
            //"INSERT_COMMON_FUNCS\n"
			"void main(void)\n"
			"{\n"
			"    gl_FragColor = (vcol * color * texture2D(texture0, vtex0.st));\n"
			"}\n"));

		paramsUI.attributes[AttribPosition] = "pos";
		paramsUI.attributes[AttribTexCoord0] = "tex0";
        paramsUI.attributes[AttribTexCoord1] = "tex1";
		paramsUI.attributes[AttribColor] = "col";

		paramsUI.addUniform(SharedUniform::getInstance().getUniform("mvp"));
		paramsUI.addUniform(SharedUniform::getInstance().getUniform("color"));

		paramsUI.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));

		defaultShaderResource = CREATE_CLASS(ShaderHandle, CREATE_CLASS(ShaderResource, "defaultUI", ShaderBucketUI, paramsUI));
        defaultFontShaderResource = CREATE_CLASS(ShaderHandle, CREATE_CLASS(ShaderResource, "defaultFontUI", ShaderBucketUI, paramsUI));
        
		// Texture 2
		{
			ShaderParams params;
			params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"attribute highp vec4 pos;\n"
				"attribute highp vec4 tex0;\n"
				"attribute highp vec4 tex1;\n"
				"attribute highp vec4 col;\n"
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vtex1;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp mat4 mvp;\n"
				"void main(void)\n"
				"{\n"
				"    gl_Position = mvp * pos;\n"
				"    vtex0 = tex0;\n"
				"    vtex1 = tex1;\n"
				"	 vcol = col;\n"
				"}\n"));

			params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"uniform sampler2D texture0;\n"
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vtex1;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp vec4 color;\n"
				"void main(void)\n"
				"{\n"
				"     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
				"	  if ((diff.a * vcol.a * color.a) <= 0.0)\n"
				"        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
				"   else\n"
				"      discard;\n"
				"}\n"));

			params.attributes[AttribPosition] = "pos";
			params.attributes[AttribTexCoord0] = "tex0";
			params.attributes[AttribTexCoord1] = "tex1";
			params.attributes[AttribColor] = "col";

			params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
			params.addUniform(SharedUniform::getInstance().getUniform("color"));

			params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
			
			ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultTex2", ShaderBucketGeometry, params);
			shader->setFlushable(false);

			defaultMaskResource = CREATE_CLASS(ShaderHandle, CREATE_CLASS(ShaderResource, "defaultMask", ShaderBucketUI, params));
		}

#if defined(CS_WINDOWS)
		GUICreatorFactory::initCreators();
#endif

	}
}
