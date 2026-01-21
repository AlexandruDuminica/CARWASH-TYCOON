#pragma once
#include <cstddef>

template <typename T>
class RunningStat {
    T sum_{};
    std::size_t count_{0};

public:
    void add(const T& v) {
        sum_ += v;
        ++count_;
    }

    std::size_t count() const noexcept { return count_; }

    double average() const noexcept {
        return count_ ? static_cast<double>(sum_) / static_cast<double>(count_) : 0.0;
    }
};
