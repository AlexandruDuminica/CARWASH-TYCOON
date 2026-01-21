#include "DeluxeService.h"
#include <ostream>

DeluxeService::DeluxeService(std::string name,
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
                  4.10) {}

DeluxeService::DeluxeService()
    : DeluxeService("Deluxe", 35, 14.5, 120, 60, 0) {}

void DeluxeService::print(std::ostream& os) const {
    os << "[DELUXE] ";
    WashService::print(os);
}

double DeluxeService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double total = WashService::finalPriceForCars(cars);
    if (cars >= 3) total *= 0.97;
    return total;
}

std::unique_ptr<WashService> DeluxeService::clone() const {
    return std::make_unique<DeluxeService>(*this);
}
