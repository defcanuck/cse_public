#pragma once

#include "gfx/Types.h"

namespace cs
{
    class Geometry;

    struct ShaderBindParams
    {
        ShaderBindParams()
            : geom(nullptr)
            , channels(TextureNone)
            , depth(TextureNone)
            , index(0)
            , blend(true)
            , srcBlend(BlendSrcAlpha)
            , dstBlend(BlendOneMinusSrcAlpha)
        { }
        
        Geometry* geom;
        TextureChannels channels;
        TextureChannels depth;
        int32 index;
        bool blend;
        BlendType srcBlend;
        BlendType dstBlend;
    };

    struct ShaderUniformBindParams
    {
        ShaderUniformBindParams()
        {
            memset(uniformBufferArray, 0, ShaderMAX * sizeof(void*));
        }
        
         void* uniformBufferArray[ShaderMAX];
    };

    struct ShaderBindParamsCompare
    {
       bool operator()(const ShaderBindParams& lhs, const ShaderBindParams& rhs) const
       {
           
           //if (lhs.geom < rhs.geom)  return true;
          // if (lhs.geom > rhs.geom)  return false;
                      
           if (lhs.channels < rhs.channels)  return true;
           if (lhs.channels > rhs.channels)  return false;
           
           if (lhs.depth < rhs.depth)  return true;
           if (lhs.depth > rhs.depth)  return false;
           
           if (lhs.blend < rhs.blend)  return true;
           if (lhs.blend > rhs.blend)  return false;
           
           if (lhs.srcBlend < rhs.srcBlend)  return true;
           if (lhs.srcBlend > rhs.srcBlend)  return false;
           
           if (lhs.dstBlend < rhs.dstBlend)  return true;
           if (lhs.dstBlend > rhs.dstBlend)  return false;

           return false;
       }
    };
}
