
#include "ClassDef.h"
#include "gfx/ShaderUtils.h"
#include "gfx/ShaderResource.h"
#include "gfx/ShaderHandle.h"

#include "gfx/RenderInterface.h"
#include "gfx/Mesh.h"

#include "os/FileManager.h"
#include "global/ResourceFactory.h"

namespace cs
{

    const char* kRemoveWords[] =
    {
        "lowp",
        "mediump",
        "highp"
    };
    
    const size_t kNumRemoveWords = sizeof(kRemoveWords) / sizeof(kRemoveWords[0]);
    
    void ShaderUtils::replaceOutputFormat(std::string& str)
    {
        const std::string kReplaceText(OUTPUT_FORMAT);
        
        while (true)
        {
            size_t pos = str.find(kReplaceText);
            if (pos != std::string::npos)
            {
                const std::string kFuncReplaceText(OUTPUT_FORMAT_STR);
                str.replace(pos, kReplaceText.length(), kFuncReplaceText);
            }
            else
            {
                break;
            }
        }
    }

    void ShaderUtils::removePrecisionQualifiers(std::string& str)
    {
        for (size_t i = 0; i < kNumRemoveWords; i++)
            removeSubstrs(str, std::string(kRemoveWords[i]));
    }
    
    void ShaderUtils::addDefaultShaders()
    {
      
        // MetalTest
        if (ShaderCompile::shouldCompileShaders(ShaderFlagsTest))
        {
            {
                ShaderParams params;
                params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                    "attribute highp vec4 pos;\n"
                    "attribute highp vec4 tex0;\n"
                    "attribute highp vec4 col;\n"
                    "varying highp vec4 vcol;\n"
                    "varying highp vec4 vtex0;\n"
                    "uniform highp mat4 mvp;\n"
                    "void main(void)\n"
                    "{\n"
                    "    gl_Position = mvp * pos;\n"
                    "    vtex0 = tex0;\n"
                    "    vcol = col;\n"
                    "}\n"));
                params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                    "varying highp vec4 vcol;\n"
                    "varying highp vec4 vtex0;\n"
                    "uniform sampler2D texture1;\n"
                    "uniform sampler2D texture0;\n"
                    "uniform highp vec4 color1;"
                    "uniform highp vec4 color2;"
                    "void main(void)\n"
                    "{\n"
                    "    highp vec4 tex0 = texture2D(texture0, vtex0.st);\n"
                    "    highp vec4 tex1 = texture2D(texture1, vtex0.st);\n"
                    "    gl_FragColor = vec4(tex0.rgb * color1.rgb + tex1.rgb * color2.rgb, 1.0);\n"
                    "}\n"));

                params.attributes[AttribPosition] = "pos";
                
                const vec4 kColor1(1.0f, 0.0f, 0.0f, 1.0f);
                const vec4 kColor2(0.0f, 1.0f, 0.0f, 1.0f);
                
                params.addUniform(CREATE_CLASS(cs::UniformDataVec4, "color1", kColor1, true, ShaderFragment));
                params.addUniform(CREATE_CLASS(cs::UniformDataVec4, "color2", kColor2, true, ShaderFragment));
                
                params.addUniform(cs::SharedUniform::getInstance().getUniform("mvp"));
                
                params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
                params.addUniform(CREATE_CLASS(UniformDataTexture, "texture1", TextureStageSpecular));

                ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "metalTest", ShaderBucketGeometry, params);
                shader->setFlushable(false);
                
                RenderInterface::kMetalTest = CREATE_CLASS(ShaderHandle, shader);
                ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            }
            
            // Solid Color
            {
                ShaderParams params;
                params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                   "attribute highp vec4 pos;\n"
                   "uniform highp mat4 mvp;\n"
                   "void main(void)\n"
                   "{\n"
                   "    gl_Position = mvp * pos;\n"
                   "}\n"));
                params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                   "void main(void)\n"
                   "{\n"
                   "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                   "}\n"));

                params.attributes[AttribPosition] = "pos";

                params.addUniform(SharedUniform::getInstance().getUniform("mvp"));

                ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "solidColor", ShaderBucketGeometry, params);
                shader->setFlushable(false);

                RenderInterface::kSolidColorTest = CREATE_CLASS(ShaderHandle, shader);
                ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            }
        }
        
        // Texture
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "}\n"));
            
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp vec4 color;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
                "     if (diff.a <= 0.00001) discard;\n"
                "    gl_FragColor = color * diff;\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            params.addUniform(SharedUniform::getInstance().getUniform("viewport_inv"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultTex", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultTextureShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Particle
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "attribute highp vec4 col;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vcol;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "    vcol = col;\n"
                "}\n"));
            
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vcol;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
                "     if (diff.a <= 0.001) discard;\n"
                "     gl_FragColor = diff * vcol;\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            //params.addUniform(SharedUniform::getInstance().getUniform("viewport_inv"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "particle", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultParticleShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Texture Screen
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp mat4 mvp;\n"
                "uniform highp vec2 viewport_inv;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = vec4(tex0.x, tex0.y, (gl_Position.x + 1.0) * 0.5, 1.0 - ((gl_Position.y + 1.0) * 0.5));\n"
                "}\n"));
            
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp vec4 color;\n"
                "uniform highp vec2 scale;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 diff = texture2D(texture0, vtex0.zw * scale);\n"
                "     if (diff.a <= 0.00001) discard;\n"
                "    gl_FragColor = color * diff;\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            params.addUniform(CREATE_CLASS(cs::UniformDataVec2, "scale", vec2(1.0f, 1.0f), true, ShaderFragment));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultTexScreen", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Texture 2
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "attribute highp vec4 tex1;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vtex1;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "     vtex1 = tex1;\n"
                "}\n"));
            
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "uniform sampler2D texture1;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vtex1;\n"
                "uniform highp vec4 color;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
                "    if (diff.a <= 0.0)\n"
                "      gl_FragColor = color * (diff + texture2D(texture1, vtex1.st));\n"
                "   else\n"
                "      discard;\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribTexCoord1] = "tex1";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture1", TextureStageSpecular));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultTex2", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultTextureShader2 = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Color
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform highp vec4 color;\n"
                "void main(void)\n"
                "{\n"
                "    gl_FragColor = color;\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultCol", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultColorShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
        }

		// vcol
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
				"    vcol = col;\n"
				"}\n"));

			params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vtex1;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp vec4 color;\n"
				"void main(void)\n"
				"{\n"
				"    if (vcol.a <= 0.0)\n"
				"      gl_FragColor = color * vcol;\n"
				"   else\n"
				"      discard;\n"
				"}\n"));

			params.attributes[AttribPosition] = "pos";
			params.attributes[AttribTexCoord0] = "tex0";
			params.attributes[AttribTexCoord1] = "tex1";
			params.attributes[AttribColor] = "col";

			params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
			params.addUniform(SharedUniform::getInstance().getUniform("color"));

			ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultVcol", ShaderBucketGeometry, params);
			shader->setFlushable(false);

			RenderInterface::kDefaultTextureShader2 = CREATE_CLASS(ShaderHandle, shader);
			ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
		}
        
        // Font Atlas
        if (ShaderCompile::shouldCompileShaders(ShaderFlagsFont))
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "void main(void)\n"
                "{\n"
                "    highp float c = texture2D(texture0, vtex0.st).a;\n"
                "    gl_FragColor = vec4(c, c, c, c);\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "fontAtlas", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kFontAtkasShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Font
        if (ShaderCompile::shouldCompileShaders(ShaderFlagsFont))
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
                "    vcol = col;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vtex1;\n"
                "varying highp vec4 vcol;\n"
                "uniform highp vec4 color;\n"
                "uniform highp vec4 padding4;\n"
                "void main(void)\n"
                "{\n"
                "    highp float hp = texture2D(texture0, vtex0.st).a;\n"
                "   gl_FragColor = vec4(color.rgb * vcol.rgb * color.rgb * hp, vcol.a * color.a * hp);\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribTexCoord1] = "tex1";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            params.addUniform(SharedUniform::getInstance().getUniform("padding4")); // 32 bit
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultFont", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultFontShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Font Outline
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "attribute highp vec4 col;\n"
                "varying highp vec4 vcol;\n"
                "varying highp vec2 vtex0;\n"
                "varying highp vec2 vtex1;\n"
                "varying highp vec2 vtex2;\n"
                "varying highp vec2 vtex3;\n"
                "varying highp vec2 vtex4;\n"
                "varying highp vec2 vtex5;\n"
                "varying highp vec2 vtex6;\n"
                "varying highp vec2 vtex7;\n"
                "varying highp vec2 vtex8;\n"
                "uniform highp mat4 mvp;\n"
                "uniform highp vec2 inv_size;\n"
                "uniform highp float range;\n"
                "void main(void)\n"
                "{\n"
                "   gl_Position = mvp * pos;\n"
                "    vcol = col;\n"
                "    vtex0 = tex0.xy;\n"
                "    vtex1 = tex0.xy + (range * vec2(0.0, -inv_size.y));\n"
                "    vtex2 = tex0.xy + (range * vec2(-inv_size.x, 0.0));\n"
                "    vtex3 = tex0.xy + (range * vec2(inv_size.x, 0.0));\n"
                "    vtex4 = tex0.xy + (range * vec2(0.0, inv_size.y));\n"
                "    vtex5 = tex0.xy + (range * vec2(-inv_size.x, -inv_size.y));\n"
                "    vtex6 = tex0.xy + (range * vec2(-inv_size.x, inv_size.y));\n"
                "    vtex7 = tex0.xy + (range * vec2(inv_size.x, -inv_size.y));\n"
                "    vtex8 = tex0.xy + (range * vec2(inv_size.x, inv_size.y));\n"
                "}\n"
                ));
            
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "uniform highp vec4 color;\n"
                "uniform highp float threshold;\n"
                "varying highp vec2 vtex0;\n"
                "varying highp vec2 vtex1;\n"
                "varying highp vec2 vtex2;\n"
                "varying highp vec2 vtex3;\n"
                "varying highp vec2 vtex4;\n"
                "varying highp vec2 vtex5;\n"
                "varying highp vec2 vtex6;\n"
                "varying highp vec2 vtex7;\n"
                "varying highp vec2 vtex8;\n"
                "varying highp vec4 vcol;\n"
                "void main(void)\n"
                "{\n"
                "    highp float depth = abs(-(8.0\n"
                "        * texture2D(texture0, vtex0).r)\n"
                "        + texture2D(texture0, vtex1).r\n"
                "        + texture2D(texture0, vtex2).r\n"
                "        + texture2D(texture0, vtex3).r\n"
                "        + texture2D(texture0, vtex4).r\n"
                "        + texture2D(texture0, vtex5).r\n"
                "        + texture2D(texture0, vtex6).r\n"
                "        + texture2D(texture0, vtex7).r\n"
                "        + texture2D(texture0, vtex8).r\n"
                "    );\n"
                "    if (depth > threshold) gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
                "    else gl_FragColor = vec4(color.rgb * vcol.rgb, vcol.a * texture2D(texture0, vtex0.st).r);\n"
                "}\n"
                ));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            params.addUniform(CREATE_CLASS(UniformDataVec2, "inv_size", vec2(1.0f / 1024.0, 1.0f / 1024.0), true, ShaderVertex));
            params.addUniform(CREATE_CLASS(UniformDataFloat, "range", 1.0f, true, ShaderVertex));
            params.addUniform(CREATE_CLASS(UniformDataFloat, "threshold", 0.1f, true, ShaderFragment));
            
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultFontOutline", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultFontOutlineShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Color Alpha
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "attribute highp vec4 col;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vcol;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "     vcol = col;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp vec4 color;\n"
                "varying highp vec4 vcol;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
                "     if (diff.a <= 0.00001) discard;\n"
                "     gl_FragColor = vec4(color.rgb * vcol.rgb, diff.a * vcol.a * color.a);\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "texColorAlpha", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kTextureAlphaShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }

		// Color Alpha
		{
			ShaderParams params;
			params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"attribute highp vec4 pos;\n"
				"attribute highp vec4 tex0;\n"
				"attribute highp vec4 col;\n"
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp mat4 mvp;\n"
				"void main(void)\n"
				"{\n"
				"    gl_Position = mvp * pos;\n"
				"    vtex0 = tex0;\n"
				"     vcol = col;\n"
				"}\n"));
			params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"uniform sampler2D texture0;\n"
				"varying highp vec4 vtex0;\n"
				"uniform highp vec4 color;\n"
				"varying highp vec4 vcol;\n"
				"void main(void)\n"
				"{\n"
				"     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
				"     if (diff.a <= 0.00001) discard;\n"
				"	  highp float greyscale = (diff.r + diff.g + diff.b) * 0.333333;\n"
				"     gl_FragColor = vec4(vec3(greyscale, greyscale, greyscale) * color.rgb * vcol.rgb, diff.a * vcol.a * color.a);\n"
				"}\n"));

			params.attributes[AttribPosition] = "pos";
			params.attributes[AttribTexCoord0] = "tex0";
			params.attributes[AttribColor] = "col";

			params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
			params.addUniform(SharedUniform::getInstance().getUniform("color"));

			params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));

			ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "texColorSingleChannel", ShaderBucketGeometry, params);
			shader->setFlushable(false);

			RenderInterface::kTextureSingleChannelShader = CREATE_CLASS(ShaderHandle, shader);
			ResourceFactory::getInstance()->addResource<ShaderResource>(shader);

		}

		// Color BW Alpha
		{
			ShaderParams params;
			params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"attribute highp vec4 pos;\n"
				"attribute highp vec4 tex0;\n"
				"attribute highp vec4 col;\n"
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp mat4 mvp;\n"
				"void main(void)\n"
				"{\n"
				"    gl_Position = mvp * pos;\n"
				"    vtex0 = tex0;\n"
				"     vcol = col;\n"
				"}\n"));
			params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"uniform sampler2D texture0;\n"
				"varying highp vec4 vtex0;\n"
				"uniform highp vec4 color;\n"
				"varying highp vec4 vcol;\n"
				"void main(void)\n"
				"{\n"
				"    highp vec4 diff = texture2D(texture0, vtex0.st);\n"
				"    if (diff.a <= 0.00001) discard;\n"
				"	 highp float bw = (diff.r + diff.g + diff.b);\n"
				"    gl_FragColor = vec4(color.rgb * vcol.rgb * bw, diff.a * vcol.a * color.a);\n"
				"}\n"));

			params.attributes[AttribPosition] = "pos";
			params.attributes[AttribTexCoord0] = "tex0";
			params.attributes[AttribColor] = "col";

			params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
			params.addUniform(SharedUniform::getInstance().getUniform("color"));

			params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));

			ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "texColorBWAlpha", ShaderBucketGeometry, params);
			shader->setFlushable(false);

			RenderInterface::kTextureAlphaBWShader = CREATE_CLASS(ShaderHandle, shader);
			ResourceFactory::getInstance()->addResource<ShaderResource>(shader);

		}
        
        // Texture with Vertex Color
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "attribute highp vec4 col;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vcol;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "    vcol = col;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "uniform highp vec4 color;\n"
                "varying highp vec4 vcol;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 diff = texture2D(texture0, vtex0.st);\n"
                "     if (diff.a <= 0.00001) discard;\n"
                "     gl_FragColor = color * vcol * diff;\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultTexCol", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultTextureColorShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Texture 2 with Vertex Color
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
                "     vcol = col;\n"
                "}\n"));
            
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "uniform sampler2D texture1;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vtex1;\n"
                "uniform highp vec4 color;\n"
                "varying highp vec4 vcol;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec4 tex0 = texture2D(texture0, vtex0.st);\n"
                "     highp vec4 tex1 = texture2D(texture1, vtex1.st);\n"
                "    gl_FragColor = color * vcol * (tex0 + tex1);\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribTexCoord1] = "tex1";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture1", TextureStageSpecular));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultTexCol2", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultTextureColorShader2 = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Debug
        {
            
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "void main(void)\n"
                "{\n"
                "    gl_FragColor = vec4(1.0, 0.5, 1.0, 1.0);\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.uniforms.push_back(SharedUniform::getInstance().getUniform("mvp"));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "defaultDebug", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kDefaultDebugShader = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Normal Color Lit
        {
            ShaderParams params;
            params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "attribute highp vec4 pos;\n"
                "attribute highp vec4 tex0;\n"
                "attribute highp vec4 col;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vcol;\n"
                "uniform highp mat4 mvp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * pos;\n"
                "    vtex0 = tex0;\n"
                "     vcol = col;\n"
                "}\n"));
            params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
                "uniform sampler2D texture0;\n"
                "varying highp vec4 vtex0;\n"
                "varying highp vec4 vcol;\n"
				"uniform highp vec4 color;\n"
                "void main(void)\n"
                "{\n"
                "     highp vec3 normal_vec = normalize((2.0 * texture2D(texture0, vtex0.st).rgb) - 1.0);\n"
                "    gl_FragColor = vec4(color.rgb * vcol.rgb * dot(normal_vec, vec3(0.0, 0.0, 1.0)), color.a * vcol.a);\n"
                "}\n"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribColor] = "col";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "normalColor", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kNormalColorLit = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
        }
        
        // Vertex Phong Lit
        {
            ShaderParams params;
            
            params.vertexSource = std::static_pointer_cast<cs::ShaderSource>(CREATE_CLASS(cs::ShaderSourceFile, "phong.vert"));
            params.fragmentSource = std::static_pointer_cast<cs::ShaderSource>(CREATE_CLASS(cs::ShaderSourceFile, "phong.frag"));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribNormal] = "nml";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            params.addUniform(SharedUniform::getInstance().getUniform("object"));
            params.addUniform(SharedUniform::getInstance().getUniform("light_position0"));
            params.addUniform(SharedUniform::getInstance().getUniform("camera_position"));
            params.addUniform(SharedUniform::getInstance().getUniform("camera_direction"));
            params.addUniform(SharedUniform::getInstance().getUniform("diffuse_intensity0"));
            params.addUniform(SharedUniform::getInstance().getUniform("ambient_intensity0"));
            
            MeshMaterialInstance::addUniforms(params, MeshMaterialInstance::GeometryTypeNormals);
            
            //params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse, "white.png"));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "vertexColorLit", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kVertexPhongLit = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }
        
        // Vertex Phong Lit Texture
        {
            ShaderParams params;
            
            StringList preprocessors;
            preprocessors.push_back("TEXTURE_DIFFUSE");
            
            params.vertexSource = std::static_pointer_cast<cs::ShaderSource>(CREATE_CLASS(cs::ShaderSourceFile, "phong.vert", preprocessors));
            params.fragmentSource = std::static_pointer_cast<cs::ShaderSource>(CREATE_CLASS(cs::ShaderSourceFile, "phong.frag", preprocessors));
            
            params.attributes[AttribPosition] = "pos";
            params.attributes[AttribTexCoord0] = "tex0";
            params.attributes[AttribNormal] = "nml";
            
            params.addUniform(SharedUniform::getInstance().getUniform("mvp"));
            params.addUniform(SharedUniform::getInstance().getUniform("color"));
            params.addUniform(SharedUniform::getInstance().getUniform("object"));
            params.addUniform(SharedUniform::getInstance().getUniform("light_position0"));
            params.addUniform(SharedUniform::getInstance().getUniform("camera_position"));
            params.addUniform(SharedUniform::getInstance().getUniform("camera_direction"));
            params.addUniform(SharedUniform::getInstance().getUniform("diffuse_intensity0"));
            params.addUniform(SharedUniform::getInstance().getUniform("ambient_intensity0"));
            
            MeshMaterialInstance::addUniforms(params, MeshMaterialInstance::GeometryTypeNormals);
            
            params.addUniform(CREATE_CLASS(UniformDataTexture, "texture_diffuse", TextureStageDiffuse));
            
            ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "vertexColorLitTexture", ShaderBucketGeometry, params);
            shader->setFlushable(false);
            
            RenderInterface::kVertexPhongLitTexture = CREATE_CLASS(ShaderHandle, shader);
            ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
            
        }

		// Color Outline
		{
			ShaderParams params;
			params.vertexSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"attribute highp vec4 pos;\n"
				"attribute highp vec4 tex0;\n"
				"attribute highp vec4 col;\n"
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp mat4 mvp;\n"
				"void main(void)\n"
				"{\n"
				"    gl_Position = mvp * pos;\n"
				"    vtex0 = tex0;\n"
				"    vcol = col;\n"
				"}\n"));

			params.fragmentSource = std::static_pointer_cast<ShaderSource>(CREATE_CLASS(ShaderSourceRaw,
				"uniform sampler2D texture0;\n"
				"varying highp vec4 vtex0;\n"
				"varying highp vec4 vcol;\n"
				"uniform highp vec4 color;\n"
				"uniform highp float offset; \n"
				"void main(void)\n"
				"{\n"
				"   highp vec4 diff = texture2D(texture0, vtex0.xy);\n"
				"   highp vec4 blank_color = vec4(0.0, 0.0, 0.0, 0.0);\n"
				//"   highp vec4 blank_color = diff;\n"
				"	if (diff.a > 0.5)\n"
				"		gl_FragColor = blank_color;\n"
				"	else {\n"
				"		highp float p0 = texture2D(texture0, vec2(vtex0.x + offset, vtex0.y)).a;\n"
				"		highp float p1 = texture2D(texture0, vec2(vtex0.x, vtex0.y - offset)).a;\n"
				"		highp float p2 = texture2D(texture0, vec2(vtex0.x - offset, vtex0.y)).a;\n"
				"		highp float p3 = texture2D(texture0, vec2(vtex0.x, vtex0.y + offset)).a;\n"
				"		highp float a = p0 + p1 + p2 + p3;\n"
				"		if (diff.a < 1.0 && a > 0.0) \n"
				"			gl_FragColor = color; \n"
				"		else \n"
				"			gl_FragColor = blank_color;\n"
				"	}\n"
				"}\n"));

			params.attributes[AttribPosition] = "pos";
			params.attributes[AttribTexCoord0] = "tex0";
			params.attributes[AttribColor] = "col";

			params.uniforms.push_back(SharedUniform::getInstance().getUniform("mvp"));
			params.uniforms.push_back(SharedUniform::getInstance().getUniform("color"));
			
			params.addUniform(CREATE_CLASS(UniformDataTexture, "texture0", TextureStageDiffuse));
			params.addUniform(CREATE_CLASS(UniformDataFloat, "offset", 1.0f / 64.0f, true, ShaderFragment));

			ShaderResourcePtr shader = CREATE_CLASS(ShaderResource, "colorOutline", ShaderBucketGeometry, params);
			shader->setFlushable(false);

			RenderInterface::kColorOutline = CREATE_CLASS(ShaderHandle, shader);
			ResourceFactory::getInstance()->addResource<ShaderResource>(shader);
		}
    }
}
