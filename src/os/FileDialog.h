#pragma once

#include "global/Values.h"

namespace cs
{
	std::string openFileLoadDialog(StringList& desc, StringList& ext);
	std::string openFileSaveDialog(const std::string& desc, const std::string& ext);
}