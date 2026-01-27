#include "Inventory.h"
#include <ostream>

bool Inventory::takeIfCan(const WashService &sp, int cars) {
    if (cars <= 0) return false;

    const int needWater = sp.needW() * cars;
    const int needShampoo = sp.needS() * cars;
    const int needWax = sp.needX() * cars;

    if (water_ < needWater) return false;
    if (shampoo_ < needShampoo) return false;
    if (wax_ < needWax) return false;

    water_ -= needWater;
    shampoo_ -= needShampoo;
    wax_ -= needWax;
    return true;
}

std::ostream &operator<<(std::ostream &os, const Inventory &inv) {
    os << "Inventory{water=" << inv.water()
            << ", shampoo=" << inv.shampoo()
            << ", wax=" << inv.wax() << "}";
    return os;
}