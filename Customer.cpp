#include "Customer.h"
#include "WashService.h"

#include <algorithm>
#include <ostream>
#include <cmath>

Customer::Customer(int id, double budget, double impatience)
    : id_(id), budget_(budget), impatience_(impatience) {}

void Customer::onServed(const WashService& svc, double pricePaid, int waitMinutes) {
    double priceFactor = pricePaid <= budget_ ? 1.0 : std::max(0.2, 1.0 - (pricePaid - budget_) / budget_);
    double timeFactor  = std::max(0.0, 1.0 - waitMinutes / (60.0 * impatience_));

    double base = 2.5 * priceFactor + 2.5 * timeFactor;
    if (base < 0.0) base = 0.0;
    if (base > 5.0) base = 5.0;
    satisfaction_ = base;
    (void)svc;
}

void Customer::print(std::ostream& os) const {
    os << "Customer#" << id_ << " type=" << type()
       << " budget=" << budget_ << " sat=" << satisfaction_;
}

std::ostream& operator<<(std::ostream& os, const Customer& c) {
    c.print(os);
    return os;
}

// RushedCustomer: alege serviciul cu durata minimă care se încadrează în buget
const WashService* RushedCustomer::chooseService(const std::vector<WashService*>& services) const {
    const WashService* best = nullptr;
    for (auto* s : services) {
        if (!s) continue;
        if (s->price() > budget_) continue;
        if (!best || s->duration() < best->duration()) {
            best = s;
        }
    }
    return best;
}

std::unique_ptr<Customer> RushedCustomer::clone() const {
    return std::make_unique<RushedCustomer>(*this);
}

// BudgetCustomer: cel mai ieftin serviciu
const WashService* BudgetCustomer::chooseService(const std::vector<WashService*>& services) const {
    const WashService* best = nullptr;
    for (auto* s : services) {
        if (!s) continue;
        if (s->price() > budget_) continue;
        if (!best || s->price() < best->price()) {
            best = s;
        }
    }
    return best;
}

std::unique_ptr<Customer> BudgetCustomer::clone() const {
    return std::make_unique<BudgetCustomer>(*this);
}

// PremiumCustomer: cel mai bun rating, apoi preț
const WashService* PremiumCustomer::chooseService(const std::vector<WashService*>& services) const {
    const WashService* best = nullptr;
    for (auto* s : services) {
        if (!s) continue;
        if (s->price() > budget_) continue;
        if (!best || s->rating() > best->rating() ||
            (std::fabs(s->rating() - best->rating()) < 1e-6 && s->price() > best->price())) {
            best = s;
        }
    }
    return best;
}

std::unique_ptr<Customer> PremiumCustomer::clone() const {
    return std::make_unique<PremiumCustomer>(*this);
}

// EcoCustomer: dacă există "Eco" îl alege, altfel cel mai mic consum de apă
const WashService* EcoCustomer::chooseService(const std::vector<WashService*>& services) const {
    const WashService* eco = nullptr;
    const WashService* minWater = nullptr;
    for (auto* s : services) {
        if (!s) continue;
        if (s->price() > budget_) continue;
        if (s->name() == "Eco" || s->name() == "eco") {
            eco = s;
        }
        if (!minWater || s->needW() < minWater->needW()) {
            minWater = s;
        }
    }
    return eco ? eco : minWater;
}

std::unique_ptr<Customer> EcoCustomer::clone() const {
    return std::make_unique<EcoCustomer>(*this);
}
