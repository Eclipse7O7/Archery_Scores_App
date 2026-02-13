#pragma once
#include "AppState.h"
#include <string>

bool LoadArchersFromJson(const std::string& path, AppState& state);
void SaveArchersToJson(const std::string& path, const AppState& state);

int CalculateOverallHandicap(const Archer& archer);

bool ArcherExists(const AppState& state, const std::string& name);
