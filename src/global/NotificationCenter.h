#pragma once

#include "global/Singleton.h"
#include "global/Event.h"

#include <map>
#include <string>

namespace cs
{
	class NotificationCenter : public Singleton<NotificationCenter>
	{
	public:

		NotificationCenter() { }
		void addNotification(const std::string& name, CallbackPtr& callback);
		bool triggerNotification(const std::string& name);
	
	private:

		typedef std::map<std::string, Event> NotificationMap;
		NotificationMap notifications;

	};
}