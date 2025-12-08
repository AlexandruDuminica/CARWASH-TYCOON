#pragma once

#include <string>
#include <iosfwd>

class CarWash;

class Goal {
protected:
    std::string description_;
    bool achieved_{false};

public:
    explicit Goal(std::string desc)
        : description_(std::move(desc)) {}

    virtual ~Goal() = default;

    const std::string& description() const noexcept { return description_; }
    bool isAchieved() const noexcept { return achieved_; }

    // verifică dacă obiectivul este atins, actualizează intern achieved_
    virtual bool check(const CarWash& wash) = 0;

    // progres între 0.0 și 1.0
    virtual double progress(const CarWash& wash) const = 0;
};

class ProfitGoal : public Goal {
    double target_;
public:
    ProfitGoal(double target, std::string desc)
        : Goal(std::move(desc)), target_(target) {}

    bool check(const CarWash& wash) override;
    double progress(const CarWash& wash) const override;
};

class CarsServedGoal : public Goal {
    int target_;
public:
    CarsServedGoal(int target, std::string desc)
        : Goal(std::move(desc)), target_(target) {}

    bool check(const CarWash& wash) override;
    double progress(const CarWash& wash) const override;
};

class RatingGoal : public Goal {
    double target_;
public:
    RatingGoal(double target, std::string desc)
        : Goal(std::move(desc)), target_(target) {}

    bool check(const CarWash& wash) override;
    double progress(const CarWash& wash) const override;
};
