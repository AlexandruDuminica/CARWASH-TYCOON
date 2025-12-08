#pragma once

#include <stdexcept>
#include <string>

class CarWashException : public std::runtime_error {
public:
    explicit CarWashException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class InvalidCommandException : public CarWashException {
public:
    explicit InvalidCommandException(const std::string& msg)
        : CarWashException(msg) {}
};

class InvalidServiceException : public CarWashException {
public:
    explicit InvalidServiceException(const std::string& msg)
        : CarWashException(msg) {}
};

class InventoryException : public CarWashException {
public:
    explicit InventoryException(const std::string& msg)
        : CarWashException(msg) {}
};

class BookingException : public CarWashException {
public:
    explicit BookingException(const std::string& msg)
        : CarWashException(msg) {}
};
