#pragma once

#include "WashService.h"

class BasicService final : public WashService {
public:
    BasicService();

    BasicService(std::string name, int durationMin, double price,
                 int waterNeed, int shampooNeed, int waxNeed,
                 double rating = 3.6);

    void print(std::ostream &os) const override;

    double finalPriceForCars(int cars) const;

    [[nodiscard]] std::unique_ptr<WashService> clone() const override;
};
