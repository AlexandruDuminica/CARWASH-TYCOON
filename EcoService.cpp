#include "EcoService.h"

#include <ostream>

void EcoService::print(std::ostream& os) const {
    os << " [Eco]";
}

double EcoService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    // Eco: mai puțină apă => reducere 10%
    return basePrice_ * cars * 0.9;
}

std::unique_ptr<WashService> EcoService::clone() const {
    return std::make_unique<EcoService>(*this);
}
