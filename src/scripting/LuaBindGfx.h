#pragma once

#include "scripting/LuaMacro.h"

#include "gfx/Renderable.h"
#include "gfx/BatchRenderable.h"
#include "gfx/TrailRenderable.h"
#include "gfx/ShaderHandle.h"
#include "gfx/MeshRenderable.h"
#include "gfx/SplineRenderable.h"
#include "gfx/RenderTarget.h"
#include "gfx/PostProcess.h"

#include "gfx/Color.h"

namespace cs
{
    PROTO_LUA_CLASS(RenderInterface);
	PROTO_LUA_CLASS(Shape);
	PROTO_LUA_CLASS(QuadShape);
	PROTO_LUA_CLASS(ParallelogramShape);
	PROTO_LUA_CLASS(RenderTargetType);
	PROTO_LUA_CLASS(ColorFList);
	PROTO_LUA_CLASS(ColorF);
	PROTO_LUA_CLASS(ColorB);
	PROTO_LUA_CLASS(Renderable);
	PROTO_LUA_CLASS(BatchRenderable);
	PROTO_LUA_CLASS(MeshRenderable);
	PROTO_LUA_CLASS(TextureHandle);
	PROTO_LUA_CLASS(Texture);
	PROTO_LUA_CLASS(TextureStage);
	PROTO_LUA_CLASS(RenderableOptions);
	PROTO_LUA_CLASS(DrawOptions);
	PROTO_LUA_CLASS(DrawOptionsBlend);
	PROTO_LUA_CLASS(DrawOptionsDepth);
	PROTO_LUA_CLASS(DepthType);
	PROTO_LUA_CLASS(BlendType);
	PROTO_LUA_CLASS(TrailRenderable);
	PROTO_LUA_CLASS(RenderTraversal);
	PROTO_LUA_CLASS(RenderTraversalMask);
	PROTO_LUA_CLASS(ShaderHandle);
	PROTO_LUA_CLASS(TextureChannels);
	PROTO_LUA_CLASS(DepthComponent);
	PROTO_LUA_CLASS(MeshHandle);
	PROTO_LUA_CLASS(Mesh);
	PROTO_LUA_CLASS(MeshShapeInstance);
	PROTO_LUA_CLASS(MeshShape); 
	PROTO_LUA_CLASS(MeshMaterialType);
	PROTO_LUA_CLASS(SplineRenderable);
	PROTO_LUA_CLASS(SharedUniform);
	PROTO_LUA_CLASS(RenderTexture);
	PROTO_LUA_CLASS(RenderTargetManager);
	PROTO_LUA_CLASS(PostProcess);
}
