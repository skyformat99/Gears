// The MIT License (MIT)

// Copyright (c) 2012-2014 Danny Y., Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef GEARS_ENUMS_HELPERS_HPP
#define GEARS_ENUMS_HELPERS_HPP

#include <type_traits>

namespace gears {
namespace enums {
namespace detail {
template<typename... Args>
struct are_enum : std::true_type {};

template<typename T>
struct are_enum<T> : std::is_enum<T> {};

template<typename T, typename U, typename... Args>
struct are_enum<T, U, Args...> : std::integral_constant<bool, std::is_enum<T>::value &&
                                                              std::is_enum<U>::value &&
                                                              are_enum<Args...>::value> {};

template<typename... Args>
using EnableIfEnum = typename std::enable_if<are_enum<typename std::remove_reference<Args>::type...>::value, bool>::type;
} // detail

template<typename Enum, typename Underlying = typename std::underlying_type<Enum>::type, detail::EnableIfEnum<Enum> = true>
constexpr Underlying to_underlying(Enum x) noexcept {
    return static_cast<Underlying>(x);
}

template<typename Enum, detail::EnableIfEnum<Enum> = true>
constexpr Enum activate_flags(const Enum& flag) noexcept {
    return flag;
}

template<typename Enum, detail::EnableIfEnum<Enum> = true>
constexpr Enum activate_flags(const Enum& first, const Enum& second) noexcept {
    return static_cast<Enum>(to_underlying(first) | to_underlying(second));
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
constexpr Enum activate_flags(const Enum& first, const Enum& second, Enums&&... rest) noexcept {
    return static_cast<Enum>(to_underlying(activate_flags(first, second)) | to_underlying(activate_flags(rest...)));
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
inline Enum& set_flags(Enum& flags, Enums&&... args) noexcept {
    return flags = activate_flags(flags, args...);
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
inline Enum& remove_flags(Enum& flags, Enums&&... args) noexcept {
    return flags = static_cast<Enum>(to_underlying(flags) & (~to_underlying(activate_flags(args...))));
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
constexpr bool has_flags(const Enum& flags, Enums&&... args) noexcept {
    return (to_underlying(flags) & to_underlying(activate_flags(args...))) == to_underlying(activate_flags(args...));
}
} // enums
} // gears

#endif // GEARS_ENUMS_HELPERS_HPP
