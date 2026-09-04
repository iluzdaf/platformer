#pragma once

#include <exception>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <signals.hpp>

template <class... Args> class EditorCommand
{
public:
    void operator()(Args... args)
    {
        requested.emplace_back(args...);
    }

    template <class Handler> fteng::connection_raw connect(Handler &&handler)
    {
        return signal.connect(std::forward<Handler>(handler));
    }

    void drain()
    {
        std::vector<std::tuple<std::decay_t<Args>...>> firing;
        firing.swap(requested);
        for (const auto &args : firing)
        {
            try
            {
                std::apply([this](const auto &...unpacked) { signal(unpacked...); }, args);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }

private:
    fteng::signal<void(Args...)> signal;
    std::vector<std::tuple<std::decay_t<Args>...>> requested;
};
