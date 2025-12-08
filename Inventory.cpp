#include "Inventory.h"
#include "WashService.h"

#include <iomanip>
#include <ostream>

Inventory::Inventory(int w, int s, int x)
    : wL_(w), sML_(s), xML_(x) {}

void Inventory::add(int w, int s, int x) {
    if (w > 0)  wL_  += w;
    if (s > 0)  sML_ += s;
    if (x > 0)  xML_ += x;
}

bool Inventory::takeIfCan(const WashService& sp, int cars) {
    if (cars <= 0) {
        return true;
    }

    // folosim direct nevoile declarate in WashService
    long needW = 1L * sp.needW() * cars;
    long needS = 1L * sp.needS() * cars;
    long needX = 1L * sp.needX() * cars;

    if (needW <= wL_ && needS <= sML_ && needX <= xML_) {
        wL_  -= static_cast<int>(needW);
        sML_ -= static_cast<int>(needS);
        xML_ -= static_cast<int>(needX);
        return true;
    }
    return false;
}

double Inventory::fullness() const {
    const double kW = 5000.0;
    const double kS = 5000.0;
    const double kX = 5000.0;

    double fw = wL_  / kW;
    double fs = sML_ / kS;
    double fx = xML_ / kX;

    return (fw + fs + fx) / 3.0 * 100.0;
}

std::ostream& operator<<(std::ostream& os, const Inventory& inv) {
    os << "Inventory{water=" << inv.w() << "L, shampoo=" << inv.s()
       << "ml, wax=" << inv.x() << "ml, full="
       << std::fixed << std::setprecision(1)
       << inv.fullness() << "%}";
    return os;
}
