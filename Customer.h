#pragma once

#include <string>
#include <vector>
#include <iosfwd>
#include <memory>

class WashService;

class Customer {
protected:
    int id_;
    double budget_;
    double impatience_;
    double satisfaction_{0.0};

public:
    Customer(int id, double budget, double impatience);

    virtual ~Customer() = default;

    int id() const noexcept { return id_; }
    double budget() const noexcept { return budget_; }
    double impatience() const noexcept { return impatience_; }
    double satisfaction() const noexcept { return satisfaction_; }

    virtual std::string type() const = 0;

    virtual const WashService *chooseService(const std::vector<WashService *> &services) const = 0;

    virtual void onServed(const WashService &svc, double pricePaid, int waitMinutes);

    virtual std::unique_ptr<Customer> clone() const = 0;

    virtual void print(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, const Customer &c);
};

class RushedCustomer : public Customer {
public:
    RushedCustomer(int id, double budget, double impatience)
        : Customer(id, budget, impatience) {
    }

    std::string type() const override { return "Rushed"; }

    const WashService *chooseService(const std::vector<WashService *> &services) const override;

    std::unique_ptr<Customer> clone() const override;
};

class BudgetCustomer : public Customer {
public:
    BudgetCustomer(int id, double budget, double impatience)
        : Customer(id, budget, impatience) {
    }

    std::string type() const override { return "Budget"; }

    const WashService *chooseService(const std::vector<WashService *> &services) const override;

    std::unique_ptr<Customer> clone() const override;
};

class PremiumCustomer : public Customer {
public:
    PremiumCustomer(int id, double budget, double impatience)
        : Customer(id, budget, impatience) {
    }

    std::string type() const override { return "Premium"; }

    const WashService *chooseService(const std::vector<WashService *> &services) const override;

    std::unique_ptr<Customer> clone() const override;
};

class EcoCustomer : public Customer {
public:
    EcoCustomer(int id, double budget, double impatience)
        : Customer(id, budget, impatience) {
    }

    std::string type() const override { return "Eco"; }

    const WashService *chooseService(const std::vector<WashService *> &services) const override;

    std::unique_ptr<Customer> clone() const override;
};
