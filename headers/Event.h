#pragma once

#include <string>
#include <iosfwd>

class CarWash;

class Event {
protected:
    std::string name_;
    std::string description_;

public:
    Event(std::string n, std::string d)
        : name_(std::move(n)), description_(std::move(d)) {
    }

    virtual ~Event() = default;

    const std::string &name() const noexcept { return name_; }
    const std::string &description() const noexcept { return description_; }

    virtual void apply(CarWash &wash) = 0;

    virtual void print(std::ostream &os) const;
};

class WeatherEvent : public Event {
    double intensity_; // 0.0 .. 1.0
public:
    explicit WeatherEvent(double intensity);

    void apply(CarWash &wash) override;

    void print(std::ostream &os) const override;
};

class HolidayEvent : public Event {
public:
    HolidayEvent();

    void apply(CarWash &wash) override;

    void print(std::ostream &os) const override;
};

class InspectionEvent : public Event {
    double severity_; // 0.0 .. 1.0
public:
    explicit InspectionEvent(double severity);

    void apply(CarWash &wash) override;

    void print(std::ostream &os) const override;
};

std::ostream &operator<<(std::ostream &os, const Event &ev);
