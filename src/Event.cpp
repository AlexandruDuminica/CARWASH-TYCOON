#include "../headers/Event.h"
#include "../headers/CarWash.h"

#include <ostream>
#include <iomanip>

void Event::print(std::ostream &os) const {
    os << name_ << " - " << description_;
}

std::ostream &operator<<(std::ostream &os, const Event &ev) {
    ev.print(os);
    return os;
}

// WeatherEvent

WeatherEvent::WeatherEvent(double intensity)
    : Event("Weather", "Vreme ploioasa / murdara"), intensity_(intensity) {
}

void WeatherEvent::apply(CarWash &wash) {
    if (intensity_ <= 0.0) return;

    // vreme urata => mai multe masini murdare => cerere mai mare
    if (intensity_ > 0.3) {
        wash.increaseBaseDemand(1);
    }
    if (intensity_ > 0.6) {
        wash.increaseBaseDemand(1);
        wash.increaseComfortBonus(-0.1); // clientii sunt mai iritati
    }
    wash.logEvent("WeatherEvent apply: intensity=" + std::to_string(intensity_));
}

void WeatherEvent::print(std::ostream &os) const {
    os << "[WeatherEvent] intensitate=" << std::fixed << std::setprecision(2)
            << intensity_ << " (" << description_ << ")";
}


HolidayEvent::HolidayEvent()
    : Event("Holiday", "Zi de weekend / sarbatoare, trafic crescut") {
}

void HolidayEvent::apply(CarWash &wash) {
    wash.increaseBaseDemand(2);
    wash.increaseComfortBonus(0.2);
    wash.logEvent("HolidayEvent apply");
}

void HolidayEvent::print(std::ostream &os) const {
    os << "[HolidayEvent] " << description_;
}

// InspectionEvent

InspectionEvent::InspectionEvent(double severity)
    : Event("Inspection", "Inspectie a autoritatilor"), severity_(severity) {
}

void InspectionEvent::apply(CarWash &wash) {
    double avg = wash.averageSatisfaction();
    double fine = 0.0;

    if (avg < 3.0) {
        fine = 150.0 * severity_;
    } else if (avg < 4.0) {
        fine = 50.0 * severity_;
    } else {
        // bonus simbolic pentru calitate buna
        wash.adjustCash(+50.0 * severity_);
        wash.logEvent("InspectionEvent: bonus pentru calitate buna");
        return;
    }

    if (fine > 0.0) {
        wash.adjustCash(-fine);
        wash.logEvent("InspectionEvent: amenda=" + std::to_string(fine));
    }
}

void InspectionEvent::print(std::ostream &os) const {
    os << "[InspectionEvent] severitate=" << std::fixed << std::setprecision(2)
            << severity_ << " (" << description_ << ")";
}
