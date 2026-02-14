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
    std::vector<Archer> archers = {};
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


    // Comp Submit states

    struct CompEntry
    {
        std::string name;
        std::string bowType;
        int score;
    };

    std::vector<CompEntry> competitionEntries = {};

    char existingArcherName[64] = "";
    int existingArcherScore = 0;
    char existingArcherBowType[8] = "";

    std::string validationCompError;


    bool allowCompDeletion = false;
    int compIndexToDelete = -1;

    char compFileName[128] = "";
};
