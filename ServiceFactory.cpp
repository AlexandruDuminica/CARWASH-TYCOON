#include "ServiceFactory.h"

#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"

static bool ieq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        const unsigned char ca = static_cast<unsigned char>(a[i]);
        const unsigned char cb = static_cast<unsigned char>(b[i]);
        if (std::tolower(ca) != std::tolower(cb)) return false;
    }
    return true;
}

std::unique_ptr<WashService> ServiceFactory::create(const std::string& key) {
    if (ieq(key, "basic"))  return std::make_unique<BasicService>();
    if (ieq(key, "deluxe")) return std::make_unique<DeluxeService>();
    if (ieq(key, "wax"))    return std::make_unique<WaxService>();
    if (ieq(key, "eco"))    return std::make_unique<EcoService>();
    return nullptr;
}

std::unique_ptr<WashService> ServiceFactory::createConfigured(
    Kind kind,
    std::string name,
    int durationMin,
    double price,
    int waterNeed,
    int shampooNeed,
    int waxNeed,
    double rating) {

    switch (kind) {
        case Kind::Basic:
            return std::make_unique<BasicService>(
                std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed,
                (rating >= 0.0 ? rating : 3.6));
        case Kind::Deluxe:
            return std::make_unique<DeluxeService>(
                std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed,
                (rating >= 0.0 ? rating : 4.2));
        case Kind::Wax:
            return std::make_unique<WaxService>(
                std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed,
                (rating >= 0.0 ? rating : 4.5));
        case Kind::Eco:
            return std::make_unique<EcoService>(
                std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed,
                (rating >= 0.0 ? rating : 3.9));
    }
    return nullptr;
}
