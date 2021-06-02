#pragma once

enum AttributeType
{
    AttribNone = -1,
    AttribPosition,
    AttribTexCoord0,
    AttribTexCoord1,
    AttribNormal,
    AttribTangent,
    AttribColor,
    AttribBones,
    AttribWeights,
    AttribMAX
};

struct Attribute
{
    AttributeType type;
    unsigned dataType;
    unsigned count;
    unsigned offset;
};
