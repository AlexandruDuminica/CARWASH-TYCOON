#include "EcoService.h"
#include <ostream>

EcoService::EcoService(std::string name,
                       int duration,
                       double price,
                       int waterNeed,
                       int shampooNeed,
                       int waxNeed)
    : WashService(std::move(name),
                  duration,
                  price,
                  waterNeed,
                  shampooNeed,
                  waxNeed,
                  3.80) {}

EcoService::EcoService()
    : EcoService("Eco", 30, 12.0, 50, 30, 0) {}

void EcoService::print(std::ostream& os) const {
    os << "[ECO] ";
    WashService::print(os);
}

double EcoService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double total = WashService::finalPriceForCars(cars);
    if (cars >= 4) total *= 0.96;
    return total;
}

std::unique_ptr<WashService> EcoService::clone() const {
    return std::make_unique<EcoService>(*this);
}
