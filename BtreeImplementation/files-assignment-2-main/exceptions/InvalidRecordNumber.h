#pragma once

#include <exception>

class InvalidRecordNumber : public std::exception {
private:
    int recordNumber;

public:
    explicit InvalidRecordNumber(int _recordNumber) : recordNumber{_recordNumber} {}
};
