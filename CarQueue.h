#pragma once
#include <deque>
#include <string>
#include <vector>

class CarQueue {
    struct CarRequest {
        std::string service;
        int id;
    };

    std::deque<CarRequest> q;
    int nextId{1};
    int demandPerHour{2};
    int lost{0};

public:
    CarQueue() = default;

    void generate(const std::vector<std::string>& services) {
        for(int i = 0; i < demandPerHour; i++) {
            if(!services.empty()) {
                q.push_back({services[rand()%services.size()], nextId++});
            }
        }
    }

    bool empty() const { return q.empty(); }
    int size() const { return q.size(); }
    int lostCustomers() const { return lost; }

    std::string get() {
        if(q.empty()) return "";
        auto c = q.front();
        q.pop_front();
        return c.service;
    }

    void fail() { lost++; }
    void increaseDemand() { if(demandPerHour < 15) demandPerHour++; }
    void decreaseDemand() { if(demandPerHour > 1) demandPerHour--; }

    int currentDemand() const { return demandPerHour; }
    void resetLost() { lost = 0; }
};
