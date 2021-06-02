#pragma once

#include "global/BitMask.h"

#include <assert.h>
#include <memory>
#include <vector>
#include <map>

namespace cs
{

	class MetaData;

	enum LoadFlags
	{
		LoadFlagsNone = -1,
		LoadFlagsSceneGraph,
		LoadFlagsRenderable,
		LoadFlagsPhysics,
		LoadFlagsScript,
		LoadFlagsAnimation,
		LoadFlagsMAX
	};

	typedef BitMask<LoadFlags, LoadFlagsMAX> LoadFlagMask;

	extern const LoadFlagMask kLoadFlagMaskAll;
	extern const LoadFlagMask kLoadFlagMaskEmpty;

	class Serializable
	{
	public:

		virtual ~Serializable() { }
		virtual void onNew() { }
		virtual void onPreLoad() { }
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll) { }
		virtual size_t getKey() const { return -1; }

		virtual const MetaData* serialize(std::ostream& oss) 
		{
			assert(false);
			return nullptr; 
		}

		virtual const MetaData* getMetaData() { return nullptr; }

	};

	typedef std::shared_ptr<Serializable> SerializablePtr;
	typedef std::vector<SerializablePtr> SerializableVector;

	typedef std::map<size_t, SerializablePtr> SerializableMap;
	typedef std::map<std::string, SerializablePtr> SerializableMapString;
}