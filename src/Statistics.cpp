#include "../headers/Statistics.h"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <ostream>
#include <vector>

/**
 * @file Statistics.cpp
 * @brief Implements advanced analytics over a sequence of DailyReport objects.
 */

/**
 * @brief Constructs a Statistics instance from a list of daily reports.
 *
 * @param reps Reports to analyze. Ownership is moved into the Statistics object.
 */
Statistics::Statistics(std::vector<DailyReport> reps)
    : reports_(std::move(reps)) {
}

/**
 * @brief Safe division helper returning 0 when denominator is zero.
 *
 * @param a Numerator.
 * @param b Denominator.
 * @return a / b if b != 0, otherwise 0.
 */
double Statistics::safeDiv(double a, double b) {
    if (b == 0.0) return 0.0;
    return a / b;
}

/**
 * @brief Returns the number of days (reports) available.
 *
 * @return Number of stored DailyReport entries.
 */
int Statistics::days() const noexcept {
    return static_cast<int>(reports_.size());
}

/**
 * @brief Computes total cars served across all reports.
 *
 * @return Sum of totalCars() across all days.
 */
int Statistics::totalCars() const noexcept {
    int sum = 0;
    for (const auto &r: reports_) sum += r.totalCars();
    return sum;
}

/**
 * @brief Computes total lost customers across all reports.
 *
 * @return Sum of lostCustomers() across all days.
 */
int Statistics::totalLost() const noexcept {
    int sum = 0;
    for (const auto &r: reports_) sum += r.lostCustomers();
    return sum;
}

/**
 * @brief Computes total revenue across all reports.
 *
 * @return Sum of totalRevenue() across all days.
 */
double Statistics::totalRevenue() const noexcept {
    double sum = 0.0;
    for (const auto &r: reports_) sum += r.totalRevenue();
    return sum;
}

/**
 * @brief Computes average cars served per day.
 *
 * @return totalCars / days (0 if no days).
 */
double Statistics::avgCarsPerDay() const noexcept {
    return safeDiv(static_cast<double>(totalCars()), static_cast<double>(days()));
}

/**
 * @brief Computes average lost customers per day.
 *
 * @return totalLost / days (0 if no days).
 */
double Statistics::avgLostPerDay() const noexcept {
    return safeDiv(static_cast<double>(totalLost()), static_cast<double>(days()));
}

/**
 * @brief Computes average revenue per day.
 *
 * @return totalRevenue / days (0 if no days).
 */
double Statistics::avgRevenuePerDay() const noexcept {
    return safeDiv(totalRevenue(), static_cast<double>(days()));
}

/**
 * @brief Computes weighted average satisfaction across days.
 *
 * Each day's average satisfaction is weighted by the number of cars served that day.
 *
 * @return Weighted average satisfaction (0 if total weight is zero).
 */
double Statistics::avgSatisfactionWeighted() const noexcept {
    double weighted = 0.0;
    double weight = 0.0;
    for (const auto &r: reports_) {
        const double w = static_cast<double>(r.totalCars());
        weighted += r.avgSatisfaction() * w;
        weight += w;
    }
    return safeDiv(weighted, weight);
}

/**
 * @brief Returns the day index with maximum revenue.
 *
 * @return Day number of the best revenue day; 0 if no reports exist.
 */
int Statistics::bestDayByRevenue() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::max_element(reports_.begin(), reports_.end(),
                               [](const DailyReport &a, const DailyReport &b) {
                                   return a.totalRevenue() < b.totalRevenue();
                               });
    return it->day();
}

/**
 * @brief Returns the day index with minimum revenue.
 *
 * @return Day number of the worst revenue day; 0 if no reports exist.
 */
int Statistics::worstDayByRevenue() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::min_element(reports_.begin(), reports_.end(),
                               [](const DailyReport &a, const DailyReport &b) {
                                   return a.totalRevenue() < b.totalRevenue();
                               });
    return it->day();
}

/**
 * @brief Returns the day index with maximum average satisfaction.
 *
 * @return Day number of the best satisfaction day; 0 if no reports exist.
 */
int Statistics::bestDayBySatisfaction() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::max_element(reports_.begin(), reports_.end(),
                               [](const DailyReport &a, const DailyReport &b) {
                                   return a.avgSatisfaction() < b.avgSatisfaction();
                               });
    return it->day();
}

/**
 * @brief Returns the day index with minimum average satisfaction.
 *
 * @return Day number of the worst satisfaction day; 0 if no reports exist.
 */
int Statistics::worstDayBySatisfaction() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::min_element(reports_.begin(), reports_.end(),
                               [](const DailyReport &a, const DailyReport &b) {
                                   return a.avgSatisfaction() < b.avgSatisfaction();
                               });
    return it->day();
}

/**
 * @brief Returns the day index with minimum number of lost customers.
 *
 * @return Day number of the best (lowest lost) day; 0 if no reports exist.
 */
int Statistics::bestDayByLost() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::min_element(reports_.begin(), reports_.end(),
                               [](const DailyReport &a, const DailyReport &b) {
                                   return a.lostCustomers() < b.lostCustomers();
                               });
    return it->day();
}

/**
 * @brief Returns the day index with maximum number of lost customers.
 *
 * @return Day number of the worst (highest lost) day; 0 if no reports exist.
 */
int Statistics::worstDayByLost() const noexcept {
    if (reports_.empty()) return 0;
    auto it = std::max_element(reports_.begin(), reports_.end(),
                               [](const DailyReport &a, const DailyReport &b) {
                                   return a.lostCustomers() < b.lostCustomers();
                               });
    return it->day();
}

