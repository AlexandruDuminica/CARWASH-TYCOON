#pragma once

#include <vector>
#include <memory>
#include <iosfwd>

#include "Event.h"

class CarWash;

class EventManager {
    std::vector<std::unique_ptr<Event> > todays_;

public:
    EventManager() = default;

    void clear();

    void generateForNewDay(const CarWash &wash);

    void applyAll(CarWash &wash);

    // helper: apelat la sfarsitul fiecarui endCurrentDay
    void startNewDay(CarWash &wash);

    void print(std::ostream &os) const;
};
