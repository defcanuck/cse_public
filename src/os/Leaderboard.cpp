#include <string>

#include "os/Leaderboard.h"
#if defined(CS_IOS)
    #include "Platform_iOS.h"
#endif

#include "scripting/ScriptNotification.h"

#define LEADERBOARD_REQUEST_ID "LeaderboardRequest"

namespace cs
{
    std::string Leaderboard::getRequestId()
    {
        return LEADERBOARD_REQUEST_ID;
    }
    
	bool Leaderboard::addScore(const std::string& leaderboardName, int score)
    {
#if defined(CS_IOS)
        return SetLeaderboardEntry(leaderboardName.c_str(), score);
#else
        return false;
#endif
    }
    
    void Leaderboard::onLeaderboardRequestComplete(int scores)
    {
        log::info("Leaderboard returned with ", scores, " scores");
        ScriptNotification::getInstance()->pushNotification(LEADERBOARD_REQUEST_ID);
    }

    bool Leaderboard::requestScores(const std::string& leaderboardName, int maxEntries)
    {
#if defined(CS_IOS)
        return RequestLeaderboardEntries(leaderboardName.c_str(), maxEntries, &Leaderboard::onLeaderboardRequestComplete) > 0;
#endif
        return false;
    }
    
    PropertySet Leaderboard::getScores(const std::string& leaderboardName, int maxScores)
	{
        PropertySet entries;
        
#if defined(CS_IOS)
        char** names = new char*[maxScores];
        char** scores = new char*[maxScores];
        int* sortValue = new int[maxScores];
        
        for (int i = 0; i < maxScores; ++i)
        {
            names[i] = new char[128];
            scores[i] = new char[128];
        }
        int retScores = GetLeaderboardEntries(names, scores, sortValue, maxScores);
        
        if (retScores > 0)
        {
            for (int i = 0; i < retScores; ++i)
            {
                PropertySetPtr entry = CREATE_CLASS(PropertySet, names[i]);
                PropertyStringPtr prop = CREATE_CLASS(PropertyString, "score", scores[i]);
                entry->addProperty(prop);
                PropertyIntegerPtr sortable = CREATE_CLASS(PropertyInteger, "sortable", sortValue[i]);
                entry->addProperty(sortable);
                entries.addProperty(entry);
                
                delete [] names[i];
                delete [] scores[i];
            }
        }
        
        delete [] names;
        delete [] scores;
        delete [] sortValue;
#endif
        
        return entries;
	}
}
