#include "Goal.h"
#include "CarWash.h"

#include <ostream>

void Goal::print(std::ostream& os) const {
    os << description_ << " [" << (achieved_ ? "OK" : "IN PROGRES") << "]";
}

std::ostream& operator<<(std::ostream& os, const Goal& g) {
    g.print(os);
    return os;
}

bool ProfitGoal::check(const CarWash& wash) {
    if (!achieved_ && wash.totalCash() >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

bool CarsServedGoal::check(const CarWash& wash) {
    if (!achieved_ && wash.totalCarsServed() >= target_) {
        achieved_ = true;
    }
    return achieved_;
}

bool RatingGoal::check(const CarWash& wash) {
    if (!achieved_ && wash.averageSatisfaction() >= target_) {
        achieved_ = true;
    }
    return achieved_;
}
