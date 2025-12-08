#include "CarQueue.h"
#include <cstdlib>
#include <ostream>

void CarQueue::generate(const std::vector<std::string> &services) {
    if (services.empty()) return;
    for (int i = 0; i < demandPerHour_; ++i) {
        const std::string &svc = services[std::rand() % services.size()];
        q_.push_back({svc, nextId_++});
    }
}

std::string CarQueue::pop() {
    if (q_.empty()) return {};
    CarRequest c = q_.front();
    q_.pop_front();
    return c.service;
}

void CarQueue::failOne() {
    ++lost_;
}

void CarQueue::increaseDemand() {
    if (demandPerHour_ < 15) ++demandPerHour_;
}

void CarQueue::decreaseDemand() {
    if (demandPerHour_ > 1) --demandPerHour_;
}

void CarQueue::print(std::ostream &os) const {
    os << "Queue{pending=" << q_.size()
       << ", lost=" << lost_
       << ", demand/h=" << demandPerHour_ << "}";
}

std::ostream& operator<<(std::ostream& os, const CarQueue& cq) {
    cq.print(os);
    return os;
}
