#pragma once

#include "ClassDef.h"

#include "global/Resource.h"

#include "gfx/Geometry.h"
#include "gfx/GeometryData.h"
#include "gfx/DisplayList.h"
#include "geom/Volume.h"
#include "global/SerializableHandle.h"

#include <float.h>

namespace cs
{
	enum MeshMaterialType
	{
		MeshMaterialTypeNone = -1,
		MeshMaterialTypeSolidColor,
		MeshMaterialTypeSolidTexture,
		MeshMaterialTypePhong,
		MeshMaterialTypePhongTexture,
		MeshMaterialTypeMAX
	};

	struct MeshAABB
	{

		MeshAABB()
			: mmin(vec3( FLT_MAX,  FLT_MAX,  FLT_MAX))
			, mmax(vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX))
		{

		}

		MeshAABB(const MeshAABB& rhs)
		{
			this->mmin = rhs.mmin;
			this->mmax = rhs.mmax;
		}

		vec3 getCenter() const
		{
			return this->mmin + ((this->mmax - this->mmin) * 0.5f);
		}

		inline void evailuate(const vec3& pos)
		{
			this->mmin.x = (pos.x < this->mmin.x) ? pos.x : this->mmin.x;
			this->mmin.y = (pos.y < this->mmin.y) ? pos.y : this->mmin.y;
			this->mmin.z = (pos.z < this->mmin.z) ? pos.z : this->mmin.z;
			this->mmax.x = (pos.x > this->mmax.x) ? pos.x : this->mmax.x;
			this->mmax.y = (pos.y > this->mmax.y) ? pos.y : this->mmax.y;
			this->mmax.z = (pos.z > this->mmax.z) ? pos.z : this->mmax.z;
		}

		vec3 mmin;
		vec3 mmax;
	};

	CLASS_DEFINITION(MeshMaterial)
	public:

		typedef std::map<uint16, TextureHandlePtr> MaterialTextures;
		typedef std::map<uint16, ColorB> MaterialColors;

		MeshMaterial(const std::string& n)
			: name(n)
			, diffuse(ColorB::White)
			, specular(ColorB::White)
			, ambient(ColorB::Black)
			, shininess(8.0f)
		{ }

		static MeshMaterialPtr kDefaultMeshMaterial;

		void addTexture(const std::string& fileName, TextureStage stage = TextureStageDiffuse);
		void print();

		std::string name;
		MaterialTextures textures;
		
		ColorB diffuse;
		ColorB specular;
		ColorB ambient;
		float32 shininess;
	};

	CLASS_DEFINITION(MeshMaterialInstance)
	public:
		enum GeometryType
		{
			GeometryTypeNone = -1,
			GeometryTypeNormals
		};

		static void addUniforms(ShaderParams& params, GeometryType geomType);
		static void initUniforms();

		MeshMaterialInstance(GeometryType gt, MeshMaterialPtr mat);

		void populate(DrawCallPtr& draw);
		void onBindMesh();

		MeshMaterialType matType;
		GeometryType geomType;
		MeshMaterialPtr material;
	};

	CLASS_DEFINITION(MeshShape)
	public:
		MeshShape(const std::string& n) 
			: name(n)
			, geom(nullptr) 
		{ }

		MeshShape(const MeshShape& rhs)
			: name(rhs.name)
			, geom(rhs.geom)
			, aabb(rhs.aabb)
			, materialInstances(rhs.materialInstances)
		{ }

		MeshShape(const std::string& n, GeometryPtr& g, MeshAABB& meshAABB, std::vector<MeshMaterialInstancePtr>& matInst)
			: name(n)
			, geom(g)
			, aabb(meshAABB)
			, materialInstances(matInst)
		{ }
		
		vec3 getCenter() const { return this->aabb.getCenter(); }

		std::string name;
		MeshAABB aabb;
		GeometryPtr geom;
		std::vector<MeshMaterialInstancePtr> materialInstances;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(Mesh, Resource)
	public:

		typedef std::vector<MeshShapePtr> MeshShapes;
		typedef std::map<std::string, MeshShapePtr> MeshShapeMap;
		typedef std::vector<MeshMaterialPtr> MeshMaterials;

		Mesh() : Resource("Error") { }
		Mesh(const std::string& fileName, const std::string& filePath);
		virtual ~Mesh();

		const MeshAABB& getAABB() const { return this->aabb; }

		void draw();
		void queueGeometry(RenderTraversal traversal, DisplayListNode& display_node);

		MeshShapePtr getShapeByName(const std::string& shapeName);
		size_t getAllShapes(MeshShapes& shape_vec);

	private:

		void init();
		bool load(const std::string& fileName, const std::string& filePath);
		
		MeshShapeMap shapes;
		MeshMaterials materials;
		MeshAABB aabb;

		static void test(const std::string& filePath, const std::string& matPath);

	};

	class MeshHandle;

	CLASS_DEFINITION_REFLECT(MeshShapeInstance)
	public:
		MeshShapeInstance() { }
		MeshShapeInstance(MeshShapePtr& s, vec3 off = kZero3)
			: shape(CREATE_CLASS(MeshShape, *(s.get())))
			, offset(off)
		{ }

		MeshMaterialType getMeshMaterialType() const;

		MeshShapePtr shape;
		vec3 offset;

		void overrideDrawCalls();

		void addOverrideShader(const std::string& shaderName, ShaderHandlePtr& shader);
		void addTexture(const std::string& textureName, TextureStage stage);

		DrawCallOverridesPtr overrides;
	};

	typedef std::map<std::string, MeshShapeInstancePtr> MeshShapeInstanceMap;

	CLASS_DEFINITION_REFLECT(MeshHandle)
	public:

		MeshHandle(MeshPtr& m);
		MeshHandle(const std::string& meshName);

		void draw();
		void queueGeometry(RenderTraversal traversal, DisplayListNode& display_node);

		void setOverride(const std::string& meshName, const std::string& shapeName);
		void addShape(const std::string& meshName, const std::string& shapeName, const std::string& newShapeName, vec3 offset);

		MeshPtr& getMesh() { return this->mesh; }

		Event onChanged;

		void onMeshChanged();

		void clear()
		{
			this->mesh = nullptr;
			this->onChanged.invoke();
		}

		int32 getNumShapeInstances() const { return int32(this->instances.size()); }
		MeshShapeInstancePtr getShapeInstanceAtIndex(int32 index);

	private:

		MeshHandle() { }

		MeshPtr mesh;
		MeshShapeInstanceMap instances;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(MeshHandleVolume, PolygonListVolume)
	public:
		MeshHandleVolume();

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual size_t getPolygons(B2PolygonList& polygons, const size_t& max_verts_per_poly);

	protected:

		void getMeshShapes(std::vector<MeshShapePtr>& geomToUse);
		void extractMeshInfo();

		MeshHandlePtr meshHandle;
		std::string collisionObject;
		bool useAllMeshShapes;
	};

	template <>
    std::string SerializableHandle<Mesh>::getExtension();

	template <>
    std::string SerializableHandle<Mesh>::getDescription();
}
