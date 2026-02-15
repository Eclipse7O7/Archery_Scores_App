#include "DataManager.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include "json.hpp"
#include <filesystem>

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

// Need to ensure the file exists for each of my LoadXFromJson methods so it doesn't error when doing a new years
// This will also check if its empty which'll help in case someone makes the season files but doesn't fill them with the right json
static void EnsureFileExists(const std::string& path)
{
    namespace fs = std::filesystem;

    if (!fs::exists(path))
    {
        // Createing empty JSON file
        std::ofstream file(path);
        file << "{}";   // Needs to be a valid empty JSON object
        file.close();
    }

    // or could be empty?

    if (fs::file_size(path) == 0) 
    {
		std::ofstream file(path);
        file << "{}";   // Again, valid empty JSON object
		file.close();
    }
}

/*
bool LoadFromJson(const std::string& path, AppState& state)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;
    
    try {
        json j;
        file >> j;

        state.archers.clear();
        if (j.contains("archers") && j["archers"].is_array()) {
            for (auto& a : j["archers"])
            {
                Archer archer;
                archer.name = a["name"];

                for (auto& s : a["scores"])
                {
                    ScoreEntry score;
                    score.bow_type = s["bow_type"];
                    score.score = s["score"];
                    score.handicap = s["handicap"];
                    archer.scores.push_back(score);
                }

                archer.overallHandicap = CalculateOverallHandicap(archer);
                state.archers.push_back(archer);
            }

        }
 
    
        state.comps.clear();

        if (j.contains("competitions") && j["competitions"].is_array())
        {
            for (auto& b : j["competitions"])
            {
                AppState::Competition comp;
                comp.name = b["name"];

                for (auto& e : b["compResults"])
                {
                    AppState::CompEntry entry;
                    entry.name = e["name"];
                    entry.bow_type = e["bow_type"];
                    entry.score = e["score"];
                    comp.compResults.push_back(entry);
                }

                state.comps.push_back(comp);
            }
        }

    }
    catch (const std::exception& e) {
        return false;
    }

    return true;
}

void SaveToJson(const std::string& path, const AppState& state)
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
                {"bow_type", s.bow_type},
                {"score", s.score},
                {"handicap", s.handicap}
                });
        }

        j["archers"].push_back(a);
    }


    j["competitions"] = json::array();

    for (const AppState::Competition comp: state.comps) {

        json b;
        b["name"] = comp.name;
      
		b["compResults"] = json::array();

        for (const AppState::CompEntry e : comp.compResults) {
            b["compResults"].push_back({
                {"name", e.name},
                {"bow_type", e.bow_type},
                {"score", e.score}
                });
        }

        j["competitions"].push_back(b);
 
    }



    std::ofstream file(path);
    file << j.dump(3);
}
*/

bool LoadArchersFromJson(const std::string& path, AppState& state)
{
	EnsureFileExists(path);

    std::ifstream file(path);
    if (!file.is_open())
        return false;

    try {
        json j;
        file >> j;

        state.archers.clear();
        if (j.contains("archers") && j["archers"].is_array()) {
            for (auto& a : j["archers"])
            {
                Archer archer;
                //archer.name = a["name"];
                archer.name = a.value("name", "");


                for (auto& s : a["scores"])
                {
                    ScoreEntry score;
                    //score.bow_type = s["bow_type"];
                    score.bow_type = a.value("bow_type", "");
                    //score.score = s["score"];
                    score.score = a.value("score", 0);
                    //score.handicap = s["handicap"];
                    score.handicap = a.value("handicap", 0);
                    archer.scores.push_back(score);
                }

                archer.overallHandicap = CalculateOverallHandicap(archer);
                state.archers.push_back(archer);
            }

        }
        else if (!j.contains("competitions"))
        {
            j["archers"] = json::array();

            std::ofstream file(path);
            file << j.dump(3);
            file.close();
        }
    }
    catch (const std::exception& e) {
        return false;
    }

    return true;
}


bool LoadCompetitionsFromJson(const std::string& path, AppState& state)
{
	EnsureFileExists(path);

    std::ifstream file(path);
    if (!file.is_open())
        return false;

    try {
        json j;
        file >> j;

        state.comps.clear();

        if (j.contains("competitions") && j["competitions"].is_array())
        {
            for (auto& b : j["competitions"])
            {
                AppState::Competition comp;
                comp.name = b.value("name", "");
                //comp.name = b["name"];

                for (auto& e : b["compResults"])
                {
                    AppState::CompEntry entry;
                    //entry.name = e["name"];
                    entry.name = e.value("name", "");
                    //entry.bow_type = e["bow_type"];
                    entry.bow_type = e.value("bow_type", "");
                    //entry.score = e["score"];
                    entry.score = e.value("score", 0);
                    comp.compResults.push_back(entry);
                }

                state.comps.push_back(comp);
            }
        }
        // If the file with that name exists, it should be overwritten, not overwriting
        //  "archers" ones in case that contains archer data still and that would be a bigger loss
        //  for all the data to be gone due to a small mistake instead of just not working as is this case
        else if (!j.contains("archers")) 
        {
			j["competitions"] = json::array();

			std::ofstream file(path);
			file << j.dump(3);
			file.close();
        }
    }
    catch (const std::exception& e) {
        return false;
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
                {"bow_type", s.bow_type},
                {"score", s.score},
                {"handicap", s.handicap}
                });
        }

        j["archers"].push_back(a);
    }
    std::ofstream file(path);
    file << j.dump(3);
}

void SaveCompetitionsToJson(const std::string& path, const AppState& state)
{
    json j;
    j["competitions"] = json::array();

    for (const AppState::Competition comp : state.comps) {

        json b;
        b["name"] = comp.name;

        b["compResults"] = json::array();

        for (const AppState::CompEntry e : comp.compResults) {
            b["compResults"].push_back({
                {"name", e.name},
                {"bow_type", e.bow_type},
                {"score", e.score}
                });
        }

        j["competitions"].push_back(b);

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
