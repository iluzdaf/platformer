#pragma once

namespace scoped
{
    template <class T> inline const T *held = nullptr;
}

template <class T> const T *inScope()
{
    return scoped::held<T>;
}

template <class T> class InScope
{
public:
    explicit InScope(const T &value) : before(scoped::held<T>)
    {
        scoped::held<T> = &value;
    }

    explicit InScope(T &&) = delete;

    ~InScope()
    {
        scoped::held<T> = before;
    }

    InScope(const InScope &) = delete;
    InScope &operator=(const InScope &) = delete;
    InScope(InScope &&) = delete;
    InScope &operator=(InScope &&) = delete;

private:
    const T *before = nullptr;
};
