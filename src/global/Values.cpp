#include "PCH.h"

#include "global/Values.h"
#include "serial/MetaMacro.h"
#include "serial/Serialize.h"

namespace cs
{
	//DEFINE_META_PRIMITIVE(uint32);
	//DEFINE_META_PRIMITIVE(uint64);
	//DEFINE_META_PRIMITIVE(std::string);
	
	DEFINE_META_PRIMITIVE(uint16, integer16);
	DEFINE_META_PRIMITIVE(int32, integer32);
	DEFINE_META_PRIMITIVE(uint32, uinteger32);
	DEFINE_META_PRIMITIVE(uint64, uinteger64);
	DEFINE_META_PRIMITIVE(std::string, str);
	DEFINE_META_PRIMITIVE(bool, boolean);
	DEFINE_META_PRIMITIVE(float32, float32);
	DEFINE_META_PRIMITIVE(vec2, vector2f);
	DEFINE_META_PRIMITIVE(vec3, vector3f)
	DEFINE_META_PRIMITIVE(vec4, vector4f);
	DEFINE_META_PRIMITIVE(quat, quaternion);
	DEFINE_META_PRIMITIVE(RectF, rectangleFloat);

	DEFINE_META_PRIMITIVE(SizeF, sizeFloat);
	DEFINE_META_PRIMITIVE(SizeI, sizeInteger);

	DEFINE_META_PRIMITIVE(ColorB, colorByte);
	DEFINE_META_PRIMITIVE(ColorF, colorFloat);

	DEFINE_META_PRIMITIVE(RenderTraversalMask, traversalMask);

	void defineValues()
	{
		log::print(LogInfo, "Primitives Initialized");
	}
}
