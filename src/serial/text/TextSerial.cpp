#include "PCH.h"

#include "serial/text/TextSerial.h"

namespace cs
{
	namespace text
	{
		unsigned level = 0;
		const std::string kClassTypeStr = "CLASS_TYPE";

		const std::string kTrue = "true";
		const std::string kFalse = "false";

		inline const std::string& to_boolean(bool b)
		{
			return b ? kTrue : kFalse;
		}
	}
}