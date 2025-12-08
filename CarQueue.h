#pragma once
#include <deque>
#include <string>
#include <vector>
#include <iosfwd>

class CarQueue {
    struct CarRequest {
        std::string service;
        int id;
    };

    std::deque<CarRequest> q_;
    int nextId_{1};
    int demandPerHour_{2};
    int lost_{0};

public:
    CarQueue() = default;

    void generate(const std::vector<std::string> &services);

    bool empty() const { return q_.empty(); }
    int  size()  const { return static_cast<int>(q_.size()); }
    int  lostCustomers() const { return lost_; }
    int  demand() const { return demandPerHour_; }

    std::string pop();
    void failOne();

    void increaseDemand();
    void decreaseDemand();

    void print(std::ostream &os) const;
    friend std::ostream& operator<<(std::ostream&, const CarQueue&);
};
