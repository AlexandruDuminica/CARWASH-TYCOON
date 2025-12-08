#pragma once

#include <string>
#include <iosfwd>
#include <memory>

class CarWash;   // forward declare, fara include

class Upgrade {
protected:
    std::string name_;
    std::string description_;
    double cost_;

public:
    Upgrade(std::string name, std::string desc, double cost)
        : name_(std::move(name)), description_(std::move(desc)), cost_(cost) {}

    virtual ~Upgrade() = default;

    const std::string& name() const noexcept { return name_; }
    const std::string& description() const noexcept { return description_; }
    double cost() const noexcept { return cost_; }

    virtual void apply(CarWash& wash) = 0;
    virtual std::unique_ptr<Upgrade> clone() const = 0;

    virtual void print(std::ostream& os) const;
    friend std::ostream& operator<<(std::ostream&, const Upgrade&);
};

class BaySpeedUpgrade : public Upgrade {
public:
    BaySpeedUpgrade()
        : Upgrade("BaySpeed", "Creste viteza bailor cu 20%", 300.0) {}

    void apply(CarWash& wash) override;
    std::unique_ptr<Upgrade> clone() const override;
};

class ComfortUpgrade : public Upgrade {
public:
    ComfortUpgrade()
        : Upgrade("Comfort", "Creste confortul clientilor", 200.0) {}

    void apply(CarWash& wash) override;
    std::unique_ptr<Upgrade> clone() const override;
};

class MarketingUpgrade : public Upgrade {
public:
    MarketingUpgrade()
        : Upgrade("Marketing", "Creste cererea initiala", 250.0) {}

    void apply(CarWash& wash) override;
    std::unique_ptr<Upgrade> clone() const override;
};