/**
 * @brief Aggregates per-service statistics across a set of reports.
 *
 * For each service name, this function sums total cars and total revenue.
 *
 * @param reps Reports to aggregate.
 * @return Map from service name to aggregated metrics.
 */
std::map<std::string, ServiceAggregate> Statistics::aggregateServices(const std::vector<DailyReport> &reps) {
    std::map<std::string, ServiceAggregate> agg;
    for (const auto &r: reps) {
        for (const auto &kv: r.perService()) {
            const auto &name = kv.first;
            const auto &st = kv.second;
            auto &a = agg[name];
            a.cars += st.cars;
            a.revenue += st.revenue;
        }
    }
    return agg;
}

/**
 * @brief Returns the top-k services by total revenue across all days.
 *
 * @param k Maximum number of entries to return.
 * @return Vector of (serviceName, aggregate) pairs sorted by revenue desc.
 */
std::vector<std::pair<std::string, ServiceAggregate> > Statistics::topServicesByRevenue(int k) const {
    if (k <= 0) return {};
    auto agg = aggregateServices(reports_);
    std::vector<std::pair<std::string, ServiceAggregate> > v(agg.begin(), agg.end());
    std::sort(v.begin(), v.end(),
              [](const auto &a, const auto &b) { return a.second.revenue > b.second.revenue; });
    if (static_cast<int>(v.size()) > k) v.resize(static_cast<size_t>(k));
    return v;
}

/**
 * @brief Returns the top-k services by total cars served across all days.
 *
 * @param k Maximum number of entries to return.
 * @return Vector of (serviceName, aggregate) pairs sorted by cars desc.
 */
std::vector<std::pair<std::string, ServiceAggregate> > Statistics::topServicesByCars(int k) const {
    if (k <= 0) return {};
    auto agg = aggregateServices(reports_);
    std::vector<std::pair<std::string, ServiceAggregate> > v(agg.begin(), agg.end());
    std::sort(v.begin(), v.end(),
              [](const auto &a, const auto &b) { return a.second.cars > b.second.cars; });
    if (static_cast<int>(v.size()) > k) v.resize(static_cast<size_t>(k));
    return v;
}

/**
 * @brief Returns a revenue time series aligned with the report order.
 *
 * @return Vector of daily revenue values.
 */
std::vector<double> Statistics::revenueSeries() const {
    std::vector<double> s;
    s.reserve(reports_.size());
    for (const auto &r: reports_) s.push_back(r.totalRevenue());
    return s;
}

/**
 * @brief Returns a satisfaction time series aligned with the report order.
 *
 * @return Vector of daily average satisfaction values.
 */
std::vector<double> Statistics::satisfactionSeries() const {
    std::vector<double> s;
    s.reserve(reports_.size());
    for (const auto &r: reports_) s.push_back(r.avgSatisfaction());
    return s;
}

/**
 * @brief Returns a lost-customers time series aligned with the report order.
 *
 * @return Vector of daily lost customer counts.
 */
std::vector<int> Statistics::lostSeries() const {
    std::vector<int> s;
    s.reserve(reports_.size());
    for (const auto &r: reports_) s.push_back(r.lostCustomers());
    return s;
}

/**
 * @brief Prints a detailed statistics report to an output stream.
 *
 * The report includes totals, averages, best/worst days, min/max values and simple
 * trend estimates based on (last - first) over the series.
 *
 * @param os Output stream.
 */
void Statistics::print(std::ostream &os) const {
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

    const auto rev = revenueSeries();
    const auto sat = satisfactionSeries();
    const auto lost = lostSeries();

    auto minmaxRev = std::minmax_element(rev.begin(), rev.end());
    auto minmaxSat = std::minmax_element(sat.begin(), sat.end());
    auto minmaxLost = std::minmax_element(lost.begin(), lost.end());

    const double revTrend = (rev.size() >= 2) ? (rev.back() - rev.front()) : 0.0;
    const double satTrend = (sat.size() >= 2) ? (sat.back() - sat.front()) : 0.0;
    const int lostTrend = (lost.size() >= 2) ? (lost.back() - lost.front()) : 0;

    os << "Revenue min/max: "
            << std::setprecision(2)
            << (rev.empty() ? 0.0 : *minmaxRev.first) << " / " << (rev.empty() ? 0.0 : *minmaxRev.second)
            << " | trend=" << revTrend << "\n";

    os << "Satisfaction min/max: "
            << std::setprecision(3)
            << (sat.empty() ? 0.0 : *minmaxSat.first) << " / " << (sat.empty() ? 0.0 : *minmaxSat.second)
            << " | trend=" << satTrend << "\n";

    os << "Lost min/max: "
            << (lost.empty() ? 0 : *minmaxLost.first) << " / " << (lost.empty() ? 0 : *minmaxLost.second)
            << " | trend=" << lostTrend << "\n";

    os << "Top services by revenue:\n";
    for (const auto &p: topServicesByRevenue(5)) {
        os << "  - " << p.first << ": revenue=" << std::setprecision(2) << p.second.revenue
                << " cars=" << p.second.cars << "\n";
    }

    os << "Top services by cars:\n";
    for (const auto &p: topServicesByCars(5)) {
        os << "  - " << p.first << ": cars=" << p.second.cars
                << " revenue=" << std::setprecision(2) << p.second.revenue << "\n";
    }

    os << "Series (day -> revenue, sat, lost):\n";
    for (const auto &r: reports_) {
        os << "  day " << r.day() << " -> "
                << std::setprecision(2) << r.totalRevenue() << ", "
                << std::setprecision(3) << r.avgSatisfaction() << ", "
                << r.lostCustomers() << "\n";
    }
}
