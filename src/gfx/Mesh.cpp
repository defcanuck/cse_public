#include "PCH.h"

#include "global/Values.h"
#include "gfx/Mesh.h"
#include "gfx/RenderInterface.h"
#include "global/ResourceFactory.h"

#include "global/Stats.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "external/tinyobj/tiny_obj_loader.h"

namespace cs
{
	const float32 kMeshOBJScale = 100.0f;
	const bool kForceUVBuffer = false;
	const bool kForceNormalBuffer = false;

	BEGIN_META_RESOURCE(Mesh)

	END_META()

	DEFINE_META_MAP_NEW(MeshShapeInstanceMap, std::string, MeshShapeInstance);

	BEGIN_META_CLASS(MeshShapeInstance)
		ADD_META_FUNCTION("Add Override Draw", &MeshShapeInstance::overrideDrawCalls);
		ADD_MEMBER_PTR(overrides);
	END_META()

	BEGIN_META_CLASS(MeshHandle)
		ADD_MEMBER_RESOURCE(mesh);
			SET_MEMBER_CALLBACK_POST(&MeshHandle::onMeshChanged);

		ADD_MEMBER(instances);
			SET_MEMBER_IGNORE_SERIALIZATION();

	END_META()

	BEGIN_META_CLASS(MeshHandleVolume)
		ADD_MEMBER_PTR(meshHandle);
		ADD_MEMBER(collisionObject);
		ADD_MEMBER(useAllMeshShapes);
	END_META()

	MeshMaterialPtr MeshMaterial::kDefaultMeshMaterial = CREATE_CLASS(MeshMaterial, "default_material");

	void MeshShapeInstance::overrideDrawCalls()
	{
		if (!this->overrides.get())
			this->overrides = CREATE_CLASS(DrawCallOverrides);
	}

	void MeshShapeInstance::addTexture(const std::string& textureName, TextureStage stage)
	{
		if (!this->overrides.get())
			this->overrides = CREATE_CLASS(DrawCallOverrides);

		if (this->overrides->textureOverrideHandles.find(stage) == this->overrides->textureOverrideHandles.end())
		{
			this->overrides->textureOverrideHandles[stage] = CREATE_CLASS(TextureHandle, textureName);
		}
	}

	void MeshShapeInstance::addOverrideShader(const std::string& shaderName, ShaderHandlePtr& shader)
	{
		if (!this->overrides.get())
			this->overrides = CREATE_CLASS(DrawCallOverrides);

		if (this->overrides->shaderOverrideHandles.find(shaderName) == this->overrides->shaderOverrideHandles.end())
		{
			this->overrides->shaderOverrideHandles[shaderName] = shader;
		}
	}

	void MeshHandle::onMeshChanged()
	{
		this->onChanged.invoke();

		this->instances.clear();
		if (this->mesh.get())
		{
			Mesh::MeshShapes shapes;
			this->mesh->getAllShapes(shapes);
			for (auto& shape : shapes)
			{
				this->instances[shape->name] = CREATE_CLASS(MeshShapeInstance, shape);
			}
		}
	}

