#include "UI.h"
#include "imgui/imgui.h"

#include "HandicapCalculator.h"
#include "DataManager.h"
#include <limits>

// An alias for readability
auto RecalcHandicap = CalculateOverallHandicap;

static std::string Normalise(const std::string& s);

void DrawMainUI(AppState& state)
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("UEA Archery Scores", __nullptr , flags);

    if (ImGui::CollapsingHeader("Scores", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static const char* seasons[] = {
        "25_26",
        "26_27",
        "27_28"
        };

        static int seasonIndex = 0;

        state.currentSeason = seasons[seasonIndex];

        ImGui::Text("Season");
        ImGui::SameLine();

        if (ImGui::Combo("##season", &seasonIndex, seasons, IM_ARRAYSIZE(seasons)))
        {
            state.currentSeason = seasons[seasonIndex];

            state.archers.clear();
            state.selectedArcher = -1;

            LoadArchersFromJson(
                "archerData_" + state.currentSeason + ".json",
                state
            );
        }


        // --- Search ---
        ImGui::InputText("Search Archer", state.searchBuffer, IM_ARRAYSIZE(state.searchBuffer));
        ImGui::Separator();

        // --- Archer List ---
        ImGui::BeginChild("ArcherList", ImVec2(250, 0), ImGuiChildFlags_Borders);
        for (int i = 0; i < state.archers.size(); i++)
        {
            const Archer& a = state.archers[i];

            if (strlen(state.searchBuffer) > 0 &&
                a.name.find(state.searchBuffer) == std::string::npos)
                continue;

            if (ImGui::Selectable(a.name.c_str(), state.selectedArcher == i))
                state.selectedArcher = i;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // --- Archer Details ---
        ImGui::BeginChild("Details", ImVec2(0, 0), ImGuiChildFlags_Borders);

        if (ImGui::Button("Add New Archer"))
        {
            state.showAddArcher = true;
        }

        if (state.showAddArcher)
        {
            ImGui::InputText("Archer Name", state.newArcherName,
                IM_ARRAYSIZE(state.newArcherName));

            if (ImGui::Button("Confirm Add"))
            {
                state.validationError.clear();

                if (state.newArcherName[0] == '\0')
                {
                    state.validationError = "Archer name cannot be empty.";
                }
                else if (ArcherExists(state, state.newArcherName))
                {
                    state.validationError = "Archer already exists.";
                }
                else
                {
                    Archer newArcher;
                    newArcher.name = state.newArcherName;
                    newArcher.overallHandicap = 0;
                    newArcher.scores.clear();

                    state.archers.push_back(newArcher);

                    SaveArchersToJson(
                        "archerData_" + state.currentSeason + ".json",
                        state
                    );

                    state.newArcherName[0] = '\0';
                    state.showAddArcher = false;
                }
            }
            if (!state.validationError.empty())
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1),
                    "%s", state.validationError.c_str());
            }



            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                state.newArcherName[0] = '\0';
                state.showAddArcher = false;
            }
        }

        ImGui::Separator();

        if (state.selectedArcher >= 0)
        {
            Archer& a = state.archers[state.selectedArcher];

            ImGui::Text("Name: %s", a.name.c_str());
            ImGui::Text("Overall Handicap: %d", a.overallHandicap);
            ImGui::Separator();

            ImGui::Text("Scores");
            ImGui::Checkbox("Enable score deletion", &state.allowScoreDeletion);

            for (int i = 0; i < a.scores.size(); i++)
            {
                const auto& s = a.scores[i];

                ImGui::BulletText(
                    "%s | Score: %d | Handicap: %d",
                    s.bowType.c_str(), s.score, s.handicap
                );

                if (state.allowScoreDeletion)
                {
                    ImGui::SameLine();
                    ImGui::PushID(i);

                    if (ImGui::Button("Delete"))
                    {
                        state.scoreIndexToDelete = i;
                    }

                    ImGui::PopID();
                }
            }
            if (state.scoreIndexToDelete >= 0 &&
                state.scoreIndexToDelete < a.scores.size())
            {
                a.scores.erase(a.scores.begin() + state.scoreIndexToDelete);

                // Recalculate overall handicap after deletion
                a.overallHandicap = RecalcHandicap(a);

                // Save updated data to current season file
                SaveArchersToJson(
                    "archerData_" + state.currentSeason + ".json",
                    state
                );

                state.scoreIndexToDelete = -1;
            }


            ImGui::Separator();
            ImGui::Text("Add New Score");

            ImGui::InputText("Bow Type", state.newBowType, IM_ARRAYSIZE(state.newBowType));
            ImGui::InputInt("Score", &state.newScore);

            if (ImGui::Button("Add Score"))
            {
                state.validationError.clear();

                std::string bow = Normalise(state.newBowType);

                if (state.selectedArcher < 0)
                {
                    state.validationError = "No archer selected.";
                }
                else if (state.newBowType[0] == '\0')
                {
                    state.validationError = "Bow type cannot be empty.";
                }
                else if (bow != std::string("recurve") &&
                    bow != std::string("compound") &&
                    bow != std::string("barebow") &&
                    bow != std::string("longbow") &&
                    bow != std::string("horsebow") &&
                    bow != std::string("warbow") &&
                    bow != std::string("other"))
                {
                    state.validationError = "Bow type must be Barebow, Recurve, Compound,\n Longbow, Horsebow, Warbow, or Other.";
                }
                else if (state.newScore < 0 || state.newScore > 600)
                {
                    state.validationError = "Score must be between 0 and 600.";
                }
                else
                {

                    Archer& a = state.archers[state.selectedArcher];

                    int handicap = CalculateHandicapFromScore(state.newScore, state.newBowType);

                    if (handicap < 0)
                    {
                        state.validationError = "Score too low for handicap calculation.";
                        return;
                    }

                    ScoreEntry s;
                    s.bowType = state.newBowType;
                    s.score = state.newScore;
                    s.handicap = handicap;

                    // Add the new score
                    a.scores.push_back(s);

                    // Recalculate overall handicap from all scores
                    // Using alias for readability
                    a.overallHandicap = RecalcHandicap(a);

                    SaveArchersToJson(
                        ("archerData_" + state.currentSeason + ".json")
                        , state
                    );

                    // Reset input fields
                    state.newScore = 0;
                    state.newBowType[0] = '\0';
                }
            }
            if (!state.validationError.empty())
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", state.validationError.c_str());
            }
        }
        else
        {
            ImGui::TextDisabled("Select an archer to view details.");
        }

        ImGui::EndChild();
    }


    if (ImGui::CollapsingHeader("Internal Competition")) {
        
        ImGui::Text("Season: %s", state.currentSeason);

        ImGui::BeginChild("ArcherList", ImVec2(250, 0), ImGuiChildFlags_Borders);

        ImGui::EndChild();
    
    }


    ImGui::End();

    
}

static std::string Normalise(const std::string& s)
{
    std::string out;
    for (char c : s)
    {
        if (!std::isspace((unsigned char)c))
            out.push_back(std::tolower((unsigned char)c));
    }
    return out;
}
