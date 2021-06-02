#include "PCH.h"

#include "gfx/ShaderProgram.h"

namespace cs
{

	void ShaderProgram::addShader(ShaderPtr& shader)
	{
		this->shaders[shader->getType()] = shader;
	}
    
    ShaderPtr ShaderProgram::getShader(ShaderType type)
    {
        std::map<ShaderType, ShaderPtr>::iterator it = this->shaders.find(type);
        if (it != this->shaders.end())
        {
            return it->second;
        }
        return ShaderPtr();
    }
}
