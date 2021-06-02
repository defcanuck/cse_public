#include "PCH.h"
#include "geom/Shape.h"

namespace cs
{
	const size_t GridShape::kDefaultGridWidth = 6;
	const size_t GridShape::kDefaultGridHeight = 6;

	BEGIN_META_CLASS(Shape)
	END_META()

	BEGIN_META_CLASS(QuadShape)
		ADD_MEMBER(rect);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
	END_META()

	BEGIN_META_CLASS(ParallelogramShape)
		ADD_MEMBER(skew);
			SET_MEMBER_MIN(-100.0f);
			SET_MEMBER_MAX( 100.0f);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
	END_META()

	BEGIN_META_CLASS(GridShape)

	END_META()

	BEGIN_META_CLASS(CircleShape)
		ADD_MEMBER(radius);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
		ADD_MEMBER(granularity);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
	END_META()

	BEGIN_META_CLASS(EllipseShape)
		ADD_MEMBER(width);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(200.0f);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
		ADD_MEMBER(height);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(200.0f);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
		ADD_MEMBER(granularity);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
		ADD_MEMBER(alphaBoarder);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(20.0f);
			SET_MEMBER_CALLBACK_POST(&Shape::onGeomChanged);
	END_META()
	
	const RectF QuadShape::kDefaultRect(-50.0f, -50.0f, 100.0f, 100.0f);

	size_t QuadShape::generatePositionsImpl(const RectF& rect, std::vector<vec3>& pos, float32 z, float32 angle)
	{
		pos.reserve(4);
		if (angle != 0.0f)
		{
			float32 halfWidth = rect.size.w * 0.5f;
			float32 halfHeight = rect.size.h * 0.5f;

			pos.push_back(vec3(-halfWidth, -halfHeight, z));
			pos.push_back(vec3(-halfWidth,  halfHeight, z));
			pos.push_back(vec3( halfWidth,  halfHeight, z));
			pos.push_back(vec3( halfWidth, -halfHeight, z));
			
			float32 offsetX = rect.pos.x + halfWidth;
			float32 offsetY = rect.pos.y + halfHeight;

			float32 cosTheta = cos(angle);
			float32 sinTheta = sin(angle);

			for (size_t i = 0; i < pos.size(); ++i)
			{
				float32 xp = (pos[i].x * cosTheta) - (pos[i].y * sinTheta);
				float32 yp = (pos[i].x * sinTheta) + (pos[i].y * cosTheta);

				pos[i].x = xp + offsetX;
				pos[i].y = yp + offsetY;
			}
		}
		else
		{
			PointF tl = rect.getTL();
			PointF bl = rect.getBL();
			PointF tr = rect.getTR();
			PointF br = rect.getBR();

			pos.push_back(vec3(bl.x, bl.y, z));
			pos.push_back(vec3(tl.x, tl.y, z));
			pos.push_back(vec3(tr.x, tr.y, z));
			pos.push_back(vec3(br.x, br.y, z));
		}
		return pos.size();
	}

	size_t QuadShape::generatePositions(std::vector<vec3>& pos, float32 z)
	{
		return QuadShape::generatePositionsImpl(this->rect, pos, z);
	}

	size_t QuadShape::generateUVImpl(const RectF& uvRect, std::vector<vec2>& uvs)
	{
		uvs.reserve(4);
		uvs.push_back(toVec2(uvRect.getBL()));
		uvs.push_back(toVec2(uvRect.getTL()));
		uvs.push_back(toVec2(uvRect.getTR()));
		uvs.push_back(toVec2(uvRect.getBR()));
		return uvs.size();
	}

	size_t QuadShape::generateUV(const RectF& uvRect, std::vector<vec2>& uvs)
	{
		return QuadShape::generateUVImpl(uvRect, uvs);
	}

	size_t QuadShape::generateIndicesImpl(std::vector<uint16>& indices)
	{
		indices.reserve(6);
		indices.push_back(kStaticQuadIndices[0]);
		indices.push_back(kStaticQuadIndices[1]);
		indices.push_back(kStaticQuadIndices[2]);
		indices.push_back(kStaticQuadIndices[3]);
		indices.push_back(kStaticQuadIndices[4]);
		indices.push_back(kStaticQuadIndices[5]);
		return indices.size();
	}

