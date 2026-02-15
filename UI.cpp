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

    ImGui::Begin("UEA Archery Scores", __nullptr, flags);

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
            ImGui::InputText("Club Archer Name", state.newArcherName,
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
                    s.bow_type.c_str(), s.score, s.handicap
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
                    s.bow_type = state.newBowType;
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





    // Internal Comp Stuff


    if (ImGui::CollapsingHeader("Competition Submit"))
    {

        ImGui::Text("Season: %s", state.currentSeason);

        ImGui::BeginChild("ArcherListComp", ImVec2(290, 0), ImGuiChildFlags_Borders);

        for (int i = 0; i < state.competitionEntries.size(); i++)
        {
            const auto& entry = state.competitionEntries[i];

            ImGui::Text("%s - (%s)  Score: %d",
                entry.name.c_str(),
                entry.bow_type.c_str(),
                entry.score);


            if (state.allowCompDeletion)
            {
                ImGui::SameLine();
                ImGui::PushID(i);

                if (ImGui::Button("Delete"))
                {
                    state.compIndexToDelete = i;
                }

                ImGui::PopID();
            }
        }

        if (state.compIndexToDelete >= 0 &&
            state.compIndexToDelete < state.competitionEntries.size())
        {
            state.competitionEntries.erase(state.competitionEntries.begin() + state.compIndexToDelete);

            state.compIndexToDelete = -1;
        }



        ImGui::EndChild();


        ImGui::SameLine();

        ImGui::BeginChild("InputsList", ImVec2(250, 0), ImGuiChildFlags_Borders);


        ImGui::SetNextItemWidth(150);
        ImGui::InputText("Archer Name", state.existingArcherName,
            IM_ARRAYSIZE(state.existingArcherName));


        ImGuiWindowFlags dropdownFlags = ImGuiComboFlags_HeightSmall;//ImGuiComboFlags_HeightRegular
        static int item_selected_idx = 0;

        static const char* bowTypes[] = {
        "Barebow",
        "Recurve",
        "Compound",
        "Longbow",
        "Horsebow",
        "Warbow",
        "Other"
        };



        const char* combo_preview_value = bowTypes[item_selected_idx];
        ImGui::SetNextItemWidth(150);
        if (ImGui::BeginCombo("Bow Type", combo_preview_value, dropdownFlags))
        {
            for (int n = 0; n < IM_ARRAYSIZE(bowTypes); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (ImGui::Selectable(bowTypes[n], is_selected))
                {
                    item_selected_idx = n;

                    strncpy_s(state.existingArcherBowType, bowTypes[item_selected_idx], sizeof(state.existingArcherBowType) - 1);
                    state.existingArcherBowType[sizeof(state.existingArcherBowType) - 1] = '\0';
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemWidth(150);
        ImGui::InputInt("Score", &state.existingArcherScore);

        if (!state.validationCompError.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", state.validationCompError.c_str());
        }


        ImGui::EndChild();


        ImGui::SameLine();



        ImGui::BeginChild("Buttons");


        if (ImGui::Button("Add Result"))
        {

            state.validationCompError.clear();

            if (state.existingArcherName[0] == '\0')
            {
                state.validationCompError = "Name cannot be empty!";
            }
            else if (state.existingArcherScore < 0 || state.existingArcherScore > 600)
            {
                state.validationCompError = "Score has to be in range 0-600";
            }
            else
            {
                AppState::CompEntry entry;
                entry.name = state.existingArcherName;
                entry.bow_type = bowTypes[item_selected_idx];
                entry.score = state.existingArcherScore;

                state.competitionEntries.push_back(entry);

                state.existingArcherName[0] = '\0';
                state.existingArcherScore = 0;
            }

        }

        /* Moved to under inputs for visibilty
        if (!state.validationCompError.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", state.validationCompError.c_str());
        }*/


        ImGui::Checkbox("Enable\n deletion", &state.allowCompDeletion);


        ImGui::Separator();


        // submit button
        ImGui::Text("Comp Name");
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##CompName", state.compName, IM_ARRAYSIZE(state.compName));


        if (ImGui::Button("Submit Comp"))
        {
            // as it says on the tin
            // check that all values are valid, including that the names exist, otherwise just
            //  put an error message
		    
            
            // validate the comp name first, as if that is wrong then no point validating the rest of the data
            
			bool nameDupe = false;
            for (AppState::Competition comp : state.comps)
            {
                if (comp.name == state.compName)
                {
                    nameDupe = true;
                    break;
                }
			}


			if (state.compName[0] == '\0')
            {
                state.validationCompError = "Competition name cannot be empty!";
            }
            else if (nameDupe) {
				state.validationCompError = "Competition name already exists, please choose a different name!";
            }
            else if (state.competitionEntries.empty())
            {
                state.validationCompError = "No competition entries to submit!";
            }
            else
			{


                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// Still adds the competition even when there is an invalid entry, just with the error message, need to fix that


				bool allValid = true;
            

                for (AppState::CompEntry entry : state.competitionEntries)
                {


                    bool entryArcherFound = false;
                    //Archer archerFound;
					Archer* archerPtr = nullptr;

                    //for (Archer archer : state.archers)
					for (Archer& archer : state.archers)
                    {
                        if (entry.name == archer.name)
                        {
                            //entryArcherFound = true;
                            //archerFound = archer;
							archerPtr = &archer;
                        }
                    }

                    //if (entryArcherFound)
					if (archerPtr != nullptr && state.validationCompError.empty())
                    {

                        // if valid then add scores to the respective names in the archers file
                        // also add the comp results to a json file to then be able to open in Comp Results window/ heading thingy 

                        state.validationCompError.clear();


                        int handicap = CalculateHandicapFromScore(entry.score, entry.bow_type);

                        if (handicap < 0)
                        {
                            state.validationError = "Score too low for handicap calculation.";
                            return;
                        }

                        ScoreEntry s;
                        s.bow_type = entry.bow_type;
                        s.score = entry.score;
                        s.handicap = handicap;

                        // Add the new score
                        //archerFound.scores.push_back(s);
						archerPtr->scores.push_back(s);

                        // Recalculate overall handicap from all scores
                        // Using alias for readability
                        //archerFound.overallHandicap = RecalcHandicap(archerFound);
						archerPtr->overallHandicap = RecalcHandicap(*archerPtr);

						if (state.validationError.empty())
                        {
                            SaveArchersToJson(
                                ("archerData_" + state.currentSeason + ".json")
                                , state
                            );
                        }

                    }
                    else
                    {
                        state.validationCompError = "Archer '" + entry.name + "' not found, either add archer or redo comp entry";
                        allValid = false;
                    }


                }

                if (allValid) {
                    
                    /*
                    LoadCompetitionsFromJson(
                        ("competitionData_" + state.currentSeason + ".json")
                        , state
					);
                    */
                    
                    AppState::Competition newComp;
                    newComp.name = state.compName;
                    newComp.comp_results = state.competitionEntries;
                       
                    state.comps.push_back(newComp);

                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "%d", state.comps.size());

                    if (state.comps.size() < 2)
                    {
                        state.validationCompError = "Comps size too small";
                    }

                    SaveCompetitionsToJson(
                        ("competitionData_" + state.currentSeason + ".json")
                        , state
					);
                  
                    state.competitionEntries.clear();
                    state.compName[0] = '\0';
                }
            }
        }
        
        if (!state.validationCompError.empty())
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", state.validationCompError.c_str());
        }

        ImGui::EndChild();


        /*
        // Only show when delete is toggled

        ImGui::BeginChild("DeleteButton");
        if (ImGui::Button("Delete Button"))
        {
            // Delete
        }

        ImGui::EndChild();
        */

        

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
