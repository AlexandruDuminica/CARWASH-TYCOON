#pragma once

#include <memory>
#include <string>
#include <iosfwd>

enum class ServiceKind { Basic, Deluxe, Wax, Eco, Custom };

class WashService {
protected:
    std::string name_;
    int duration_{0};
    double price_{0.0};
    int needW_{0};
    int needS_{0};
    int needX_{0};
    double rating_{0.0};
    ServiceKind kind_{ServiceKind::Custom};

public:
    WashService(std::string name, int durationMin, double price,
                int waterNeed, int shampooNeed, int waxNeed, double rating,
                ServiceKind kind = ServiceKind::Custom);

    virtual ~WashService() = default;

    const std::string &name() const noexcept { return name_; }
    int duration() const noexcept { return duration_; }
    double price() const noexcept { return price_; }
    int needW() const noexcept { return needW_; }
    int needS() const noexcept { return needS_; }
    int needX() const noexcept { return needX_; }
    double rating() const noexcept { return rating_; }

    ServiceKind kind() const noexcept { return kind_; }
    bool isPremium() const noexcept { return kind_ == ServiceKind::Deluxe || kind_ == ServiceKind::Wax; }

    virtual void print(std::ostream &os) const;

    virtual std::unique_ptr<WashService> clone() const = 0;

    void applyFactor(double factor);

    static std::string kindToString(ServiceKind k);
};

std::ostream &operator<<(std::ostream &os, const WashService &s);
