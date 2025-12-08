#include "Inventory.h"
#include "WashService.h"

#include <iomanip>
#include <ostream>

Inventory::Inventory(int w, int s, int x)
    : wL_(w), sML_(s), xML_(x) {}

void Inventory::add(int w, int s, int x) {
    if (w > 0) wL_ += w;
    if (s > 0) sML_ += s;
    if (x > 0) xML_ += x;
}

bool Inventory::takeIfCan(const WashService& sp, int cars) {
    long W = 1L * sp.needW() * cars;
    long S = 1L * sp.needS() * cars;
    long X = 1L * sp.needX() * cars;
    if (W <= wL_ && S <= sML_ && X <= xML_) {
        wL_ -= static_cast<int>(W);
        sML_ -= static_cast<int>(S);
        xML_ -= static_cast<int>(X);
        return true;
    }
    return false;
}

double Inventory::fullness() const {
    const double kW = 5000.0, kS = 5000.0, kX = 5000.0;
    return (wL_ / kW + sML_ / kS + xML_ / kX) / 3.0 * 100.0;
}

std::ostream& operator<<(std::ostream& os, const Inventory& inv) {
    os << "Inventory{water=" << inv.w() << "L, shampoo=" << inv.s()
       << "ml, wax=" << inv.x() << "ml, full=" << std::fixed
       << std::setprecision(1) << inv.fullness() << "%}";
    return os;
}
