#pragma once

#include "ClassDef.h"
#include "math/GLM.h"
#include "global/Event.h"
#include "geom/AABB.h"
#include "gfx/Types.h"
#include "gfx/Color.h"
#include "math/Circle.h"

#include <Box2D/Box2D.h>

#include <numeric>

namespace cs
{

	typedef std::vector<vec3> Vec3List;
	typedef std::vector<vec2> Vec2List;

	CLASS_DEFINITION_REFLECT(Polygon)
	public:
		
		typedef std::vector<vec3> PolygonPositionList;
		Polygon() { }

		std::vector<vec3> pos;
	};

	typedef std::vector<Polygon> PolygonList;

	inline size_t getPositions(const PolygonList& poly_list)
	{
		size_t count = 0;
		for (auto& it : poly_list) count += it.pos.size();
		return count;
	}

	inline void collapsePolys(std::vector<vec3>& pos_list, const PolygonList& poly_list)
	{
		for (auto& it : poly_list)
			pos_list.insert(pos_list.end(), it.pos.begin(), it.pos.end());
	}

	CLASS_DEFINITION_REFLECT(Volume)
	public:
		Volume() { }
		
		virtual DrawType getDrawType() const { return DrawLines; }
		virtual vec2 getRandomValue() { return vec2(); }

		virtual bool test(const vec3& point) const { return false; }
		virtual bool intersects(const Ray& ray, vec3& hit_pos) { return false; }
	
		virtual size_t getNumEdges() const { return 0; }
		virtual size_t getEdges(std::vector<uint16>& indices) { return 0; }

		virtual size_t getNumPositions() const { return 0; }
		virtual size_t getPositions(std::vector<vec3>& pos) { return 0; }

		virtual vec3 getCenter() const { return kZero3; }
		virtual void setCenter(const vec3& center) { }

		virtual RectF getRect() const
		{
			static RectF kDefaultRect;
			return kDefaultRect;
		}

	};

	CLASS_DEFINITION_DERIVED_REFLECT(LineVolume, Volume)
	public:
		LineVolume()
			: Volume()
			, start(kZero3)
			, end(kOne3)
		{ }

		LineVolume(const vec3& p0, const vec3& p1)
			: Volume()
			, start(p0)
			, end(p1)
		{ }

		virtual DrawType getDrawType() const { return DrawLines; }

		virtual size_t getNumEdges() const { return 2; }
		virtual size_t getEdges(std::vector<uint16>& indices)
		{
			indices.push_back(0);
			indices.push_back(1);
			return 2;
		}

		virtual size_t getNumPositions() const { return 2; }
		virtual size_t getPositions(std::vector<vec3>& pos)
		{
			pos.push_back(start);
			pos.push_back(end);
			return pos.size();
		}

		virtual vec3 getCenter() const
		{
			return (this->start + this->end) * 0.5f;
		}

		vec3 start;
		vec3 end;
	};
	
	CLASS_DEFINITION_DERIVED_REFLECT(QuadVolume, Volume)
	public:
		QuadVolume() 
			: Volume()
			, rect(kDefaultVolume) { }
		QuadVolume(const RectF& r)
			: Volume()
			, rect(r) { }
		QuadVolume(const vec2& pos, const vec2& dimm)
			: Volume()
			, rect(pos, dimm) { }
		QuadVolume(const float32& x, const float32& y, const float32& w, const float32& h)
			: Volume()
			, rect(x, y, w, h) { }

		virtual DrawType getDrawType() const { return DrawLineLoop; }

		virtual vec2 getRandomValue();
		virtual bool intersects(const Ray& ray, vec3& hit_pos);

		virtual size_t getNumEdges() const { return 4; }
		virtual size_t getEdges(std::vector<uint16>& indices);

		virtual size_t getNumPositions() const { return 4; }
		virtual size_t getPositions(std::vector<vec3>& pos);

		virtual RectF getRect() const { return this->rect; }
		const RectF& getRectRef() const { return this->rect; }

		static RectF kDefaultVolume;

		virtual vec3 getCenter() const 
		{
			PointF center = this->rect.getCenter();
			return vec3(center.x, center.y, 0.0f);
		}

		virtual void setCenter(const vec3& center)
		{
			this->rect.pos.x = center.x - this->rect.size.w / 2.0f;
			this->rect.pos.y = center.y - this->rect.size.h / 2.0f;
		}

	private:

		RectF rect;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(CircleVolume, Volume)
	public:
		CircleVolume()
			: Volume()
			, cir(kDefaultCircle) 
			, granularity(32) 
		{ }

		CircleVolume(const vec3& pos, float32 radius, int32 gran = 32)
			: Volume()
			, cir(Circle(radius, pos))
			, granularity(gran)
		{ }

		static Circle kDefaultCircle;

		virtual DrawType getDrawType() const { return DrawLineLoop; }

		virtual vec2 getRandomValue();
		virtual bool intersects(const Ray& ray, vec3& hit_pos);

		virtual size_t getNumEdges() const { return this->granularity; }
		virtual size_t getEdges(std::vector<uint16>& indices);

		virtual size_t getNumPositions() const { return this->granularity; }
		virtual size_t getPositions(std::vector<vec3>& pos);

		virtual vec3 getCenter() const
		{
			return this->cir.position;
		}

		virtual void setCenter(const vec3& center)
		{
			this->cir.position = center;
		}

		const Circle& getCircle() const { return this->cir; }

	private:

		Circle cir;
		int32 granularity;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EllipseVolume, Volume)
	public:
		EllipseVolume()
			: Volume()
			, position(kZero3)
			, width(1.0f)
			, height(1.0f)
			, granularity(32)
		{ }

		EllipseVolume(const vec3& pos, float32 w, float32 h, int32 gran = 32)
			: Volume()
			, position(pos)
			, width(w)
			, height(h)
			, granularity(gran)
		{ }

		virtual DrawType getDrawType() const { return DrawLineLoop; }

		virtual vec2 getRandomValue();
		virtual bool intersects(const Ray& ray, vec3& hit_pos);
		virtual bool test(const vec3& point) const;

		virtual size_t getNumEdges() const { return this->granularity; }
		virtual size_t getEdges(std::vector<uint16>& indices);

		virtual size_t getNumPositions() const { return this->granularity; }
		virtual size_t getPositions(std::vector<vec3>& pos);

		virtual vec3 getCenter() const
		{
			return this->position;
		}

		virtual void setCenter(const vec3& center)
		{
			this->position = center;
		}

		virtual RectF getRect() const
		{
			RectF rect(
				this->position.x - this->width, 
				this->position.y - this->height, 
				this->width * 2.0f, 
				this->height * 2.0f);
			return rect;
		}

	private:

		vec3 position;
		float32 width;
		float32 height;
		int32 granularity;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(TriangleVolume, Volume)
	public:
		TriangleVolume()
			: Volume() { }

		TriangleVolume(const vec3& _p0, const vec3& _p1, const vec3& _p2)
			: p0(_p0)
			, p1(_p1)
			, p2(_p2)
		{ }

		virtual DrawType getDrawType() const { return DrawLines; }

		virtual bool intersects(const Ray& ray, vec3& hit_pos);

		virtual size_t getNumEdges() const { return 6; }
		virtual size_t getEdges(std::vector<uint16>& indices);

		virtual size_t getNumPositions() const { return 3; }
		virtual size_t getPositions(std::vector<vec3>& pos);

		virtual vec3 getCenter() const
		{
			return (p0 + p1 + p2) * (1.0f / 3.0f);
		}

	private:

		vec3 p0, p1, p2;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ChainVolume, Volume)
	public:

		ChainVolume()
			: Volume() { }

		ChainVolume(const std::vector<vec3>& pos)
			: positions(pos)
		{ }

		virtual DrawType getDrawType() const { return DrawLineStrip; }
		virtual bool intersects(const Ray& ray, vec3& hit_pos) { return false; }

		virtual size_t getNumEdges() const { return this->positions.size(); }
		virtual size_t getEdges(std::vector<uint16>& indices)
		{
			for (size_t i = 0; i < this->positions.size(); i++)
				indices.push_back(uint16(i));
			return indices.size();
		}

		virtual size_t getNumPositions() const { return this->positions.size(); }
		virtual size_t getPositions(std::vector<vec3>& pos) { pos = this->positions; return pos.size(); }

		const Vec3List& getPositionList() const { return this->positions; }

		virtual vec3 getCenter() const
		{
			if (this->positions.size() == 0)
				return kZero3;

			vec3 center = kZero3;
			for (auto& it : this->positions)
			{
				center = center + it;
			}
			return center * (1.0f / float32(this->positions.size()));
		}

	private:

		Vec3List positions;
		
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PolygonVolume, Volume)
	public:
		PolygonVolume()
			: Volume() { }

		PolygonVolume(std::vector<vec3>& pos)
			: positions(pos)
		{ }

		virtual DrawType getDrawType() const { return DrawLines; }

		virtual bool intersects(const Ray& ray, vec3& hit_pos);

		virtual size_t getNumEdges() const { return this->positions.size() * 2; }
		virtual size_t getEdges(std::vector<uint16>& indices)
		{
			for (size_t i = 0; i < this->positions.size() - 1; i++)
			{
				indices.push_back(uint16(i));
				indices.push_back(uint16(i) + 1);
			}

			indices.push_back(uint16(this->positions.size()) - 1);
			indices.push_back(0);
			return indices.size();
		}

		virtual size_t getNumPositions() const { return this->positions.size(); }
		virtual size_t getPositions(std::vector<vec3>& pos) 
		{
			for (auto& it : this->positions)
				pos.push_back(it);
			return pos.size();
		}


		virtual vec3 getCenter() const
		{
			if (positions.size() == 0)
				return kZero3;

			return std::accumulate(this->positions.begin(), this->positions.end(), kZero3) / float32(this->positions.size());
		}

	private:

		Vec3List positions;
	};


	CLASS_DEFINITION_DERIVED_REFLECT(AABBVolume, Volume)
	public:
		AABBVolume() 
			: bounds(AABB())
		{

		}

		AABBVolume(const vec3& minb, const vec3& maxb)
			: bounds(AABB(minb, maxb))
		{

		}

		virtual bool intersects(const Ray& ray, vec3& hit_pos);

		virtual size_t getNumEdges() const { return 24; }
		virtual size_t getEdges(std::vector<uint16>& indices);

		virtual size_t getNumPositions() const { return 8; }
		virtual size_t getPositions(std::vector<vec3>& pos);

	private:

		AABB bounds;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PolygonListVolume, Volume)
	public:
		PolygonListVolume() { }

		PolygonListVolume(const PolygonList& poly_list)
			: polygonList(poly_list)
		{ 
			this->refresh();
		}

		struct B2Polygon
		{
			B2Polygon();
			B2Polygon(const B2Polygon& rhs);
			void operator=(const B2Polygon& rhs);
			bool isValid();

			b2Vec2 positions[b2_maxPolygonVertices];
			size_t count;
		};

		typedef std::vector<B2Polygon> B2PolygonList;
		
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual DrawType getDrawType() const { return DrawLines; }

		virtual size_t getNumEdges() const { return index.size(); }
		virtual size_t getEdges(std::vector<uint16>& indices)
		{
			indices.insert(indices.end(), this->index.begin(), this->index.end());
			return indices.size();
		}

		virtual size_t getNumPositions() const { return positions.size(); }
		virtual size_t getPositions(std::vector<vec3>& pos)
		{
			pos.insert(pos.end(), this->positions.begin(), this->positions.end());
			return pos.size();
		}

		virtual size_t getPolygons(B2PolygonList& polygons, const size_t& max_verts_per_poly);

		std::vector<vec3>& getPositions() { return this->positions; }
		PolygonList getPolygonList() const { return this->polygonList; }
		
	protected:

		void refresh();

		std::vector<uint16> index;
		Vec3List positions;
		PolygonList polygonList;

	};

	enum SelectableVolumeType
	{
		SelectableVolumeTypeNone = -1,
		SelectableVolumeTypeDraw,
		SelectableVolumeTypePhysics,
		SelectableVolumeTypeFace,
		SelectableVolumeTypeTriangle,
		SelectableVolumeTypeVertex,
		SelectableVolumeTypeCollision,
		SelectableVolumeTypeReference,
		SelectableVolumeTypeDirection,
		SelectableVolumeTypePositionAnimation,
		SelectableVolumeTypeSizeAnimation,
		SelectableVolumeTypeLight,

		// ..
		SelectableVolumeTypeMAX
	};

	inline bool isSelectableSubType(SelectableVolumeType type)
	{
		return
			type == SelectableVolumeTypeFace ||
			type == SelectableVolumeTypeTriangle ||
			type == SelectableVolumeTypeVertex;
	}

	struct SelectableVolumeCallbackInfo
	{

		SelectableVolumeCallbackInfo()
			: onClick(nullptr)
			, description("error!")
		{ }

		SelectableVolumeCallbackInfo(CallbackPtr oc, const char* desc)
			: onClick(oc)
			, description(desc)
		{ }

		SelectableVolumeCallbackInfo(const SelectableVolumeCallbackInfo& rhs)
			: onClick(rhs.onClick)
			, description(rhs.description)
		{ }

		void operator=(const SelectableVolumeCallbackInfo& rhs)
		{
			this->onClick = rhs.onClick;
			this->description = rhs.description;
		}

		CallbackPtr onClick;
		const char* description;
	};

	typedef std::map<char, SelectableVolumeCallbackInfo> SelectableVolumeCallbackInfoList;
	struct SelectableVolume
	{
		SelectableVolume()
			: volume(nullptr)
			, indexData(-1)
			, type(SelectableVolumeTypeNone)
			, positionWatcher(nullptr)
			, distance(0.0f)
			, useInitialTransform(false)
			, offset(kZero3)
		{ }

		VolumePtr volume;
		size_t indexData;
		SelectableVolumeType type;
		CallbackArg2<void, vec3, size_t> onChangedCallback;
		vec3* positionWatcher;
		float32 distance;
		SelectableVolumeCallbackInfoList onKeyPressCallbacks;
		bool useInitialTransform;
		vec3 offset;

	};

	typedef std::vector<VolumePtr> VolumeList;
	typedef std::vector<SelectableVolume> SelectableVolumeList;
}