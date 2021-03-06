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

#ifndef GEARS_ENUMS_OPERATORS_HPP
#define GEARS_ENUMS_OPERATORS_HPP

#include <gears/enums/helpers.hpp>

namespace gears {
namespace enums {
namespace operators {
namespace detail {
template<typename Enum>
using Underlying = typename std::underlying_type<Enum>::type;
} // detail

/**
 * @ingroup enums
 * @brief Implements ~x on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator~(const Enum& x) noexcept {
    return static_cast<Enum>(~ to_underlying(x));
}

/**
 * @ingroup enums
 * @brief Implements lhs | rhs on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator|(const Enum& lhs, const Enum& rhs) noexcept {
    return static_cast<Enum>(to_underlying(lhs) | to_underlying(rhs));
}

/**
 * @ingroup enums
 * @brief Implements lhs & rhs on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator&(const Enum& lhs, const Enum& rhs) noexcept {
    return static_cast<Enum>(to_underlying(lhs) & to_underlying(rhs));
}

/**
 * @ingroup enums
 * @brief Implements lhs ^ rhs on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator^(const Enum& lhs, const Enum& rhs) noexcept {
    return static_cast<Enum>(to_underlying(lhs) ^ to_underlying(rhs));
}

/**
 * @ingroup enums
 * @brief Implements lhs |= rhs on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
inline Enum& operator|=(Enum& lhs, const Enum& rhs) noexcept {
    return lhs = static_cast<Enum>(to_underlying(lhs) | to_underlying(rhs));
}

/**
 * @ingroup enums
 * @brief Implements lhs &= rhs on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
inline Enum& operator&=(Enum& lhs, const Enum& rhs) noexcept {
    return lhs = static_cast<Enum>(to_underlying(lhs) & to_underlying(rhs));
}

/**
 * @ingroup enums
 * @brief Implements lhs ^= rhs on enums.
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
inline Enum& operator^=(Enum& lhs, const Enum& rhs) noexcept {
    return lhs = static_cast<Enum>(to_underlying(lhs) ^ to_underlying(rhs));
}

//@{
/**
 * @ingroup enums
 * @brief Implements lhs != rhs
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator!=(const Enum& lhs, const Enum& rhs) noexcept {
    return to_underlying(lhs) != to_underlying(rhs);
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator!=(const Enum& lhs, const detail::Underlying<Enum>& rhs) noexcept {
    return to_underlying(lhs) != rhs;
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator!=(const detail::Underlying<Enum>& lhs, const Enum& rhs) noexcept {
    return lhs != to_underlying(rhs);
}
//@}

//@{
/**
 * @ingroup enums
 * @brief Implements lhs == rhs
 */
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator==(const Enum& lhs, const Enum& rhs) noexcept {
    return to_underlying(lhs) == to_underlying(rhs);
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator==(const Enum& lhs, const detail::Underlying<Enum>& rhs) noexcept {
    return to_underlying(lhs) == rhs;
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator==(const detail::Underlying<Enum>& lhs, const Enum& rhs) noexcept {
    return lhs == to_underlying(rhs);
}
//@}
} // operators
} // enums
} // gears

#endif // GEARS_ENUMS_OPERATORS_HPP
