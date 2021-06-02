#include "scripting/ScriptNotification.h"

namespace cs
{
    void ScriptNotification::pushNotification(const std::string& name)
    {
        this->notificationsRecieved.push_back(name);
    }
    
    void ScriptNotification::flushNotifications()
    {
        StringList iterNotificationsRecieved = notificationsRecieved;
        for (auto& n : iterNotificationsRecieved)
        {
            NotificationListeners iterListeners = this->listeners;
            for (auto& listener : iterListeners)
            {
                if (listener.first.name == n)
                {
                    (*listener.second.get())();
                }
            }
        }
        this->notificationsRecieved.clear();
    }
    
    bool ScriptNotification::addListener(const std::string& name, const std::string& owner, LuaCallbackPtr& callback)
    {
        ScriptNotificationKey key;
        key.name = name;
        key.owner = owner;
        NotificationListeners::iterator it = this->listeners.find(key);
        if (it != this->listeners.end())
        {
            log::error("Duplicate listeners added for ", name);
            return false;
        }
        this->listeners[key] = callback;
        return true;
    }
    
    bool ScriptNotification::removeListener(const std::string& name, const std::string& owner)
    {
        ScriptNotificationKey key;
        key.name = name;
        key.owner = owner;
        NotificationListeners::iterator it = this->listeners.find(key);
        if (it != this->listeners.end())
        {
            this->listeners.erase(it);
            return true;
        }
        return false;
    }
}
