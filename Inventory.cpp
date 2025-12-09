#include "Inventory.h"
#include "WashService.h"
#include <ostream>

Inventory::Inventory(int w, int s, int x)
    : waterML_(w), shampooML_(s), waxML_(x) {}

bool Inventory::takeIfCan(const WashService& sp, int cars) {
    // consum de apă per mașină (constant, simplu)
    const int needWater   = 10 * cars;
    // consum de șampon/ceară definit în WashService
    const int needShampoo = sp.needS() * cars;
    const int needWax     = sp.needX() * cars;

    if (waterML_   < needWater ||
        shampooML_ < needShampoo ||
        waxML_     < needWax) {
        return false;
        }

    waterML_   -= needWater;
    shampooML_ -= needShampoo;
    waxML_     -= needWax;
    return true;
}

void Inventory::addSupplies(int waterML, int shampooML, int waxML) {
    if (waterML   > 0) waterML_   += waterML;
    if (shampooML > 0) shampooML_ += shampooML;
    if (waxML     > 0) waxML_     += waxML;
}

std::ostream& operator<<(std::ostream& os, const Inventory& inv) {
    os << "Inventory{water="   << inv.waterML_
       << ", shampoo="         << inv.shampooML_
       << ", wax="             << inv.waxML_
       << "}";
    return os;
}
