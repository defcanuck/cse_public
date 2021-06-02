#pragma once

#include <string>


#include "global/PropertySet.h"

namespace cs
{
	class Leaderboard
    {
    public:
        
        static std::string getRequestId();
        static bool addScore(const std::string& leaderboardName, int score);
		
        static bool requestScores(const std::string& leaderboardName, int maxEntries);
        static PropertySet getScores(const std::string& leaderboardName, int maxEntries);
        
    private:
        
        static void onLeaderboardRequestComplete(int scores);
    };
}
