#pragma once
#include "WashService.h"

class WaxService final : public WashService {
public:
    WaxService(std::string name,
               int duration,
               double price,
               int waterNeed,
               int shampooNeed,
               int waxNeed);

    WaxService();

    void print(std::ostream& os) const override;
    double finalPriceForCars(int cars) const override;
    std::unique_ptr<WashService> clone() const override;
};
