#include "DeluxeService.h"

#include <ostream>

DeluxeService::DeluxeService()
    : WashService("Deluxe", 35, 14.5, 120, 60, 0, 4.2) {}

DeluxeService::DeluxeService(std::string name, int durationMin, double price,
                             int waterNeed, int shampooNeed, int waxNeed,
                             double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating) {}

void DeluxeService::print(std::ostream& os) const {
    os << "DeluxeService(" << name_ << ", " << duration_ << " min, "
       << price_ << " EUR, rating=" << rating_ << ")";
}

double DeluxeService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double base = price_ * cars;
    if (cars >= 5) base *= 0.95;
    return base;
}

std::unique_ptr<WashService> DeluxeService::clone() const {
    return std::make_unique<DeluxeService>(*this);
}
