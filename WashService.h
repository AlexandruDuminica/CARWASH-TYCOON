#pragma once
#include <string>
#include <iosfwd>
#include <memory>

class WashService {
protected:
    std::string name_;
    int durationMin_{};
    double basePrice_{};
    int waterL_{};
    int shampooMl_{};
    int waxMl_{};

    virtual void print(std::ostream &os) const = 0;

public:
    WashService(std::string name, int durationMin, double basePrice,
                int waterL, int shampooMl, int waxMl);

    virtual ~WashService() = default;

    const std::string& name()   const noexcept { return name_; }
    int duration()              const noexcept { return durationMin_; }
    double price()              const noexcept { return basePrice_; }
    int needW()                 const noexcept { return waterL_; }
    int needS()                 const noexcept { return shampooMl_; }
    int needX()                 const noexcept { return waxMl_; }

    bool applyFactor(double f);

    virtual double finalPriceForCars(int cars) const = 0;
    virtual double rating() const { return 4.0; }

    virtual std::unique_ptr<WashService> clone() const = 0;

    friend std::ostream& operator<<(std::ostream&, const WashService&);
};
