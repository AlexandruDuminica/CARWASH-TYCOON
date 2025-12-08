#include "DeluxeService.h"
#include <ostream>

void DeluxeService::print(std::ostream &os) const {
    os << " [Deluxe]";
}

double DeluxeService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double total = basePrice_ * cars;
    if (cars >= 5) total *= 0.9;
    return total;
}

std::unique_ptr<WashService> DeluxeService::clone() const {
    return std::make_unique<DeluxeService>(*this);
}
