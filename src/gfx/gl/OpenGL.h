#pragma once

#include "os/LogManager.h"

#if defined(WIN32)

	#include <Windows.h>
	
	#define USING_GLEW 1
	//#define USING_GL3W 1

	#if defined(USING_GL3W)
		#define GL3_PROTOTYPES 1

		#include <GL/gl3w.h>
		#include <GL/glcorearb.h>

		#define GL_GLEXT_PROTOTYPES 1
		#include <GL/glext.h>

	#elif defined(USING_GLEW)
	#define GLEW_STATIC 1
		#include <GL/glew.h>
		#include <GL/gl.h>

	#else
		#include <gl/gl.h>
	#endif

#endif

#if defined(CS_OSX)

    #include <OpenGL/gl.h>

#endif

#if defined(CS_IOS) || defined(CS_IPHONE)

    #define GL_GLEXT_PROTOTYPES 1
    #define GL_OES_mapbuffer

    #include <OpenGLES/gltypes.h>
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>

    #define glMapBuffer glMapBufferOES
    #define glUnmapBuffer glUnmapBufferOES

    #define glGenVertexArrays glGenVertexArraysOES
    #define glBindVertexArray glBindVertexArrayOES
    #define glDeleteVertexArrays glDeleteVertexArraysOES

    #define GL_WRITE_ONLY GL_WRITE_ONLY_OES

	#define glTexStorage2D glTexStorage2DEXT

#endif

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
		        } while (0)

void CheckOpenGLError(const char* stmt, const char* fname, int line);

#else
	#define GL_CHECK(stmt) stmt
#endif

inline void glInit()
{

#if defined(USING_GL3W)

	if (gl3wInit()) 
	{
		cs::log::error("failed to initialize OpenGL");
		return;
	}
	
	if (!gl3wIsSupported(3, 2)) 
	{
		cs::log::error("OpenGL 3.2 not supported");
		return;
	}
	cs::log::info("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

#elif defined(USING_GLEW)
	glewExperimental = TRUE; 
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		cs::log::print(cs::LogError, glewGetErrorString(err));
	}
	cs::log::print(cs::LogDebug, "Status: Using GLEW ", glewGetString(GLEW_VERSION));
	GL_CHECK(void(0));
#endif

}

namespace cs
{
	void* GetAnyGLFuncAddress(const char *name);
}

namespace gltest
{
    void update();
    int initialize();
    bool testFrameBuffers();
}
