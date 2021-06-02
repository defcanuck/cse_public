#pragma once

#if defined(CS_METAL)
enum MTLDebugCounter
{
    MTLDebugCounter_RenderPass,
    MTLDebugCounter_Texture,
    MTLDebugCounter_Buffer,
    MTLDebugCounter_PipelineDescriptor,
    MTLDebugCounter_Shader,
    MTLDebugCounter_VertexDescription,
    MTLDebugCounter_SamplerState,
    MTLDebugCounter_UniformBuffer,
    MTLDebugCounter_MAX
};
#endif

enum Type
{
    TypeNone = -1,
    TypeByte,
    TypeUnsignedByte,
    TypeShort,
    TypeUnsignedShort,
    TypeInt,
    TypeUnsignedInt,
    TypeFloat,
#if defined(CS_WINDOWS)
    TypeDouble,
#endif
    TypeMAX
};

enum BlendType
{
    BlendNone = -1,
    BlendZero,
    BlendOne,
    BlendSrcAlpha,
    BlendOneMinusSrcAlpha,
};

enum DrawType
{
    DrawNone = -1,
    DrawPoints,
    DrawLines,
    DrawLineLoop,
    DrawLineStrip,
    DrawTriangles,
    DrawTriangleStrip,
    DrawTriangleFan,
#if defined(CS_WINDOWS)
    DrawQuads,
#endif
    DrawMAX
};

int getTypeSize(Type type);

enum TextureSample
{
    TextureSampleNone = -1,
    TextureSampleClamp,
    TextureSampleRepeat
};

enum TextureFilter
{
    TextureFilterNone = -1,
    TextureFilterNearest,
    TextureFilterLinear
};

enum ShaderType
{
    ShaderNone = -1,
    ShaderVertex,
    ShaderFragment,
    ShaderMAX
};

enum ShaderFlags
{
    ShaderFlagsNone = -1,
    ShaderFlagsUI,
    ShaderFlagsRenderer,
    ShaderFlagsFont,
    ShaderFlagsTest,
    ShaderFlagsGame,
    //...
    ShaderFlagsMAX
};

enum TextureUsage
{
    TextureUsageUnknown = -1,
    TextureUsageShaderRead,
    TextureUsageShaderWrite,
    TextureUsageUsageRenderTarget,
    //...
    TextureUsageMAX
};

enum TextureChannels
{
    TextureNone = -1,
    TextureAlpha,
    TextureRGB,
    TextureRGBA,
    TextureRGBFloat,
    TextureRGBAFloat,
    TextureRGB565,
    TextureDepth16,
    TextureDepth24,
    TextureDepth32,
    TextureBGR,
	TextureBGRA,
    TextureMAX
};

enum ClearMode
{
    ClearNone = -1,
    ClearColor,
    ClearDepth,
    ClearMAX
};

enum StatType
{
    StatTypeUIElement,
    StatTypeUIDocument,
    StatTypeEntity,
    StatTypeMesh,
    StatTypeTexture,
    StatTypeFXHeap,
    StatTypeBufferSize,
    StatTypeTextureSize,
    //...
    StatTypeMAX
};

unsigned getTextureSize(TextureChannels channels);

struct ShaderCompile
{
    static bool shouldCompileShaders(ShaderFlags flag);
};

void modifyEngineStat(StatType type, int value);
