#pragma once
#include "WashService.h"

class BasicService : public WashService {
protected:
    void print(std::ostream &os) const override;

public:
    BasicService(std::string name, int durationMin, double basePrice,
                 int waterL, int shampooMl, int waxMl)
        : WashService(std::move(name), durationMin, basePrice,
                      waterL, shampooMl, waxMl) {}

    double finalPriceForCars(int cars) const override;
    std::unique_ptr<WashService> clone() const override;
};
