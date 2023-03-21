#pragma once

#include <exception>

class InvalidPairNumber : public std::exception {
private:
    int pairNumber;
public:
    explicit InvalidPairNumber(int _pairNumber) : pairNumber{_pairNumber} {}
};