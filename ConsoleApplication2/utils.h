#pragma once
#include "header.h"
#include <optional>
#include <functional>
#include <type_traits>

struct ProcessWindowInfo {
    DWORD pid;
    HWND hwnd;
};

template <typename _Ty>
class Maybe : public std::optional<_Ty> {
public:
    using std::optional<_Ty>::optional;
    using super = std::optional<_Ty>;

    template <class _Fn, typename... Args>
    constexpr auto and_then(_Fn&& _Func) const& {
        using _Uty = std::invoke_result_t<_Fn, const _Ty&>;

        if (super::has_value()) {
            return std::invoke(std::forward<_Fn>(_Func), static_cast<const _Ty&>(super::value()));
        }
        return _Uty{};
    }

    template <class _Fn, typename... Args>
    constexpr Maybe<_Ty> or_else(_Fn&& _Func, Args&&... args)&& {
        if (super::has_value()) {
            return std::move(*this);
        }
        else {
            return std::forward<_Fn>(_Func)(std::forward<Args>(args)...);
        }
    }
};

HANDLE GetProcessByName(std::wstring name, ProcessWindowInfo* pid);
BOOL enumWindowCallback(HWND hWnd, LPARAM lparam);