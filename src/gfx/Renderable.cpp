#include "PCH.h"

#include "gfx/Renderable.h"

namespace cs
{

	BEGIN_META_CLASS(Renderable)
		ADD_MEMBER(visible);
			SET_MEMBER_IGNORE_SERIALIZATION();
		ADD_MEMBER(layer);
			SET_MEMBER_DEFAULT(0);
			SET_MEMBER_MIN(-10);
			SET_MEMBER_MAX(10);
		ADD_MEMBER(traversalMask);
			ADD_COMBO_META_INTEGER(RenderTraversalShadow, "Depth");
			ADD_COMBO_META_INTEGER(RenderTraversalMain, "Main");
	END_META()

	Renderable::Renderable()
		: traversalMask(RenderTraversalMask::Flags({ RenderTraversalMain }))
		, color(vec4(1.0f, 1.0f, 1.0f, 1.0f))
		, visible(true)
		, layer(0)
	{

	}

	Renderable::Renderable(GeometryPtr& geom)
		: traversalMask(RenderTraversalMask::Flags({ RenderTraversalMain }))
		, color(vec4(1.0f, 1.0f, 1.0f, 1.0f))
		, visible(true)
		, layer(0)
	{
		this->addGeometry(geom);
	}

	Renderable::~Renderable()
	{
		
	}

	void Renderable::addGeometry(const GeometryPtr& geom)
	{
		geometry.push_back(geom);
	}

	void Renderable::process(float32 dt)
	{
		for (auto it : this->geometry) 
		{
			GeometryPtr& geo = it;
			if (!geo)
				continue;
			geo->update();
		}
	}

	void Renderable::draw() const
	{

		for (auto it : this->geometry) 
		{
			GeometryPtr& geo = it;
			if (!geo)
				continue;
			geo->draw(nullptr);
		}
	}

	void Renderable::queueGeometry(RenderTraversal traversal, DisplayListNode& display_node)
	{
		display_node.geomList.insert(display_node.geomList.end(), this->geometry.begin(), this->geometry.end());
	}
}
