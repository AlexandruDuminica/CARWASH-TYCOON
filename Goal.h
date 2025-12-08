#pragma once

#include <string>
#include <iosfwd>

class CarWash;

class Goal {
protected:
    std::string description_;
    bool achieved_{false};
    double lastProgress_{0.0}; // 0.0 .. 1.0

    void setProgressInternal(double p) {
        if (p < 0.0) p = 0.0;
        if (p > 1.0) p = 1.0;
        lastProgress_ = p;
    }

public:
    explicit Goal(std::string desc)
        : description_(std::move(desc)) {}

    virtual ~Goal() = default;

    const std::string& description() const noexcept { return description_; }
    bool isAchieved() const noexcept { return achieved_; }

    // verifica si actualizeaza progresul + achieved_
    virtual bool check(const CarWash& wash) = 0;

    // acces la progres (NU mai depinde de CarWash)
    double progress() const noexcept { return lastProgress_; }
};

class ProfitGoal : public Goal {
    double target_;
public:
    ProfitGoal(double target, std::string desc)
        : Goal(std::move(desc)), target_(target) {}

    bool check(const CarWash& wash) override;
};

class CarsServedGoal : public Goal {
    int target_;
public:
    CarsServedGoal(int target, std::string desc)
        : Goal(std::move(desc)), target_(target) {}

    bool check(const CarWash& wash) override;
};

class RatingGoal : public Goal {
    double target_;
public:
    RatingGoal(double target, std::string desc)
        : Goal(std::move(desc)), target_(target) {}

    bool check(const CarWash& wash) override;
};
