#include "WashBay.h"
#include "WashService.h"
#include <algorithm>
#include <ostream>

int WashBay::bayCount_ = 0;

char *WashBay::dup(const std::string &s) {
    char *p = new char[s.size() + 1];
    std::copy(s.c_str(), s.c_str() + s.size() + 1, p);
    return p;
}

WashBay::WashBay(int id, int startMin, const std::string &text)
    : id_(id), availMin_(startMin), label_(dup(text)) {
    ++bayCount_;
}

WashBay::WashBay(const WashBay &o)
    : id_(o.id_), availMin_(o.availMin_), label_(dup(o.label_)),
      canBasic_(o.canBasic_), canDeluxe_(o.canDeluxe_), canWax_(o.canWax_) {
}

WashBay &WashBay::operator=(const WashBay &o) {
    if (this != &o) {
        id_ = o.id_;
        availMin_ = o.availMin_;
        delete [] label_;
        label_ = dup(o.label_);
        canBasic_ = o.canBasic_;
        canDeluxe_ = o.canDeluxe_;
        canWax_ = o.canWax_;
    }
    return *this;
}

WashBay::~WashBay() {
    delete [] label_;
}

bool WashBay::canDo(const std::string &n) const {
    if (n == "Basic" || n == "basic" || n == "Eco") return canBasic_;
    if (n == "Deluxe" || n == "deluxe") return canDeluxe_;
    if (n == "Wax" || n == "wax") return canWax_;
    return false;
}

int WashBay::bookOne(const WashService &sp, int earliest) {
    int start = (earliest > availMin_) ? earliest : availMin_;
    int finish = start + sp.duration();
    availMin_ = finish;
    return finish;
}

std::ostream &operator<<(std::ostream &os, const WashBay &b) {
    os << "Bay #" << b.id() << " (at=" << b.availAt() << "m, [";
    bool first = true;
    if (b.hasBasic()) {
        os << "Basic";
        first = false;
    }
    if (b.hasDeluxe()) {
        os << (first ? "" : ",") << "Deluxe";
        first = false;
    }
    if (b.hasWax()) { os << (first ? "" : ",") << "Wax"; }
    os << "])";
    return os;
}