	void populateShape(
		tinyobj::shape_t& obj_shape,
		Mesh::MeshShapes& shapes,
		Mesh::MeshMaterials& materials)
	{
		GeometryDataPtr data = CREATE_CLASS(GeometryData);
		MeshAABB aabb;
		bool hasNormals = obj_shape.mesh.normals.size() > 0 || kForceNormalBuffer;

		uint32 offset = 0;
		data->decl.addAttrib(AttribPosition, { AttribPosition, TypeFloat, 3, 0 });
		offset += sizeof(vec3);

		if (obj_shape.mesh.texcoords.size() > 0 || kForceUVBuffer)
		{
			data->decl.addAttrib(AttribTexCoord0, { AttribTexCoord0, TypeFloat, 2, offset });
			offset += sizeof(vec2);
		}

		if (hasNormals)
		{
			data->decl.addAttrib(AttribNormal, { AttribNormal, TypeFloat, 3, offset });
			offset += sizeof(vec3);
		}

		size_t numVertices = obj_shape.mesh.positions.size() / 3;
		data->vertexSize = numVertices;
		data->indexSize = obj_shape.mesh.indices.size();

		uchar* vertex_data = (uchar*) new uchar[data->decl.getStride() * data->vertexSize];
		const Attribute* posAttrib = data->decl.getAttrib(AttribPosition);
		if (posAttrib)
		{
			for (size_t p = 0; p < obj_shape.mesh.positions.size() / 3; ++p)
			{
				vec3* pos = data->decl.getAttributePointerAtIndex<vec3>(vertex_data, AttributeType::AttribPosition, p);
				(*pos) = vec3(
					obj_shape.mesh.positions[3 * p + 0],
					obj_shape.mesh.positions[3 * p + 1],
					obj_shape.mesh.positions[3 * p + 2]) * kMeshOBJScale;

				aabb.evailuate(*pos);
			}
		}

		const Attribute* texAttrib = data->decl.getAttrib(AttribTexCoord0);
		if (texAttrib)
		{
			if (obj_shape.mesh.texcoords.size() == 0 && kForceUVBuffer)
			{
				for (size_t t = 0; t < obj_shape.mesh.positions.size() / 3; t++)
				{
					vec2* tex = data->decl.getAttributePointerAtIndex<vec2>(vertex_data, AttributeType::AttribTexCoord0, t);
					(*tex) = vec2(0.0f, 0.0f);
				}
			}
			else
			{
				//assert(obj_shape.mesh.texcoords.size() / 2 == numVertices);
				for (size_t t = 0; t < obj_shape.mesh.texcoords.size() / 2; ++t)
				{
					vec2* tex = data->decl.getAttributePointerAtIndex<vec2>(vertex_data, AttributeType::AttribTexCoord0, t);
					(*tex) = vec2(
						obj_shape.mesh.texcoords[2 * t + 0],
						1.0f - obj_shape.mesh.texcoords[2 * t + 1]);
				}
			}
		}

		const Attribute* nmlAttrib = data->decl.getAttrib(AttribNormal);
		if (nmlAttrib)
		{

			//assert(obj_shape.mesh.normals.size() / 3 == numVertices);
			for (size_t n = 0; n < obj_shape.mesh.normals.size() / 3; ++n)
			{
				vec3* nml = data->decl.getAttributePointerAtIndex<vec3>(vertex_data, AttributeType::AttribNormal, n);
				(*nml) = vec3(
					obj_shape.mesh.normals[3 * n + 0],
					obj_shape.mesh.normals[3 * n + 1],
					obj_shape.mesh.normals[3 * n + 2]);
			}
		}

		// copy indices
		uint16* index_data = new uint16[data->indexSize];

		int32 matIndex = obj_shape.mesh.material_ids[0];
		size_t elementCount = 0;
		offset = 0;

		MeshMaterialInstance::GeometryType geomType = MeshMaterialInstance::GeometryTypeNone;
		if (hasNormals)
		{
			geomType = MeshMaterialInstance::GeometryTypeNormals;
		}


		std::vector<MeshMaterialInstancePtr> materialInstances;
		MeshMaterialPtr material = (matIndex >= 0 && matIndex < int32(materials.size())) ? materials[matIndex] : MeshMaterial::kDefaultMeshMaterial;
		MeshMaterialInstancePtr materialInstance = CREATE_CLASS(MeshMaterialInstance, geomType, material);
		materialInstances.push_back(materialInstance);

		size_t total_indices = obj_shape.mesh.indices.size();
		size_t total_triangles = total_indices / 3;

		for (size_t index = 0; index < total_triangles; index++)
		{
			index_data[(index * 3) + 0] = (uint16)obj_shape.mesh.indices[(index * 3) + 0];
			index_data[(index * 3) + 1] = (uint16)obj_shape.mesh.indices[(index * 3) + 1];
			index_data[(index * 3) + 2] = (uint16)obj_shape.mesh.indices[(index * 3) + 2];

			int32 newMatIndex = obj_shape.mesh.material_ids[index];
			if (newMatIndex != matIndex)
			{
				DrawCallPtr dc = CREATE_CLASS(DrawCall);
				dc->offset = static_cast<uint32>(offset) * 3;
				dc->type = DrawTriangles;
				dc->indexType = TypeUnsignedShort;
				dc->shaderHandle = RenderInterface::kDefaultColorShader;
				dc->textures[0] = RenderInterface::kWhiteTexture;
				dc->color = ColorB::White;
				dc->depthTest = true;
				dc->depthFunc = DepthLess;
				dc->count = static_cast<uint32>(elementCount) * 3;
                dc->frontFace = FrontFaceCCW;
#if defined(CS_METAL)
                dc->cullFace = CullNone;
#endif
                
				dc->uniformCallback = std::bind(&MeshMaterialInstance::onBindMesh, materialInstance);
				materialInstance->populate(dc);
                
				data->drawCalls.push_back(dc);

				offset += elementCount;
				matIndex = newMatIndex;
				elementCount = 0;
				material = (matIndex >= 0 && matIndex < materials.size()) ? materials[matIndex] : MeshMaterial::kDefaultMeshMaterial;
				materialInstance = CREATE_CLASS(MeshMaterialInstance, geomType, material);
				materialInstances.push_back(materialInstance);

			}

			elementCount++;

		}

		if (elementCount > 0)
		{
			DrawCallPtr dc = CREATE_CLASS(DrawCall);
			dc->offset = static_cast<uint32>(offset) * 3;
			dc->type = DrawTriangles;
			dc->indexType = TypeUnsignedShort;
			dc->shaderHandle = RenderInterface::kDefaultColorShader;
			dc->textures[0] = RenderInterface::kWhiteTexture;
			dc->color = ColorB::White;
			dc->depthTest = true;
			dc->depthFunc = DepthLess;
			dc->count = static_cast<uint32>(elementCount) * 3;
			dc->uniformCallback = std::bind(&MeshMaterialInstance::onBindMesh, materialInstance);
            dc->frontFace = FrontFaceCCW;
#if defined(CS_METAL)
            dc->cullFace = CullNone;
#endif
            
			materialInstance->populate(dc);
			data->drawCalls.push_back(dc);

		}

		GeometryPtr geom = CREATE_CLASS(Geometry, data, vertex_data, (uchar*)index_data, false);
		MeshShapePtr shape = CREATE_CLASS(MeshShape, obj_shape.name, geom, aabb, materialInstances);

		shapes.push_back(shape);
	}

