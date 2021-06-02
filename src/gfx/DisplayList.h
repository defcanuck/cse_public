#pragma once

#include "ClassDef.h"

#include "global/BitMask.h"
#include "gfx/Geometry.h"
#include "scene/Camera.h"

#include "global/LinkedList.h"
#include "gfx/RenderTexture.h"

namespace cs
{

	CLASS_DEFINITION_REFLECT(RenderTraversalBase)
	public:
		virtual RenderTraversal getType() { return RenderTraversalNone; }
		static RenderTraversalBasePtr getTypePtr(RenderTraversal traversal);
	};

	CLASS_DEFINITION_DERIVED_REFLECT(RenderTraversalMainPass, RenderTraversalBase)
	public:
		virtual RenderTraversal getType() { return RenderTraversalMain; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(RenderTraversalShadowPass, RenderTraversalBase)
	public:
		virtual RenderTraversal getType() { return RenderTraversalShadow; }
	};

	class Scene;

	namespace DisplayListUtil
	{
		struct DrawParams
		{
			DrawParams()
				: tint(ColorF::White)
				, scene(nullptr)
			{ }

			ColorF tint;
			Scene* scene;
		};
	}

	struct DisplayListGeom
	{
		DisplayListGeom()
			: geom(nullptr)
			, drawIndex(-1)
		{ }

		DisplayListGeom(const GeometryPtr& g, int32 index = -1)
			: geom(g)
			, drawIndex(index)
		{ }

		DisplayListGeom(const DisplayListGeom& rhs)
			: geom(rhs.geom)
			, drawIndex(rhs.drawIndex)
			, overrides(rhs.overrides)
		{ }

		void operator=(const DisplayListGeom& rhs)
		{
			this->geom = rhs.geom;
			this->drawIndex = rhs.drawIndex;
			this->overrides = rhs.overrides;
		}

		GeometryPtr geom;
		int32 drawIndex;
		DrawCallOverridesPtr overrides;
	};

	struct DisplayListNode
	{
		DisplayListNode() 
			: tag("")
			, color(1.0f, 1.0f, 1.0f, 1.0f) 
			, layer(0)
			, flags(0)
		{ }

		DisplayListNode(const DisplayListNode& rhs)
			: tag(rhs.tag)
			, geomList(rhs.geomList)
			, mvp(rhs.mvp)
			, objectToWorld(rhs.objectToWorld)
			, color(rhs.color)
			, layer(rhs.layer)
			, flags(rhs.flags)
		{ }

		void operator=(const DisplayListNode& rhs)
		{
			this->tag = rhs.tag;
			this->geomList = rhs.geomList;
			this->mvp = rhs.mvp;
			this->objectToWorld = rhs.objectToWorld;
			this->color = rhs.color;
			this->layer = rhs.layer;
			this->flags = rhs.flags;
		}

		static void draw(const DisplayListNode& node, void* data);

		static bool sort(const DisplayListNode& a, const DisplayListNode& b)
		{
			return a.layer < b.layer;
		}

		std::string tag;
		mat4 mvp;
		mat4 objectToWorld;
		vec4 color;
		std::vector<DisplayListGeom> geomList;
		int32 layer;
		int32 flags;

		static UniformPtr objectToWorldMatrix;
		static UniformPtr modelViewProjectionMatrix;
		static UniformPtr globalColor;
	};

	struct DisplayListTraversal
	{
		struct PassParams
		{
			Dimensions targetSize;
		};

		CameraPtr camera;
		SharedLinkedList<DisplayListNode> nodes;
		RenderTraversal type;

		void draw(DisplayListUtil::DrawParams* params, const PassParams& passParams);

		static UniformPtr cameraPositionUniform;
		static UniformPtr cameraDirectionUniform;
		static UniformPtr viewportUniform;
		static UniformPtr viewportInverseUniform;
		static UniformPtr postPercentUniform;
	};

	class DisplayList;

	struct DisplayListPass
	{
		typedef std::function<void(Scene*)> displayCallbackFunc;

		DisplayListPass(const std::string& name, int32 ord)
			: passName(name)
			, target(nullptr)
            , zNear(0.01f)
            , zFar(1000.0f)
			, traversalOrder(ord)
			, preCallback(nullptr)
			, postCallback(nullptr)
		{

		}

		void draw(DisplayList* display_list, DisplayListUtil::DrawParams* params);

		std::string passName;
		uint32 traversalOrder;

		RenderTexturePtr target;
		RectI targetViewport;
        float32 zNear;
        float32 zFar;

		ColorF clearColor;
		std::vector<ClearMode> clearModes;

		RenderTraversalMask traversalMask;
		SharedLinkedList<DisplayListNode> nodes;

		displayCallbackFunc preCallback;
		displayCallbackFunc postCallback;

	};

	typedef std::shared_ptr<DisplayListPass> DisplayListPassPtr;
	typedef std::vector<DisplayListPassPtr> DisplayListPassList;

	class DisplayList
	{
	public:

		
		DisplayList();

		static void initUniforms();

		DisplayListPassPtr addPass(const std::string& passName, RenderTraversal traversal);
		DisplayListPassPtr addPass(const std::string& passName, uint32 order);

		void addCallback(const std::string& name, DisplayListPass::displayCallbackFunc func, bool pre = false);

		void draw(DisplayListUtil::DrawParams* params);

		DisplayListPassList passes;
		DisplayListTraversal traversals[RenderTraversalMAX];
		uint32 traversalCount[RenderTraversalMAX];

	};
}
