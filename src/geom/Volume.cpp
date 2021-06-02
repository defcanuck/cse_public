#include "PCH.h"

#include "geom/Volume.h"
#include "geom/Shape.h"

#include "gfx/RenderInterface.h"
#include "physics/PhysicsGlobals.h"
#include "gfx/Mesh.h"

#include <float.h>

namespace cs
{

	DEFINE_META_VECTOR(Vec3List, vec3, Vec3List);
	DEFINE_META_VECTOR(Vec2List, vec2, Vec2List);
	
	DEFINE_META_VECTOR(PolygonList, Polygon, PolygonList);

	BEGIN_META_CLASS(Polygon)
		ADD_MEMBER(pos);
	END_META()

	BEGIN_META_CLASS(Volume)

	END_META()

	BEGIN_META_CLASS(LineVolume)
		ADD_MEMBER(start);
		ADD_MEMBER(end);
	END_META()

	BEGIN_META_CLASS(QuadVolume)
		ADD_MEMBER(rect);
	END_META()

	BEGIN_META_CLASS(CircleVolume)
		ADD_MEMBER(granularity);
	END_META()

	BEGIN_META_CLASS(EllipseVolume)
		ADD_MEMBER(granularity);
			SET_MEMBER_MIN(6);
			SET_MEMBER_MAX(64);
			SET_MEMBER_DEFAULT(32);
		ADD_MEMBER(position);
		ADD_MEMBER(width);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(300.0f);
			SET_MEMBER_DEFAULT(100.0f);
		ADD_MEMBER(height);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(300.0f);
			SET_MEMBER_DEFAULT(50.0f);
	END_META()

	BEGIN_META_CLASS(TriangleVolume)
		ADD_MEMBER(p0);
		ADD_MEMBER(p1);
		ADD_MEMBER(p2);
	END_META()

	BEGIN_META_CLASS(ChainVolume)
		ADD_MEMBER(positions);
	END_META()

	BEGIN_META_CLASS(AABBVolume)
		ADD_MEMBER(bounds);
	END_META()

	BEGIN_META_CLASS(PolygonVolume)
	END_META()

	BEGIN_META_CLASS(PolygonListVolume)
		ADD_MEMBER(polygonList);
			SET_MEMBER_IGNORE_GUI();
	END_META()


	Circle CircleVolume::kDefaultCircle = Circle(1.0f, kZero3, kDefalutZAxis);

	RectF QuadVolume::kDefaultVolume = RectF(-50.0f, -50.0f, 100.0f, 100.0f);

	vec2 QuadVolume::getRandomValue()
	{
		vec2 p;
		p.x = rect.pos.x + randomRange(0.0f, rect.size.w);
		p.y = rect.pos.y + randomRange(0.0f, rect.size.h);
		return p;
	}

	bool QuadVolume::intersects(const Ray& ray, vec3& hit_pos)
	{
		return rectIntersect(ray, this->rect, hit_pos);
	}

	size_t QuadVolume::getPositions(std::vector<vec3>& pos)
	{
		pos.clear();
		QuadShape::generatePositionsImpl(this->rect, pos);
		return pos.size();
	}

	size_t QuadVolume::getEdges(std::vector<uint16>& indices)
	{
		indices.clear();
		QuadShape::generateWireframeImpl(indices);
		return indices.size();
	}

	bool AABBVolume::intersects(const Ray& ray, vec3& hit_pos)
	{
		return this->bounds.intersects(ray, hit_pos);
	}

	size_t AABBVolume::getPositions(std::vector<vec3>& pos)
	{
		const std::vector<vec3>& positions = this->bounds.getPositions();
		pos.insert(pos.end(), positions.begin(), positions.end());
		return pos.size();
	}

	size_t AABBVolume::getEdges(std::vector<uint16>& indices)
	{
		for (uint16 i = 0; i < 4; i++)
		{
			// bottom
			indices.push_back(i);
			indices.push_back((i == 3) ? 0 : i + 1);

			// pillars
			indices.push_back(i);
			indices.push_back(i + 4);

			// top
			indices.push_back(i + 4);
			indices.push_back((i == 3) ? 4 : i + 5);

		}

		return indices.size();
	}

