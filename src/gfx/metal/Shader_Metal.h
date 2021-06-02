#pragma once

#include "ClassDef.h"
#include "gfx/Shader.h"
#include "gfx/gl/OpenGL.h"

#include <set>

namespace cs
{

	extern uint32 kShaderType[];

	CLASS_DEFINITION_DERIVED(Shader_Metal, Shader)
	public:
		Shader_Metal(ShaderType t)
            : Shader(t)
            , uniformBufferSize(0)
            , shader(nullptr)
        { }

        virtual ~Shader_Metal();

        virtual void compile(const std::string& name, const std::string& str, bool printSource = false);
        virtual uint32 getUniformSize() const { return this->uniformBufferSize; }
    
        void* getShaderPtr() { return this->shader; }
    
        std::string source;
    
        std::map<std::string, uint32> textureStageMap;
    
        struct UniformInfo
        {
            UniformInfo(uint32 o)
                : offset(o)
            { }
            
            uint32 offset;
        };
        uint32 uniformBufferSize;
        typedef std::map<std::string, UniformInfo> UniformInfoMap;
        UniformInfoMap uniformMap;
    
        typedef std::set<AttributeType> AttributeExistsSet;
        AttributeExistsSet attributes;

	protected:
    
        void* shader;
    
	};
}
