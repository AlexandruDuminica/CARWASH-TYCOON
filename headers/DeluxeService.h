#pragma once

#include "WashService.h"
#include <memory>
#include <string>

class DeluxeService final : public WashService {
public:
    DeluxeService();

    DeluxeService(std::string name, int durationMin, double price,
                  int waterNeed, int shampooNeed, int waxNeed,
                  double rating);

    ~DeluxeService() override = default;

    std::unique_ptr<WashService> clone() const override;
};
