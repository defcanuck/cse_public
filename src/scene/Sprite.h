#pragma once

#include "ClassDef.h"
#include "scene/Actor.h"
#include "math/GLM.h"
#include "gfx/TextureHandle.h"
#include "gfx/ShaderResource.h"
#include "gfx/Geometry.h"
#include "scene/SceneManager.h"

#include "math/Rect.h"

namespace cs
{

	namespace sprite
	{

		struct DrawGeometry
		{
			DrawGeometry()
				: color(ColorB::White)
			{ }

			void draw(const mat4& mvp);

			ColorB color;
			GeometryPtr geometry;
		};

		void createSprite(ActorPtr& actor, const RectF& rect, TextureHandlePtr texture = RenderInterface::kWhiteTexture);
		void createGrid(ActorPtr& actor, const RectF& rect);

		GeometryPtr getLineGeometry(const std::vector<vec3>& nodes, const ColorB& color);
		GeometryPtr getSpriteGeometry(const RectF& rect, TextureHandlePtr texture = RenderInterface::kWhiteTexture);
		GeometryPtr getGridGeometry(const RectF& rect, uint32 div, const ColorB& color);
    };
}
