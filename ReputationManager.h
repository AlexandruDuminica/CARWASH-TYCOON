#pragma once

#include "RunningStat.h"

class ReputationManager {
    double score_{3.0}; // 0..5
    int events_{0};

    RunningStat<double> satisfaction_{};
    RunningStat<int> servedCount_{};

public:
    void onServed(double satisfaction);

    void onLost();

    double score() const noexcept { return score_; }

    double avgSatisfaction() const noexcept {
        return satisfaction_.count() > 0 ? satisfaction_.average() : 0.0;
    }

    int totalServedSamples() const noexcept {
        return servedCount_.count();
    }
};
