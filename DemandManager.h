#pragma once

class DemandManager {
    int score{0};

public:
    void success() { score++; }
    void fail() { score--; }

    int adjust() const {
        if(score >= 2) return +1;
        if(score <= -2) return -1;
        return 0;
    }

    void reset() { score = 0; }
};
