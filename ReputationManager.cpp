#include "ReputationManager.h"

#include "MathUtils.h"

#include <algorithm>

void ReputationManager::onServed(double satisfaction) {
    double sat = clampValue<double>(satisfaction, 0.0, 5.0);
    satisfaction_.add(sat);
    servedCount_.add(1);

    events_++;
    score_ = 0.8 * score_ + 0.2 * sat;
    score_ = clampValue<double>(score_, 0.0, 5.0);
}

void ReputationManager::onLost() {
    events_++;
    score_ -= 0.10;
    score_ = clampValue<double>(score_, 0.0, 5.0);
}
