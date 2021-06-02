#pragma once

#include "ClassDef.h"

#include "gfx/Types.h"

namespace cs
{
	CLASS_DEFINITION(Shader)
	public:
		Shader(ShaderType t) : 
			type(t),
			compiled(false) { }

		~Shader() { }

        virtual void compile(const std::string& name, const std::string& str, bool printSource = false) = 0;
        virtual uint32 getUniformSize() const = 0;
    
		ShaderType getType() const { return this->type; }

	protected:

		ShaderType type;
		bool compiled;
	};
}
