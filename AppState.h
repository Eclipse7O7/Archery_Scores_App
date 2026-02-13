#pragma once
#include <string>
#include <vector>

struct ScoreEntry
{
    std::string bowType = "";
    int score = 0;
    int handicap = 0;
};

struct Archer
{
    std::string name = "";
    std::vector<ScoreEntry> scores = {};
    int overallHandicap = 0;
};

struct AppState
{
    std::vector<Archer> archers;
    int selectedArcher = -1;

    char searchBuffer[128] = "";
    char newBowType[64] = "";
    int newScore = 0;

    std::string validationError;

    std::string currentSeason = "25_26";

    bool allowScoreDeletion = false;
    int scoreIndexToDelete = -1;

    char newArcherName[64] = "";
    bool showAddArcher = false;


    // Internal Comp states



};