	void populateShapeTest(tinyobj::shape_t& obj_shape, Mesh::MeshShapes& shapes, Mesh::MeshMaterials& materials)
	{
		/*
		GeometryDataPtr data = CREATE_CLASS(GeometryData);
		MeshAABB aabb;

		const static float kTestBuffer[] =
		{
			-100.0f, -100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			-100.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			100.0f, 100.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			100.0f, -100.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		};

		size_t offset = 0;
		offset = data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		offset = data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, offset });
		offset = data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, offset });

		data->vertexSize = 4;
		data->indexSize = 6;
		GeometryPtr geom = CREATE_CLASS(Geometry, data, (uchar*)kTestBuffer, (uchar*)kStaticQuadIndices, false);

		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->offset = 0;
		dc->type = DrawTriangles;
		dc->indexType = TypeUnsignedShort;
		dc->shaderHandle = RenderInterface::kDefaultTextureColorShader;
		dc->textures[0] = RenderInterface::kWhiteTexture;
		dc->color = ColorB::White;
		dc->count = 6;
		dc->depthTest = false;
		dc->depthFunc = DepthLess;

		data->drawCalls.push_back(dc);

		MeshShapePtr shape = CREATE_CLASS(MeshShape, "TestShape", geom, aabb);
		shapes.push_back(shape);
		*/
	}

