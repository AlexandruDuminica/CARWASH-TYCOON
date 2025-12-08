#include "GoalManager.h"
#include "Goal.h"
#include <ostream>

void GoalManager::checkAll(const CarWash& wash) {
    for (auto& g : goals_) {
        if (g) g->check(wash);
    }
}

bool GoalManager::allAchieved() const {
    for (const auto& g : goals_) {
        if (g && !g->isAchieved()) return false;
    }
    return !goals_.empty();
}

void GoalManager::print(std::ostream& os) const {
    os << "Obiective:\n";
    for (const auto& g : goals_) {
        if (g) {
            os << "  - " << *g << "\n";
        }
    }
}

std::ostream& operator<<(std::ostream& os, const GoalManager& gm) {
    gm.print(os);
    return os;
}
