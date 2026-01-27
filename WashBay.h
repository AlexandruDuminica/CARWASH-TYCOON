#pragma once
#include <string>
#include <iosfwd>

class WashService;

class WashBay {
    int id_;
    int availMin_;
    char *label_;
    bool canBasic_{true};
    bool canDeluxe_{false};
    bool canWax_{false};

    static int bayCount_;

    char *dup(const std::string &s);

public:
    WashBay(int id, int startMin, const std::string &text);

    WashBay(const WashBay &o);

    WashBay &operator=(const WashBay &o);

    ~WashBay();

    int id() const noexcept { return id_; }
    int availAt() const noexcept { return availMin_; }
    bool hasBasic() const noexcept { return canBasic_; }
    bool hasDeluxe() const noexcept { return canDeluxe_; }
    bool hasWax() const noexcept { return canWax_; }

    void addDeluxe() noexcept { canDeluxe_ = true; }
    void addWax() noexcept { canWax_ = true; }

    bool canDo(const std::string &n) const;

    int bookOne(const WashService &sp, int earliest);

    void reset(int openMin) { availMin_ = openMin; }

    static int totalBaysCreated() noexcept { return bayCount_; }

    friend std::ostream &operator<<(std::ostream &, const WashBay &);
};