	void populateMaterial(Mesh::MeshMaterials& materials, tinyobj::material_t& obj_material)
	{
		struct local
		{
			static void toColorB(float* values, ColorB& color, float32 scale = 1.0f)
			{
				color = ColorB(uchar(values[0] * 255 * scale), uchar(values[1] * 255 * scale), uchar(values[2] * 255 * scale), 255);
			}
		};
		struct MaterialAssoc
		{
			const char* name;
			TextureStage stage;
		};

		const MaterialAssoc kAssoc[] =
		{
			{
				obj_material.ambient_texname.c_str(),
				TextureStageAmbient
			},
			{
				obj_material.diffuse_texname.c_str(),
				TextureStageDiffuse
			},
			{
				obj_material.specular_texname.c_str(),
				TextureStageSpecular
			},
			{
				obj_material.specular_highlight_texname.c_str(),
				TextureStageSpecularHighlight
			},
		};

		MeshMaterialPtr material = CREATE_CLASS(MeshMaterial, obj_material.name);
		const size_t kAssocSize = sizeof(kAssoc) / sizeof(kAssoc[0]);
		for (size_t i = 0; i < kAssocSize; ++i)
		{
			const MaterialAssoc& assoc = kAssoc[i];
			if (strlen(assoc.name) > 0)
			{
				material->addTexture(assoc.name, assoc.stage);
			}
		}

		local::toColorB(obj_material.diffuse, material->diffuse);
		local::toColorB(obj_material.specular, material->specular);
		local::toColorB(obj_material.diffuse, material->ambient, 0.5f);
		
		material->shininess = obj_material.shininess;

		material->print();
		materials.push_back(material);
	}

	void populateGeometry(
		Mesh::MeshShapes& shapes,
		Mesh::MeshMaterials& materials,
		std::vector<tinyobj::shape_t>& obj_shapes,
		std::vector<tinyobj::material_t>& obj_materials)
	{

		for (size_t i = 0; i < obj_materials.size(); i++)
		{
			populateMaterial(materials, obj_materials[i]);
		}

		for (size_t i = 0; i < obj_shapes.size(); i++)
		{
			tinyobj::shape_t& shape = obj_shapes[i];
			populateShape(shape, shapes, materials);
		}
	}

	MeshMaterialInstance::MeshMaterialInstance(GeometryType gt, MeshMaterialPtr mat)
		: geomType(gt)
		, material(mat)
	{
		switch (geomType)
		{
			case GeometryTypeNone:
				this->matType = (this->material->textures.size() > 0) ? MeshMaterialTypeSolidTexture : MeshMaterialTypeSolidColor;
				break;
			case GeometryTypeNormals:
				this->matType = (this->material->textures.size() > 0) ? MeshMaterialTypePhongTexture : MeshMaterialTypePhong;
				break;
			default:
				assert(false);
		}
	}

