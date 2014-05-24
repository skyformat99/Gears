// The MIT License (MIT)

// Copyright (c) 2012-2013 Danny Y., Rapptz

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

#ifndef GEARS_MATH_UINTX_HPP
#define GEARS_MATH_UINTX_HPP

#include <cstddef>
#include <limits>
#include <vector>
#include <string>
#include <stdexcept>
#include "../meta/alias.hpp"

#ifndef GEARS_NO_IOSTREAM
#include <iosfwd>
#endif // GEARS_NO_IOSTREAM

namespace gears {
namespace math {
using namespace gears::meta; // workaround for extremely strange bug
namespace detail {
constexpr size_t pow(size_t base, size_t exponent) {
    return exponent == 0 ? 1 : (base * pow(base, exponent - 1));
}

template<typename Cont>
inline void reverse(Cont& c) {
    auto first = std::begin(c);
    auto last = std::end(c);

    if(first == last)
        return;
    --last;
    while(first < last) {
        using std::swap;
        swap(*first, *last);
        ++first;
        --last;
    }
}

template<typename InIt, typename OutIt, typename UnaryF>
inline OutIt map(InIt first, InIt last, OutIt result, UnaryF&& f) {
    for(; first != last; ++first, ++result) {
        *result = f(*first);
    }
    return result;
}

template<typename InIt, typename InIt2, typename OutIt, typename BinaryF>
inline OutIt map(InIt first, InIt last, InIt2 start, OutIt result, BinaryF&& f) {
    for (; first != last; ++first, ++start, ++result) {
        *result = f(*first, *start);
    }
    return result;
}

template<typename T>
struct partial_cast {
    template<typename U>
    T operator()(const U& u, size_t base) const {
        T result{};
        for(int i = u.size() - 1; i >= 0; --i) {
            result = result * base + u[i];
        }
        return result;
    }
};

template<>
struct partial_cast<std::string> {
    template<typename U>
    std::string operator()(const U& u, size_t base) const {
        unsigned long long x = 0;
        for(int i = u.size() - 1; i >= 0; --i) {
            x = x * base + u[i];
        }
        return std::to_string(x);
    }
};

} // detail

template<size_t Bits = static_cast<size_t>(-1), typename Digit = unsigned int, typename Digits = unsigned long long>
struct uintx {
private:
    using signed_type = gears::meta::Type<std::make_signed<Digits>>;
    static constexpr size_t digit_bits = sizeof(Digits) * 8;
    static constexpr size_t digit_count = Bits == size_t(-1) ? -1 : (Bits / digit_bits);
    static_assert(digit_count, "Invalid bits parameter. Note: Use -1 for \"infinite\" precision");

    std::vector<Digits> digits;

    struct add {
        Digits operator()(Digits a, Digits b) {
            return a + b;
        }
    };

    struct multiply {
        Digits k;
        multiply() = default;
        multiply(Digits n): k(n) {}

        Digits operator()(Digits a) {
            return a * k;
        }
    };

    struct carry {
        Digits& c;
        carry(Digits& a): c(a) {}

        Digits operator()(Digits n) {
            n += c;
            c = n / base;
            n %= base;

            return n;
        }
    };

    template<class Base>
    struct operator_carry : public Base, public carry {
        operator_carry(Digits& c): carry(c) {}

        Digits operator()(Digits a) {
            return carry::operator()(Base::operator()(a));
        }

        Digits operator()(Digits a, Digits b) {
            return carry::operator()(Base::operator()(a, b));
        }
    };

    struct borrow {
        signed_type& b;
        borrow(signed_type& b): b(b) {}

        signed_type operator()(signed_type n) {
            n -= b;
            b = 0;
            while(n < 0) {
                n += base;
                ++b;
            }
            return n;
        }
    };

    struct sub_borrow {
        signed_type& borrow;
        sub_borrow(signed_type& b): borrow(b) {}

