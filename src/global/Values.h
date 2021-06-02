#pragma once

#include <cstdint>

#include <Box2D/Common/b2Settings.h>

#define GTFO(arr) exit(arr)

#include <string>
#include <vector>
#include <map>

typedef std::vector<std::string> StringList;
typedef std::map<std::string, std::string> StringMap;

namespace cs
{
	// kludge
	// Turns out Box2D actually typedefs these values for us
	// So go ahead and use their typedefs to avoid conflicts
	/*
	typedef int64_t int64;
	typedef int32_t int32;
	typedef short int16;

	typedef unsigned int uint32;
	typedef unsigned long uint64;
	typedef unsigned short uint16;

	typedef float float32;
	*/

	typedef unsigned char uchar;

	void defineValues();
	
}