#pragma once

#include "global/Values.h"
#include "global/Singleton.h"
#include "scripting/LuaCallback.h"

namespace cs
{
    struct ScriptNotificationKey
    {
        std::string name;
        std::string owner;
    };
    
    struct ScriptNotificationKeyCompare
    {
        bool operator()(const ScriptNotificationKey& lhs, const ScriptNotificationKey& rhs) const
        {
            if (lhs.name < rhs.name)  return true;
            if (lhs.name > rhs.name)  return false;
            
            if (lhs.owner < rhs.owner)  return true;
            if (lhs.owner > rhs.owner)  return false;
            
            return false;
        }
    };
    
    class ScriptNotification : public Singleton<ScriptNotification>
    {
    public:
        
        void pushNotification(const std::string& name);
        
        void flushNotifications();
        bool addListener(const std::string& name, const std::string& owner, LuaCallbackPtr& callback);
        bool removeListener(const std::string& name, const std::string& owner);
        
    private:
        
        StringList notificationsRecieved;
        
        typedef std::map<ScriptNotificationKey, LuaCallbackPtr, ScriptNotificationKeyCompare> NotificationListeners;
        NotificationListeners listeners;
        
    };
}
