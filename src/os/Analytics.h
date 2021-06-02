#pragma once

#include <string>


#include "global/PropertySet.h"

namespace cs
{
	class Analytics
    {
    public:
        
        static void startLevel(const std::string& type, const std::string& level);
        static void finishLevel(const std::string& type, const std::string& level, int score);
        
        static void modifyCurrency(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId);
    };
}
