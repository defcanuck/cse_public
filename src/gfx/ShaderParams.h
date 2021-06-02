#pragma once

#include "ClassDef.h"
#include "gfx/Types.h"
#include "gfx/Attribute.h"
#include "gfx/Uniform.h"
#include "gfx/VertexDeclaration.h"

#include <string>
#include <unordered_map>

namespace cs
{
    typedef std::vector<UniformPtr> UniformList;
    typedef std::map<int32, std::string> UniformNameMap;
    
    typedef std::map<uint16, TextureHandlePtr> TextureStages;
    typedef BitMask<TextureStage, TextureStageMAX> TextureStageMask;
    
    enum ShaderBucket
    {
        ShaderBucketNone = -1,
        ShaderBucketGeometry,
        ShaderBucketPostProcess,
        ShaderBucketUI,
        //...
        ShaderBucketMAX
    };
    
    CLASS_DEFINITION(ShaderSource)
    public:
        ShaderSource() { }
        ShaderSource(const StringList& pre)
            : preprocessors(pre)
        { }
    
        virtual std::string getSource() = 0;
        void setPreprocessors(const StringList& pre) { this->preprocessors = pre; }

    protected:
    
        void appendPreprocessors(std::string& source);
    
    private:
    
        StringList preprocessors;
    };

    CLASS_DEFINITION_DERIVED(ShaderSourceRaw, ShaderSource)
    public:

        ShaderSourceRaw(const std::string& src)
            : source(src)
        { }

        virtual std::string getSource();

        std::string source;
    };

    CLASS_DEFINITION_DERIVED(ShaderSourceFile, ShaderSource)
    public:
        ShaderSourceFile(const std::string& fname)
            : fileName(fname)
        { }

        ShaderSourceFile(const std::string& fname, const StringList& preprocessors)
            : ShaderSource(preprocessors)
            , fileName(fname)
        { }

        virtual std::string getSource();

        std::string fileName;
    };

    CLASS_DEFINITION_REFLECT(ShaderUniform)
    public:
        ShaderUniform(const std::string& n)
            : name(n)
        { }

        virtual ~ShaderUniform()
        { }

        virtual void clear() { }
        virtual UniformType getType() const { return UniformNone; }
        const std::string& getName() const { return this->name; }

        virtual void bind() { assert(false); }
        virtual bool equals(ShaderUniformPtr& uniform) { return false; }

        virtual void setValue(const int32& val) { assert(false); }
        virtual void setValue(const float32& val) { assert(false); }
        virtual void setValue(const vec2& val) { assert(false); }
        virtual void setValue(const vec3& val) { assert(false); }
        virtual void setValue(const vec4& val) { assert(false); }
        virtual void setValue(const mat4& val) { assert(false); }

    protected:

        ShaderUniform()
            : name("empty")
        { }

        std::string name;

    };

    CLASS_DEFINITION_DERIVED_REFLECT(ShaderUniformFloat, ShaderUniform)
    public:

        ShaderUniformFloat(UniformDataFloatPtr uni, const float32& val = 0.0f)
            : ShaderUniform(uni->getName())
            , uniform(uni)
            , value(val)
        { }

        virtual void clear() { value = 0.0f; }
        virtual UniformType getType() const { return UniformFloat; }

        void setUniform(UniformDataFloatPtr uni) { this->uniform = uni; }
        virtual void bind() { if (this->uniform) this->uniform->setDataValue(this->value); }

        float32 getValue() const { return this->value; }
        virtual void setValue(const float32& val) { this->value = val; }

        virtual bool equals(ShaderUniformPtr& uniform)
        {
            assert(uniform.get() && uniform->getName() == this->getName());
            return this->value == std::static_pointer_cast<ShaderUniformFloat>(uniform)->getValue();
        }

    protected:

        ShaderUniformFloat()
            : ShaderUniform()
            , value(0.0f)
            , uniform(nullptr)
        { }

        UniformDataFloatPtr uniform;
        float32 value;
    };

    CLASS_DEFINITION_DERIVED_REFLECT(ShaderUniformInt, ShaderUniform)
    public:
        ShaderUniformInt(UniformDataIntPtr uni, const int32& val = 0)
            : ShaderUniform(uni->getName())
            , value(val)
            , uniform(uni)
        { }

        virtual void clear() { value = 0; }
        virtual UniformType getType() const { return UniformInt; }

        void setUniform(UniformDataIntPtr uni) { this->uniform = uni; }
        virtual void bind() { if (this->uniform) this->uniform->setDataValue(this->value); }

        int32 getValue() const { return this->value; }
        virtual void setValue(const int32& val) { this->value = val; }

        virtual bool equals(ShaderUniformPtr& uniform)
        {
            assert(uniform.get() && uniform->getName() == this->getName());
            return this->value == std::static_pointer_cast<ShaderUniformInt>(uniform)->getValue();
        }

    private:

        ShaderUniformInt()
            : ShaderUniform()
            , value(0)
            , uniform(nullptr)
        { }

        int32 value;
        UniformDataIntPtr uniform;
    };

    CLASS_DEFINITION_DERIVED_REFLECT(ShaderUniformVec2, ShaderUniform)
    public:
        ShaderUniformVec2(UniformDataVec2Ptr uni, const vec2& val = kZero2)
            : ShaderUniform(uni->getName())
            , value(val)
            , uniform(uni)
        { }

        virtual void clear() { value = kZero2; }
        virtual UniformType getType() const { return UniformVec2; }

        void setUniform(UniformDataVec2Ptr uni) { this->uniform = uni; }
        virtual void bind() { if (this->uniform) this->uniform->setDataValue(this->value); }

        vec2 getValue() const { return this->value; }
        virtual void setValue(const vec2& val) { this->value = val; }

        virtual bool equals(ShaderUniformPtr& uniform)
        {
            assert(uniform.get() && uniform->getName() == this->getName());
            return this->value == std::static_pointer_cast<ShaderUniformVec2>(uniform)->getValue();
        }

    private:

        ShaderUniformVec2()
            : ShaderUniform()
            , value(kZero2)
            , uniform(nullptr)
        { }

        vec2 value;
        UniformDataVec2Ptr uniform;
    };

    CLASS_DEFINITION_DERIVED_REFLECT(ShaderUniformVec3, ShaderUniform)
    public:
        ShaderUniformVec3(UniformDataVec3Ptr uni, const vec3& val = kZero3)
            : ShaderUniform(uni->getName())
            , value(val)
            , uniform(uni)
        { }

        virtual void clear() { value = kZero3; }
        virtual UniformType getType() const { return UniformVec3; }

        void setUniform(UniformDataVec3Ptr uni) { this->uniform = uni; }
        virtual void bind() { if (this->uniform) this->uniform->setDataValue(this->value); }

        vec3 getValue() const { return this->value; }
        virtual void setValue(const vec3& val) { this->value = val; }

        virtual bool equals(ShaderUniformPtr& uniform)
        {
            assert(uniform.get() && uniform->getName() == this->getName());
            return this->value == std::static_pointer_cast<ShaderUniformVec3>(uniform)->getValue();
        }

    private:

        ShaderUniformVec3()
            : ShaderUniform()
            , value(kZero3)
            , uniform(nullptr)
        { }

        vec3 value;
        UniformDataVec3Ptr uniform;
    };

    CLASS_DEFINITION_DERIVED_REFLECT(ShaderUniformVec4, ShaderUniform)
    public:
        ShaderUniformVec4(UniformDataVec4Ptr uni, const vec4& val = kZero4)
            : ShaderUniform(uni->getName())
            , value(val)
            , uniform(uni)
        { }

        virtual void clear() { value = kZero4; }
        virtual UniformType getType() const { return UniformVec4; }

        void setUniform(UniformDataVec4Ptr uni) { this->uniform = uni; }
        virtual void bind() { if (this->uniform) this->uniform->setDataValue(this->value); }

        vec4 getValue() const { return this->value; }
        virtual void setValue(const vec4& val) { this->value = val; }

        virtual bool equals(ShaderUniformPtr& uniform)
        {
            assert(uniform.get() && uniform->getName() == this->getName());
            return this->value == std::static_pointer_cast<ShaderUniformVec4>(uniform)->getValue();
    }

    private:

        ShaderUniformVec4()
            : ShaderUniform()
            , value(kZero4)
            , uniform(nullptr)
        { }

        vec4 value;
        UniformDataVec4Ptr uniform;
    };

    CLASS_DEFINITION_DERIVED_REFLECT(ShaderUniformMat4, ShaderUniform)
    public:
        ShaderUniformMat4(UniformDataMat4Ptr uni, const mat4& val = kIdentity)
            : ShaderUniform(uni->getName())
            , value(val)
            , uniform(uni)
        { }

        virtual void clear() { value = kIdentity; }
        virtual UniformType getType() const { return UniformMat4; }

        void setUniform(UniformDataMat4Ptr uni) { this->uniform = uni; }
        virtual void bind() { if (this->uniform) this->uniform->setDataValue(this->value); }

        mat4 getValue() const { return this->value; }
        virtual void setValue(const mat4& val) { this->value = val; }

        virtual bool equals(ShaderUniformPtr& uniform)
        {
            assert(uniform.get() && uniform->getName() == this->getName());
            return this->value == std::static_pointer_cast<ShaderUniformMat4>(uniform)->getValue();
        }

    private:

        ShaderUniformMat4()
            : ShaderUniform()
            , value(kIdentity)
            , uniform(nullptr)
        { }

        mat4 value;
        UniformDataMat4Ptr uniform;
    };

    typedef std::vector<ShaderUniformPtr> ShaderUniformList;
    typedef std::map<std::string, ShaderUniformPtr> ShaderUniformMap;

    CLASS_DEFINITION(ShaderParams)

    public:

        ShaderParams()
            : vertexSource(nullptr)
            , fragmentSource(nullptr)
            , printSource(false)
        { }

        ShaderParams(const ShaderParams& rhs)
            : vertexSource(rhs.vertexSource)
            , fragmentSource(rhs.fragmentSource)
            , attributes(rhs.attributes)
            , uniforms(rhs.uniforms)
            , printSource(rhs.printSource)
        { }

        void operator=(const ShaderParams& rhs)
        {
            this->vertexSource = rhs.vertexSource;
            this->fragmentSource = rhs.fragmentSource;
            this->attributes = rhs.attributes;
            this->uniforms = rhs.uniforms;
            this->printSource = rhs.printSource;
        }

        template <class T>
        void addUniform(std::shared_ptr<T> uniform)
        {
#if defined(_DEBUG)
            assert(uniform.get());
            for (auto& it : this->uniforms)
            {
                bool matchStage = true;
#if !defined(CS_METAL)
                matchStage = uniform->getShaderType() == it->getShaderType();
#endif
                if (uniform->getName() == it->getName() && matchStage)
                {
					log::error("Error - duplicate uniform being declared - ", it->getName());
                    assert(false);
                }
            }
#endif
            
            UniformPtr uni = std::static_pointer_cast<Uniform>(uniform);
            this->uniforms.push_back(uni);
        }

        void getTextureStages(std::vector<TextureStage>& stages);
        //static void setTexture

        ShaderSourcePtr vertexSource;
        ShaderSourcePtr fragmentSource;
        UniformNameMap attributes;

        UniformList uniforms;
        bool printSource;
    };
}