	bool TriangleVolume::intersects(const Ray& ray, vec3& hit_pos)
	{
		float32 u, v, t;
		if (rayIntersectsTriangle(ray.getDirection(), ray.getOrigin(), this->p0, this->p1, this->p2, u, v, t))
		{
			hit_pos = ray.getPointAt(t);
			return true;
		}

		return false;
	}

	size_t TriangleVolume::getEdges(std::vector<uint16>& indices)
	{
		const std::vector<uint16> kTriangleIndices = { 0, 1, 1, 2, 2, 0 };
		indices.insert(indices.begin(), kTriangleIndices.begin(), kTriangleIndices.end());
		return indices.size();
	}

	size_t TriangleVolume::getPositions(std::vector<vec3>& pos)
	{
		pos.push_back(this->p0);
		pos.push_back(this->p1);
		pos.push_back(this->p2);
		return pos.size();
	}

	PolygonListVolume::B2Polygon::B2Polygon()
		: count(0)
	{
		memset(this->positions, 0, b2_maxPolygonVertices * sizeof(b2Vec2));
	}
	PolygonListVolume::B2Polygon::B2Polygon(const PolygonListVolume::B2Polygon& rhs)
		: count(rhs.count)
	{
		memcpy(positions, rhs.positions, rhs.count * sizeof(b2Vec2));
	}

	void PolygonListVolume::B2Polygon::operator=(const PolygonListVolume::B2Polygon& rhs)
	{
		this->count = rhs.count;
		memcpy(positions, rhs.positions, rhs.count * sizeof(b2Vec2));
	}

	bool PolygonListVolume::B2Polygon::isValid()
	{
		float32 min_x = FLT_MAX, min_y = FLT_MAX;
		float32 max_x = -FLT_MAX, max_y = -FLT_MAX;
		for (size_t i = 0; i < count; i++)
		{
			b2Vec2& vec = positions[i];
			min_x = (vec.x < min_x) ? vec.x : min_x;
			min_y = (vec.y < min_y) ? vec.y : min_y;

			max_x = (vec.x > max_x) ? vec.x : max_x;
			max_y = (vec.y > max_y) ? vec.y : max_y;
		}

		return ((max_x - min_x) > 0) && ((max_y - min_y) > 0);
	}

	void PolygonListVolume::onPostLoad(const LoadFlagMask& flags)
	{
		if (flags.test(LoadFlagsPhysics))
		{
			this->refresh();
		}
	}

	void PolygonListVolume::refresh()
	{
		this->index.clear();
		this->positions.clear();

		collapsePolys(this->positions, this->polygonList);

		size_t offset = 0;
		for (auto it : this->polygonList)
		{

			for (size_t v = 0; v < (it.pos.size() - 1); ++v)
			{
				this->index.push_back(uint16(offset + v));
				this->index.push_back(uint16(offset + v + 1));
			}

			this->index.push_back(uint16(offset + (it.pos.size() - 1)));
			this->index.push_back(uint16(offset));

			offset += it.pos.size();
		}
	}

	size_t PolygonListVolume::getPolygons(B2PolygonList& polygons, const size_t& max_verts_per_poly)
	{
		for (size_t i = 0; i < this->polygonList.size(); i++)
		{
			Polygon& poly = this->polygonList[i];
			polygons.push_back(B2Polygon());
			B2Polygon* polyToUse = &polygons[polygons.size() - 1];

			for (size_t v = 0; v < poly.pos.size(); v++)
			{
				const vec3& pos = poly.pos[v];
				polyToUse->positions[polyToUse->count++] = PhysicsConst::worldToBox2D(pos.x, pos.y);
			}
		}

		return polygons.size();
	}

	size_t MeshHandleVolume::getPolygons(B2PolygonList& polygons, const size_t& max_verts_per_poly)
	{

		assert(this->meshHandle);
		MeshPtr& mesh = this->meshHandle->getMesh();
		if (!mesh)
		{
			log::error("Bad mesh resource for PolygonListVolume!");
			return 0;
		}

		std::vector<MeshShapePtr> geomToUse;
		this->getMeshShapes(geomToUse);

		size_t offset = 0;


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

			polygons.push_back(B2Polygon());
			B2Polygon* polyToUse = &polygons[polygons.size() - 1];
			for (size_t i = 0; i < numVertices; i++)
			{
				if (polyToUse->count >= max_verts_per_poly)
				{
					polygons.push_back(B2Polygon());
					polyToUse = &polygons[polygons.size() - 1];
				}

				vec3* pos = decl.getAttributePointerAtIndex<vec3>(vbData, AttributeType::AttribPosition, i);
				polyToUse->positions[polyToUse->count++] = b2Vec2((*pos).x, (*pos).y);
			}
		}

