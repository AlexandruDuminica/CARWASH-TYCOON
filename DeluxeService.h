#pragma once
#include "WashService.h"

class DeluxeService final : public WashService {
public:
    DeluxeService(std::string name,
                  int duration,
                  double price,
                  int waterNeed,
                  int shampooNeed,
                  int waxNeed);

    DeluxeService();

    void print(std::ostream& os) const override;
    double finalPriceForCars(int cars) const override;
    std::unique_ptr<WashService> clone() const override;
};
