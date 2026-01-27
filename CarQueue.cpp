#include "CarQueue.h"
#include "Customer.h"

#include <cstdlib>
#include <ostream>

void CarQueue::generateRandomCustomers() {
    for (int i = 0; i < demandPerHour_; ++i) {
        double baseBudget = 10.0 + std::rand() % 20; // 10 .. 30 EUR
        double impatience = 0.5 + (std::rand() % 150) / 100.0; // 0.5 .. 2.0

        int t = std::rand() % 4;
        std::unique_ptr<Customer> c;
        if (t == 0) c = std::make_unique<RushedCustomer>(nextId_, baseBudget, impatience);
        else if (t == 1) c = std::make_unique<BudgetCustomer>(nextId_, baseBudget, impatience);
        else if (t == 2) c = std::make_unique<PremiumCustomer>(nextId_, baseBudget * 1.5, impatience);
        else c = std::make_unique<EcoCustomer>(nextId_, baseBudget, impatience);

        ++nextId_;
        q_.push_back(std::move(c));
    }
}

std::unique_ptr<Customer> CarQueue::pop() {
    if (q_.empty()) return nullptr;
    std::unique_ptr<Customer> c = std::move(q_.front());
    q_.pop_front();
    return c;
}

void CarQueue::failOne() {
    ++lost_;
}

void CarQueue::increaseDemand() {
    if (demandPerHour_ < 20) ++demandPerHour_;
}

void CarQueue::decreaseDemand() {
    if (demandPerHour_ > 1) --demandPerHour_;
}

void CarQueue::print(std::ostream &os) const {
    os << "Queue{pending=" << q_.size()
            << ", lost=" << lost_
            << ", demand/h=" << demandPerHour_ << "}";
}

std::ostream &operator<<(std::ostream &os, const CarQueue &cq) {
    cq.print(os);
    return os;
}
