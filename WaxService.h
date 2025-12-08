#pragma once
#include "WashService.h"

class WaxService : public WashService {
protected:
    void print(std::ostream &os) const override;

public:
    WaxService(std::string name, int durationMin, double basePrice,
               int waterL, int shampooMl, int waxMl)
        : WashService(std::move(name), durationMin, basePrice,
                      waterL, shampooMl, waxMl) {}

    double finalPriceForCars(int cars) const override;
    double rating() const override { return 4.5; }
    std::unique_ptr<WashService> clone() const override;
};
