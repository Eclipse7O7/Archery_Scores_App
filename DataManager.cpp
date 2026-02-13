#include "DataManager.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include "json.hpp"

using json = nlohmann::json;

int CalculateOverallHandicap(const Archer& archer)
{
    std::vector<int> handicaps = {};
    for (const auto& s : archer.scores)
        handicaps.push_back(s.handicap);

    if (handicaps.empty())
        return 0;
    if (handicaps.size() == 1)
		return handicaps[0];
	if (handicaps.size() == 2)
        return ((handicaps[0] + handicaps[1]) / 2);

    std::sort(handicaps.begin(), handicaps.end());
    return (handicaps[0] + handicaps[1] + handicaps[2]) / 3;
}

bool LoadArchersFromJson(const std::string& path, AppState& state)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    json j;
    file >> j;

    state.archers.clear();

    for (auto& a : j["archers"])
    {
        Archer archer;
        archer.name = a["name"];

        for (auto& s : a["scores"])
        {
            ScoreEntry score;
            score.bowType = s["bow_type"];
            score.score = s["score"];
            score.handicap = s["handicap"];
            archer.scores.push_back(score);
        }

        archer.overallHandicap = CalculateOverallHandicap(archer);
        state.archers.push_back(archer);
    }

    return true;
}

void SaveArchersToJson(const std::string& path, const AppState& state)
{
    json j;
    j["archers"] = json::array();

    for (const auto& archer : state.archers)
    {
        json a;
        a["name"] = archer.name;
        a["overall_handicap"] = archer.overallHandicap;

        for (const auto& s : archer.scores)
        {
            a["scores"].push_back({
                {"bow_type", s.bowType},
                {"score", s.score},
                {"handicap", s.handicap}
                });
        }

        j["archers"].push_back(a);
    }

    std::ofstream file(path);
    file << j.dump(3);
}


static std::string ToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool ArcherExists(const AppState& state, const std::string& name)
{
    const std::string target = ToLower(name);

    for (const auto& a : state.archers)
    {
        if (ToLower(a.name) == target)
            return true;
    }
    return false;
}
