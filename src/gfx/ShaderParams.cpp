#include "PCH.h"

#include "gfx/ShaderParams.h"

namespace cs
{
    DEFINE_META_MAP_NEW(TextureStages, uint16, TextureHandle);
    DEFINE_META_VECTOR_NEW(ShaderUniformList, ShaderUniform, ShaderUniformList);
    
    BEGIN_META_CLASS(ShaderUniform)
        ADD_MEMBER(name);
        SET_MEMBER_CONST();
    END_META()
    
    BEGIN_META_CLASS(ShaderUniformFloat)
        ADD_MEMBER(value);
        SET_MEMBER_NO_SLIDER();
    END_META()
    
    BEGIN_META_CLASS(ShaderUniformInt)
        ADD_MEMBER(value);
        SET_MEMBER_NO_SLIDER();
    END_META()
    
    BEGIN_META_CLASS(ShaderUniformVec2)
        ADD_MEMBER(value);
    END_META()
    
    BEGIN_META_CLASS(ShaderUniformVec3)
        ADD_MEMBER(value);
    END_META()
    
    BEGIN_META_CLASS(ShaderUniformVec4)
        ADD_MEMBER(value);
    END_META()
    
    BEGIN_META_CLASS(ShaderUniformMat4)
        ADD_MEMBER(value);
    END_META()
    
    void ShaderSource::appendPreprocessors(std::string& source)
    {
        std::stringstream str;
        for (auto& it : this->preprocessors)
        {
            str << "#define " << it << " 1" << std::endl;
        }
        source = str.str() + source;
    }
    
    std::string ShaderSourceFile::getSource()
    {
        AutoRelease<DataStream> stream;
        if (FileManager::getInstance()->openStreamFile(this->fileName, &stream.value))
        {
            std::string buffer;
            std::istream& ifs = stream->getStream();
            
            ifs.seekg(0, std::ios::end);
            buffer.reserve((uint32)ifs.tellg());
            ifs.seekg(0, std::ios::beg);
            
            buffer.assign((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
            
            this->appendPreprocessors(buffer);
            
            return buffer;
        }
        
        log_error("Shader Error - Cannot find ", this->fileName);
        return "";
    }
    
    std::string ShaderSourceRaw::getSource()
    {
        std::string tmpSource;
        tmpSource = source;
        this->appendPreprocessors(tmpSource);
        return tmpSource;
    }

}

