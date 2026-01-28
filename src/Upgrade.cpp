#include "../headers/Upgrade.h"
#include "../headers/CarWash.h"

#include <ostream>

void Upgrade::print(std::ostream &os) const {
    os << name() << " | " << description() << " | cost=" << cost();
}

std::ostream &operator<<(std::ostream &os, const Upgrade &up) {
    up.print(os);
    return os;
}

std::string BaySpeedUpgrade::name() const { return "Bay Speed"; }
std::string BaySpeedUpgrade::description() const { return "Increase processing speed"; }
double BaySpeedUpgrade::cost() const { return 120.0; }
void BaySpeedUpgrade::apply(CarWash &cw) const { cw.increaseSpeedFactor(0.15); }

std::string ComfortUpgrade::name() const { return "Comfort"; }
std::string ComfortUpgrade::description() const { return "Increase satisfaction bonus"; }
double ComfortUpgrade::cost() const { return 90.0; }
void ComfortUpgrade::apply(CarWash &cw) const { cw.increaseComfortBonus(0.15); }

std::string MarketingUpgrade::name() const { return "Marketing"; }
std::string MarketingUpgrade::description() const { return "Increase base demand"; }
double MarketingUpgrade::cost() const { return 110.0; }
void MarketingUpgrade::apply(CarWash &cw) const { cw.increaseBaseDemand(1); }

std::string NanoCoatingUpgrade::name() const { return "Nano Coating"; }
std::string NanoCoatingUpgrade::description() const { return "Enhance Wax services (price + rating)"; }
double NanoCoatingUpgrade::cost() const { return 140.0; }
void NanoCoatingUpgrade::apply(CarWash &cw) const { cw.enableNanoCoating(); }
