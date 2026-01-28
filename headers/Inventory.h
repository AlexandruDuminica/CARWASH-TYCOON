#pragma once

#include <iosfwd>
#include "WashService.h"

class Inventory {
    int water_{0};
    int shampoo_{0};
    int wax_{0};

public:
    explicit Inventory(int w = 0, int s = 0, int x = 0) : water_(w), shampoo_(s), wax_(x) {
    }

    int water() const noexcept { return water_; }
    int shampoo() const noexcept { return shampoo_; }
    int wax() const noexcept { return wax_; }

    void addWater(int v) noexcept { if (v > 0) water_ += v; }
    void addShampoo(int v) noexcept { if (v > 0) shampoo_ += v; }
    void addWax(int v) noexcept { if (v > 0) wax_ += v; }

    bool takeIfCan(const WashService &sp, int cars);

    friend std::ostream &operator<<(std::ostream &os, const Inventory &inv);
};
