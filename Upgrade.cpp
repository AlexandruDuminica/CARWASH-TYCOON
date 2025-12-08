#include "Upgrade.h"
#include "CarWash.h"
#include <ostream>

void Upgrade::print(std::ostream& os) const {
    os << name_ << " (" << description_ << ", cost=" << cost_ << " EUR)";
}

std::ostream& operator<<(std::ostream& os, const Upgrade& u) {
    u.print(os);
    return os;
}

void BaySpeedUpgrade::apply(CarWash& wash) {
    wash.increaseSpeedFactor(0.2); // +20%
}

std::unique_ptr<Upgrade> BaySpeedUpgrade::clone() const {
    return std::make_unique<BaySpeedUpgrade>(*this);
}

void ComfortUpgrade::apply(CarWash& wash) {
    wash.increaseComfortBonus(0.5);
}

std::unique_ptr<Upgrade> ComfortUpgrade::clone() const {
    return std::make_unique<ComfortUpgrade>(*this);
}

void MarketingUpgrade::apply(CarWash& wash) {
    wash.increaseBaseDemand(1);
}

std::unique_ptr<Upgrade> MarketingUpgrade::clone() const {
    return std::make_unique<MarketingUpgrade>(*this);
}
