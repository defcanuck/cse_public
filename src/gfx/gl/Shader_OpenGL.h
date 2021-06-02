#pragma once

#include "ClassDef.h"
#include "gfx/Shader.h"
#include "gfx/gl/OpenGL.h"

// #define TEST_METAL 1

namespace cs
{

	extern uint32 kShaderType[];

	CLASS_DEFINITION_DERIVED(Shader_OpenGL, Shader)
	public:
		Shader_OpenGL(ShaderType t) : 
			Shader(t), 
			handle(0) 
		{ 
			this->init();
		}

		virtual ~Shader_OpenGL() 
		{ 
			this->free();
		}

        virtual void compile(const std::string& name, const std::string& str, bool printSource = false);
		GLuint getHandle() const { return handle; }
        virtual uint32 getUniformSize() const { return 0; }
    
	protected:
		
		void init();
		void free();

		GLuint handle;

#if defined(TEST_METAL)
		std::string metalSource;
#endif
	};
}
