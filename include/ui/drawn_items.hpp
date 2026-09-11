#pragma once

#include <optional>
#include <string>

class DrawnItems
{
public:
    DrawnItems();
    ~DrawnItems();
    DrawnItems(const DrawnItems &) = delete;
    DrawnItems &operator=(const DrawnItems &) = delete;
    DrawnItems(DrawnItems &&) = delete;
    DrawnItems &operator=(DrawnItems &&) = delete;

    void startAgain();
    std::optional<std::string> twoWithTheSameId() const;
};
