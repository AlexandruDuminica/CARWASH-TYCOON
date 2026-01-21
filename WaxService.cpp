#include "WaxService.h"

WaxService::WaxService()
    : WashService("Wax", 25, 16.0, 60, 20, 50, 4.5) {}

WaxService::WaxService(std::string name, int durationMin, double price,
                       int waterNeed, int shampooNeed, int waxNeed,
                       double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating) {}

void WaxService::print(std::ostream &os) const {
    os << "WaxService(" << name_ << ", " << duration_ << " min, "
       << price_ << " EUR, rating=" << rating_ << ")";
}

double WaxService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double base = price_ * cars;
    if (wax_ > 0) base += 2.0 * cars;
    return base;
}

std::unique_ptr<WashService> WaxService::clone() const {
    return std::make_unique<WaxService>(*this);
}
