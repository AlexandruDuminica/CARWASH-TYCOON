#pragma once

#include "MathUtils.h"

class DemandManager {
    int score_{0}; // poate fi negativ

public:
    void success() { ++score_; }
    void fail() { --score_; }

    int adjust() const {
        const int s = clampValue<int>(score_, -5, 5);
        if (s >= 2) return +1;
        if (s <= -2) return -1;
        return 0;
    }

    void reset() { score_ = 0; }
};
