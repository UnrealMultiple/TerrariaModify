// ConditionalSkipper.h
// A small helper (header-only) that uses Dobby to install a hook which
// calls a user-provided callback before jumping to a target "skip" address.
// If the callback returns true, execution jumps to `end_addr` (skipping logic),
// otherwise the original function execution continues via the Dobby trampoline.
//
// Notes:
// - This header is written for ARM64 (uses `br` to branch to an address).
// - Callback has signature `bool(void*)`. If the hooked function has a `this`
//   pointer (member function), the first argument passed to the callback is
//   the `this` pointer; otherwise it will be `nullptr`.
// - You must link with Dobby and include the proper header search path so
//   `dobby.h` is found.

#ifndef CONDITIONAL_SKIPPER_H
#define CONDITIONAL_SKIPPER_H

#include <functional>
#include <tuple>
#include <type_traits>
#include <cstdint>
#include "Dobby/dobby.h"

template<typename Ret, typename... Args>
class ConditionalSkipper {
public:
    using Func = Ret(*)(Args...);

    // Install the conditional skipper:
    // - `start` : address where we install the hook (the place you previously
    //             patched with a B instruction)
    // - `end`   : address to jump to when skipping (the original skip target)
    // - `cb`    : a callback `std::function<bool(Args...)>`; it receives the
    //             same arguments as the original function and can inspect them
    //             (e.g. `this` and `int i`). Return `true` to skip to `end`.
    static bool Install(void* start, void* end, std::function<bool(Args...)> cb) {
        end_addr_ = end;
        cb_ = std::move(cb);

        void* replace = (void*)replacement;
        void* origin_ptr = nullptr;
        int r = DobbyHook(start, replace, &origin_ptr);
        if (r == RS_SUCCESS) {
            origin_ = (Func)origin_ptr;
            return true;
        }
        return false;
    }

    static bool Uninstall(void* start) {
        // DobbyDestroyHook returns int in some builds; ignore return here.
        DobbyDestroy(start);
        origin_ = nullptr;
        return true;
    }

private:
    // trampoline/original pointer provided by Dobby
    static Func origin_;
    static void* end_addr_;
    static std::function<bool(Args...)> cb_;

    // Replacement function called by Dobby instead of the original function.
    // It forwards args to the callback: if callback returns true, branch to
    // `end_addr_`. Otherwise call the original trampoline `origin_`.
    static Ret replacement(Args... args) {
        if (cb_) {
            bool skip = false;
            try {
                skip = cb_(args...);
            } catch (...) {
                skip = false;
            }
            if (skip && end_addr_) {
                void* target = end_addr_;
                asm volatile("br %0" :: "r"(target));
                __builtin_unreachable();
            }
        }

        if (origin_) {
            return origin_(args...);
        }

        if constexpr (!std::is_same_v<Ret, void>) {
            return Ret();
        }
    }
};

template<typename Ret, typename... Args>
typename ConditionalSkipper<Ret, Args...>::Func ConditionalSkipper<Ret, Args...>::origin_ = nullptr;

template<typename Ret, typename... Args>
void* ConditionalSkipper<Ret, Args...>::end_addr_ = nullptr;

template<typename Ret, typename... Args>
std::function<bool(Args...)> ConditionalSkipper<Ret, Args...>::cb_ = nullptr;

#endif // CONDITIONAL_SKIPPER_H
