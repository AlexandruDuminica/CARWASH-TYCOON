#include "ReputationManager.h"
#include <algorithm>

void ReputationManager::onServed(double satisfaction) {
    double sat = std::max(0.0, std::min(5.0, satisfaction));
    // medie mobila simpla
    score_ = 0.8 * score_ + 0.2 * sat;
    ++events_;
}

void ReputationManager::onLost() {
    // pierderea clientului scade reputatia
    score_ -= 0.2;
    if (score_ < 0.0) score_ = 0.0;
    ++events_;
}
