#include "EventManager.h"
#include "CarWash.h"

#include <cstdlib>
#include <ctime>
#include <ostream>

void EventManager::clear() {
    todays_.clear();
}

void EventManager::generateForNewDay(const CarWash &wash) {
    if (todays_.empty()) {
        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            seeded = true;
        }

        double rep = wash.reputationScore(); // 0..5 aproximativ
        double r = (std::rand() % 100) / 100.0;

        // sanse de evenimente in functie de reputatie
        if (r < 0.4) {
            double intensity = (std::rand() % 100) / 100.0;
            todays_.push_back(std::make_unique<WeatherEvent>(intensity));
        }
        if (r > 0.6) {
            todays_.push_back(std::make_unique<HolidayEvent>());
        }
        // reputatie mica => sanse mai mari de inspectie
        double inspProb = rep < 3.5 ? 0.5 : 0.2;
        double r2 = (std::rand() % 100) / 100.0;
        if (r2 < inspProb) {
            double severity = (std::rand() % 100) / 100.0;
            todays_.push_back(std::make_unique<InspectionEvent>(severity));
        }
    }
}

void EventManager::applyAll(CarWash &wash) {
    for (auto &e: todays_) {
        if (e) e->apply(wash);
    }
}

void EventManager::startNewDay(CarWash &wash) {
    clear();
    generateForNewDay(wash);
    applyAll(wash);
}

void EventManager::print(std::ostream &os) const {
    if (todays_.empty()) {
        os << "Evenimente azi: (nimic special)\n";
        return;
    }
    os << "Evenimente azi:\n";
    for (const auto &e: todays_) {
        if (e) {
            os << "  - " << *e << "\n";
        }
    }
}
