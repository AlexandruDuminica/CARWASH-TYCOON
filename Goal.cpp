#include "Goal.h"
#include "CarWash.h"

#include <algorithm>

bool ProfitGoal::check(const CarWash& wash) {
    if (!achieved_ && wash.totalCash() >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

double ProfitGoal::progress(const CarWash& wash) const {
    if (target_ <= 0.0) return 1.0;
    double p = wash.totalCash() / target_;
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;
    return p;
}

bool CarsServedGoal::check(const CarWash& wash) {
    if (!achieved_ && wash.totalCarsServed() >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

double CarsServedGoal::progress(const CarWash& wash) const {
    if (target_ <= 0) return 1.0;
    double p = static_cast<double>(wash.totalCarsServed()) / static_cast<double>(target_);
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;
    return p;
}

bool RatingGoal::check(const CarWash& wash) {
    if (!achieved_ && wash.averageSatisfaction() >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

double RatingGoal::progress(const CarWash& wash) const {
    if (target_ <= 0.0) return 1.0;
    double p = wash.averageSatisfaction() / target_;
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;
    return p;
}
