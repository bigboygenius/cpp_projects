#include <iostream>
#include <string>
#include <vector>

class BigInteger;
bool operator<(const BigInteger& one, const BigInteger& other);
bool operator>(const BigInteger& one, const BigInteger& other);
bool operator<=(const BigInteger& one, const BigInteger& other);
bool operator>=(const BigInteger& one, const BigInteger& other);
BigInteger operator+(const BigInteger& one, const BigInteger& other);
BigInteger operator*(const BigInteger& one, const BigInteger& other);
BigInteger operator/(const BigInteger& one, const BigInteger& other);
BigInteger operator%(const BigInteger& one, const BigInteger& other);


class BigInteger {
private:
    const int cDigit = 1000000000;
    std::vector<int> number_;
    int sign_ = 1;

    void Shift() {
        number_.push_back(number_[number_.size() - 1]);
        for (int i = number_.size() - 2; i > 0; --i) {
          number_[i] = number_[i - 1];
        }
      number_[0] = 0;
    }

    void RemoveZeros() {
        while (number_.back() == 0 && number_.size() > 1) {
            number_.pop_back();
        }
    }
public:
    BigInteger() {
        number_.resize(1, 0);
    }

    BigInteger(long long x) {
        if (x < 0) {
          sign_ = -1;
          x = -x;
        }
        size_t size = 0;
        unsigned long long z = x;
        while (z >= 1) {
            ++size;
            z /= cDigit;
        }
        number_.resize(size);
        if (x == 0) {
            size = 1;
            number_.push_back(0);
        } else {
            for (size_t i = 0; i < size; ++i) {
              number_[i] = x % cDigit;
                x /= cDigit;
            }
        }
    }

    BigInteger(const BigInteger& other) = default;

    explicit BigInteger(std::string str) {
        if (str == "0" || str == "-0") {
            number_.resize(1);
          number_[0] = 0;
            return;
        }
        if (str[0] == '-') {
          sign_ = -1;
            str = str.substr(1, str.size() - 1);
        }
        long long last = str.size();
        for (long long i = str.size() - 9; i >= 0; i = i - 9) {
            std::string substring = str.substr(i, 9);
            if (i == 0) {
                number_.push_back(std::stoi(substring));
            } else {
                number_.push_back(std::stoi(substring));
            }
            last = i;
        }
        if (last > 0) {
            std::string substring = str.substr(0, last);
            number_.push_back(std::stoi(substring));
        }
      RemoveZeros();
    }

    std::string ToString() const {
        if (number_.size() == 1 && number_[0] == 0) {
            return "0";
        }
        std::string s;
        if (sign_ == -1) {
            s += "-";
        }
        s += std::to_string(number_[number_.size() - 1]);
        for (int i = number_.size() - 2; i >= 0; --i) {
            std::string s1(9 - std::to_string(number_[i]).length(), '0');
            s += s1 + std::to_string(number_[i]);
        }
        return s;
    }

    BigInteger Abs() const {
        BigInteger copy = *this;
        copy.sign_ = 1;
        return copy;
    }

    friend bool operator==(const BigInteger& one, const BigInteger& other);
    friend bool operator<(const BigInteger& one, const BigInteger& other);

    BigInteger& operator=(const BigInteger& other) {
        if (this == &other) {
            return *this;
        }
      number_ = other.number_;
      sign_ = other.sign_;
        return *this;
    }

    BigInteger& operator+=(const BigInteger& other) {
        if (other == 0) {
            return *this;
        }
        if (sign_ == other.sign_) {
            BigInteger one = (Abs() < other.Abs()) ? other : *this;
            BigInteger two = (Abs() < other.Abs()) ? *this : other;
            for (size_t i = 0; i < one.number_.size(); ++i) {
                if (i < two.number_.size()) {
                    one.number_[i] += two.number_[i];
                }
                if (one.number_[i] >= cDigit) {
                    one.number_[i] %= cDigit;
                    if (i + 1 < one.number_.size()) {
                        one.number_[i + 1] += 1;
                    } else {
                        one.number_.push_back(1);
                    }
                }
            }
            *this = one;
            return *this;
        }
        *this -= -other;
        return *this;
    }

