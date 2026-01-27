#include "Statistics.h"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <ostream>

Statistics::Statistics(std::vector<DailyReport> reps)
    : reports_(std::move(reps)) {}

double Statistics::safeDiv(double a, double b) {
    if (b == 0.0) return 0.0;
    return a / b;
}

int Statistics::days() const noexcept {
    return static_cast<int>(reports_.size());
}

int Statistics::totalCars() const noexcept {
    int sum = 0;
    for (const auto& r : reports_) sum += r.totalCars();
    return sum;
}

int Statistics::totalLost() const noexcept {
    int sum = 0;
    for (const auto& r : reports_) sum += r.lostCustomers();
    return sum;
}

double Statistics::totalRevenue() const noexcept {
    double sum = 0.0;
    for (const auto& r : reports_) sum += r.totalRevenue();
    return sum;
}

double Statistics::avgCarsPerDay() const noexcept {
    return safeDiv(static_cast<double>(totalCars()), static_cast<double>(days()));
}

double Statistics::avgLostPerDay() const noexcept {
    return safeDiv(static_cast<double>(totalLost()), static_cast<double>(days()));
}

double Statistics::avgRevenuePerDay() const noexcept {
    return safeDiv(totalRevenue(), static_cast<double>(days()));
}

double Statistics::avgSatisfactionWeighted() const noexcept {
    double weighted = 0.0;
    double weight = 0.0;
    for (const auto& r : reports_) {
        const double w = static_cast<double>(r.totalCars());
        weighted += r.avgSatisfaction() * w;
        weight += w;
    }
    return safeDiv(weighted, weight);
}

int Statistics::bestDayByRevenue() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::max_element(reports_.begin(), reports_.end(),
                               [](const DailyReport& a, const DailyReport& b) {
                                   return a.totalRevenue() < b.totalRevenue();
                               });
    return it->day();
}

int Statistics::worstDayByRevenue() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::min_element(reports_.begin(), reports_.end(),
                               [](const DailyReport& a, const DailyReport& b) {
                                   return a.totalRevenue() < b.totalRevenue();
                               });
    return it->day();
}

int Statistics::bestDayBySatisfaction() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::max_element(reports_.begin(), reports_.end(),
                               [](const DailyReport& a, const DailyReport& b) {
                                   return a.avgSatisfaction() < b.avgSatisfaction();
                               });
    return it->day();
}

int Statistics::worstDayBySatisfaction() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::min_element(reports_.begin(), reports_.end(),
                               [](const DailyReport& a, const DailyReport& b) {
                                   return a.avgSatisfaction() < b.avgSatisfaction();
                               });
    return it->day();
}

int Statistics::bestDayByLost() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::min_element(reports_.begin(), reports_.end(),
                               [](const DailyReport& a, const DailyReport& b) {
                                   return a.lostCustomers() < b.lostCustomers();
                               });
    return it->day();
}

int Statistics::worstDayByLost() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::max_element(reports_.begin(), reports_.end(),
                               [](const DailyReport& a, const DailyReport& b) {
                                   return a.lostCustomers() < b.lostCustomers();
                               });
    return it->day();
}

std::map<std::string, ServiceAggregate> Statistics::aggregateServices(const std::vector<DailyReport>& reps) {
    std::map<std::string, ServiceAggregate> agg;
    for (const auto& r : reps) {
        for (const auto& [name, st] : r.perService()) {
            auto& a = agg[name];
            a.cars += st.cars;
            a.revenue += st.revenue;
        }
    }
    return agg;
}

std::vector<std::pair<std::string, ServiceAggregate>> Statistics::topServicesByRevenue(int k) const {
    if (k <= 0) return {};
    auto agg = aggregateServices(reports_);
    std::vector<std::pair<std::string, ServiceAggregate>> v(agg.begin(), agg.end());
    std::sort(v.begin(), v.end(),
              [](const auto& a, const auto& b) { return a.second.revenue > b.second.revenue; });
    if (static_cast<int>(v.size()) > k) v.resize(static_cast<size_t>(k));
    return v;
}

std::vector<std::pair<std::string, ServiceAggregate>> Statistics::topServicesByCars(int k) const {
    if (k <= 0) return {};
    auto agg = aggregateServices(reports_);
    std::vector<std::pair<std::string, ServiceAggregate>> v(agg.begin(), agg.end());
    std::sort(v.begin(), v.end(),
              [](const auto& a, const auto& b) { return a.second.cars > b.second.cars; });
    if (static_cast<int>(v.size()) > k) v.resize(static_cast<size_t>(k));
    return v;
}

std::vector<double> Statistics::revenueSeries() const {
    std::vector<double> s;
    s.reserve(reports_.size());
    for (const auto& r : reports_) s.push_back(r.totalRevenue());
    return s;
}

std::vector<double> Statistics::satisfactionSeries() const {
    std::vector<double> s;
    s.reserve(reports_.size());
    for (const auto& r : reports_) s.push_back(r.avgSatisfaction());
    return s;
}

std::vector<int> Statistics::lostSeries() const {
    std::vector<int> s;
    s.reserve(reports_.size());
    for (const auto& r : reports_) s.push_back(r.lostCustomers());
    return s;
}

void Statistics::print(std::ostream& os) const {
    os << "=== STATISTICS ===\n";
    os << "Days: " << days() << "\n";
    os << "Total cars: " << totalCars() << "\n";
    os << "Total lost: " << totalLost() << "\n";
    os << "Total revenue: " << std::fixed << std::setprecision(2) << totalRevenue() << "\n";
    os << "Avg cars/day: " << std::setprecision(2) << avgCarsPerDay() << "\n";
    os << "Avg lost/day: " << std::setprecision(2) << avgLostPerDay() << "\n";
    os << "Avg revenue/day: " << std::setprecision(2) << avgRevenuePerDay() << "\n";
    os << "Avg satisfaction (weighted): " << std::setprecision(3) << avgSatisfactionWeighted() << "\n";

    os << "Best day by revenue: " << bestDayByRevenue() << "\n";
    os << "Worst day by revenue: " << worstDayByRevenue() << "\n";
    os << "Best day by satisfaction: " << bestDayBySatisfaction() << "\n";
    os << "Worst day by satisfaction: " << worstDayBySatisfaction() << "\n";
    os << "Best day by lost: " << bestDayByLost() << "\n";
    os << "Worst day by lost: " << worstDayByLost() << "\n";

    os << "Top services by revenue:\n";
    for (const auto& [name, a] : topServicesByRevenue(5)) {
        os << "  - " << name << ": revenue=" << std::setprecision(2) << a.revenue << " cars=" << a.cars << "\n";
    }

    os << "Top services by cars:\n";
    for (const auto& [name, a] : topServicesByCars(5)) {
        os << "  - " << name << ": cars=" << a.cars << " revenue=" << std::setprecision(2) << a.revenue << "\n";
    }

    os << "Series (day -> revenue, sat, lost):\n";
    for (const auto& r : reports_) {
        os << "  day " << r.day() << " -> "
           << std::setprecision(2) << r.totalRevenue() << ", "
           << std::setprecision(3) << r.avgSatisfaction() << ", "
           << r.lostCustomers() << "\n";
    }
}
