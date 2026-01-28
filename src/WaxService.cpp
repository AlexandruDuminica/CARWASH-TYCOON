#include "../headers/WaxService.h"

#include <algorithm>
#include <utility>

WaxService::WaxService()
    : WashService("Wax", 25, 16.0, 60, 20, 50, 4.6, ServiceKind::Wax),
      basePrice_(price_),
      baseRating_(rating_) {
}

WaxService::WaxService(std::string name, int durationMin, double price,
                       int waterNeed, int shampooNeed, int waxNeed,
                       double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating, ServiceKind::Wax),
      basePrice_(price_),
      baseRating_(rating_) {
}

void WaxService::enableNanoCoating(bool enabled) {
    if (enabled == nanoCoatingEnabled_) return;

    nanoCoatingEnabled_ = enabled;

    if (nanoCoatingEnabled_) {
        price_ = basePrice_ * 1.15;
        rating_ = std::min(5.0, baseRating_ + 0.20);
    } else {
        price_ = basePrice_;
        rating_ = baseRating_;
    }
}

std::unique_ptr<WashService> WaxService::clone() const {
    return std::make_unique<WaxService>(*this);
}
