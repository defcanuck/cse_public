#pragma once

#include "ClassDef.h"

#include "math/GLM.h"
#include "gfx/Types.h"
#include "gfx/TextureHandle.h"

#include <unordered_map>
#include <assert.h>
#include <functional>

#define SetSingleValue(uniform_type, raw_type, union_type) \
inline void setValue(const raw_type& fvalue) \
{ \
	assert(this->count == 1); \
	assert(this->type == uniform_type); \
	if (*(this->value.union_type) == fvalue) \
		return; \
	*(this->value.union_type) = fvalue; \
	this->dirty = true; \
}

namespace cs
{
	enum UniformType
	{
		UniformNone = -1,
		UniformFloat,
		UniformInt,
		UniformVec2,
		UniformVec3,
		UniformVec4,
		UniformMat4,
		UniformMAX
	};

	extern const size_t kUniformPrimitiveSize[];

	CLASS_DEFINITION(Uniform)
	
	public:

		struct UpdateParams
		{
			float32 dt;
		};

		Uniform(const std::string& nname, UniformType ttype, int count = 1, ShaderType st = ShaderFragment, bool allocate = true);
		virtual ~Uniform();

		void dealloc();

		const std::string& getName() const { return name; }
		UniformType getType() const { return type; }
		void* getData() const { return value.value_raw; }
		int getCount() const { return count; }
        ShaderType getShaderType() const { return this->shaderType; }

		template<class T>
		static T* getValue(Uniform& uniform)
		{
			return reinterpret_cast<T*>(uniform.getData());
		}

		template<class T>
		static T* getValue(UniformPtr& ptr)
		{
			return Uniform::getValue<T>(*ptr.get());
		}

		SetSingleValue(UniformFloat, float, value_float);
		SetSingleValue(UniformInt, int, value_int);
		SetSingleValue(UniformVec2, vec2, value_vec2);
		SetSingleValue(UniformVec3, vec3, value_vec3);
		SetSingleValue(UniformVec4, vec4, value_vec4);
		SetSingleValue(UniformMat4, mat4, value_mat4);

		inline void setValue(const ColorB& color)
		{
			assert(this->count == 1);
			assert(this->type == UniformVec4);
			vec4 fvalue = toVec4(color);
			if (*(this->value.value_vec4) == fvalue)
				return;
			*(this->value.value_vec4) = fvalue;
			this->dirty = true;
		}

		virtual bool isTexture() const { return false; }
		virtual bool canOverload() const { return false; }
		virtual void update(const UpdateParams& params) { }

		void setUserData(uintptr_t* data, size_t dataSize) 
		{ 
			this->userData = data; 
			this->userDataSize = dataSize; 
		}

	protected:

		std::string name;
		UniformType type;
		int count;
		bool dirty;
		size_t size;
        ShaderType shaderType;
		bool allocated;

		uintptr_t* userData;
		size_t userDataSize;

		union
		{
			void* value_raw;
			float32* value_float;
			int32* value_int;
			vec2* value_vec2;
			vec3* value_vec3;
			vec4* value_vec4;
			mat4* value_mat4;
		} value;
	};

	template <class T, int uniform_type>
	class UniformData : public Uniform
	{
	public:

		typedef std::function<T(const Uniform::UpdateParams&, uintptr_t*, size_t)> UpdateCallback;

		UniformData(const std::string& nname, int32 count, T* value_data, bool over = false, ShaderType st = ShaderFragment)
			: Uniform(nname, (UniformType) uniform_type, count, st, false)
			, valueData(nullptr)
			, overload(over)
		{
			this->valueData = new T[count];
			memcpy(this->valueData, value_data, sizeof(T) * count);
			this->value.value_raw = (void*) this->valueData;
		}

		UniformData(const std::string& nname, const T& value_data, bool over = false, ShaderType st = ShaderFragment)
			: Uniform(nname, (UniformType)uniform_type, 1, st, false)
			, valueData(nullptr)
			, overload(over)
		{
			this->valueData = new T();
			(*this->valueData) = value_data;
			this->value.value_raw = (void*) this->valueData;
		}

		const T& getDataValue() const { return *this->valueData; }
		void setDataValue(const T& data) { *this->valueData = data; }

		virtual ~UniformData()
		{
			if (this->getCount() > 1)
				delete[] this->valueData;
			else
				delete this->valueData;
		}

		virtual bool canOverload() const { return this->overload; }

		virtual void update(const UpdateParams& params) 
		{ 
			assert(!this->canOverload());
			if (this->onUpdate)
			{
				(*this->valueData) = this->onUpdate(params, this->userData, this->userDataSize);
			}
		}


		T* valueData;
		bool overload;
		int32 padding[3];
		UpdateCallback onUpdate;

	};

	typedef UniformData<int32, UniformInt> UniformDataInt;
	typedef UniformData<float32, UniformFloat> UniformDataFloat;
	typedef UniformData<vec2, UniformVec2> UniformDataVec2;
	typedef UniformData<vec3, UniformVec3> UniformDataVec3;
	typedef UniformData<vec4, UniformVec4> UniformDataVec4;
	typedef UniformData<mat4, UniformMat4> UniformDataMat4;

	typedef std::shared_ptr<UniformDataInt> UniformDataIntPtr;
	typedef std::shared_ptr<UniformDataFloat> UniformDataFloatPtr;
	typedef std::shared_ptr<UniformDataVec2> UniformDataVec2Ptr;
	typedef std::shared_ptr<UniformDataVec3> UniformDataVec3Ptr;
	typedef std::shared_ptr<UniformDataVec4> UniformDataVec4Ptr;
	typedef std::shared_ptr<UniformDataMat4> UniformDataMat4Ptr;


	class UniformDataTexture : public UniformDataInt
	{
	public:

		UniformDataTexture(const std::string& nname, TextureStage st);
		UniformDataTexture(const std::string& nname, TextureStage st, const std::string& defaultTexName);
		UniformDataTexture(const std::string& nname, TextureStage st, TexturePtr& texture);
		UniformDataTexture(const std::string& nname, TextureStage st, TextureHandlePtr& texture);

		virtual ~UniformDataTexture() { }
		virtual bool isTexture() const { return true; }
		virtual TextureStage getStage() const { return this->stage; }
		virtual TextureHandlePtr getDefaultTexture() { return this->defaultTexture; }

		TextureHandlePtr defaultTexture;
		TextureStage stage;
	};

	typedef std::shared_ptr<UniformDataTexture> UniformDataTexturePtr;

	class UniformDataRenderTexture : public UniformDataTexture
	{
	public:

		UniformDataRenderTexture(const std::string& n, TextureStage st, RenderTargetType type, bool useDepth = false);

	};

	typedef std::shared_ptr<UniformDataRenderTexture> UniformDataRenderTexturePtr;

	class SharedUniform
	{
	public:
		static SharedUniform& getInstance()
		{
			static SharedUniform instance;
			return instance;
		}

		void addUniform(UniformPtr uniform);
		UniformPtr getUniform(const std::string& name);
		
	private:

		SharedUniform() { }
		SharedUniform(SharedUniform const&) = delete;
		void operator=(SharedUniform const&) = delete;
	
		typedef std::unordered_map<std::string, UniformPtr> UniformMap;
		UniformMap uniforms;
	
	};
}
