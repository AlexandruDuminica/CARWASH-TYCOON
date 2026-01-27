#pragma once

#include "WashService.h"
#include <memory>
#include <string>

class WaxService : public WashService {
    bool nanoCoatingEnabled_{false};
    double basePrice_{0.0};
    double baseRating_{0.0};

public:
    WaxService();

    WaxService(std::string name, int durationMin, double price,
               int waterNeed, int shampooNeed, int waxNeed,
               double rating);

    void enableNanoCoating(bool enabled);

    bool nanoCoatingEnabled() const noexcept { return nanoCoatingEnabled_; }

    std::unique_ptr<WashService> clone() const override;
};
