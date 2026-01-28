#include "../headers/Goal.h"
#include "../headers/CarWash.h"

#include <algorithm>

bool ProfitGoal::check(const CarWash &wash) {
    double cash = wash.totalCash();
    if (target_ <= 0.0) {
        setProgressInternal(1.0);
        achieved_ = true;
        return true;
    }
    double p = cash / target_;
    setProgressInternal(p);
    if (!achieved_ && cash >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

bool CarsServedGoal::check(const CarWash &wash) {
    int cars = wash.totalCarsServed();
    if (target_ <= 0) {
        setProgressInternal(1.0);
        achieved_ = true;
        return true;
    }
    double p = static_cast<double>(cars) / static_cast<double>(target_);
    setProgressInternal(p);
    if (!achieved_ && cars >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

bool RatingGoal::check(const CarWash &wash) {
    double avg = wash.averageSatisfaction();
    if (target_ <= 0.0) {
        setProgressInternal(1.0);
        achieved_ = true;
        return true;
    }
    double p = avg / target_;
    setProgressInternal(p);
    if (!achieved_ && avg >= target_) {
        achieved_ = true;
    }
    return achieved_;
}
