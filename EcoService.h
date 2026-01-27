#pragma once

#include "WashService.h"
#include <memory>
#include <string>

class EcoService final : public WashService {
public:
    EcoService();

    EcoService(std::string name, int durationMin, double price,
               int waterNeed, int shampooNeed, int waxNeed,
               double rating);

    ~EcoService() override = default;

    std::unique_ptr<WashService> clone() const override;
};
