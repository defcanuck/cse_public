#include "PCH.h"

#include "gfx/gl/Shader_OpenGL.h"
#include "os/LogManager.h"
#include "global/Utils.h"
#include "gfx/ShaderUtils.h"

#if defined(TEST_METAL)
	#include "src/glsl/glsl_optimizer.h"

	#if defined(CS_EDITOR)
		const glslopt_target kTargetVersion = kGlslTargetMetal;
	#else
		const glslopt_target kTargetVersion = kGlslTargetOpenGLES20;
	#endif

	static glslopt_ctx* gOptContext = nullptr;
#endif

#if defined(CS_METAL)


#endif

namespace cs
{
	uint32 kShaderType[] =
	{
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER
	};

	void Shader_OpenGL::init()
	{
		GL_CHECK(this->handle = glCreateShader(kShaderType[this->type]));
	}

	void Shader_OpenGL::free()
	{
		GL_CHECK(glDeleteShader(this->handle));
		this->handle = 0;
		this->compiled = false;
	}

    void Shader_OpenGL::compile(const std::string& name, const std::string& str, bool printSource)
	{
		std::string adjustedString = str;
        
#if defined(CS_WINDOWS)
        ShaderUtils::removePrecisionQualifiers(adjustedString);
#endif
        
        ShaderUtils::replaceOutputFormat(adjustedString);

#if defined(TEST_METAL)
		gOptContext = glslopt_initialize(kTargetVersion);
		
        
		glslopt_shader_type glslType = (glslopt_shader_type) this->type;
		unsigned int options = kGlslOptionSkipPreprocessor;
		glslopt_shader* tmpShader = glslopt_optimize(gOptContext, glslType, adjustedString.c_str(), options);
		if (glslopt_get_status(tmpShader)) 
		{
			adjustedString = std::string(glslopt_get_output(tmpShader));
		}
		else 
		{
			log::error("Error optimizing source ", name, " : ", glslopt_get_log(tmpShader));
		}
		glslopt_shader_delete(tmpShader);
		glslopt_cleanup(gOptContext);
#endif
        
		const char* source[] =
		{
            adjustedString.c_str()
		};

		GL_CHECK(glShaderSource(this->handle, sizeof(source) / sizeof(source[0]), source, nullptr));
		GL_CHECK(glCompileShader(this->handle));

		GLint success = 0;
		GL_CHECK(glGetShaderiv(this->handle, GL_COMPILE_STATUS, &success));
		if (!(this->compiled = !(success == GL_FALSE)))
        {
			GLint maxLength = 0;
			GL_CHECK(glGetShaderiv(this->handle, GL_INFO_LOG_LENGTH, &maxLength));

            if (maxLength > 0)
            {
                // The maxLength includes the NULL character
                GLchar* errorLog = new GLchar[maxLength];
                GL_CHECK(glGetShaderInfoLog(this->handle, maxLength, &maxLength, errorLog));
                log::print(LogError, name, " - ", errorLog);
                delete [] errorLog;
            }
            else
            {
                log::print(LogError, "Unknown Shader Compilation Error!");
            }

			this->free();
		}
	}
}
