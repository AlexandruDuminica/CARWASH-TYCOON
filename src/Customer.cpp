#include "../headers/Customer.h"
#include "../headers/WashService.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

Customer::Customer(int id, double budget, double impatience)
    : id_(id), budget_(budget), impatience_(impatience) {
}

void Customer::onServed(const WashService &svc, double pricePaid, int waitMinutes) {
    double priceFactor = (pricePaid <= budget_)
                             ? 1.0
                             : std::max(0.2, 1.0 - (pricePaid - budget_) / budget_);

    double timeFactor = std::max(0.0, 1.0 - waitMinutes / (60.0 * impatience_));

    const double qualityFactor = std::max(0.0, std::min(1.0, svc.rating() / 5.0));
    double base = 2.0 * priceFactor + 2.0 * timeFactor + 1.0 * qualityFactor;

    if (base < 0.0) base = 0.0;
    if (base > 5.0) base = 5.0;
    satisfaction_ = base;
}

void Customer::print(std::ostream &os) const {
    os << "Customer#" << id_ << " type=" << type()
            << " budget=" << budget() << " impatience=" << impatience()
            << " sat=" << satisfaction();
}

std::ostream &operator<<(std::ostream &os, const Customer &c) {
    c.print(os);
    return os;
}

const WashService *RushedCustomer::chooseService(const std::vector<WashService *> &services) const {
    const WashService *best = nullptr;
    for (auto *s: services) {
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

const WashService *BudgetCustomer::chooseService(const std::vector<WashService *> &services) const {
    const WashService *best = nullptr;
    for (auto *s: services) {
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

const WashService *PremiumCustomer::chooseService(const std::vector<WashService *> &services) const {
    const WashService *bestPremium = nullptr;
    const WashService *bestAny = nullptr;

    for (auto *s: services) {
        if (!s) continue;
        if (s->price() > budget_) continue;

        if (s->isPremium()) {
            if (!bestPremium || s->rating() > bestPremium->rating() ||
                (std::fabs(s->rating() - bestPremium->rating()) < 1e-6 && s->price() > bestPremium->price())) {
                bestPremium = s;
            }
        } else {
            if (!bestAny || s->rating() > bestAny->rating() ||
                (std::fabs(s->rating() - bestAny->rating()) < 1e-6 && s->price() > bestAny->price())) {
                bestAny = s;
            }
        }
    }

    return bestPremium ? bestPremium : bestAny;
}

std::unique_ptr<Customer> PremiumCustomer::clone() const {
    return std::make_unique<PremiumCustomer>(*this);
}

const WashService *EcoCustomer::chooseService(const std::vector<WashService *> &services) const {
    const WashService *eco = nullptr;
    for (auto *s: services) {
        if (!s) continue;
        if (s->price() > budget_) continue;

        if (s->kind() == ServiceKind::Eco || s->name().find("Eco") != std::string::npos) {
            if (!eco || s->price() < eco->price()) {
                eco = s;
            }
        }
    }
    if (eco) return eco;

    const WashService *best = nullptr;
    for (auto *s: services) {
        if (!s) continue;
        if (s->price() > budget_) continue;
        if (!best || s->price() < best->price()) {
            best = s;
        }
    }
    return best;
}

std::unique_ptr<Customer> EcoCustomer::clone() const {
    return std::make_unique<EcoCustomer>(*this);
}
