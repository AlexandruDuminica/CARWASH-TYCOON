#pragma once

#include <string>
#include <iosfwd>
#include <memory>

class CarWash;

class Goal {
protected:
    std::string description_;
    bool achieved_{false};

public:
    explicit Goal(std::string desc) : description_(std::move(desc)) {}
    virtual ~Goal() = default;

    const std::string& description() const noexcept { return description_; }
    bool isAchieved() const noexcept { return achieved_; }

    virtual bool check(const CarWash& wash) = 0;
    virtual void print(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream&, const Goal&);
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
