#pragma once

#include "gfx/Renderable.h"

#include "math/Rect.h"
#include "gfx/TextureHandle.h"
#include "gfx/ShaderHandle.h"
#include "gfx/BatchDraw.h"
#include "geom/Shape.h"
#include "geom/Volume.h"
#include "math/Transform.h"
#include "gfx/DrawOptions.h"
#include "gfx/TextureAtlas.h"

namespace cs
{
	struct TextureAtlasReference;

	CLASS_DEFINITION_DERIVED_REFLECT(BatchRenderable, Renderable)
		
	public:

		BatchRenderable();
		virtual ~BatchRenderable();

		virtual void onNew();

		void setTexture(const std::string& name);
		void setTexture(const TextureHandlePtr& tex);
		TextureHandlePtr& getTextureHandle() { return this->texture; }

		void setShader(const std::string& name);
		void setShader(const ShaderHandlePtr& shader);
		ShaderHandlePtr getShader() { return this->shader; }

		void setTint(const ColorB& col);
		const ColorB& getTint() const;

		void setLayer(int32 l) { this->layer = l; }
		int32 getLayer() const { return this->layer; }

		virtual void adjustSize(const vec2& sz, AnimationScaleType type);
		virtual void adjustTexRect(const RectF& rect);

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		template<class T>
		void setShape(std::shared_ptr<T>& s)
		{
			this->shape = std::static_pointer_cast<Shape>(s);
			if (this->shape)
			{
				this->shape->onChanged += createCallbackArg0(&BatchRenderable::setGeometryImpl, this);
			}
			this->setGeometryImpl();
		}

		virtual void batch(
			const std::string& tag,
			BatchDrawList& display_list, 
			const BatchRenderableParams& params,
			uint32& numVertices, 
			uint16& numIndices);

		virtual void process(float32 dt) { }
		virtual void refresh();

		virtual bool batchable() const { return true; }
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);

		void setWorldUV();
		void clearWorldUV();

		void mapToTexture();
		void mapToTextureHalf();

		virtual VolumePtr getVolume();

		void setAsShadow();

		void setFlag(RenderableOptions flag, bool enabled);

		DrawOptionsBlend blend;

	private:

		void resetTexture();
		void setTextureImpl();
		void setUVImpl(float32 dt = 0.0f);
		void setShaderImpl();
		void setGeometryImpl();
		void setTextureAtlasImpl();

		void mapToTextureImpl(float32 scale);

		TextureHandlePtr& getTextureHandleImpl();

		float32 updateDepth(const mat4& objToWorld, const RectF& orthoRect, SortMethod sortMethod);

		void onShapeChanged();

		void build();

		ShapePtr shape;
		ShapePtr animShape;

		TextureHandlePtr texture;
		RectF* animRect;

		ShaderHandlePtr shader;
		VolumePtr volume;
		BatchDrawDataPtr drawData;
		ColorB tint;
		
		std::vector<vec2> worldUV;
		vec2 worldUVScale;

		TextureAtlasReferencePtr textureAtlas;
		TextureHandlePtr atlasHandle;

		bool culling;
		int32 flags;
	};

}