    BigInteger& operator-=(const BigInteger& other) {
        if (other == 0) {
            return *this;
        }
        if (other == *this) {
            *this = 0;
            return *this;
        }
        if (sign_ != other.sign_) {
            *this += -other;
            return *this;
        }
        BigInteger minuend = (Abs() < other.Abs()) ? other : *this;
        BigInteger subtrahend = (Abs() < other.Abs()) ? *this : other;
        int flag = 1;
        if (minuend == *this) {
            flag = minuend.sign_;
        } else {
            flag = minuend.sign_ * -1;
        }
        for (size_t i = 0; i < minuend.number_.size(); ++i) {
            minuend.number_[i] -= i < subtrahend.number_.size() ? subtrahend.number_[i] : 0;
            if (minuend.number_[i] < 0) {
                minuend.number_[i] += cDigit;
                minuend.number_[i + 1] -= 1;
            }
        }
        *this = minuend;
      sign_ = flag;
      RemoveZeros();
        return *this;
    }

    BigInteger& operator*=(const BigInteger& other) {
        if (*this == 0) {
            return *this;
        }
        if (other == 0) {
            *this = 0;
            return *this;
        }
        BigInteger ans = 0;
        ans.sign_ = sign_ * other.sign_;
        ans.number_.resize(number_.size() + other.number_.size() + 1);
        for (size_t i = 0; i < number_.size(); ++i) {
            size_t remainder = 0;
            for (size_t j = 0; j < other.number_.size() || remainder != 0; ++j) {
                size_t k = ans.number_[i + j] + number_[i] * 1ull * (j < other.number_.size() ? other.number_[j] : 0) + remainder;
                remainder = k / cDigit;
                ans.number_[i + j] = k - remainder * cDigit;
            }
        }
        *this = ans;
      RemoveZeros();
        return *this;
    }

    BigInteger& operator/=(const BigInteger& other) {
        if (Abs() == other.Abs()) {
            *this = sign_ * other.sign_;
            return *this;
        }
        if (other.Abs() == 1) {
            *this *= other.sign_;
            return *this;
        }
        if (Abs() < other.Abs()) {
            *this = 0;
            return *this;
        }
        BigInteger ans = 0;
        ans.number_.resize(number_.size());
        ans.sign_ = sign_ * other.sign_;
        BigInteger current = 0;
        for (int i = number_.size() - 1; i >= 0; --i) {
          current.Shift();
            current.number_[0] = number_[i];
          current.RemoveZeros();
            int x = 0;
            int left = 0;
            int right = cDigit - 1;
            while (left <= right) {
                int m = (left + right) / 2;
                BigInteger inter = other.Abs() * m;
                if (inter <= current) {
                    x = m;
                    left = m + 1;
                } else {
                    right = m - 1;
                }
            }
            ans.number_[i] = x;
            current -= other.Abs() * x;
        }
        *this = ans;
      RemoveZeros();
        return *this;
    }

    BigInteger& operator%=(const BigInteger& other) {;
        *this -= *this / other * other;
        return *this;
    }

    BigInteger operator-() const {
        BigInteger copy = *this;
        if (number_.size() > 1 || number_[0] > 0) {
            copy.sign_ *= -1;
        }
        return copy;
    }

    BigInteger& operator++() {
        *this += 1;
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger copy = *this;
        *this += 1;
        return copy;
    }

    BigInteger& operator--() {
        *this -= 1;
        return *this;
    }

    BigInteger operator--(int) {
        BigInteger copy = *this;
        *this -= 1;
        return copy;
    }

    explicit operator bool() const {
        return !(number_.size() == 1 && number_[0] == 0);
    }

    void Swap(BigInteger& b) {
        std::swap(*this, b);
    }

    BigInteger Gcd(BigInteger b) {
        BigInteger copy = *this;
        while (b) {
            copy %= b;
          copy.Swap(b);
        }
        return copy;
    }

    BigInteger Lcm(const BigInteger& b) {
        BigInteger copy = *this;
        return (copy * b) / Gcd(b);
    }
};

