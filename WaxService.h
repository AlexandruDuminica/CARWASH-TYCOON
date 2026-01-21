#pragma once

#include "WashService.h"

class WaxService final : public WashService {
public:
    WaxService();

    WaxService(std::string name, int durationMin, double price,
               int waterNeed, int shampooNeed, int waxNeed,
               double rating = 4.5);

    void print(std::ostream& os) const override;
    double finalPriceForCars(int cars) const;

    std::unique_ptr<WashService> clone() const override;
};
