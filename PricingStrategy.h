#pragma once

#include <string>

class CarWash;

class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;

    virtual std::string name() const = 0;

    virtual void apply(CarWash &wash) = 0;
};

class AggressivePricing : public PricingStrategy {
public:
    std::string name() const override { return "aggressive"; }

    void apply(CarWash &wash) override;
};

class BalancedPricing : public PricingStrategy {
public:
    std::string name() const override { return "balanced"; }

    void apply(CarWash &wash) override;
};

class ConservativePricing : public PricingStrategy {
public:
    std::string name() const override { return "conservative"; }

    void apply(CarWash &wash) override;
};