bool operator==(const BigInteger& one, const BigInteger& other) {
    if (one.sign_ != other.sign_ || one.number_.size() != other.number_.size()) {
        return false;
    }
    for (size_t i = 0; i < one.number_.size(); ++i) {
        if (one.number_[i] != other.number_[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const BigInteger& one, const BigInteger& other) {
    return !(one == other);
}

bool operator<(const BigInteger& one, const BigInteger& other) {
    if (one.sign_ == other.sign_) {
        if (one.sign_ == 1) {
            if (one.number_.size() == other.number_.size()) {
                for (int i = one.number_.size() - 1; i >= 0; --i) {
                    if (one.number_[i] < other.number_[i]) {
                        return true;
                    }
                    if (one.number_[i] > other.number_[i]) {
                        return false;
                    }
                }
                return false;
            }
            return (one.number_.size() < other.number_.size());
        }
        if (one.number_.size() == other.number_.size()) {
            for (int i = one.number_.size() - 1; i >= 0; --i) {
                if (one.number_[i] > other.number_[i]) {
                    return true;
                }
                if (one.number_[i] < other.number_[i]) {
                    return false;
                }
            }
            return false;
        }
        return (one.number_.size() > other.number_.size());
    }
    return one.sign_ < other.sign_;
}

bool operator>(const BigInteger& one, const BigInteger& other) {
    return other < one;
}

bool operator<=(const BigInteger& one, const BigInteger& other) {
    return !(other < one);
}

bool operator>=(const BigInteger& one, const BigInteger& other) {
    return !(one < other);
}

BigInteger operator+(const BigInteger& one, const BigInteger& other) {
    BigInteger copy = one;
    copy += other;
    return copy;
}

BigInteger operator-(const BigInteger& one, const BigInteger& other) {
    BigInteger copy = one;
    copy -= other;
    return copy;
}

BigInteger operator*(const BigInteger& one, const BigInteger& other) {
    BigInteger copy = one;
    copy *= other;
    return copy;
}

BigInteger operator/(const BigInteger& one, const BigInteger& other) {
    BigInteger copy = one;
    copy /= other;
    return copy;
}

BigInteger operator%(const BigInteger& one, const BigInteger& other) {
    BigInteger copy = one;
    copy %= other;
    return copy;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& num) {
    out << num.ToString();
    return out;
}

std::istream& operator>>(std::istream& in, BigInteger& other) {
    std::string str;
    in >> str;
    other = BigInteger{str};
    return in;
}

BigInteger operator ""_bi(unsigned long long x) {
    return BigInteger{static_cast<long long>(x)};
}

BigInteger operator ""_bi(const char* str, size_t) {
    std::string str1 = str;
    return BigInteger{str1};
}

class Rational;
bool operator==(const Rational& one, const Rational& other);
bool operator!=(const Rational& one, const Rational& other);
bool operator<(const Rational& one, const Rational& other);
bool operator>(const Rational& one, const Rational& other);
bool operator<=(const Rational& one, const Rational& other);
bool operator>=(const Rational& one, const Rational& other);
Rational operator+(const Rational& one, const Rational& other);
Rational operator-(const Rational& one, const Rational& other);
Rational operator*(const Rational& one, const Rational& other);
Rational operator/(const Rational& one, const Rational& other);


class Rational {
private:
    BigInteger numerator = 1;
    BigInteger denominator = 1;
    int sign = 1;

    void reduce() {
        if (numerator == 0) {
            *this = 0;
            return;
        }
        BigInteger divider = numerator.Gcd(denominator);
        numerator /= divider;
        denominator /= divider;
    }

    Rational abs() const {
        Rational copy = *this;
        copy.sign = 1;
        return copy;
    }
public:

    Rational(const Rational& other) = default;

    Rational(const BigInteger& n) : numerator(n) {
        if (n < 0) {
            numerator = numerator.Abs();
            sign = -1;
        }
    }

    Rational(long long n) : numerator(n) {
        if (n < 0) {
            numerator = numerator.Abs();
            sign = -1;
        }
    }

    std::string toString() const {
        std::string str;
        if (numerator == 0) {
            str = "0";
            return str;
        }
        if (sign == -1) {
            str = "-";
        }
        if (denominator == 1) {
            str += numerator.ToString();
        } else {
            str += numerator.ToString() + "/" + denominator.ToString();
        }
        return str;
    }

    std::string asDecimal(size_t precision = 0) const {
        if (*this == 0) {
            return "0";
        }
        std::string str;
        if (precision == 0) {
            str += (numerator / denominator).ToString();
            return str;
        }
        if (sign == -1) {
            str += "-";
        }
        str += (numerator / denominator).ToString() + '.';
        BigInteger remainder = numerator % denominator;
        size_t count = -1;
        while (remainder < denominator) {
            ++count;
            remainder *= 10;
        }
        std::string str1(precision + (denominator.ToString()).size() + 10, '0');
        str1 = remainder.ToString() + str1;
        remainder = BigInteger(str1);
        remainder /= denominator;
        str1 = remainder.ToString().substr(0, precision - count + 1);
        std::string str2(count, '0');
        if (0 <= std::stoi(str1.substr(precision - count - 1, 1)) && std::stoi(str1.substr(precision - count - 1, 1)) <= 4) {
            str += str2 + str1.substr(0, precision - count);
        } else {
            remainder = BigInteger(str1.substr(0, precision - count)) + 1;
            str += str2 + remainder.ToString();
        }
        return str;
    }

    explicit operator double() const {
        return std::stod(asDecimal(17));
    }

    friend bool operator==(const Rational& one, const Rational& other);

    friend bool operator<(const Rational& one, const Rational& other);

    Rational& operator=(const Rational& other) {
        if (this == &other) {
            return *this;
        }
        numerator = other.numerator;
        denominator = other.denominator;
        sign = other.sign;
        return *this;
    }

    Rational& operator+=(const Rational& other) {
        if (sign == other.sign) {
            Rational copy = other;
            BigInteger denom = denominator.Lcm(copy.denominator);
            numerator *= denom / denominator;
            copy.numerator *= denom / copy.denominator;
            denominator = denom;
            numerator += copy.numerator;
            reduce();
            return *this;
        }
        *this -= -other;
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        if (sign == other.sign) {
            Rational copy = other;
            BigInteger denom = denominator.Lcm(copy.denominator);
            numerator *= denom / denominator;
            copy.numerator *= denom / copy.denominator;
            denominator = denom;
            numerator -= copy.numerator;
            sign *= numerator < 0 ? -1 : 1;
            numerator = numerator.Abs();
            if (numerator == 0) {
                *this = 0;
            }
            reduce();
            return *this;
        }
        *this += -other;
        return *this;
    }

    Rational& operator*=(const Rational& other) {
        if (other == 0 || *this == 0) {
            *this = 0;
            return *this;
        }
        numerator *= other.numerator;
        denominator *= other.denominator;
        sign = sign * other.sign;
        reduce();
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        if (*this == 0) {
            *this = 0;
            return *this;
        }
        numerator *= other.denominator;
        denominator *= other.numerator;
        sign = sign * other.sign;
        reduce();
        return *this;
    }

    Rational operator-() const {
        if (*this == 0) {
            return *this;
        }
        Rational copy = *this;
        copy.sign *= -1;
        return copy;
    }
};

bool operator==(const Rational& one, const Rational& other) {
    if (one.sign != other.sign) {
        return false;
    }
    return one.numerator * other.denominator == other.numerator * one.denominator;
}

bool operator!=(const Rational& one, const Rational& other) {
    return !(one == other);
}

bool operator<(const Rational& one, const Rational& other) {
    if (one.sign == other.sign) {
        if (one.sign == 1) {
            return one.numerator * other.denominator < other.numerator * one.denominator;
        }
        return one.numerator * other.denominator > other.numerator * one.denominator;
    }
    return one.sign < other.sign;
}

bool operator>(const Rational& one, const Rational& other) {
    return other < one;
}

bool operator<=(const Rational& one, const Rational& other) {
    return !(other < one);
}

bool operator>=(const Rational& one, const Rational& other) {
    return !(one < other);
}

Rational operator+(const Rational& one, const Rational& other) {
    Rational copy = one;
    copy += other;
    return copy;
}

Rational operator-(const Rational& one, const Rational& other) {
    Rational copy = one;
    copy -= other;
    return copy;
}

Rational operator*(const Rational& one, const Rational& other) {
    Rational copy = one;
    copy *= other;
    return copy;
}

Rational operator/(const Rational& one, const Rational& other) {
    Rational copy = one;
    copy /= other;
    return copy;
}
