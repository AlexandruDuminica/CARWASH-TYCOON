#include "DailyReport.h"

#include <ostream>
#include <iomanip>

void DailyReport::beginDay(int day) {
    day_ = day;
    totalCars_ = 0;
    lostCustomers_ = 0;
    totalRevenue_ = 0.0;
    avgSatisfaction_ = 0.0;
    perService_.clear();
}

void DailyReport::addServiceSale(const std::string& name, double price) {
    ServiceStats& st = perService_[name];
    st.cars += 1;
    st.revenue += price;
}

void DailyReport::finalize(int totalCars, int lost, double avgSat, double totalRevenue) {
    totalCars_ = totalCars;
    lostCustomers_ = lost;
    avgSatisfaction_ = avgSat;
    totalRevenue_ = totalRevenue;
}

void DailyReport::print(std::ostream& os) const {
    os << "Raport ziua " << day_ << ":\n";
    os << "  Masini spalate: " << totalCars_ << "\n";
    os << "  Clienti pierduti: " << lostCustomers_ << "\n";
    os << "  Venit total: " << std::fixed << std::setprecision(2)
       << totalRevenue_ << " EUR\n";
    os << "  Satisfactie medie: " << std::setprecision(2)
       << avgSatisfaction_ << "\n";

    if (perService_.empty()) {
        os << "  (fara servicii inregistrate)\n";
    } else {
        os << "  Detaliu pe servicii:\n";
        for (const auto& [name, st] : perService_) {
            os << "    - " << name << ": " << st.cars
               << " masini, venit " << std::fixed << std::setprecision(2)
               << st.revenue << " EUR\n";
        }
    }
}

std::ostream& operator<<(std::ostream& os, const DailyReport& rep) {
    rep.print(os);
    return os;
}
