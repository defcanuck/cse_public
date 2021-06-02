#include "PCH.h"

#include "global/NotificationCenter.h"

namespace cs
{

	void NotificationCenter::addNotification(const std::string& name, CallbackPtr& callback)
	{
		NotificationMap::iterator it = this->notifications.find(name);
		Event* notification = nullptr;
		if (it == this->notifications.end())
		{
			this->notifications[name] = Event();
			it = this->notifications.find(name);
			notification = &(it)->second;
		}
		else
		{
			notification = &(it)->second;
		}
		it->second.addCallback(callback);
	}

	bool NotificationCenter::triggerNotification(const std::string& name)
	{
		NotificationMap::iterator it = this->notifications.find(name);
		if (it == this->notifications.end())
		{
			return false;
		}
		it->second.invoke();
		return true;
	}
}