#pragma once

#include <utility>
#include <signals.hpp>

template <class Owner, class... Args> class Event
{
public:
    template <class Handler> fteng::connection_raw connect(Handler &&handler) const
    {
        return signal.connect(std::forward<Handler>(handler));
    }

private:
    friend Owner;

    void operator()(Args... args)
    {
        signal(args...);
    }

    fteng::signal<void(Args...)> signal;
};