        Digits operator()(signed_type a, signed_type b) {
            signed_type n = b - a - borrow;
            borrow = 0;
            while(n < 0) {
                n += base;
                ++borrow;
            }
            return n;
        }
    };

    void normalize() {
        while(digits.size() > 1) {
            if(digits.back() == 0)
                digits.pop_back();
            else
                break;
        }
    }

    void add_zeroes(Digits n) {
        digits.resize(digits.size() + n);
        digits.insert(digits.begin(), n, 0);
    }

    void check_bits() {
        if(digit_count == Bits)
            return;
        if(digits.capacity() >= digit_count)
            digits.resize(digit_count);
    }

    void multiply_helper(uintx& x, Digits digit) {
        Digits c = 0;
        operator_carry<multiply> mulwc(c);
        mulwc.k = digit;
        detail::map(x.digits.begin(), x.digits.end(), x.digits.begin(), mulwc);

        while(c) {
            x.digits.push_back(c % base);
            c /= base;
        }

        x.normalize();
        x.check_bits();
    }

    Digit divide_helper(const uintx& remainder, const uintx& denominator) {
        Digit minimum = 0;
        Digit maximum = base - 1;
        while(maximum - minimum > 0) {
            Digit total = maximum + minimum;
            Digit half_total = total / 2;
            total = total - half_total * 2 ? half_total + 1 : half_total;
            auto product = denominator * total;
            if(product == remainder) {
                return total;
            }

            if(remainder > product) {
                minimum = total;
            }
            else {
                maximum = total - 1;
            }
        }

        return minimum;
    }

public:
    static constexpr size_t digits10 = std::numeric_limits<Digit>::digits10;
    static constexpr size_t base = detail::pow(10, digits10);
    uintx(): digits(1) {}

    template<typename Integer, gears::meta::EnableIf<std::is_integral<Integer>> = gears::meta::_>
    uintx(Integer value) {
        value = abs(value);
        do {
            digits.push_back(value % base);
            value /= base;
        }
        while(value > 0);
    }

    uintx(const std::string& s) {
        size_t count = digits10 - (s.size() % digits10);
        std::string str(count, '0');
        str += s;
        digits.reserve(str.size() / digits10 + 1);
        auto size = str.size();
        size_t position = 0;

        while(position != size) {
            digits.push_back(std::stoull(str.substr(position, digits10)));
            position += digits10;
        }

        detail::reverse(digits);
        check_bits();
    }

    uintx& operator+=(const uintx& other) {
        if(digits.size() < other.digits.size()) {
            digits.resize(other.digits.size());
        }

        Digits c = 0;
        operator_carry<add> addwc(c);

        auto it = detail::map(other.digits.begin(), other.digits.end(), digits.begin(), digits.begin(), addwc);
        detail::map(it, digits.end(), it, carry(c));

        if(c)
            digits.push_back(c);

        normalize();
        check_bits();
        return *this;
    }

    uintx& operator-=(const uintx& other) {
        signed_type b = 0;
        auto it = detail::map(other.digits.begin(), other.digits.end(), digits.begin(), digits.begin(), sub_borrow(b));
        detail::map(it, digits.end(), it, borrow(b));
        normalize();
        check_bits();
        return *this;
    }

    uintx& operator*=(const uintx& other) {
        const uintx* first = this;
        const uintx* second = &other;

        auto first_size = first->digits.size();
        auto second_size = second->digits.size();

        if(first_size < second_size) {
            using std::swap;
            swap(first, second);
            swap(first_size, second_size);
        }

        std::vector<uintx> ints(second_size);

        for(unsigned i = 0; i < second_size; ++i) {
            ints[i].digits = first->digits;
            multiply_helper(ints[i], second->digits[i]);
        }

        *this = ints.front();
        digits.resize(first_size + second_size);

        for(unsigned i = 1; i < second_size; ++i) {
            ints[i].add_zeroes(i);
            *this += ints[i];
        }

        normalize();
        check_bits();
        return *this;
    }

