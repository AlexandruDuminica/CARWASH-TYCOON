#include "../headers/DeluxeService.h"

DeluxeService::DeluxeService()
    : WashService("Deluxe", 35, 14.5, 120, 60, 0, 4.2, ServiceKind::Deluxe) {
}

DeluxeService::DeluxeService(std::string name, int durationMin, double price,
                             int waterNeed, int shampooNeed, int waxNeed,
                             double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating, ServiceKind::Deluxe) {
}

std::unique_ptr<WashService> DeluxeService::clone() const {
    return std::make_unique<DeluxeService>(*this);
}
