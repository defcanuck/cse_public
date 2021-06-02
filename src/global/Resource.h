#pragma once

#include "ClassDef.h"

#include <string>
#include <fstream>
#include "os/LogManager.h"


namespace cs
{
	CLASS_DEFINITION_REFLECT(Resource)
	
	public:

		Resource(const std::string& nname) 
			: name(nname)
			, flushable(true)
		{ 
#if defined(LOG_RESOURCE_CREATION)
			log::print(LogInfo, "Created Resource: ", this->name);
#endif
		}

		virtual ~Resource() 
		{
#if defined(LOG_RESOURCE_CREATION)
			log::print(LogInfo, "Deleted Resource : ", this->name);
#endif
		}

		const std::string& getName() const { return this->name; }
		void setFlushable(bool flush) { this->flushable = flush; }

	protected:

		Resource() : name("error") { }

		std::string name;
		bool flushable;
	
	};
}