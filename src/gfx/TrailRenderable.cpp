#include "PCH.h"

#include "gfx/TrailRenderable.h"
#include "geom/Spline.h"

#include "global/Timer.h"
#include "game/Context.h"

// #define DEBUG_SPLINE_SEGMENTS 1

namespace cs
{
	BEGIN_META_CLASS(TrailRenderable)
	
	END_META()

	const float32 TrailRenderable::kDefaultSplineWidth = 4.0f;
	const float32 TrailRenderable::kDefaultSplineSmooth = 0.9f;
	const float32 TrailRenderable::kDefaultSplineControl = 1.0f;
	const size_t TrailRenderable::kDefaultSplineSize = 100;

	const vec3 TrailRenderable::kDefaultWidthDir = kZero3;
	const float32 TrailRenderable::kDefaultWidthDirMag = 0.0f;
	const ColorB TrailRenderable::kDefaultSplineColor = ColorB::White;

	void TrailRenderable::initGeometry()
	{

		GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);

		data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 2, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec2) });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, sizeof(vec2) + sizeof(vec2) });

		data->vertexSize = this->maxSize * 2;
		data->indexSize = this->maxSize * 2;
		data->storage = BufferStorageDynamic;

		data->indexData.resize(data->indexSize);
		for (size_t i = 0; i < data->indexSize; i++)
		{
			data->indexData[i] = uint16(i);
		}

		this->splineGeom = CREATE_CLASS(DynamicGeometry, data);

		DynamicGeometry::VertexUpdateFunc vfunc = std::bind(
			&TrailRenderable::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->splineGeom->setVertexUpdateFunc(vfunc);

		/*
		DynamicGeometry::IndexUpdateFunc ifunc = std::bind(
			&TrailRenderable::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->splineGeom->setIndexUpdateFunc(ifunc);
		*/

		DynamicGeometry::AdjustDrawCallFunc dcfunc = std::bind(
			&TrailRenderable::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->splineGeom->setDrawCallAdjustFunc(dcfunc);

		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&TrailRenderable::getVertexBufferSize, this);
		this->splineGeom->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&TrailRenderable::getIndexBufferSize, this);
		this->splineGeom->setIndexBufferSizeFunc(numIFunc);

        GeometryPtr baseGeom = std::static_pointer_cast<Geometry>(this->splineGeom);
		this->addGeometry(baseGeom);
	}

	void TrailRenderable::adjust(const vec3& offset)
	{
		for (size_t i = 0; i < this->params.size(); i++)
		{
			this->params[i].center += offset;
			this->params[i].positions.first += offset;
			this->params[i].positions.second += offset;
		}
	}

	size_t TrailRenderable::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		
		char* pos_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);
		char* uv_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);
		char* color_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribColor, 0);
		
		size_t stride = decl.getStride();
		
		vec3 last_center = kZero3;
		float32 inc = (this->params.size() > 0) ? 1.0f / float32(this->params.size()) : 0.0f;

		struct local
		{
			static float32 getPercentIndex(int32 index, float32 range, float32 age, float32 maxAge, TrailPointParams& params)
			{
				return index / range;
			}
			static float32 getPercentAge(int32 index, float32 range, float32 age, float32 maxAge, TrailPointParams& params)
			{
				return 1.0f - std::max(std::min((age - params.age) / maxAge, 1.0f), 0.0f);
			}
		};
		float32(*onUpdateParams)(int32, float32, float32, float32, TrailPointParams&) = &local::getPercentIndex;
		if (this->pointMaxAge > 0.0f)
		{
			onUpdateParams = &local::getPercentAge;
		}

		for (size_t i = 0; i < this->params.size(); i++)
		{
			float32 pct = (*onUpdateParams)(i, inc, this->age, this->pointMaxAge, this->params[i]);
			
			int32 top_index = int32((i * 2) + 0);
			int32 bot_index = int32((i * 2) + 1);

			vec3* pos_top = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, top_index * stride));
			vec3* pos_bot = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, bot_index * stride));
			(*pos_bot) = this->params[i].positions.first + (this->params[i].perp * this->depthWidth * (1.0f - pct));
			(*pos_top) = this->params[i].positions.second - (this->params[i].perp * this->depthWidth * (1.0f - pct));
		
			vec2* uv_top = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, top_index * stride));
			vec2* uv_bot = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, bot_index * stride));
			(*uv_top) = vec2(0.0f, 0.0f);
			(*uv_bot) = vec2(0.0f, 1.0f);

			ColorF* color_top = reinterpret_cast<ColorF*>(PTR_ADD(color_ptr, top_index * stride));
			ColorF* color_bot = reinterpret_cast<ColorF*>(PTR_ADD(color_ptr, bot_index * stride));
			
#if defined(DEBUG_SPLINE_SEGMENTS)
			(*color_top) = this->params[i].color;
			(*color_bot) = this->params[i].color;
#else
			(*color_top) = toColorF(this->splineColor * ((this->depthFade) ? pct : 1.0f));
			(*color_bot) = toColorF(this->splineColor * ((this->depthFade) ? pct : 1.0f));
