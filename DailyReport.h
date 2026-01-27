#pragma once

#include <string>
#include <map>
#include <iosfwd>

struct ServiceStats {
    int cars{0};
    double revenue{0.0};
};

class DailyReport {
    int day_{0};
    int totalCars_{0};
    int lostCustomers_{0};
    double totalRevenue_{0.0};
    double avgSatisfaction_{0.0};
    std::map<std::string, ServiceStats> perService_;

public:
    DailyReport() = default;

    void beginDay(int day);

    void addServiceSale(const std::string &name, double price);

    void finalize(int totalCars, int lost, double avgSat, double totalRevenue);

    int day() const noexcept { return day_; }
    int totalCars() const noexcept { return totalCars_; }
    int lostCustomers() const noexcept { return lostCustomers_; }
    double totalRevenue() const noexcept { return totalRevenue_; }
    double avgSatisfaction() const noexcept { return avgSatisfaction_; }
    const std::map<std::string, ServiceStats> &perService() const noexcept { return perService_; }

    void print(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const DailyReport &rep);