#pragma once

#include <vector>
#include <memory>
#include <iosfwd>

#include "Goal.h"

class CarWash;

class GoalManager {
    std::vector<std::unique_ptr<Goal>> goals_;

public:
    GoalManager() = default;

    void add(std::unique_ptr<Goal> g) {
        goals_.push_back(std::move(g));
    }

    void checkAll(const CarWash& wash);

    bool allAchieved() const;

    // procent global realizat (0.0 .. 1.0)
    double completionRatio() const;

    // afiseaza fiecare obiectiv cu procentul lui de progres
    void print(std::ostream& os, const CarWash& wash) const;
};
