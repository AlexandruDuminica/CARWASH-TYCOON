#pragma once

#include <deque>
#include <memory>
#include <iosfwd>

#include "Customer.h"

class CarQueue {
    std::deque<std::unique_ptr<Customer>> q_;
    int nextId_{1};
    int demandPerHour_{2};
    int lost_{0};

public:
    CarQueue() = default;


    void generateRandomCustomers();

    bool empty() const { return q_.empty(); }
    int  size()  const { return static_cast<int>(q_.size()); }
    int  lostCustomers() const { return lost_; }
    int  demand() const { return demandPerHour_; }

    std::unique_ptr<Customer> pop();
    void failOne();

    void increaseDemand();
    void decreaseDemand();

    void print(std::ostream &os) const;
    friend std::ostream& operator<<(std::ostream&, const CarQueue&);
};
