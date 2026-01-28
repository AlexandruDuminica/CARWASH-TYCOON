#include "../headers/EcoService.h"

EcoService::EcoService()
    : WashService("Eco", 30, 12.0, 50, 30, 0, 4.0, ServiceKind::Eco) {
}

EcoService::EcoService(std::string name, int durationMin, double price,
                       int waterNeed, int shampooNeed, int waxNeed,
                       double rating)
    : WashService(std::move(name), durationMin, price, waterNeed, shampooNeed, waxNeed, rating, ServiceKind::Eco) {
}

std::unique_ptr<WashService> EcoService::clone() const {
    return std::make_unique<EcoService>(*this);
}
