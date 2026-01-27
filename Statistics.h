#pragma once

#include <vector>
#include <string>
#include <map>
#include <iosfwd>

#include "DailyReport.h"

struct ServiceAggregate {
    int cars{0};
    double revenue{0.0};
};

class Statistics {
    std::vector<DailyReport> reports_;

    static std::map<std::string, ServiceAggregate> aggregateServices(const std::vector<DailyReport> &reps);

    static double safeDiv(double a, double b);

public:
    explicit Statistics(std::vector<DailyReport> reps);

    int days() const noexcept;

    int totalCars() const noexcept;

    int totalLost() const noexcept;

    double totalRevenue() const noexcept;

    double avgCarsPerDay() const noexcept;

    double avgLostPerDay() const noexcept;

    double avgRevenuePerDay() const noexcept;

    double avgSatisfactionWeighted() const noexcept;

    int bestDayByRevenue() const noexcept;

    int worstDayByRevenue() const noexcept;

    int bestDayBySatisfaction() const noexcept;

    int worstDayBySatisfaction() const noexcept;

    int bestDayByLost() const noexcept;

    int worstDayByLost() const noexcept;

    std::vector<std::pair<std::string, ServiceAggregate> > topServicesByRevenue(int k) const;

    std::vector<std::pair<std::string, ServiceAggregate> > topServicesByCars(int k) const;

    std::vector<double> revenueSeries() const;

    std::vector<double> satisfactionSeries() const;

    std::vector<int> lostSeries() const;

    void print(std::ostream &os) const;
};