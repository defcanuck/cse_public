#include "PCH.h"

#include "physics/PhysicsShape.h"
#include "gfx/Mesh.h"
#include "geom/Shape.h"

namespace cs
{

	BEGIN_META_CLASS(PhysicsShape)
		
	END_META()

	BEGIN_META_CLASS(PhysicsShapeBox)
		ADD_MEMBER(dimm);
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeBox::onChangedCallback);
	END_META()

	BEGIN_META_CLASS(PhysicsShapeCircle)
		ADD_MEMBER(radius);
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeCircle::onChangedCallback);
	END_META()

	BEGIN_META_CLASS(PhysicsShapeTriangle)
		ADD_MEMBER(point0);
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeTriangle::onChangedCallback);
		ADD_MEMBER(point1);
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeTriangle::onChangedCallback);
		ADD_MEMBER(point2);
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeTriangle::onChangedCallback);
	END_META()

	BEGIN_META_CLASS(PhysicsShapeChain)
		ADD_MEMBER(adj_value_x);
			SET_MEMBER_MIN(0.1f);
			SET_MEMBER_MAX(0.1f);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeChain::onAdjustChanged);

		ADD_MEMBER(adj_value_y);
			SET_MEMBER_MIN(0.1f);
			SET_MEMBER_MAX(0.1f);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeChain::onAdjustChanged);

			ADD_MEMBER_PTR(chain);
	END_META()

	BEGIN_META_CLASS(PhysicsShapeMesh)

		ADD_MEMBER(adj_value_x);
			SET_MEMBER_MIN(0.1f);
			SET_MEMBER_MAX(0.1f);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeMesh::onAdjustChanged);

		ADD_MEMBER(adj_value_y);
			SET_MEMBER_MIN(0.1f);
			SET_MEMBER_MAX(0.1f);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&PhysicsShapeMesh::onAdjustChanged);
	
		ADD_MEMBER_PTR(poly);
			ADD_COMBO_META_LABEL(PolygonListVolume, "Mesh Renderable");
			ADD_COMBO_META_LABEL(MeshHandleVolume, "Mesh Handle");
				SET_MEMBER_CALLBACK_POST(&PhysicsShape::onChangedCallback);

	END_META()

	VolumePtr PhysicsShapeBox::getVolume() 
	{ 
		if (!this->quad)
		{
			this->quad = CREATE_CLASS(QuadVolume, RectF(-this->dimm.x * 0.5f, -this->dimm.y * 0.5f, this->dimm.x, this->dimm.y));
		}

		return std::static_pointer_cast<Volume>(this->quad); 
	}

	bool PhysicsShapeBox::createPolygon(b2Body* body, PhysicsShapeCollection& shapes)
	{
		if (!this->quad)
			return false;

		b2PolygonShape* shape = new b2PolygonShape();
		RectF rect = this->getVolume()->getRect();
		float32 w = PhysicsConst::dimmToBox2D(dimm.x);
		float32 h = PhysicsConst::dimmToBox2D(dimm.y);
		shape->SetAsBox(w, h);
		shapes.list.push_back(shape);
		return true;
	}

	void PhysicsShapeBox::onPostLoad(const LoadFlagMask& flags)
	{
		this->initVolume();
	}

	void PhysicsShapeBox::initVolume()
	{
		this->quad = CREATE_CLASS(QuadVolume,
			this->dimm.x * -0.5f,
			this->dimm.y * -0.5f,
			this->dimm.x,
			this->dimm.y);
	}

	void PhysicsShapeTriangle::initVolume()
	{
		this->tri = CREATE_CLASS(TriangleVolume,
			vec3(this->point0, 0.0),
			vec3(this->point1, 0.0f),
			vec3(this->point2, 0.0f));
	}

	void PhysicsShapeTriangle::onChangedCallback()
	{
		this->initVolume();

		BASECLASS::onChangedCallback();
	}

	bool PhysicsShapeTriangle::createPolygon(b2Body* body, PhysicsShapeCollection& shapes)
	{
		b2PolygonShape* shape = new b2PolygonShape();
		b2Vec2 points[3];
		points[0] = PhysicsConst::worldToBox2D(this->point0);
		points[1] = PhysicsConst::worldToBox2D(this->point1);
		points[2] = PhysicsConst::worldToBox2D(this->point2);
		shape->Set(points, 3);
		shapes.list.push_back(shape);
		return true;
	}

	void PhysicsShapeTriangle::onPostLoad(const LoadFlagMask& flags)
	{
		this->initVolume();
	}

	VolumePtr PhysicsShapeTriangle::getVolume()
	{
		if (!this->tri.get())
		{
			this->initVolume();
		}

		return std::static_pointer_cast<Volume>(this->tri);
	}

	void PhysicsShapeCircle::onChangedCallback() 
	{
		this->initVolume();

		BASECLASS::onChangedCallback();
	}

	bool PhysicsShapeCircle::createPolygon(b2Body* body, PhysicsShapeCollection& shapes) 
	{ 
		if (!this->circle.get())
			return false;

		b2CircleShape* circleShape = new b2CircleShape;
		circleShape->m_p.Set(0, 0);
		circleShape->m_radius = PhysicsConst::worldToBox2D(this->radius);

		shapes.list.push_back(circleShape);
		return true;
	}

	void PhysicsShapeCircle::onPostLoad(const LoadFlagMask& flags) 
	{ 
		this->initVolume();
	}

	VolumePtr PhysicsShapeCircle::getVolume() 
	{ 
		if (!this->circle)
		{
			this->circle = CREATE_CLASS(CircleVolume, kZero3, this->radius);
		}

		return std::static_pointer_cast<Volume>(this->circle);
	}

	void PhysicsShapeCircle::initVolume() 
	{ 
		this->circle = CREATE_CLASS(CircleVolume, kZero3, this->radius);
	}

	VolumePtr PhysicsShapeChain::getVolume()
	{
		return std::static_pointer_cast<Volume>(this->chain);
	}

	bool PhysicsShapeChain::createPolygon(b2Body* body, PhysicsShapeCollection& shapes)
	{
		if (!this->chain.get())
			return false;

		const Vec3List& pos_list = this->chain->getPositionList();
		if (pos_list.size() < 2)
			return false;

		b2Vec2* box_verts = new b2Vec2[pos_list.size()];
		for (size_t i = 0; i < pos_list.size(); i++)
			box_verts[i] = PhysicsConst::worldToBox2D(pos_list[i].x, pos_list[i].y);

		b2ChainShape* chain_shape = new b2ChainShape();
		chain_shape->CreateLoop(box_verts, int32(pos_list.size()));
		delete[] box_verts;

		shapes.list.push_back(chain_shape);
		return true;
	}

	void PhysicsShapeChain::onPostLoad(const LoadFlagMask& flags)
	{

	}

	void PhysicsShapeChain::onAdjustChanged()
	{
		this->onChanged.invoke();
	}

	VolumePtr PhysicsShapeMesh::getVolume()
	{
		return std::static_pointer_cast<Volume>(this->adj_poly);
	}

	bool PhysicsShapeMesh::createPolygon(b2Body* body, PhysicsShapeCollection& shapes)
	{
		const int32 kMaxVerticesPerPolygon = 6;
		PolygonListVolumePtr poly_list = this->adj_poly;
		if (!poly_list.get())
			poly_list = this->poly;

		if (!poly_list.get())
			return false;

		PolygonListVolume::B2PolygonList polyList;
		poly_list->getPolygons(polyList, kMaxVerticesPerPolygon);
		for (size_t i = 0; i < polyList.size(); ++i)
		{
			if (polyList[i].count <= 0 || !polyList[i].isValid())
				continue;
			b2PolygonShape* shape = new b2PolygonShape();

			shape->Set(polyList[i].positions, int32(polyList[i].count));
			shapes.list.push_back(shape);
		}

		return true;
	}

	void PhysicsShapeMesh::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->poly)
		{
			this->adjust();
		}
	}

	void PhysicsShapeMesh::onAdjustChanged()
	{
		this->adjust();
		this->onChanged.invoke();
	}

	void PhysicsShapeMesh::adjust()
	{
		assert(this->poly.get());
		PolygonList polyList = this->poly->getPolygonList();

		for (size_t i = 0; i < polyList.size(); i++)
		{
			Polygon& poly = polyList[i];
			vec3 avg_pos(0.0f, 0.0f, 0.0f);
			for (size_t v = 0; v < poly.pos.size(); v++)
			{
				avg_pos += poly.pos[v];
			}

			// get the center of the poly, we want to contract inwards to this point
			avg_pos = avg_pos * 1.0f / float32(poly.pos.size());

			for (size_t v = 0; v < poly.pos.size(); v++)
			{
				vec3& pos = poly.pos[v];

				// adjust inwards by the vector into the center of the poly
				vec3 to_center = avg_pos - pos;
				glm::normalize(to_center);

				pos.x = pos.x + (to_center.x * this->adj_value_x);
				pos.y = pos.y + (to_center.y * this->adj_value_y);

			}
		}

		this->adj_poly = CREATE_CLASS(PolygonListVolume, polyList);
	}

	void PhysicsShapeBox::onChangedCallback()
	{
		this->initVolume();

		BASECLASS::onChangedCallback();
	}

	void PhysicsShape::onChangedCallback()
	{
		this->onChanged.invoke();
	}
}