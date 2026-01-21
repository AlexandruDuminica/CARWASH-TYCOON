#include "BasicService.h"
#include <ostream>

BasicService::BasicService(std::string name,
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
                  3.20) {}

BasicService::BasicService()
    : BasicService("Basic", 20, 8.0, 80, 40, 0) {}

void BasicService::print(std::ostream& os) const {
    os << "[BASIC] ";
    WashService::print(os);
}

double BasicService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    double total = WashService::finalPriceForCars(cars);
    if (cars >= 5) total *= 0.95;
    return total;
}

std::unique_ptr<WashService> BasicService::clone() const {
    return std::make_unique<BasicService>(*this);
}
