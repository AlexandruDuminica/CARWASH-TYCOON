#pragma once
#include <iosfwd>

class WashService;

class Inventory {
    int wL_;
    int sML_;
    int xML_;

public:
    Inventory(int w = 0, int s = 0, int x = 0);

    void add(int w, int s, int x);
    bool takeIfCan(const WashService &sp, int cars);

    int w() const noexcept { return wL_; }
    int s() const noexcept { return sML_; }
    int x() const noexcept { return xML_; }

    double fullness() const;

    friend std::ostream& operator<<(std::ostream&, const Inventory&);
};
