#pragma once
#include <string>
#include <unordered_map>

bool LoadHandicapTable(const std::string& path);
int CalculateHandicapFromScore(int score, const std::string& bowType);
