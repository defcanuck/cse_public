#include "PCH.h"

#include "gfx/SplineRenderable.h"
#include "geom/Spline.h"

#include "global/Timer.h"
#include "game/Context.h"

// #define DEBUG_SPLINE_SEGMENTS 1

namespace cs
{
	BEGIN_META_CLASS(SplineRenderable)
		ADD_MEMBER(p0);
			SET_MEMBER_CALLBACK_POST(&SplineRenderable::updateVolume);
		ADD_MEMBER(p1);
			SET_MEMBER_CALLBACK_POST(&SplineRenderable::updateVolume);
		ADD_MEMBER(p2);
			SET_MEMBER_CALLBACK_POST(&SplineRenderable::updateVolume);
		ADD_MEMBER(p3);
			SET_MEMBER_CALLBACK_POST(&SplineRenderable::updateVolume);
		ADD_MEMBER(width);
		ADD_MEMBER(splineColor);
		ADD_MEMBER(smooth);

		ADD_MEMBER_PTR(texture);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

	END_META()

	const float32 SplineRenderable::kDefaultSplineWidth = 4.0f;
	const float32 SplineRenderable::kDefaultSplineSmooth = 0.0f;
	const float32 SplineRenderable::kDefaultSplineControl = 1.0f;
	const size_t SplineRenderable::kDefaultSplineSize = 100;
	const ColorB SplineRenderable::kDefaultSplineColor = ColorB::White;

	const vec3 SplineRenderable::kDefaultP0(-100.0f, 200.0f, 0.0f);
	const vec3 SplineRenderable::kDefaultP1(-50.0f, 100.0f, 0.0f);
	const vec3 SplineRenderable::kDefaultP2(50.0f, -100.0f, 0.0f);
	const vec3 SplineRenderable::kDefaultP3(100.0f, -200.0f, 0.0f);

	void SplineRenderable::initGeometry()
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
			&SplineRenderable::updateVertices,
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
			&SplineRenderable::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->splineGeom->setDrawCallAdjustFunc(dcfunc);

		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&SplineRenderable::getVertexBufferSize, this);
		this->splineGeom->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&SplineRenderable::getIndexBufferSize, this);
		this->splineGeom->setIndexBufferSizeFunc(numIFunc);

        GeometryPtr baseGeom = std::static_pointer_cast<Geometry>(this->splineGeom);
		this->addGeometry(baseGeom);
	}

	void SplineRenderable::setTexture(const TextureHandlePtr& tex)
	{
		this->texture = tex;
	}

	void SplineRenderable::setTexture(const std::string& textureName)
	{
		TextureHandlePtr texture_handle = CREATE_CLASS(TextureHandle, textureName);
		this->setTexture(texture_handle);
	}

	size_t SplineRenderable::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		
		char* pos_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);
		char* uv_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);
		char* color_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribColor, 0);
		
		size_t stride = decl.getStride();
		
		vec3 last_center = kZero3;

		float32 last_x = this->p0.x;
		float32 last_y = this->p0.y;

		for (size_t i = 0; i < this->maxSize; i++)
		{
			int32 top_index = int32((i * 2) + 0);
			int32 bot_index = int32((i * 2) + 1);

			float32 pct = i / float32(this->maxSize - 1);

			float32 x = 0.0f, y = 0.0f;
			switch (this->type)
			{
				case SplineTypeHermite:
				{
					x = spline::evalHermite(this->p0.x, this->p1.x, this->p2.x, this->p3.x, pct);
					y = spline::evalHermite(this->p0.y, this->p1.y, this->p2.y, this->p3.y, pct);

				} break;
			}

			vec3* pos_top = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, top_index * stride));
			vec3* pos_bot = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, bot_index * stride));

			if (this->smooth > 0)
			{
				vec3 dir = vec3(x - last_x, y - last_y, 0.0f);
				dir = glm::normalize(dir);
				vec3 perp = glm::cross(dir, kDefalutZAxis);
				perp = glm::normalize(perp);

				(*pos_bot) = vec3(x + (this->width * perp.x), y + (this->width * perp.y), 0.0f);
				(*pos_top) = vec3(x - (this->width * perp.x), y - (this->width * perp.y), 0.0f);
			}
			else
			{
				(*pos_bot) = vec3(x - this->width, y, 0.0f);
				(*pos_top) = vec3(x + this->width, y, 0.0f);
			}
			
			vec2* uv_top = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, top_index * stride));
			vec2* uv_bot = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, bot_index * stride));
			
			float32 v = (i % 2 == 0) ? 0.0f : 1.0f;
			(*uv_top) = vec2(0.0f, v);
			(*uv_bot) = vec2(1.0f, v);

			ColorF* color_top = reinterpret_cast<ColorF*>(PTR_ADD(color_ptr, top_index * stride));
			ColorF* color_bot = reinterpret_cast<ColorF*>(PTR_ADD(color_ptr, bot_index * stride));
			
#if defined(DEBUG_SPLINE_SEGMENTS)
			(*color_top) = toColorF(this->params[i].color);
			(*color_bot) = toColorF(this->params[i].color);
#else
			(*color_top) = toColorF(this->splineColor);
			(*color_bot) = toColorF(this->splineColor);
#endif

			last_x = x;
			last_y = y;
		}

		return this->maxSize * 2;
	}

	size_t SplineRenderable::updateIndices(uchar* data, size_t bufferSize)
	{
		// ?
		return 0;
	}

	void SplineRenderable::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->tag = "Spline";
		dc->type = DrawTriangleStrip;
		dc->indexType = TypeUnsignedShort;
		dc->count = static_cast<uint32>(this->maxSize) * 2;
		dc->offset = 0;
		dc->shaderHandle = RenderInterface::kDefaultTextureColorShader;
		dc->textures[TextureStageDiffuse] = this->texture;
		dc->color = ColorB::White;

		this->options.populate(dc);

		dcs.push_back(dc);
	}

	void SplineRenderable::setControlPoint(int32 idx, vec3 pos)
	{
		vec3* controlPoints[] =
		{
			&this->p0,
			&this->p1,
			&this->p2,
			&this->p3
		};

		*controlPoints[idx] = pos; 
		this->updateVolume();
	}


	size_t SplineRenderable::getVertexBufferSize()
	{
		size_t stride = this->splineGeom->getGeometryData()->decl.getStride();
		return stride * this->maxSize * 2;
	}
	
	size_t SplineRenderable::getIndexBufferSize()
	{
		return this->maxSize * 2 * sizeof(uint16);
	}

	void SplineRenderable::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		if (!this->volume.get())
			return;

		SelectableVolume volume;
		volume.volume = this->volume;
		volume.type = SelectableVolumeTypeDraw;

		selectable_volumes.push_back(volume);
	}

	void SplineRenderable::updateVolume()
	{
		FloatExtentCalculator xExtent;
		FloatExtentCalculator yExtent;

		switch (this->type)
		{
			case SplineTypeHermite:
				xExtent.evaluate(this->p1.x);
				xExtent.evaluate(this->p2.x);

				yExtent.evaluate(this->p1.y);
				yExtent.evaluate(this->p2.y);
				break;
			default:
				assert(false);
				break;
		}

		this->volume = CREATE_CLASS(QuadVolume, xExtent.minValue, yExtent.minValue, xExtent.span(), yExtent.span());

	}

}