		return polygons.size();
	}

	bool PolygonVolume::intersects(const Ray& ray, vec3& hit_pos)
	{
		for (size_t i = 0; i < this->positions.size() - 2; i++)
		{
			float32 u, v, t;

			vec3& p0 = this->positions[i];
			vec3& p1 = this->positions[i + 1];
			vec3& p2 = this->positions[i + 2];

			if (rayIntersectsTriangle(ray.getDirection(), ray.getOrigin(), p0, p1, p2, u, v, t))
			{
				hit_pos = ray.getPointAt(t);
				return true;
			}
		}
		return false;
	}

	vec2 CircleVolume::getRandomValue()
	{
		float32 random_pi = randomRange(0.0f, float32(CS_PI) * 2.0f);
		return vec2(this->cir.position.x, this->cir.position.y)
			+ vec2(cos(random_pi) * this->cir.radius, sin(random_pi) * this->cir.radius);
	}

	bool CircleVolume::intersects(const Ray& ray, vec3& hit_pos)
	{
		return this->cir.intersects(ray, hit_pos);
	}

	size_t CircleVolume::getEdges(std::vector<uint16>& indices)
	{
		for (uint16 i = 0; i < this->getNumEdges(); i++)
			indices.push_back(i);
		return indices.size();
	}

	size_t CircleVolume::getPositions(std::vector<vec3>& pos)
	{
		for (size_t i = 0; i < this->getNumPositions(); i++)
		{
			float32 theta = i / float32(this->getNumPositions()) * (2.0f * float32(CS_PI));
			float32 x = (cos(theta) * this->cir.radius) + this->cir.position.x;
			float32 y = (sin(theta) * this->cir.radius) + this->cir.position.y;

			pos.push_back(vec3(x, y, this->cir.position.z));
		}
		return pos.size();
	}

	vec2 EllipseVolume::getRandomValue()
	{
		float32 random_pi = randomRange(0.0f, float32(CS_PI) * 2.0f);
		return vec2(this->position.x, this->position.y)
			+ vec2(cos(random_pi) * this->width, sin(random_pi) * this->height);
	}

	bool EllipseVolume::intersects(const Ray& ray, vec3& hit_pos)
	{
		// assuming vectors are all normalized
		float32 denom = glm::dot(-kDefalutZAxis, ray.direction);
		if (denom > 1e-6) 
		{
			vec3 p0l0 = this->position - ray.origin;
			float32 t = glm::dot(p0l0, -kDefalutZAxis) / denom;
			if (t >= 0)
			{
				vec3 pos = ray.origin + ray.direction * t;
				return this->test(pos);
			}
		}

		return false;
	}

	bool EllipseVolume::test(const vec3& pos) const
	{
		float32 x = (float32) (pow(pos.x - this->position.x, 2.0) * pow(this->height, 2.0));
		float32 y = (float32) (pow(pos.y - this->position.y, 2.0) * pow(this->width, 2.0));
		return (x + y) <= (pow(this->width, 2.0) * pow(this->height, 2.0));
	}

	size_t EllipseVolume::getEdges(std::vector<uint16>& indices)
	{
		for (uint16 i = 0; i < this->getNumEdges(); i++)
			indices.push_back(i);
		return indices.size();
	}

	size_t EllipseVolume::getPositions(std::vector<vec3>& pos)
	{
		for (size_t i = 0; i < this->getNumPositions(); i++)
		{
			float32 theta = i / float32(this->getNumPositions()) * float32(2.0f * CS_PI);
			float32 x = (cos(theta) * this->width) + this->position.x;
			float32 y = (sin(theta) * this->height) + this->position.y;

			pos.push_back(vec3(x, y, this->position.z));
		}
		return pos.size();
	}
  }
