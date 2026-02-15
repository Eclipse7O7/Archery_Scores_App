#pragma once
#include "AppState.h"
#include <string>

bool LoadFromJson(const std::string& path, AppState& state);
void SaveToJson(const std::string& path, const AppState& state);

int CalculateOverallHandicap(const Archer& archer);

bool ArcherExists(const AppState& state, const std::string& name);
