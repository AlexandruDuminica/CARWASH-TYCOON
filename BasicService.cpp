#include "BasicService.h"
#include <ostream>

void BasicService::print(std::ostream &os) const {
    (void)os;
}

double BasicService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    return basePrice_ * cars;
}

std::unique_ptr<WashService> BasicService::clone() const {
    return std::make_unique<BasicService>(*this);
}
