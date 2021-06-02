#pragma once

#include <cstdint>
#include "global/Values.h"
#include "global/BitMask.h"
#include "gfx/Attrib.h"
#include "gfx/TypesGlobal.h"
#include "math/Rect.h"

namespace cs
{
	extern uint32 kTypeConvert[];
	extern uint32 kTypeSize[];
    
    struct ScreenView
    {
        RectI scaledView;
        RectI rawView;
    };

	enum RenderableOptions
	{
		RenderableOptionIgnoreGlobalTint,
		RenderableOptionMAX
	};

	enum StateType
	{
		StateScissorTest,
		StateDepthTest,
		StateBlend,
		StateCullFace,
		StateTypeMAX

	};

	extern uint32 kStateConvert[];
	extern uint32 kDrawConvert[];

	extern uint16 kStaticQuadIndices[];
	
	enum ProjectionType
	{
		ProjectionNone = -1,
		ProjectionPerspective,
		ProjectionOrthographic,
		ProjectionMAX
	};

	extern uint32 kClearConvert[];


	extern uint32 kTextureConvertSrc[];
	extern uint32 kTextureConvertDst[];

	enum TextureStageNum
	{
		TextureStageNumNone = -1,
		TextureStageNum0,
		TextureStageNum1,
		TextureStageNum2,
		TextureStageNum3,
		TextureStageNum4,
		TextureStageNum5,
		TextureStageNum6,
		TextureStageNum7,
		TextureStageNum8,
		TextureStageNumMAX
	};

	enum TextureStage
	{
		TextureStageNone = -1,

		// Normal object rendering
		TextureStageDiffuse,
		TextureStageSpecular,
		TextureStageNormal,
		TextureStageSpecularHighlight,
		TextureStageAmbient,
		TextureStageDisplacement,
		TextureStageGobo,
		//...
		// Post Processing
		TexturePostDepth,
		TexturePostColor,

		TextureStageMAX
	};

	extern uint32 kTextureSampleConvert[];
	
	extern const char* kTextureStageName[]; 
	TextureStage getStageForName(const std::string& name);
	TextureStageNum getPhysicalStage(TextureStage stage);
	
	enum DepthComponent
	{
		DepthComponentNone = -1,
		DepthComponent16,
		DepthComponent24,
		DepthComponent32,
		DepthComponentMAX
	};
	extern uint32 kDepthComponentConvert[];

	enum BufferType
	{
		BufferTypeNone = -1,
		BufferTypeVertex,
		BufferTypeIndex,
        BufferTypeUniform,
		BufferTypeMAX
	};

	enum BufferStorage
	{
		BufferStorageNone = -1,
		BufferStorageStatic,
		BufferStorageDynamic,
		BufferStorageStream,
		BufferStorageMAX
	};

	enum BufferAccess
	{
		BufferAccessNone = -1,
		BufferAccessRead,
		BufferAccessWrite,
		BufferAccessReadWrite,
		BufferAccessMAX
	};

	extern uint32 kBlendTypeConvert[];
	extern uint32 kDepthTypeConvert[];

	enum DepthType
	{
		DepthNone = -1,
		DepthNever,
		DepthEqual,
		DepthLess,
		DepthLessEqual,
		DepthGreater,
		DepthGreaterEqual,
		DepthNotEqual, 
		DepthAlways,
		DepthMAX
	};

	enum CullFace
	{
		CullNone = -1,
		CullFront,
		CullBack,
		CullFrontAndBack,
		CullMax
	};
	extern uint32 kCullFaceConvert[];

	enum FrontFace
	{
		FrontFaceCW,
		FrontFaceCCW,
		FrontFaceMAX
	};
	extern uint32 kFrontFaceConvert[];

	enum RenderTargetType
	{
		RenderTargetTypeNone = -1,
		RenderTargetTypeBackBuffer,
		RenderTargetTypeBackBufferFloat,
		RenderTargetTypeCopyBuffer,
		RenderTargetTypeCopyBuffer2,
		RenderTargetTypeCopyBufferHalf,
		RenderTargetTypeCopyBufferQuarter,

		//...
		RenderTargetTypeMAX
	};

	enum RenderTraversal
	{
		RenderTraversalNone = -1,
		RenderTraversalShadow,
		RenderTraversalMain,
		//...
		RenderTraversalMAX
	};

	typedef BitMask<RenderTraversal, RenderTraversalMAX> RenderTraversalMask;
}
