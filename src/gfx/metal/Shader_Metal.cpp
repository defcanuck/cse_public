#include "PCH.h"

#include "gfx/metal/Shader_Metal.h"
#include "os/LogManager.h"
#include "global/Utils.h"
#include "gfx/ShaderUtils.h"

#include "MTLRenderInterface.h"
#include "src/glsl/glsl_optimizer.h"

const glslopt_target kTargetVersion = kGlslTargetMetal;
glslopt_ctx* gOptContext = nullptr;

#define ShaderInfoLog(...) log::info(__VA_ARGS__)


namespace cs
{
    struct MTLShaderDecompressor
    {
        static void getShaderInfo(const char* name, glslopt_shader* mtlShader, Shader_Metal* shader, bool printSource)
        {
            if (printSource)
            {
                ShaderInfoLog("Info for Shader: ", name);
            }
            
            int inputCount = glslopt_shader_get_input_count(mtlShader);
            for (int i = 0; i < inputCount; ++i)
            {
                const char* outName;
                glslopt_basic_type type;
                glslopt_precision precision;
                int vecSize;
                int matSize;
                int arraySize;
                int outLocation;
                
                glslopt_shader_get_input_desc(mtlShader, i, &outName, &type, &precision, &vecSize, &matSize, &arraySize, &outLocation);
                
                if (printSource)
                {
                    ShaderInfoLog("Input Name: ", outName, " type:", type, " vsize: ", vecSize, " msize:", matSize, " arraysize: ", arraySize, " out: ", outLocation);
                }
                
                if (shader->getType() == ShaderVertex)
                {
                    AttributeType attribType = AttribNone;
                    if (strcmp("pos", outName) == 0) attribType = AttribPosition;
                    else if (strcmp("tex0", outName) == 0) attribType = AttribTexCoord0;
                    else if (strcmp("tex1", outName) == 0) attribType = AttribTexCoord1;
                    else if (strcmp("nml", outName) == 0) attribType = AttribNormal;
                    else if (strcmp("tgt", outName) == 0) attribType = AttribTangent;
                    else if (strcmp("col", outName) == 0) attribType = AttribColor;
                    else if (strcmp("bones", outName) == 0) attribType = AttribBones;
                    else if (strcmp("weights", outName) == 0) attribType = AttribWeights;
                    
                    assert(attribType != AttribNone);
                    shader->attributes.insert(attribType);
                }
            }
            
            
            int uniform_count = glslopt_shader_get_uniform_count (mtlShader);
            shader->uniformBufferSize = glslopt_shader_get_uniform_total_size (mtlShader);
            
            if (printSource)
            {
                ShaderInfoLog("Uniforms Total Size: ", shader->uniformBufferSize);
            }
            
            for (int i = 0; i < uniform_count; ++i)
            {
                const char* outName;
                glslopt_basic_type type;
                glslopt_precision precision;
                int vecSize;
                int matSize;
                int arraySize;
                int outLocation;
                
                glslopt_shader_get_uniform_desc(mtlShader, i, &outName, &type, &precision, &vecSize, &matSize, &arraySize, &outLocation);
                if (printSource)
                {
                    ShaderInfoLog("Uniform Name: ", outName, " type:", type, " vsize: ", vecSize, " msize:", matSize, " arraysize: ", arraySize, " out: ", outLocation);
                }
                
                shader->uniformMap.emplace(outName, outLocation);
            }
            
            
            int texCount = glslopt_shader_get_texture_count(mtlShader);
            for (int i = 0; i < texCount; ++i)
            {
                const char* outName;
                glslopt_basic_type type;
                glslopt_precision precision;
                int vecSize;
                int matSize;
                int arraySize;
                int outLocation;
                
                glslopt_shader_get_texture_desc(mtlShader, i, &outName, &type, &precision, &vecSize, &matSize, &arraySize, &outLocation);
                if (printSource)
                {
                    ShaderInfoLog("Texture Name: ", outName, " type:", type, " vsize: ", vecSize, " msize:", matSize, " arraysize: ", arraySize, " out: ", outLocation);
                }
                
                shader->textureStageMap[outName] = outLocation;
            }
        }
    };
    
    Shader_Metal::~Shader_Metal()
    {
        MTLReleaseProgram(this->shader);
        this->shader = nullptr;
    }
    
    void Shader_Metal::compile(const std::string& name, const std::string& str, bool printSource)
    {
        std::string adjustedString = str;
        
        ShaderUtils::removePrecisionQualifiers(adjustedString);
        ShaderUtils::replaceOutputFormat(adjustedString);
        
        gOptContext = glslopt_initialize(kTargetVersion);
        glslopt_shader_type glslType = (glslopt_shader_type) this->type;
        unsigned int options = 0; //kGlslOptionSkipPreprocessor;
        glslopt_shader* tmpShader = glslopt_optimize(gOptContext, glslType, adjustedString.c_str(), options);
        
        if (glslopt_get_status(tmpShader))
        {
            adjustedString = std::string(glslopt_get_output(tmpShader));
        }
        else
        {
            log::error("Error optimizing source ", name, " : ", glslopt_get_log(tmpShader));
            log::error(this->source);
            return;
        }
        
        if (printSource)
        {
            ShaderInfoLog("Shader Source: ", adjustedString);
        }
        
        MTLShaderDecompressor::getShaderInfo(name.c_str(), tmpShader, this, printSource);
    
        glslopt_shader_delete(tmpShader);
        glslopt_cleanup(gOptContext);
        
        this->source = adjustedString;
        this->shader = MTLCompileProgram(name.c_str(), this->source.c_str());
       
    }
}
