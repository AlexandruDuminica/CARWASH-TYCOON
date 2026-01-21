#pragma once
#include "WashService.h"

class BasicService final : public WashService {
public:
    BasicService(std::string name,
                 int duration,
                 double price,
                 int waterNeed,
                 int shampooNeed,
                 int waxNeed);

    BasicService();

    void print(std::ostream& os) const override;
    double finalPriceForCars(int cars) const override;
    std::unique_ptr<WashService> clone() const override;
};
