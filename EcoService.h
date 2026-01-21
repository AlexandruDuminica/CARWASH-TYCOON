#pragma once
#include "WashService.h"

class EcoService final : public WashService {
public:
    EcoService(std::string name,
               int duration,
               double price,
               int waterNeed,
               int shampooNeed,
               int waxNeed);

    EcoService();

    void print(std::ostream& os) const override;
    double finalPriceForCars(int cars) const override;
    std::unique_ptr<WashService> clone() const override;
};
