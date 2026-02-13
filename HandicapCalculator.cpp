#include "HandicapCalculator.h"
#include <fstream>
#include <sstream>
#include <map>

//static std::unordered_map<int, int> portsmouth;
//static std::unordered_map<int, int> portsmouthCompound; 

static std::map<int, int> portsmouth;
static std::map<int, int> portsmouthCompound;

int GetHandicapFromScore(const std::map<int, int>& table, int score)
{
    int bestHandicap = -1;
    int bestScore = std::numeric_limits<int>::max();

    for (const auto& [handicap, tableScore] : table)
    {
        if (tableScore >= score && tableScore < bestScore)
        {
            bestScore = tableScore;
            bestHandicap = handicap;
        }
    }

    return bestHandicap;
}



bool LoadHandicapTable(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::string line;
    std::getline(file, line); // header

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;

        int handicap = 0;
        int portScore = 0;
        int portCompScore = 0;

        std::getline(ss, token, ','); handicap = std::stoi(token);
        std::getline(ss, token, ','); portScore = std::stoi(token);
		std::getline(ss, token, ','); // skip portTriple Score
        std::getline(ss, token, ','); portCompScore = std::stoi(token);
		std::getline(ss, token, ','); // skip portCompTriple Score

        portsmouth[handicap] = portScore;
        portsmouthCompound[handicap] = portCompScore;
    }

    return true;
}

int CalculateHandicapFromScore(int score, const std::string& bowType)
{
    const bool isCompound = (bowType == "Compound");

    auto& table = isCompound ? portsmouthCompound : portsmouth;

    if (table.empty()) return 0; // table not loaded

	return GetHandicapFromScore(table, score);

}
