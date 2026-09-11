#pragma once

#include <optional>
#include <string>

class ImGuiComplaints
{
public:
    ImGuiComplaints();
    ~ImGuiComplaints();
    ImGuiComplaints(const ImGuiComplaints &) = delete;
    ImGuiComplaints &operator=(const ImGuiComplaints &) = delete;
    ImGuiComplaints(ImGuiComplaints &&) = delete;
    ImGuiComplaints &operator=(ImGuiComplaints &&) = delete;

    void startAgain();
    std::optional<std::string> anything() const;
};
