#pragma once

#include "ClassDef.h"
#include "gfx/VertexDeclaration.h"
#include "global/Resource.h"
#include "gfx/ShaderProgram.h"
#include "gfx/ShaderParams.h"
#include "gfx/ShaderBinding.h"

#include <string>
#include <unordered_map>

class GeometryDataPtr;

namespace cs
{


	CLASS_DEFINITION_DERIVED_REFLECT(ShaderResource, Resource)

	public:

		ShaderResource(const std::string& name, ShaderBucket , const ShaderParams& shader_params);
		~ShaderResource();

		ShaderBucket getBucket() const { return this->bucket; }

		void bind(const ShaderBindParams& params, ShaderUniformBindParams& uniformParams);
		void refresh();

		static void refreshShaders();

		ShaderProgramPtr& getProgram() { return this->program; }
        ShaderPtr getShader(ShaderType type);

		void getTextureStages(std::vector<TextureStage>& stages, TextureStageMask& ignoredMask);
		void populateTextureStages(TextureStages& textures, TextureStageMask& ignoredMask);

		void populateMutableUniforms(ShaderUniformList& uniforms);
		void mapMutableUniforms(ShaderUniformList& uniforms);

		bool equals(const ShaderResourcePtr& rhs) const;
		void update(const Uniform::UpdateParams& updateParams);

#if defined(CS_METAL)
        const std::map<uint32, uint32>* getTextureRemap();
        void* getPipelineDescriptor(const ShaderBindParams& bindParams);
#endif

	private:

		void init();

		ShaderBucket bucket;
		ShaderProgramPtr program;
		ShaderParams params;

		std::vector<AttributeType> attributes;
		
	};

}

