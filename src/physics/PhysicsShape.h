#pragma once

#include "math/GLM.h"
#include "math/Ray.h"

#include "global/Event.h"
#include "scene/Camera.h"
#include "gfx/DynamicGeometry.h"
#include "geom/Volume.h"

#include "physics/PhysicsGlobals.h"
#include <Box2D/Box2D.h>

#include "ClassDef.h"

namespace cs
{

	struct PhysicsShapeCollection
	{
		PhysicsShapeCollection() { }
		~PhysicsShapeCollection() { for (size_t i = 0; i < this->list.size(); i++) delete list[i]; }

		std::vector<b2Shape*> list;
	};

	CLASS_DEFINITION_REFLECT(PhysicsShape)
	public:

		PhysicsShape() { }
		virtual VolumePtr getVolume() { return std::shared_ptr<Volume>(nullptr); }
		virtual bool createPolygon(b2Body* body, PhysicsShapeCollection& shapes) { return false; }
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll) { }

		virtual void onChangedCallback();
		
		Event onChanged;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsShapeBox, PhysicsShape)
	public:
		PhysicsShapeBox() 
			: PhysicsShape()
			, dimm(0.0f, 0.0f)
			, quad(nullptr)
		{
			this->initVolume();
		}

		PhysicsShapeBox(const vec2& d) 
			: PhysicsShape()
			, dimm(d)
			, quad(nullptr)
		{
			this->initVolume();
		}

		PhysicsShapeBox(const PointF& p)
			: PhysicsShape()
			, dimm(p.x, p.y)
			, quad(nullptr)
		{
			this->initVolume();
		}

		PhysicsShapeBox(QuadVolumePtr& ptr)
			: PhysicsShape()
			, dimm(ptr->getRectRef().size.w, ptr->getRectRef().size.h)
			, quad(ptr)
		{
			this->initVolume();
		}

		virtual void onChangedCallback();
		virtual bool createPolygon(b2Body* body, PhysicsShapeCollection& shapes);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual VolumePtr getVolume();

	private:

		void initVolume();

		vec2 dimm;
		QuadVolumePtr quad;
		
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsShapeTriangle, PhysicsShape)
	public:
		PhysicsShapeTriangle()
			: PhysicsShape()
			, point0(0.0f, 0.0f)
			, point1(0.0f, 1.0f)
			, point2(1.0f, 0.0f)
		{
			this->initVolume();
		}

		PhysicsShapeTriangle(const vec2& p0, const vec2& p1, const vec2& p2)
			: PhysicsShape()
			, point0(p0)
			, point1(p1)
			, point2(p2)
		{
			this->initVolume();
		}

		virtual bool createPolygon(b2Body* body, PhysicsShapeCollection& shapes);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void onChangedCallback();
		virtual VolumePtr getVolume();

	private:

		vec2 point0;
		vec2 point1;
		vec2 point2;
		TriangleVolumePtr tri;

		void initVolume();
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsShapeCircle, PhysicsShape)
	public:
		PhysicsShapeCircle()
			: PhysicsShape()
			, radius(1.0f)
			, circle(nullptr)
		{
			this->initVolume();
		}

		PhysicsShapeCircle(float32 rad)
			: PhysicsShape()
			, radius(rad)
			, circle(nullptr)
		{
			this->initVolume();
		}

		PhysicsShapeCircle(CircleVolumePtr& ptr)
			: PhysicsShape()
			, radius(ptr->getCircle().radius)
			, circle(ptr)
		{
			this->initVolume();
		}

		virtual void onChangedCallback();
		virtual bool createPolygon(b2Body* body, PhysicsShapeCollection& shapes);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual VolumePtr getVolume();

	private:

		void initVolume();
		
		float32 radius;
		CircleVolumePtr circle;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsShapeChain, PhysicsShape)
	public:
		PhysicsShapeChain()
			: PhysicsShape()
			, chain(std::make_shared<ChainVolume>())
			, adj_value_x(0.01f)
			, adj_value_y(0.01f)
		{ }

		PhysicsShapeChain(ChainVolumePtr& ptr)
			: PhysicsShape()
			, chain(ptr)
			, adj_value_x(0.01f)
			, adj_value_y(0.01f)
		{
			
		}

		virtual VolumePtr getVolume();
		virtual bool createPolygon(b2Body* body, PhysicsShapeCollection& shapes);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

	private:

		void onAdjustChanged();

		ChainVolumePtr chain;
		float32 adj_value_x;
		float32 adj_value_y;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsShapeMesh, PhysicsShape)
	public:
		PhysicsShapeMesh()
			: PhysicsShape()
			, poly(std::make_shared<PolygonListVolume>())
			, adj_poly(nullptr)
			, adj_value_x(0.01f)
			, adj_value_y(0.01f)
		{ }

		PhysicsShapeMesh(PolygonListVolumePtr& ptr)
			: PhysicsShape()
			, poly(ptr)
			, adj_poly(nullptr)
			, adj_value_x(0.01f)
			, adj_value_y(0.01f)
		{ 
			this->adjust();
		}

		virtual VolumePtr getVolume();
		virtual bool createPolygon(b2Body* body, PhysicsShapeCollection& shapes);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

	private:

		void adjust();
		void onAdjustChanged();

		PolygonListVolumePtr poly;
		PolygonListVolumePtr adj_poly;

		float32 adj_value_x;
		float32 adj_value_y;
	};


	typedef std::vector<PhysicsShapePtr> PhysicsShapeList;
}