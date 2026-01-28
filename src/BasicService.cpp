#include "../headers/BasicService.h"

#include <ostream>

BasicService::BasicService()
    : WashService("Basic", 20, 8.0, 80, 40, 0, 3.6, ServiceKind::Basic) {
}

BasicService::BasicService(std::string name, int durationMin, double price,
                           int waterNeed, int shampooNeed, int waxNeed,
                           double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating, ServiceKind::Basic) {
}

void BasicService::print(std::ostream &os) const {
    os << "BasicService(" << name_ << ", " << duration_ << " min, "
            << price_ << " EUR"
            << ", 3cars=" << finalPriceForCars(3)
            << ", rating=" << rating_ << ")";
}

double BasicService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    return price_ * cars;
}

std::unique_ptr<WashService> BasicService::clone() const {
    return std::make_unique<BasicService>(*this);
}
