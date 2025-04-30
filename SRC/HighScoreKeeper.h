#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>

struct HighScoreEntry {
    std::string name;
    int score;
};

class HighScoreKeeper {
public:
    static std::vector<HighScoreEntry> LoadScores(const std::string& filename = "highscores.txt") {
        std::vector<HighScoreEntry> scores;
        std::ifstream file(filename);
        HighScoreEntry entry;
        while (file >> entry.name >> entry.score) {
            scores.push_back(entry);
        }
        file.close();
        return scores;
    }

    static void SaveScore(const std::string& name, int score, const std::string& filename = "highscores.txt") {
        auto scores = LoadScores(filename);
        scores.push_back({ name, score });
        std::sort(scores.begin(), scores.end(), [](auto& a, auto& b) { return a.score > b.score; });
        if (scores.size() > 10) scores.resize(10); // keep top 10

        std::ofstream file(filename);
        for (auto& s : scores) {
            file << s.name << " " << s.score << "\n";
        }
        file.close();
    }

    static void ShowScores(const std::string& filename = "highscores.txt") {
        auto scores = LoadScores(filename);
        std::cout << "\n--- High Scores ---\n";
        for (auto& s : scores) {
            std::cout << s.name << ": " << s.score << "\n";
        }
    }
};
