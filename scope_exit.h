#pragma once

#include <utility>


// Andrei Alexandrescu implementation:
// https://www.youtube.com/watch?v=WjTrfoiB0MQ

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) \
    CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) \
    CONCATENATE(str, __LINE__)
#endif


namespace detail {

    template<typename Func>
    struct ScopeGuard
    {
        explicit ScopeGuard(Func&& func) : m_Func(std::forward<Func>(func)) {}
        ~ScopeGuard() { m_Func(); }

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

        ScopeGuard(ScopeGuard&&) = default;
        ScopeGuard& operator=(ScopeGuard&&) = default;

    private:
        Func m_Func;
    };

    struct ScopeGuardExit{};

    template<typename Func>
    ScopeGuard<Func> operator+(ScopeGuardExit, Func&& fn) {
        return ScopeGuard<Func>(std::forward<Func>(fn));
    }

}

#define SCOPE_EXIT \
    auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE) = ::detail::ScopeGuardExit() + [&] ()
