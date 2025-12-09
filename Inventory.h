#pragma once

#include <iosfwd>

class WashService;

class Inventory {
    int waterML_;
    int shampooML_;
    int waxML_;

public:
    Inventory(int w = 0, int s = 0, int x = 0);

    bool takeIfCan(const WashService& sp, int cars);

    // adăugare stoc (folosită de CarWash::buySupplies)
    void addSupplies(int waterML, int shampooML, int waxML);

    friend std::ostream& operator<<(std::ostream& os, const Inventory& inv);
};
