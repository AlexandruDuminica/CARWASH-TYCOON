#pragma once

#include <memory>
#include <string>

#include "WashService.h"

class ServiceFactory {
public:
    enum class Kind { Basic, Deluxe, Wax, Eco };

    static std::unique_ptr<WashService> create(const std::string &key);

    static std::unique_ptr<WashService> createConfigured(
        Kind kind,
        std::string name,
        int durationMin,
        double price,
        int waterNeed,
        int shampooNeed,
        int waxNeed,
        double rating = -1.0);
};