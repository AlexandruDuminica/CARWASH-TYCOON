#include "WaxService.h"

#include <ostream>

void WaxService::print(std::ostream& os) const {
    os << " [Wax]";
}

double WaxService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    // mică suprataxă pentru ceară
    return basePrice_ * cars * 1.05;
}

std::unique_ptr<WashService> WaxService::clone() const {
    return std::make_unique<WaxService>(*this);
}
