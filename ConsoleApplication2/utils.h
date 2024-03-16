#pragma once
#include "header.h"
#include <optional>
#include <functional>
#include <random>
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


void type_string(const std::string& str);

void click(int x, int y);

inline int random() {
    // 使用 Mersenne Twister 随机数引擎
    std::random_device rd; // 从硬件生成随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法，以随机设备的随机数种子进行初始化

    // 定义生成随机数的范围
    std::uniform_int_distribution<> dis(1, 100); // 生成 1 到 100 之间的随机整数

    // 生成随机数
    return dis(gen);
}