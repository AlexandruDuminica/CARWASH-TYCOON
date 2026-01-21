#pragma once

#include <memory>
#include <string>

class WashService {
protected:
    std::string name_;
    int duration_;
    double price_;
    int water_;
    int shampoo_;
    int wax_;
    double rating_;

public:
    WashService(std::string name, int duration, double price,
                int water, int shampoo, int wax,
                double rating = 3.0);

    virtual ~WashService() = default;

    virtual std::unique_ptr<WashService> clone() const = 0;

    const std::string& name() const noexcept;
    int duration() const noexcept;
    double price() const noexcept;

    int waterNeed() const noexcept;
    int shampooNeed() const noexcept;
    int waxNeed() const noexcept;

    int needW() const noexcept { return waterNeed(); }
    int needS() const noexcept { return shampooNeed(); }
    int needX() const noexcept { return waxNeed(); }

    double rating() const noexcept;

    void applyFactor(double factor) {
        if (factor <= 0.0) return;
        price_ *= factor;
        if (price_ < 0.0) price_ = 0.0;
    }

    virtual void print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const WashService& s);
};
