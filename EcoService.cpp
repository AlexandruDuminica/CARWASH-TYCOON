#include "EcoService.h"

#include <ostream>

EcoService::EcoService()
    : WashService("Eco", 30, 12.0, 50, 30, 0, 3.9) {}

EcoService::EcoService(std::string name, int durationMin, double price,
                       int waterNeed, int shampooNeed, int waxNeed,
                       double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating) {}

void EcoService::print(std::ostream& os) const {
    os << "EcoService(" << name_ << ", " << duration_ << " min, "
       << price_ << " EUR, rating=" << rating_ << ")";
}

double EcoService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double base = price_ * cars;
    if (water_ <= 60) base *= 0.98;
    return base;
}

std::unique_ptr<WashService> EcoService::clone() const {
    return std::make_unique<EcoService>(*this);
}
