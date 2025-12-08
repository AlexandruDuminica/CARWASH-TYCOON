#pragma once

class ReputationManager {
    double score_{3.0}; // 0..5
    int events_{0};

public:
    ReputationManager() = default;

    void onServed(double satisfaction);   // 0..5
    void onLost();                        // client pierdut

    double score() const noexcept { return score_; }
};
