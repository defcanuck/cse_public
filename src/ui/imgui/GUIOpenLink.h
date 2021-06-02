#pragma once

#include "ClassDef.h"

#include "global/Singleton.h"
#include "global/Callback.h"

#include <map>

namespace cs
{

	typedef CallbackArg1<bool, std::string> GUIOpenMetaLink;
	typedef std::shared_ptr<GUIOpenMetaLink> GUIOpenMetaLinkPtr;
	
	class GUIOpenLink : public Singleton<GUIOpenLink>
	{
	public:

		void addMetaOpenLink(const std::string& meta_name, GUIOpenMetaLinkPtr& func)
		{
			const MetaData* metadata = MetaManager::getInstance()->get(meta_name);
			assert(metadata);
			this->addMetaOpenLink(metadata, func);
		}

		void addMetaOpenLink(const MetaData* metadata, GUIOpenMetaLinkPtr& func)
		{
			this->linkmap[metadata] = func;
		}

		bool hasMetaOpenLink(const MetaData* metadata)
		{
			return this->linkmap.find(metadata) != this->linkmap.end();
		}

		bool invokeLinkMap(const MetaData* metadata, const std::string& name)
		{
			GUIOpenLinkMap::iterator it = this->linkmap.find(metadata);
			assert(it != this->linkmap.end());
			
			// Probably need a real invoke with a return value here :(
			it->second->invoke(name);
			return true;
		}

	private:
		
		typedef std::map<const MetaData*, GUIOpenMetaLinkPtr> GUIOpenLinkMap;
		GUIOpenLinkMap linkmap;
	
	};
}