#include "PCH.h"

#include "os/Analytics.h"

#if defined(CS_IOS)
    #include "GameAnalytics_iOS.h"
#endif

namespace cs
{
#if defined(CS_IOS)

    void Analytics::startLevel(const std::string& type, const std::string& eventId)
    {
        startProgression(type.c_str(), eventId.c_str());
    }
    
    void Analytics::finishLevel(const std::string& type, const std::string& eventId, int score)
    {
        endProgression(type.c_str(), eventId.c_str(), score);
    }

    void Analytics::modifyCurrency(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId)
    {
        changeCurrency(currency.c_str(), amount, itemType.c_str(), itemId.c_str());
    }

#else

    void Analytics::startLevel(const std::string& type, const std::string& eventId) { }
    void Analytics::finishLevel(const std::string& type, const std::string& eventId, int score) { }
    void Analytics::modifyCurrency(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId) { }

#endif

}
