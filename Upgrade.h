#pragma once

#include <memory>
#include <ostream>
#include <string>

class CarWash;

class Upgrade {
public:
    virtual ~Upgrade() = default;

    virtual std::string name() const = 0;

    virtual std::string description() const = 0;

    virtual double cost() const = 0;

    virtual void apply(CarWash &cw) const = 0;

    virtual void print(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const Upgrade &up);

class BaySpeedUpgrade final : public Upgrade {
public:
    std::string name() const override;

    std::string description() const override;

    double cost() const override;

    void apply(CarWash &cw) const override;
};

class ComfortUpgrade final : public Upgrade {
public:
    std::string name() const override;

    std::string description() const override;

    double cost() const override;

    void apply(CarWash &cw) const override;
};

class MarketingUpgrade final : public Upgrade {
public:
    std::string name() const override;

    std::string description() const override;

    double cost() const override;

    void apply(CarWash &cw) const override;
};

class NanoCoatingUpgrade final : public Upgrade {
public:
    std::string name() const override;

    std::string description() const override;

    double cost() const override;

    void apply(CarWash &cw) const override;
};
