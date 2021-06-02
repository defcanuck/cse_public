#include "PCH.h"

#include "gfx/TypesGlobal.h"
#include "global/BitMask.h"
#include "global/Stats.h"

#include <assert.h>

unsigned kTextureSize[] =
{
    1,  // TextureAlpha
    3,  // TextureRGB
    4,  // TextureRGBA
    2,  // TextureRGB565

#if defined(CS_METAL)
	3 * sizeof(float),	// TextureRGBFloat
	4 * sizeof(float),  // TextureRGBAFloat
#else
	3,  // TextureRGBFloat (TextureRGB)
	4,  // TextureRGBAFloat (TextureRGBA)
#endif

    2,  // TextureDepth16
    3,  // TextureDepth24
    4,  // TextureDepth32
    3,  // TextureBGR
	4,  // TextureBGRA
};

typedef cs::BitMask<ShaderFlags, ShaderFlagsMAX> ShaderFlagsMask;
ShaderFlagsMask gShaderFlags;

bool ShaderCompile::shouldCompileShaders(ShaderFlags flag)
{
    static bool gInitialized = false;
    if (!gInitialized)
    {
        gShaderFlags.setAll();
        gInitialized = true;
    }
    
    return gShaderFlags.test(flag);
}


unsigned getTextureSize(TextureChannels channels)
{
	assert(sizeof(kTextureSize) / sizeof(kTextureSize[0]) == TextureMAX);
	return kTextureSize[channels];
}

int getTypeSize(Type type)
{
    switch(type)
    {
        case TypeByte:
        case TypeUnsignedByte:
            return sizeof(char);
        case TypeShort:
        case TypeUnsignedShort:
            return sizeof(short);
        case TypeInt:
        case TypeUnsignedInt:
            return sizeof(int);
        case TypeFloat:
            return sizeof(float);
#if defined(CS_WINDOWS)
        case TypeDouble:
            return sizeof(double);
#endif
        default:
            assert(false);
            return -1;
    }
    return 0;
}

void modifyEngineStat(StatType type, int value)
{
    cs::EngineStats::incrementStatBy(type, value);
}
