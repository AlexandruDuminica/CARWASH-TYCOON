#include "../headers/GoalManager.h"
#include "../headers/CarWash.h"

#include <iomanip>
#include <ostream>

void GoalManager::checkAll(const CarWash &wash) {
    for (auto &g: goals_) {
        if (g) g->check(wash);
    }
}

bool GoalManager::allAchieved() const {
    int total = 0;
    int done = 0;
    for (const auto &g: goals_) {
        if (!g) continue;
        ++total;
        if (g->isAchieved()) ++done;
    }
    return total > 0 && done == total;
}

double GoalManager::completionRatio() const {
    int total = 0;
    int done = 0;
    for (const auto &g: goals_) {
        if (!g) continue;
        ++total;
        if (g->isAchieved()) ++done;
    }
    if (total == 0) return 0.0;
    return static_cast<double>(done) / static_cast<double>(total);
}

void GoalManager::print(std::ostream &os, const CarWash & /*wash*/) const {
    os << "Obiective:\n";
    if (goals_.empty()) {
        os << "  (niciun obiectiv definit)\n";
        return;
    }

    for (const auto &g: goals_) {
        if (!g) continue;
        double p = g->progress() * 100.0;
        if (p < 0.0) p = 0.0;
        if (p > 100.0) p = 100.0;
        os << "  - " << g->description()
                << " [" << (g->isAchieved() ? "OK" : "IN PROGRES") << "] "
                << std::fixed << std::setprecision(1) << p << "%\n";
    }

    double global = completionRatio() * 100.0;
    if (global < 0.0) global = 0.0;
    if (global > 100.0) global = 100.0;

    os << "Progres global obiective: "
            << std::fixed << std::setprecision(1)
            << global << "%\n";
}
