#pragma once

class DemandManager {
    int score_{0};

public:
    void success() { ++score_; }
    void fail()    { --score_; }

    int adjust() const {
        if (score_ >= 2)  return +1;
        if (score_ <= -2) return -1;
        return 0;
    }

    void reset() { score_ = 0; }
};
