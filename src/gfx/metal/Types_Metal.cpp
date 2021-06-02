#include "PCH.h"

#include "gfx/Types.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{
	uint32 kTypeConvert[] =
	{
		GL_BYTE,
		GL_UNSIGNED_BYTE,
		GL_SHORT,
		GL_UNSIGNED_SHORT,
		GL_INT,
		GL_UNSIGNED_INT,
		GL_FLOAT,
#if defined(CS_WINDOWS)
		GL_DOUBLE
#endif
	};
    
	uint32 kTypeSize[] =
	{
		sizeof(GLbyte),
		sizeof(GLubyte),
		sizeof(GLshort),
		sizeof(GLushort),
		sizeof(GLint),
		sizeof(GLuint),
		sizeof(GLfloat),
#if defined(CS_WINDOWS)
		sizeof(GLdouble)
#endif
	};

	uint32 kStateConvert[] =
	{
		GL_SCISSOR_TEST,
		GL_DEPTH_TEST,
		GL_BLEND,
		GL_CULL_FACE
	};
	
	uint32 kDrawConvert[] =
	{
		GL_POINTS,
		GL_LINES,
		GL_LINE_LOOP,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
#if defined(CS_WINDOWS)
		GL_QUADS,
#endif
	};

	uint32 kTextureSampleConvert[] =
	{
		GL_CLAMP_TO_EDGE,
		GL_REPEAT,
	};

	uint32 kClearConvert[] =
	{
		GL_COLOR_BUFFER_BIT,
		GL_DEPTH_BUFFER_BIT
	};

#if defined(CS_WINDOWS)
    
	uint32 kTextureConvertSrc[] = 
	{
		GL_ALPHA,
		GL_RGB,
		GL_RGBA,
		GL_RGB,
		GL_RGBA,
        GL_RGB565,
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT32
	};

	uint32 kTextureConvertDst[] =
	{
		GL_ALPHA,
		GL_RGB,
		GL_RGBA,
		GL_RGB,
		GL_RGBA,
        GL_RGB565,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT
	};
#else
    
    uint32 kTextureConvertSrc[] =
    {
        GL_ALPHA,
        GL_RGB,
        GL_RGBA,
		GL_RGB,
		GL_RGBA,
        GL_RGB565,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT16
    };
    
    uint32 kTextureConvertDst[] =
    {
        GL_ALPHA,
        GL_RGB,
        GL_BGRA,
		GL_RGB,
		GL_RGBA,
        GL_RGB565,
        GL_DEPTH_COMPONENT,
        GL_DEPTH_COMPONENT,
        GL_DEPTH_COMPONENT
    };
#endif

	uint32 kTextureSize[] =
	{
        1,  // TextureAlpha
        3,  // TextureRGB
        4,  // TextureRGBA
        2,  // TextureRGB565
        2,  // TextureDepth16
        3,  // TextureDepth24
        4,  // TextureDepth32
	};

#if defined(CS_WINDOWS)
	uint32 kDepthComponentConvert[] =
	{
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT32
	};
#else
    uint32 kDepthComponentConvert[] =
    {
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT16
    };
#endif

	uint32 kBlendTypeConvert[] =
	{
		GL_ZERO,
		GL_ONE,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA
	};

	uint32 kDepthTypeConvert[] =
	{
		GL_NEVER,
		GL_EQUAL,
		GL_LESS,
		GL_LEQUAL,
		GL_GREATER,
		GL_GEQUAL,
		GL_NOTEQUAL,
		GL_ALWAYS
	};

	uint32 kCullFaceConvert[]
	{
		GL_FRONT,
		GL_BACK,
		GL_FRONT_AND_BACK,
	};

	uint32 kFrontFaceConvert[]
	{
		GL_CW,
		GL_CCW
	};
}
