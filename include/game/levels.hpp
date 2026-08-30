#pragma once

#include <string>
#include <vector>

class Levels
{
public:
    explicit Levels(const std::string &jsonFilePath);
    const std::string &getFirst() const;
    void setFirst(const std::string &levelPath);
    void save() const;

private:
    std::string path;
    std::string first;
};

std::vector<std::string> levelPathsIn(const std::string &directory);
std::string levelName(const std::string &levelPath);
std::string directoryOf(const std::string &levelPath);
