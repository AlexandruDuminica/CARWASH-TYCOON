#pragma once

#include "WashService.h"

class EcoService final : public WashService {
public:
    EcoService();

    EcoService(std::string name, int durationMin, double price,
               int waterNeed, int shampooNeed, int waxNeed,
               double rating = 3.9);

    void print(std::ostream& os) const override;
    double finalPriceForCars(int cars) const;

    std::unique_ptr<WashService> clone() const override;
};
