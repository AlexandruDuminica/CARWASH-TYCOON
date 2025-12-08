#pragma once

#include <iosfwd>

// forward declaration, nu avem nevoie de detaliile clasei aici
class WashService;

class Inventory {
    int wL_;
    int sML_;
    int xML_;

public:
    explicit Inventory(int w = 0, int s = 0, int x = 0);

    void add(int w, int s, int x);

    // folosim WashService ca tip, dar implementarea din .cpp nu depinde de API-ul lui
    bool takeIfCan(const WashService& sp, int cars);

    int w() const noexcept { return wL_; }
    int s() const noexcept { return sML_; }
    int x() const noexcept { return xML_; }

    double fullness() const;
};

std::ostream& operator<<(std::ostream& os, const Inventory& inv);
