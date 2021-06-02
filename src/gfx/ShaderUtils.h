#pragma once

#include "ClassDef.h"
#include "gfx/gl/OpenGL.h"

#define OUTPUT_FORMAT "OUTPUT_FORMAT"
#if defined(CS_METAL)
    #define OUTPUT_FORMAT_STR "zyxw" // BGRA
#else
    #define OUTPUT_FORMAT_STR "xyzw" // RGBA
#endif

namespace cs
{

	namespace ShaderUtils
    {

        void replaceOutputFormat(std::string& str);
        void removePrecisionQualifiers(std::string& str);
        void addDefaultShaders();
    }
}
