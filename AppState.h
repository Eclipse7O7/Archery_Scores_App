#pragma once
#include <string>
#include <vector>

struct ScoreEntry
{
    std::string bow_type = "";
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

    std::string validationError = "";

    std::string currentSeason = "25_26";

    bool allowScoreDeletion = false;
    int scoreIndexToDelete = -1;

    char newArcherName[64] = "";
    bool showAddArcher = false;

 

    // Comp Submit states

    struct CompEntry
    {
        std::string name = "";
        std::string bow_type = "";
        int score = 0;
        int scoreWithAtTheTimeHandicap = 0;
    };

    struct Competition
    {
        std::string name = "";
        std::vector<CompEntry> comp_results = {};
    };

    std::vector<Competition> comps = {};
    int selectedComp = 0; // Has to be 0, not -1 to prevent silent crashes

   
    // This is for all the current entries in the submitting current comp
    std::vector<CompEntry> competitionEntries = {};

    char existingArcherName[64] = "";
    int existingArcherScore = 0;
    char existingArcherBowType[8] = "";
	
    std::string validationCompError;


    bool allowCompDeletion = false;
    int compIndexToDelete = -1;

    char compName[128] = "";

	// These are for the comp results view

	bool showHandicapAtTimeOfComp = false;
	bool showBarebow = false;
    bool showRecurve = false;
	bool showCompound = false;
	bool showLongbow = false;
	bool showOther = false;

    char compSearchBuffer[128] = "";

};
