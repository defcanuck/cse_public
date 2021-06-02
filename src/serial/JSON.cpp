#include "PCH.h"

#include "serial/JSON.h"

namespace cs
{
	namespace json
	{
		const char* kJsonTypes[] =
		{
			"number",	// JSON_NUMBER,
			"string",	// JSON_STRING,
			"array",	// JSON_ARRAY,
			"object",	// JSON_OBJECT,
			"true",		// JSON_TRUE,
			"false",	// JSON_FALSE,
		};
		
		void debug(JsonValue o, std::ostream& oss, uint32 depth)
		{
			oss << kJsonTypes[o.getTag()] << std::endl;

			switch (o.getTag())
			{
			case JSON_NUMBER:
				oss << o.toNumber() << std::endl;
				break;

			case JSON_STRING:
				oss << o.toString() << std::endl;
				break;

			case JSON_ARRAY:
				for (auto i : o)
				{
					debug(i->value, oss, depth + 1);
				}
				break;

			case JSON_OBJECT:
				for (auto i : o)
				{
					oss << i->key << " = ";
					debug(i->value, oss, depth + 1);
				}
				break;
			case JSON_TRUE:
				oss << "true" << std::endl;
				break;
			case JSON_FALSE:
				oss << "false" << std::endl;
				break;
			case JSON_NULL:
				oss << "null" << std::endl;
				break;
			}
		}
	}
}