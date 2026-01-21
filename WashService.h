#pragma once
#include <iosfwd>
#include <memory>
#include <string>

class WashService {
protected:
    std::string name_;
    int duration_{0};
    double price_{0.0};

    int waterNeed_{0};
    int shampooNeed_{0};
    int waxNeed_{0};

    double rating_{0.0};

public:
    WashService(std::string name,
                int duration,
                double price,
                int waterNeed,
                int shampooNeed,
                int waxNeed,
                double rating);

    virtual ~WashService() = default;

    const std::string& name() const noexcept { return name_; }
    int duration() const noexcept { return duration_; }
    double price() const noexcept { return price_; }

    int waterNeed() const noexcept { return waterNeed_; }
    int shampooNeed() const noexcept { return shampooNeed_; }
    int waxNeed() const noexcept { return waxNeed_; }

    int needW() const noexcept { return waterNeed_; }
    int needS() const noexcept { return shampooNeed_; }
    int needX() const noexcept { return waxNeed_; }

    double rating() const noexcept { return rating_; }

    virtual void applyFactor(double factor);

    virtual void print(std::ostream& os) const;
    virtual double finalPriceForCars(int cars) const;

    virtual std::unique_ptr<WashService> clone() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const WashService& s);
};
