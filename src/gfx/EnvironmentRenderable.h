#pragma once

#include "ClassDef.h"

#include "gfx/Renderable.h"
#include "math/Rect.h"
#include "geom/Volume.h"

#include <set>

#include "serial/text/TextDeserialize.h"
#include "serial/text/TextSerialize.h"


namespace cs
{
	struct EnvTriangle
	{
		uint16 f0, f1, f2;

		inline void add(std::set<uint16>& index)
		{
			index.insert(f0);
			index.insert(f1);
			index.insert(f2);
		}
	};

	struct EnvVertex
	{
		vec3 position;
		vec2 uv0;
		vec2 uv1;
		ColorB color;
	};

	struct EnvQuad
	{
		EnvQuad()
			: botLeft(0)
			, topLeft(0)
			, topRight(0)
			, botRight(0)
			, t0(-1)
			, t1(-1)
			, parent(-1)
			, isVisible(true)
			, isFlipped(false)
		{ }

		uint16 botLeft;
		uint16 topLeft;
		uint16 topRight;
		uint16 botRight;

		int32 t0;
		int32 t1;
		int32 parent;

		bool isVisible;
		bool isFlipped;

		inline bool containsIndex(uint16 index) const
		{
			return
				this->botLeft == index ||
				this->topLeft == index ||
				this->topRight == index ||
				this->botRight == index;
		}

		inline bool containsVisibleIndex(uint16 index) const
		{
			return this->isVisible && this->containsIndex(index);
		}

		inline void getUniqueIndices(std::vector<uint16>& index)
		{
			// order matters
			if (t0 >= 0 && t1 >= 0)
			{
				index.push_back(this->botLeft);
				index.push_back(this->topLeft);
				index.push_back(this->topRight);
				index.push_back(this->botRight);
			}
			else if (t0 >= 0)
			{
				index.push_back(this->botLeft);
				index.push_back(this->topLeft);
				index.push_back(this->topRight);
			}
			else if (t1 >= 0)
			{
				index.push_back(this->botLeft);
				index.push_back(this->topRight);
				index.push_back(this->botRight);
			}
		}
	};


	typedef std::vector<EnvVertex> EnvVertexList;
	typedef std::vector<EnvTriangle> EnvTriangleList;
	typedef std::vector<EnvQuad> EnvEdgeList;
	typedef std::set<uint16> EnvBoundaryVertexIndex;
	typedef std::vector<uint16> EnvBoundaryVertexIndexSerialize;

	enum EnvironmentModificationType
	{
		EnvironmentModifyTypeNone = -1,
		EnvironmentModifyTypeVertex,
		EnvironmentModifyTypeFace,
		EnvironmentModifyTypeTriangle,
		//...
		EnvironmentModifyTypeMAX
	};

	CLASS_DEFINITION_REFLECT(EnvironmentModificationTypeDrop)
	public:
		EnvironmentModificationTypeDrop() { }
		virtual EnvironmentModificationType getType() const { return EnvironmentModifyTypeNone; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EnvironmentModificationTypeDropVertex, EnvironmentModificationTypeDrop)
	public:
		EnvironmentModificationTypeDropVertex() { }
		virtual EnvironmentModificationType getType() const { return EnvironmentModifyTypeVertex; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EnvironmentModificationTypeDropFace, EnvironmentModificationTypeDrop)
	public:
		EnvironmentModificationTypeDropFace() { }
		virtual EnvironmentModificationType getType() const { return EnvironmentModifyTypeFace; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EnvironmentModificationTypeDropTriangle, EnvironmentModificationTypeDrop)
	public:
		EnvironmentModificationTypeDropTriangle() { }
		virtual EnvironmentModificationType getType() const { return EnvironmentModifyTypeTriangle; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EnvironmentRenderable, Renderable)
	public:

		EnvironmentRenderable();
		virtual ~EnvironmentRenderable();

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual void draw() const;
		virtual void queueGeometry(RenderTraversal traversal, DisplayListNode& display_node);

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);

		void refresh();

		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);

		void onVertexChanged(vec3 new_pos, size_t index);
		const Vec3List& getBoundaryVertices() const { return this->boundary; }

		void refreshBoundaryVertices();

		bool subdivideFace(size_t index);
		bool removeFace(size_t index);

		bool removeTriangle(size_t index);

		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		void onModificationTypeChanged();

	private:
		void addFace(size_t top_left, size_t top_right, size_t bot_left, size_t bot_right, bool flip);
		void setEdgeParams(EnvVertex& vertex);
		void updateEdgeParams();
		void updateCenterParams();

		void initGeometry();
		void populateGeometry();
		void addVolumeCallbacks(size_t index, SelectableVolume& volume);

		void setShaderImpl();

		size_t getNumVertices() const;
		size_t getNumIndices() const;

		size_t getNumBaseVertices() const;
		size_t getNumBaseIndices() const;

		void adjustEdgeForRemovedFace(size_t face_index);
		void adjustEdgeForDuplicates();

		bool indexIsEdge(uint16 index) const;
		bool indexIsInVisibleQuad(uint16 index) const;

		ShaderHandlePtr shaderHandle;
		TextureHandlePtr mainTextureHandle;

		TextureHandlePtr edgeTextureHandle;
		
		ColorB edgeColor;
		ColorB centerColor;

		DynamicGeometryPtr geometry;
		DynamicGeometryPtr shadowGeometry;

		RectF size;
		RectF selectSize;
		SizeI subdivisions;
		SizeF stretch;
		float32 borderDimm;

		EnvVertexList positions;
		EnvTriangleList faces;
		EnvTriangleList stitches;
		EnvEdgeList edges;

		QuadVolumePtr volume;
		EnvironmentModificationTypeDropPtr modifyType;

		Vec3List boundary;
		EnvBoundaryVertexIndex boundaryIndex;
		EnvBoundaryVertexIndexSerialize boundaryIndexSerialize;

		bool dirty;
		size_t visibleIndices;
	};

	namespace text
	{
		template <>
		void serializePrim<EnvVertex>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<EnvTriangle>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<EnvQuad>(std::ostream& oss, RefVariant prim);

		template <>
		void deserializePrim<EnvVertex>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<EnvTriangle>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<EnvQuad>(JsonValue value, RefVariant prim);
	}
}