	size_t QuadShape::generateIndices(std::vector<uint16>& indices)
	{
		return QuadShape::generateIndicesImpl(indices);
	}

	size_t QuadShape::generateWireframeImpl(std::vector<uint16>& indices)
	{
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);
		return indices.size();
	}

	size_t QuadShape::generateWireframe(std::vector<uint16>& indices) 
	{ 
		return QuadShape::generateWireframeImpl(indices); 
	}

	VolumePtr QuadShape::createVolume() 
	{ 
		return std::static_pointer_cast<Volume>(CREATE_CLASS(QuadVolume, this->rect)); 
	}

	size_t ParallelogramShape::generatePositions(std::vector<vec3>& pos, float32 z)
	{
		PointF tl = this->rect.getTL();
		PointF bl = this->rect.getBL();
		PointF tr = this->rect.getTR();
		PointF br = this->rect.getBR();

		pos.push_back(vec3(bl.x + this->skew, bl.y, z));
		pos.push_back(vec3(tl.x - this->skew, tl.y, z));
		pos.push_back(vec3(tr.x - this->skew, tr.y, z));
		pos.push_back(vec3(br.x + this->skew, br.y, z));
		return pos.size();
	}

	size_t CircleShape::generatePositions(std::vector<vec3>& pos, float32 z)
	{
		pos.push_back(kZero3);

		float32 thetaInc = (float32(CS_PI) * 2.0f) / float32(this->granularity);
		for (int32 i = 0; i < granularity; ++i)
		{
			float32 angle = i * thetaInc;
			float32 xp = cos(angle) * this->radius;
			float32 yp = sin(angle) * this->radius;

			pos.push_back(vec3(xp, yp, z));
		}
		return pos.size();
	}

	size_t CircleShape::generateUV(const RectF& uvRect, std::vector<vec2>& uvs)
	{
		struct local
		{
			static vec2 offset(float32 u, float32 v, const RectF& uvRect)
			{
				return vec2(uvRect.pos.x + (u * uvRect.size.w), uvRect.pos.y + (v * uvRect.size.h));
			}
		};

		// uvs.push_back(local::offset(0.5f, 0.5f, uvRect));
		uvs.push_back(vec2(0.5f, 0.5f));

		float32 thetaInc = (float32(CS_PI) * 2.0f) / float32(this->granularity);
		for (int32 i = 0; i < granularity; ++i)
		{
			float32 angle = i * thetaInc;
			float32 xp = cos(angle) * 0.5f + 0.5f;
			float32 yp = sin(angle) * 0.5f + 0.5f;

			// uvs.push_back(local::offset(0.5f + xp, 0.5f + yp, uvRect));
			uvs.push_back(vec2(xp, yp));
		}
		return uvs.size();
	}

	size_t CircleShape::generateIndices(std::vector<uint16>& indices)
	{
		for (int32 i = 1; i <= granularity; ++i)
		{
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back((i == granularity) ? 1 : (i + 1));
		}
		return indices.size();
	}

	size_t CircleShape::generateWireframe(std::vector<uint16>& indices)
	{
		for (int32 i = 0; i < granularity; ++i)
		{
			indices.push_back(i);
			indices.push_back((i == (granularity - 1)) ? 0 : (i + 1));
		}
		return indices.size();
	}

	VolumePtr CircleShape::createVolume()
	{
		return std::static_pointer_cast<Volume>(CREATE_CLASS(CircleVolume, kZero3, this->radius, this->granularity));
	}

	size_t EllipseShape::generatePositions(std::vector<vec3>& pos, float32 z)
	{
		pos.push_back(kZero3);

		float32 thetaInc = (float32(CS_PI) * 2.0f) / float32(this->granularity);
		for (int32 i = 0; i < granularity; ++i)
		{
			float32 angle = i * thetaInc;
			float32 xp = cos(angle) * this->width;
			float32 yp = sin(angle) * this->height;

			pos.push_back(vec3(xp, yp, z));
		}

		if (this->alphaBoarder > 0.0f)
		{
			float32 boarderAdjust = this->width / this->height;
			float32 adjustWidth = boarderAdjust * this->alphaBoarder;
			float32 adjustHeight = 1.0f / boarderAdjust * this->alphaBoarder;
			for (int32 i = 0; i < granularity; ++i)
			{
				float32 angle = i * thetaInc;
				float32 xp = cos(angle) * (this->width + adjustWidth);
				float32 yp = sin(angle) * (this->height + adjustHeight);

				pos.push_back(vec3(xp, yp, z));
			}
		}
		return pos.size();
	}

	size_t EllipseShape::generateUV(const RectF& uvRect, std::vector<vec2>& uvs)
	{
		struct local
		{
			static vec2 offset(float32 u, float32 v, const RectF& uvRect)
			{
				return vec2(uvRect.pos.x + (u * uvRect.size.w), uvRect.pos.y + (v * uvRect.size.h));
			}
		};

		// uvs.push_back(local::offset(0.5f, 0.5f, uvRect));
		uvs.push_back(kZero2);

		float32 thetaInc = (float32(CS_PI) * 2.0f) / float32(this->granularity);

		float32 aspect = this->height / this->width;
		float32 boarder = (this->alphaBoarder > 0) ? (1.0f - (this->alphaBoarder * 2.0f / this->width)) : 1.0f;
		for (int32 i = 0; i < granularity; ++i)
		{
			float32 angle = i * thetaInc;
			float32 xp = (cos(angle) * 0.5f) * boarder;
			float32 yp = (sin(angle) * 0.5f) * aspect * boarder;

			// uvs.push_back(local::offset(0.5f + xp, 0.5f + yp, uvRect));
			uvs.push_back(vec2(xp, yp));
		}
		if (this->alphaBoarder > 0)
		{
			
			for (int32 i = 0; i < granularity; ++i)
			{
				float32 angle = i * thetaInc;
				float32 xp = (cos(angle) * 0.5f);
				float32 yp = (sin(angle) * 0.5f) * aspect;

				// uvs.push_back(local::offset(0.5f + xp, 0.5f + yp, uvRect));
				uvs.push_back(vec2(xp, yp));
			}
		}
		return uvs.size();
	}

	size_t EllipseShape::generateIndices(std::vector<uint16>& indices)
	{
		for (int32 i = 1; i <= granularity; ++i)
		{
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back((i == granularity) ? 1 : (i + 1));
		}

		if (this->alphaBoarder > 0)
		{
			for (int32 i = 1; i <= granularity; ++i)
			{	
				int32 idx0 = i;
				int32 idx1 = (i == granularity) ? 1 : (i + 1);
				indices.push_back(idx0);
				indices.push_back(idx0 + granularity);
				indices.push_back(idx1 + granularity);
			
				indices.push_back(idx0);
				indices.push_back(idx1 + granularity);
				indices.push_back(idx1);
			}
		}
		return indices.size();
	}

	size_t EllipseShape::generateWireframe(std::vector<uint16>& indices)
	{
		for (int32 i = 0; i < granularity; ++i)
		{
			indices.push_back(i);
			indices.push_back((i == (granularity - 1)) ? 0 : (i + 1));
		}
		return indices.size();
	}

	VolumePtr EllipseShape::createVolume()
	{
		float32 aspect = this->height / this->width;
		float32 adjustWidth = this->alphaBoarder;
		float32 adjustHeight = aspect * this->alphaBoarder;
		return std::static_pointer_cast<Volume>(CREATE_CLASS(EllipseVolume, kZero3, this->width + adjustWidth, this->height + adjustHeight, this->granularity));
	}

	size_t EllipseShape::generateColors(std::vector<ColorB>& colors, ColorB& color)
	{
		if (this->alphaBoarder > 0)
		{
			for (int32 i = 0; i <= granularity; ++i)
			{
				colors.push_back(color);
			}
			ColorB alphaColor = ColorB(color.r, color.g, color.b, 0);
			for (int32 i = 0; i < granularity; ++i)
			{
				colors.push_back(alphaColor);
			}
		}
		return colors.size();
	}
}