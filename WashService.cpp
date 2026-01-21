#include "WashService.h"

#include <ostream>

WashService::WashService(std::string name, int duration, double price,
                         int water, int shampoo, int wax,
                         double rating)
    : name_(std::move(name)),
      duration_(duration),
      price_(price),
      water_(water),
      shampoo_(shampoo),
      wax_(wax),
      rating_(rating) {}

const std::string& WashService::name() const noexcept { return name_; }
int WashService::duration() const noexcept { return duration_; }
double WashService::price() const noexcept { return price_; }

int WashService::waterNeed() const noexcept { return water_; }
int WashService::shampooNeed() const noexcept { return shampoo_; }
int WashService::waxNeed() const noexcept { return wax_; }

double WashService::rating() const noexcept { return rating_; }

std::ostream& operator<<(std::ostream& os, const WashService& s) {
    os << s.name() << " | dur=" << s.duration() << " min"
       << " | price=" << s.price()
       << " | needs(W=" << s.needW()
       << ", S=" << s.needS()
       << ", X=" << s.needX() << ")";
    return os;
}