#endif
		}

		this->curSize = this->params.size();
		return this->curSize * 2;
	}

	size_t TrailRenderable::updateIndices(uchar* data, size_t bufferSize)
	{
		// ?
		return 0;
	}

	void TrailRenderable::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->tag = "Spline";
		dc->type = DrawTriangleStrip;
		dc->indexType = TypeUnsignedShort;
		dc->count = static_cast<uint32>(this->curSize) * 2;
		dc->offset = 0;
		dc->shaderHandle = RenderInterface::kDefaultTextureColorShader;
		dc->textures[TextureStageDiffuse] = RenderInterface::kWhiteTexture;
		dc->color = ColorB::White;

		this->options.populate(dc);

		dcs.push_back(dc);
	}

	size_t TrailRenderable::getVertexBufferSize()
	{
		size_t stride = this->splineGeom->getGeometryData()->decl.getStride();
		return stride * this->maxSize * 2;
	}
	
	size_t TrailRenderable::getIndexBufferSize()
	{
		return this->maxSize * 2 * sizeof(uint16);
	}

	void TrailRenderable::setDirectionWidth(const vec3& vec, float32 w)
	{
		this->dirWidth = vec;
		this->dirWidthMag = w;
	}

	float32 TrailRenderable::getWidthInternal(const vec3& direction)
	{
		if (glm::length(this->dirWidth) == 0)
		{
			return this->width;
		}

		vec3 adj_dir = glm::normalize(direction);
		float32 pct = glm::dot(adj_dir, this->dirWidth);

		return lerp<float32>(this->width, this->dirWidthMag, clamp<float32>(0.0f, 1.0f, pct));
	}

	std::pair<vec3, vec3> TrailRenderable::getPositionInternal(const vec3& position, const vec3& direction)
	{
		float32 adjWidth = this->getWidthInternal(direction);

		vec3 perp = glm::normalize(glm::cross(direction, kDefalutZAxis));
		return{ position + (perp * adjWidth), position - (perp * adjWidth) };
	}

	void TrailRenderable::update(const vec3& position, const vec3& direction)
	{
		if (this->params.size() < 2)
		{
			this->push(position, direction);
			return;
		}

		// only update if there was a big change in direction, otherwise
		// the direction is static and there should be no need to update
		if (this->restrictDirection)
		{
			vec3& top_direction = this->params.back().direction;
			float diff = glm::dot(glm::normalize(top_direction), glm::normalize(direction));
			if (diff < 1.0f)
			{
				this->push(position, direction);
			}
			else
			{
				this->top(position, direction);
			}
		}
		else
		{
			this->push(position, direction);
		}
	}

	void TrailRenderable::clear()
	{
		this->curSize = 0;
		this->params.clear();
	}

	void TrailRenderable::process(float32 dt)
	{
		this->age += dt;

		BASECLASS::process(dt);
	}

	void TrailRenderable::pushInternal(const vec3& position, const vec3& direction, const ColorB& color)
	{
		if (this->params.size() >= this->maxSize)
		{
			this->params.pop_front();
		}

		this->params.push_back(TrailPointParams());
		TrailPointParams& params = this->params.back();
		params.center = position;
		params.positions = this->getPositionInternal(position, direction);
		params.direction = direction;
		params.perp = glm::cross(glm::normalize(direction), vec3(0.0, 0.0, 1.0));
		params.color = color;
		params.age = this->age;
	}

	void TrailRenderable::push(const vec3& position, const vec3& direction)
	{
		bool interp = false;
		if (this->params.size() > 0)
		{
			TrailPointParams& last_params = this->params.back();

			vec3 d0 = glm::normalize(direction);
			vec3 d1 = glm::normalize(last_params.direction);

			float32 dotp = glm::dot(d0, d1);
			if (dotp < kDefaultSplineSmooth)
			{
				
				vec3 p0 = last_params.center - (last_params.direction * this->control);
				vec3 p1 = last_params.center;
				vec3 p2 = position;
				vec3 p3 = position + (direction * this->control);

				vec3 new_position = spline::pointOnCurve<vec3>(p0, p1, p2, p3, 0.5f);
				vec3 new_direction = (direction + last_params.direction) * 0.5f;

				// re-adjust last parameter a bit to get rid of flat-ness
				last_params.color = ColorB::Blue;
				last_params.positions = this->getPositionInternal(last_params.center, (glm::normalize(last_params.direction) + glm::normalize(direction)) * 0.5f);

				this->pushInternal(new_position, new_direction, ColorB::Red);
				interp = true;
			}
		}
		
		this->pushInternal(position, direction, (interp) ? ColorB::Green : ColorB::White);
	}

	void TrailRenderable::top(const vec3& position, const vec3& direction)
	{
		std::pair<vec3, vec3>& top_position = this->params.back().positions;
		top_position = this->getPositionInternal(position, direction);

		vec3& top_direction = this->params.back().direction;
		top_direction = direction;
	}


}
