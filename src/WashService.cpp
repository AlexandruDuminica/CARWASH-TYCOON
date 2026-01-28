#include "../headers/WashService.h"

#include <ostream>
#include <utility>

WashService::WashService(std::string name, int durationMin, double price,
                         int waterNeed, int shampooNeed, int waxNeed, double rating,
                         ServiceKind kind)
    : name_(std::move(name)),
      duration_(durationMin),
      price_(price),
      needW_(waterNeed),
      needS_(shampooNeed),
      needX_(waxNeed),
      rating_(rating),
      kind_(kind) {
}

void WashService::applyFactor(double factor) {
    if (factor <= 0.0) return;
    price_ *= factor;
}

std::string WashService::kindToString(ServiceKind k) {
    switch (k) {
        case ServiceKind::Basic: return "Basic";
        case ServiceKind::Deluxe: return "Deluxe";
        case ServiceKind::Wax: return "Wax";
        case ServiceKind::Eco: return "Eco";
        case ServiceKind::Custom: return "Custom";
    }
    return "Custom";
}

void WashService::print(std::ostream &os) const {
    os << kindToString(kind_) << "("
            << name_ << ", " << duration_ << " min, "
            << price_ << ", W=" << needW_
            << ", S=" << needS_
            << ", X=" << needX_
            << ", rating=" << rating_ << ")";
}

std::ostream &operator<<(std::ostream &os, const WashService &s) {
    s.print(os);
    return os;
}
