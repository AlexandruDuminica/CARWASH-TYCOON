#pragma once

#include <iosfwd>

// Forward declaration: nu avem nevoie de detaliile clasei aici
class WashService;

class Inventory {
    int wL_;   // apa (L)
    int sML_;  // sampon (ml)
    int xML_;  // ceara (ml)

public:
    explicit Inventory(int w = 0, int s = 0, int x = 0);

    void add(int w, int s, int x);

    // Consuma resursele necesare pentru 'cars' masini ale serviciului dat
    bool takeIfCan(const WashService& sp, int cars);

    int w() const noexcept { return wL_; }
    int s() const noexcept { return sML_; }
    int x() const noexcept { return xML_; }

    double fullness() const;
};

std::ostream& operator<<(std::ostream& os, const Inventory& inv);