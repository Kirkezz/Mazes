#pragma once
#include "Space.h"
#include <chrono>
#include <map>
struct SpaceProfiling {
    static Space* spacePtr;
    static const std::map<size_t, std::string> mazes;
    static auto getMazeTime(size_t maze) {
        spacePtr->clear();
        auto start = std::chrono::steady_clock::now();
        (spacePtr->*(spacePtr->fillArr[maze]))();
        return std::chrono::steady_clock::now() - start;
    }
    static auto printMazeAverageTime(size_t maze, size_t n) {
        auto total = getMazeTime(maze);
        for(size_t i = 1; i < n; ++i) {
            total += getMazeTime(maze);
        }
        std::cout << maze << ") t average = " << total / n << "; n = " << n << std::endl;
    }
};
Space* SpaceProfiling::spacePtr = nullptr;
// const std::map<size_t, std::string> SpaceProfiling::mazes = {{1, "Recursive backtracker"}, {2, "Ellers"},        {3, "Kruskals"}, {4, "Prims"},
//                                                              {5, "Recursive devision"},    {6, "Aldous-Broder"}, {7, "Wilsons"},  {8, "Hunt-and-kill"}};
