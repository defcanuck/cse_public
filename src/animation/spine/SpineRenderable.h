#pragma once

#include "gfx/Renderable.h"
#include "gfx/RenderInterface.h"

#include "animation/spine/SpineAnimation.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(SpineRenderable, Renderable)
	public:

		typedef std::vector<TextureHandleConstPtr> AtlasTextureHandleList;
		typedef std::vector<TextureHandlePtr> AtlasTextureHandleRenderList;

		SpineRenderable();
    virtual ~SpineRenderable();

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void onSkeletonChanged();
		void onAtlasChanged();
		void onAnimationChanged();

		virtual void process(float32 dt);

		virtual void refresh() { }
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual void queueGeometry(RenderTraversal traversal, DisplayListNode& display_node);
		
		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);

		SpineAnimationInstance& getInstance() { return this->instance; }

		void setAnimation(const std::string& anim_name) { this->animName = anim_name; }
		void onSkinChanged();

		void setSpeed(float32 speed) { this->animSpeed = speed; }

		virtual VolumePtr getVolume() { return std::static_pointer_cast<Volume>(this->volume); }

	private:

		struct AnimVertexData
		{
			AnimVertexData() 
				: vertices(nullptr)
				, vertexCount(0) 
			{ }
			
			float* vertices;
			uint32 vertexCount;
			uint32 offset;
		};
		typedef std::map<std::string, AnimVertexData> AnimationVertices;

		size_t getNumIndices();
		size_t getNumVertices();
		void refreshGeometry();
		void refreshBoundingVolume();

		SpineSkeletonHandlePtr skeletonHandle;
		SpineAtlasHandlePtr textureAtlas;

		SpineAnimationInstance instance;
		bool instanceLoaded;

		AtlasTextureHandleList atlasPageTextures;
		AtlasTextureHandleRenderList atlasPageRenderTextures;

		ColorB tint;

		DynamicGeometryPtr geometry;
		QuadVolumePtr volume;
		ShaderHandlePtr shader;

		int32 numVertices;
		int32 numIndices;
		int32 numActiveSprites;
		int32 maxActiveSprites;

		bool flipHorizontal;
		bool flipVertical;
		float32 skew;
		float32 scaleX;
		float32 scaleY;

		AnimationVertices vertexMap;

		DrawOptions options;

		std::string skinName;

		float32 animSpeed;
		std::string animName;
	};
}
