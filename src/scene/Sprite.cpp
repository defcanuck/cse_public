#include "PCH.h"

#include "scene/Sprite.h"
#include "gfx/Geometry.h"
#include "gfx/RenderInterface.h"
#include "ecs/comp/DrawableComponent.h"
#include "geom/Shape.h"
#include "gfx/Renderer.h"

namespace cs
{

	void sprite::DrawGeometry::draw(const mat4& mvp)
	{
		renderer::draw(mvp, this->color, this->geometry);
	}

	GeometryPtr sprite::getSpriteGeometry(const RectF& rect, TextureHandlePtr texture)
	{
		cs::GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);

		std::vector<vec3> pos;
		data->indexSize = QuadShape::generateIndicesImpl(data->indexData);
		data->vertexSize = QuadShape::generatePositionsImpl(rect, pos);

		std::vector<vec2> uvs = { vec2(0.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 0.0f) };
		if (texture)
		{
			const RectF& uvRect = texture->getUVRect();
			uvs.clear();
			QuadShape::generateUVImpl(uvRect, uvs);
		}
		
		for (int i = 0; i < 4; i++)
		{
			// pos + texture dada
			data->pushVertexData(pos[i]);
			data->pushVertexData(uvs[i]);
		}

		data->decl.addAttrib(AttribPosition, { AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttribTexCoord0, { AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });

		cs::DrawCallPtr drawCall = CREATE_CLASS(cs::DrawCall);
		drawCall->type = DrawTriangles;
		drawCall->offset = 0;
		drawCall->count = 6;
		drawCall->textures[0] = texture;
		drawCall->shaderHandle = RenderInterface::kDefaultTextureShader;
		drawCall->indexType = TypeUnsignedShort;
		drawCall->indices = data->indexData;

		data->drawCalls.push_back(drawCall);

		return CREATE_CLASS(cs::Geometry, data);
	}


	GeometryPtr sprite::getLineGeometry(const std::vector<vec3>& nodes, const ColorB& color)
	{
		assert(nodes.size() >= 2);
		cs::GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);

		data->indexSize = (nodes.size() * 2) - 2;
		data->vertexSize = nodes.size();

		ColorF col = toColorF(color);
		for (uint32 i = 0; i < nodes.size(); i++)
		{
			data->pushVertexData(nodes[i]);
			data->pushVertexData(col);
		}

		data->indexData.push_back(0);
		for (uint16 i = 1; i < uint16(nodes.size()) - 1; i++)
		{
			data->indexData.push_back(i);
			data->indexData.push_back(i);
		}
		data->indexData.push_back(uint16(nodes.size()) - 1);

		data->decl.addAttrib(AttribPosition, { AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttribColor, { AttribColor, TypeFloat, 4, sizeof(vec3) });

		cs::DrawCallPtr drawCall = CREATE_CLASS(cs::DrawCall);
		drawCall->type = DrawLines;
		drawCall->offset = 0;
		drawCall->count = (static_cast<uint32>(nodes.size()) * 2) - 2;
		drawCall->textures[0] = RenderInterface::kWhiteTexture;
		drawCall->shaderHandle = RenderInterface::kDefaultColorShader;
		drawCall->indexType = TypeUnsignedShort;
		drawCall->indices = data->indexData;
		drawCall->color = color;

		data->drawCalls.push_back(drawCall);

		return CREATE_CLASS(cs::Geometry, data);
	}

	GeometryPtr sprite::getGridGeometry(const RectF& rect, uint32 div, const ColorB& color)
	{
		cs::GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);
		
		float32 startX = rect.pos.x;
		float32 startY = rect.pos.y;
		float32 incX = rect.size.w / float32(div);
		float32 incY = rect.size.h / float32(div);

		data->indexSize = ((div + 1) + (div + 1)) * 2;
		data->vertexSize = ((div + 1) + (div + 1)) * 2;

		uint16 idxCtr = 0;

		const float32 kZOffset = -0.5f;

		ColorF vertexColor = toColorF(color);
		for (uint32 i = 0; i < div + 1; i++)
		{
			vec3 hStart(rect.pos.x, startY, kZOffset);
			vec3 hEnd(rect.pos.x + rect.size.w, startY, kZOffset);
			
			data->pushVertexData(hStart);
			data->pushVertexData(vertexColor);

			data->pushVertexData(hEnd);
			data->pushVertexData(vertexColor);
			
			data->indexData.push_back(idxCtr++);
			data->indexData.push_back(idxCtr++);

			vec3 vStart(startX, rect.pos.y, kZOffset);
			vec3 vEnd(startX, rect.pos.y + rect.size.h, kZOffset);
			
			data->pushVertexData(vStart);
			data->pushVertexData(vertexColor);

			data->pushVertexData(vEnd);
			data->pushVertexData(vertexColor);
			
			data->indexData.push_back(idxCtr++);
			data->indexData.push_back(idxCtr++);

			startX += incX;
			startY += incY;
		}

		data->decl.addAttrib(AttribPosition, { AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttribColor, { AttribColor, TypeFloat, 4, sizeof(vec3) });

		cs::DrawCallPtr drawCall = CREATE_CLASS(cs::DrawCall);
		drawCall->type = DrawLines;
		drawCall->offset = 0;
		drawCall->count = idxCtr;
		drawCall->textures[0] = RenderInterface::kWhiteTexture;
		drawCall->shaderHandle = RenderInterface::kDefaultColorShader;
		drawCall->indexType = TypeUnsignedShort;
		drawCall->indices = data->indexData;
		drawCall->color = color;

		data->drawCalls.push_back(drawCall);

		return CREATE_CLASS(cs::Geometry, data);
		
	}

	void sprite::createSprite(ActorPtr& actor, const RectF& rect, TextureHandlePtr texture)
	{
		GeometryPtr geo = getSpriteGeometry(rect, texture);
		RenderablePtr rend = CREATE_CLASS(Renderable);
		rend->addGeometry(geo);
		actor->getComponent<cs::DrawableComponent>()->setRenderable(rend);
	}
}
