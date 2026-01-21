#include "WaxService.h"
#include <ostream>

WaxService::WaxService(std::string name,
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
                  4.50) {}

WaxService::WaxService()
    : WaxService("Wax", 25, 16.0, 60, 20, 50) {}

void WaxService::print(std::ostream& os) const {
    os << "[WAX] ";
    WashService::print(os);
}

double WaxService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    return WashService::finalPriceForCars(cars);
}

std::unique_ptr<WashService> WaxService::clone() const {
    return std::make_unique<WaxService>(*this);
}
