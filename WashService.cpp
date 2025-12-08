#include "WashService.h"

#include <iomanip>
#include <ostream>

WashService::WashService(std::string name, int durationMin, double basePrice,
                         int waterL, int shampooMl, int waxMl)
    : name_(std::move(name)),
      durationMin_(durationMin),
      basePrice_(basePrice),
      waterL_(waterL),
      shampooMl_(shampooMl),
      waxMl_(waxMl) {}

bool WashService::applyFactor(double f) {
    if (f <= 0.0) return false;
    basePrice_ *= f;
    return true;
}

std::ostream& operator<<(std::ostream& os, const WashService& s) {
    os << "Service{name='" << s.name_ << "', t=" << s.durationMin_
       << "min, basePrice=" << std::fixed << std::setprecision(2) << s.basePrice_
       << " EUR, use=[" << s.waterL_ << "L, " << s.shampooMl_
       << "ml, " << s.waxMl_ << "ml]}";
    s.print(os); // apel virtual
    return os;
}