	void MeshMaterialInstance::initUniforms()
	{
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "diffuse_color", UniformVec4));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "ambient_color", UniformVec4));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "specular_color", UniformVec4));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "shininess", UniformFloat));
		
	}


	void MeshMaterialInstance::addUniforms(ShaderParams& params, GeometryType type)
	{
		if (type == GeometryType::GeometryTypeNormals)
		{
			params.addUniform(SharedUniform::getInstance().getUniform("diffuse_color"));
			params.addUniform(SharedUniform::getInstance().getUniform("ambient_color"));
			params.addUniform(SharedUniform::getInstance().getUniform("specular_color"));
			params.addUniform(SharedUniform::getInstance().getUniform("shininess"));

		}
	}

	void MeshMaterialInstance::onBindMesh()
	{
		switch (this->matType)
		{

			case MeshMaterialTypePhong:
			case MeshMaterialTypePhongTexture:
			{
				cs::UniformPtr diffuseColorUniform = SharedUniform::getInstance().getUniform("diffuse_color");
				if (diffuseColorUniform.get())
				{
					diffuseColorUniform->setValue(this->material->diffuse);
				}
				cs::UniformPtr ambientColorUniform = SharedUniform::getInstance().getUniform("ambient_color");
				if (ambientColorUniform.get())
				{
					ambientColorUniform->setValue(this->material->ambient);
				}
				cs::UniformPtr specularColorUniform = SharedUniform::getInstance().getUniform("specular_color");
				if (specularColorUniform.get())
				{
					specularColorUniform->setValue(this->material->specular);
				}
				cs::UniformPtr shininessUniform = SharedUniform::getInstance().getUniform("shininess");
				if (shininessUniform.get())
				{
					shininessUniform->setValue(32.0f);
					// shininessUniform->setValue(this->material->shininess);
				}
			} break;
		}
	}

	void MeshMaterial::addTexture(const std::string& fileName, TextureStage stage)
	{
		assert(this->textures.find((uint16)stage) == this->textures.end());
		this->textures[(uint16)stage] = CREATE_CLASS(TextureHandle, fileName);
	}

	void MeshMaterial::print()
	{
		for (auto it : this->textures)
		{
			log::info(kTextureStageName[it.first], " - ", (it.second->hasTexture()) ? it.second->getTextureName() : "Error");
		}
	}

	void MeshMaterialInstance::populate(DrawCallPtr& draw)
	{
		switch (this->matType)
		{
			case MeshMaterialTypeSolidColor:
				draw->shaderHandle = RenderInterface::kDefaultColorShader;
				break;
			case MeshMaterialTypePhong:
				draw->shaderHandle = CREATE_CLASS(ShaderHandle, RenderInterface::kVertexPhongLit);
				break;
			case MeshMaterialTypePhongTexture:
				draw->shaderHandle = CREATE_CLASS(ShaderHandle, RenderInterface::kVertexPhongLitTexture);
				for (auto& it : this->material->textures)
				{
					draw->textures[it.first] = it.second;
				}
				break;
			default:
				assert(false);
		}
	}


	MeshHandle::MeshHandle(MeshPtr& m)
		: mesh(m)
	{
		this->onMeshChanged();
	}

	MeshHandle::MeshHandle(const std::string& meshName)
	{
		this->mesh = std::static_pointer_cast<Mesh>(ResourceFactory::getInstance()->loadResource<Mesh>(meshName));
		this->onMeshChanged();
	}

	MeshShapeInstancePtr MeshHandle::getShapeInstanceAtIndex(int32 index)
	{
		MeshShapeInstanceMap::iterator it = this->instances.begin();
		std::advance(it, index);
		return it->second;
	}

	Mesh::Mesh(const std::string& fileName, const std::string& filePath)
		: Resource(fileName)
	{
		this->load(fileName, filePath);
		EngineStats::incrementStat(StatTypeMesh);
	}

	Mesh::~Mesh()
	{
		EngineStats::decrementStat(StatTypeMesh);
	}

	bool Mesh::load(const std::string& fileName, const std::string& filePath)
	{
        char s = FileManager::getInstance()->separator();
        size_t pos = filePath.find_last_of(s);
        if (pos == std::string::npos)
        {
            log::error("material path malformed!");
        }
		std::string matPath = filePath.substr(0, pos + 1);
        std::vector<tinyobj::shape_t> obj_shapes;
		std::vector<tinyobj::material_t> obj_materials;

		std::string err;
		bool ret = tinyobj::LoadObj(obj_shapes, obj_materials, err, filePath.c_str(), matPath.c_str());
		if (!err.empty()) 
		{
			log::error("TinyObj: ", err);
		}

		if (ret)
		{
			Mesh::MeshShapes shapes_vec;
			populateGeometry(shapes_vec, this->materials, obj_shapes, obj_materials);

			// setup AABB bounds
			for (auto it : shapes_vec)
			{
				MeshShapePtr& shape = it;
				this->aabb.evailuate(shape->aabb.mmax);
				this->aabb.evailuate(shape->aabb.mmin);

				std::string shapeName = to_lowercase(shape->name);
				int32 ctr = 1;
				while (this->shapes.find(shapeName) != this->shapes.end())
				{
					log::info("Dupilicate shape ", shapeName, " found in ", fileName);
					std::stringstream str;
					str << shape->name << ctr++;
					shapeName = str.str();
				}
				this->shapes[shapeName] = shape;
			}
			return true;
		}

		log::error("Error - could not load Mesh ", fileName);
		return false;
	}

	void Mesh::draw()
	{
		for (auto it : this->shapes)
		{
			GeometryPtr& geom = it.second->geom;
			geom->draw(nullptr);
		}
	}

	void Mesh::queueGeometry(RenderTraversal traversal, DisplayListNode& display_node)
	{
		for (auto it : this->shapes)
		{
			GeometryPtr& geom = it.second->geom;
			display_node.geomList.push_back(geom);
		}
	}

	void MeshHandle::draw()
	{
		if (this->mesh)
		{
			this->mesh->draw();
		}
	}

	void MeshHandle::setOverride(const std::string& meshName, const std::string& shapeName)
	{
		MeshPtr mesh = std::static_pointer_cast<Mesh>(ResourceFactory::getInstance()->loadResource<Mesh>(meshName));
		if (mesh.get())
		{
			MeshShapePtr shape = mesh->getShapeByName(shapeName);
			if (shape.get())
			{
				MeshShapeInstanceMap::iterator it = this->instances.find(shapeName);
				if (it != this->instances.end())
				{
					this->instances[shapeName] = CREATE_CLASS(MeshShapeInstance, shape);
				}
			}
			else
			{
				log::error("Mesh does not contain a shape called ", shapeName);
			}
		}
		else
		{
			log::error("Cannot find mesh ", meshName);
		}
	}

	void MeshHandle::addShape(const std::string& meshName, const std::string& shapeName, const std::string& newShapeName, vec3 offset)
	{
		MeshPtr mesh = std::static_pointer_cast<Mesh>(ResourceFactory::getInstance()->loadResource<Mesh>(meshName));
		if (mesh.get())
		{
			MeshShapePtr shape = mesh->getShapeByName(shapeName);
			if (shape.get())
			{
				MeshShapeInstanceMap::iterator it = this->instances.find(newShapeName);
				if (it != this->instances.end())
				{
					log::error("Shape with this name already exists: ", newShapeName);
					return;
				}
				this->instances[newShapeName] = CREATE_CLASS(MeshShapeInstance, shape, offset);
			}
			else
			{
				log::error("Mesh does not contain a shape called ", shapeName);
			}
		}
		else
		{
			log::error("Cannot find mesh ", meshName);
		}
	}

	void MeshHandle::queueGeometry(RenderTraversal traversal, DisplayListNode& display_node)
	{
		for (auto& it : this->instances)
		{
			GeometryPtr& geom = it.second->shape->geom;
			
			display_node.geomList.push_back(DisplayListGeom());
			DisplayListGeom& node = display_node.geomList.back();
			node.geom = geom;
			node.overrides = it.second->overrides;
		}
	}

	void Mesh::test(const std::string& fullPath, const std::string& matPath)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;


		std::string err;
		bool ret = tinyobj::LoadObj(shapes, materials, err, fullPath.c_str(), matPath.c_str());

		if (!err.empty()) 
		{
			std::cerr << err << std::endl;
		}

		if (!ret)
		{
			exit(1);
		}

		std::cout << "# of shapes    : " << shapes.size() << std::endl;
		std::cout << "# of materials : " << materials.size() << std::endl;

		for (size_t i = 0; i < shapes.size(); i++)
		{
			printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
			printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
			printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
			assert((shapes[i].mesh.indices.size() % 3) == 0);
			for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++)
			{
				printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, shapes[i].mesh.indices[3 * f + 0], shapes[i].mesh.indices[3 * f + 1], shapes[i].mesh.indices[3 * f + 2], shapes[i].mesh.material_ids[f]);
			}

			printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
			assert((shapes[i].mesh.positions.size() % 3) == 0);
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++)
			{
				printf("  v[%ld] = (%f, %f, %f)\n", v,
					shapes[i].mesh.positions[3 * v + 0],
					shapes[i].mesh.positions[3 * v + 1],
					shapes[i].mesh.positions[3 * v + 2]);
			}
		}

		for (size_t i = 0; i < materials.size(); i++)
		{
			printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
			printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
			printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
			printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
			printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
			printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
			printf("  material.Ns = %f\n", materials[i].shininess);
			printf("  material.Ni = %f\n", materials[i].ior);
			printf("  material.dissolve = %f\n", materials[i].dissolve);
			printf("  material.illum = %d\n", materials[i].illum);
			printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
			printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
			printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
			printf("  material.map_Ns = %s\n", materials[i].specular_highlight_texname.c_str());
			std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
			std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
			for (; it != itEnd; it++)
			{
				printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
			}
			printf("\n");
		}
	}

	MeshShapePtr Mesh::getShapeByName(const std::string& shapeName)
	{
		MeshShapeMap::iterator it = this->shapes.find(to_lowercase(shapeName));
		if (it != this->shapes.end())
			return it->second;

		return MeshShapePtr();
	}

	size_t Mesh::getAllShapes(MeshShapes& shape_vec)
	{
		for (auto it : this->shapes)
			shape_vec.push_back(it.second);
		return shape_vec.size();
	}

	void MeshHandleVolume::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->meshHandle)
		{
			this->extractMeshInfo();
		}
		BASECLASS::onPostLoad(flags);
	}

	MeshHandleVolume::MeshHandleVolume()
		: meshHandle(CREATE_CLASS(MeshHandle, RenderInterface::kCubeMesh))
		, collisionObject("")
		, useAllMeshShapes(true)
	{
		this->extractMeshInfo();
	}

	void MeshHandleVolume::getMeshShapes(std::vector<MeshShapePtr>& geomToUse)
	{
		assert(this->meshHandle);
		MeshPtr& mesh = this->meshHandle->getMesh();
		if (!mesh)
		{
			log::error("Bad mesh resource for PolygonListVolume!");
			return;
		}

		if (this->collisionObject.length() > 0)
		{
			MeshShapePtr collisionMesh = mesh->getShapeByName(this->collisionObject);
			if (!collisionMesh)
			{
				log::error("No part named ", this->collisionObject, " in mesh ", mesh->getName());
				return;
			}
			geomToUse.push_back(collisionMesh);
		}
		else if (this->useAllMeshShapes)
		{
			mesh->getAllShapes(geomToUse);
		}
	}

	void MeshHandleVolume::extractMeshInfo()
	{

		assert(this->meshHandle);
		MeshPtr& mesh = this->meshHandle->getMesh();
		if (!mesh)
		{
			log::error("Bad mesh resource for PolygonListVolume!");
			return;
		}

		std::vector<MeshShapePtr> geomToUse;
		this->getMeshShapes(geomToUse);

		this->positions.clear();
		this->index.clear();

		uint16 offset = 0;
		for (auto it : geomToUse)
		{
			MeshShapePtr& shape = it;
			GeometryPtr& geom = shape->geom;

			VertexDeclaration& decl = geom->getGeometryData()->decl;
			const Attribute* posAttrib = decl.getAttrib(AttribPosition);
			if (!posAttrib)
			{
				log::error("No positions in Mesh ", mesh->getName(), " - shape ", shape->name);
				continue;
			}

			size_t numVertices = geom->getNumVertices();
			uchar* vbData = geom->getVertexBufferStagingData();
			std::vector<vec3> tmpPositions;
			for (size_t i = 0; i < numVertices; i++)
			{
				vec3* pos = decl.getAttributePointerAtIndex<vec3>(vbData, AttributeType::AttribPosition, i);
				tmpPositions.push_back(*pos);
			}

			this->positions.insert(this->positions.end(), tmpPositions.begin(), tmpPositions.end());
			uint16 numIndices = static_cast<uint16>(geom->getNumIndices());
			uint16* ibData = (uint16*)geom->getIndexBufferStagingData();
			for (uint16 i = 0; i < numIndices / 3; i++)
			{
				uint16 i0 = offset + ibData[(i * 3) + 0];
				uint16 i1 = offset + ibData[(i * 3) + 1];
				uint16 i2 = offset + ibData[(i * 3) + 2];

				// first segment
				this->index.push_back(i0);
				this->index.push_back(i1);

				// second segment
				this->index.push_back(i1);
				this->index.push_back(i2);

				// third segment
				this->index.push_back(i2);
				this->index.push_back(i0);
			}

			offset += uint16(tmpPositions.size());
		}
	}
    
    template <>
    std::string SerializableHandle<Mesh>::getExtension()
    {
        return "obj";
    }
    
    template <>
    std::string SerializableHandle<Mesh>::getDescription()
    {
        return "CSE Mesh";
    }
}