    uintx& operator/=(const uintx& other) {
        if(other == 0)
            throw std::logic_error("Division by zero");

        uintx remainder;
        auto copy = digits;
        digits.clear();
        digits.resize(copy.size(), 0);
        remainder.digits.reserve(digits.size());
        for(int i = digits.size() - 1; i >= 0; --i) {
            remainder.add_zeroes(1);
            remainder.digits[0] = copy[i];
            remainder.normalize();
            Digit count = divide_helper(remainder, other);
            remainder -= other * count;
            digits[i] += count;
        }

        normalize();
        check_bits();
        return *this;
    }

    uintx& operator%=(const uintx& other) {
        if(other == 0)
            throw std::logic_error("Division by zero");

        auto copy = digits;
        digits.clear();
        for(int i = copy.size() - 1; i >= 0; --i) {
            add_zeroes(1);
            digits[0] = copy[i];
            normalize();
            *this -= other * divide_helper(*this, other);
        }
        normalize();
        check_bits();
        return *this;
    }

    uintx operator+(const uintx& other) const {
        uintx result(*this);
        return (result += other);
    }

    uintx operator-(const uintx& other) const {
        uintx result(*this);
        return (result -= other);
    }

    uintx operator*(const uintx& other) const {
        uintx result(*this);
        return (result *= other);
    }

    uintx operator/(const uintx& other) const {
        uintx result(*this);
        return (result /= other);
    }

    uintx operator%(const uintx& other) const {
        uintx result(*this);
        return (result %= other);
    }

    bool operator==(const uintx& other) const {
        return digits == other.digits;
    }

    bool operator!=(const uintx& other) const {
        return digits != other.digits;
    }

    bool operator<(const uintx& other) const {
        auto i = digits.size();

        if(i < other.digits.size())
            return true;
        if(other.digits.size() < i)
            return false;

        --i;

        do {
            if(digits[i] < other.digits[i])
                return true;
            if(other.digits[i] < digits[i])
                return false;
            --i;
        } while (i > 0);

        return false;
    }

    bool operator>(const uintx& other) const {
        return !(*this < other);
    }

    bool operator<=(const uintx& other) const {
        return !(other < *this);
    }

    bool operator>=(const uintx& other) const {
        return !(*this < other);
    }

    uintx operator++(int) {
        auto copy = *this;
        *this += 1;
        return copy;
    }

    const uintx& operator++() {
        *this += 1;
        return *this;
    }

    uintx operator--(int) {
        auto copy = *this;
        *this -= 1;
        return copy;
    }

    const uintx& operator--() {
        *this -= 1;
        return *this;
    }

    explicit operator bool() const noexcept {
        return digits.back() > 0;
    }

    #ifndef GEARS_NO_IOSTREAM
    template<typename Elem, typename Traits>
    friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& out, const uintx& n) {
        auto first = n.digits.crbegin();
        auto last = n.digits.crend();
        if(first != last)
            out << *first++;
        while(first != last) {
            out.fill('0');
            out.width(uintx::digits10);
            out << *first++;
        }
        return out;
    }

    template<typename Elem, typename Traits>
    friend std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& in, uintx& n) {
        std::string str;
        in >> str;
        n = str;
        return in;
    }

    #endif // GEARS_NO_IOSTREAM

    template<typename T, size_t N, typename U, typename V>
    friend T uintx_cast(const uintx<N, U, V>& obj);
};

template<typename T, size_t N, typename U, typename V>
inline T uintx_cast(const uintx<N, U, V>& obj) {
    return detail::partial_cast<T>()(obj.digits, uintx<N, U, V>::base);
}

namespace literals {
template<char... Numbers>
inline uintx<> operator"" _x() noexcept {
    const char str[sizeof...(Numbers)] = { Numbers... };
    return { std::string(str, sizeof(str)) };
}

inline uintx<> operator"" _x(const char* s, size_t len) {
    return { std::string(s, len) };
}
} // literals
} // math
} // gears

#endif // GEARS_MATH_UINTX_HPP