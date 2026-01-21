#include "WashService.h"
#include <ostream>
#include <iomanip>
#include <stdexcept>

WashService::WashService(std::string name,
                         int duration,
                         double price,
                         int waterNeed,
                         int shampooNeed,
                         int waxNeed,
                         double rating)
    : name_(std::move(name)),
      duration_(duration),
      price_(price),
      waterNeed_(waterNeed),
      shampooNeed_(shampooNeed),
      waxNeed_(waxNeed),
      rating_(rating) {
    if (duration_ <= 0) throw std::invalid_argument("WashService: durata invalida");
    if (price_ < 0.0) throw std::invalid_argument("WashService: pret invalid");
    if (waterNeed_ < 0 || shampooNeed_ < 0 || waxNeed_ < 0)
        throw std::invalid_argument("WashService: consum invalid");
    if (rating_ < 0.0) rating_ = 0.0;
    if (rating_ > 5.0) rating_ = 5.0;
}

void WashService::applyFactor(double factor) {
    if (factor <= 0.0) return;
    price_ *= factor;
    if (price_ < 0.0) price_ = 0.0;
}

void WashService::print(std::ostream& os) const {
    os << name_
       << " | dur=" << duration_
       << " | price=" << std::fixed << std::setprecision(2) << price_
       << " | rating=" << std::setprecision(2) << rating_
       << " | needs(W=" << waterNeed_
       << ", S=" << shampooNeed_
       << ", X=" << waxNeed_ << ")";
}

double WashService::finalPriceForCars(int cars) const {
    if (cars <= 0) return 0.0;
    return price_ * static_cast<double>(cars);
}

std::ostream& operator<<(std::ostream& os, const WashService& s) {
    s.print(os);
    return os;
}
