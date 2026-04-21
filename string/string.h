#include <algorithm>
#include <iostream>
#include <cstring>

class String {
private:
    char* str;
    size_t sz;
    size_t cap;
    String(size_t n): str(new char[n + 1]), sz(n), cap(n + 1) {}

    char* bigger(size_t cp) {
        char* str1 = new char[cp];
        std::memcpy(str1, str, sz);
        delete[] str;
        return str1;
    }
public:
    String(): String(static_cast<size_t>(0)) {
        str[0] = '\0';
    }

    String(size_t n, char c): String(n) {
        std::fill(str, str + n, c);
        str[sz] = '\0';
    }

    String(const char* c) {
        sz = strlen(c);
        cap = sz + 1;
        str = new char[cap];
        memcpy(str, c, cap);
    }

    String(const String& other): String(other.sz) {
        memcpy(str, other.str, sz + 1);
    }

    String& operator=(const String& other) {
        if (this == &other) {
            return *this;
        }
        if (other.sz + 1 <= cap) {
            sz = other.sz;
            memcpy(str, other.str, sz + 1);
            return *this;
        }
        delete[] str;
        str = new char[other.cap];
        sz = other.sz;
        cap = other.cap;
        memcpy(str, other.str, cap);
        return *this;
    }

    String& operator=(char c) {
        String copy(1, c);
        *this = copy;
        return *this;
    }

    String& operator=(const char* c) {
        String copy(c);
        *this = copy;
        return *this;
    }

    String& operator+=(const String& other) {
        if (sz + other.sz + 1 <= cap) {
            memcpy(str + sz, other.str, other.sz);
            sz += other.sz;
            str[sz] = '\0';
            return *this;
        }
        cap = sz + other.cap + 1;
        str = bigger(cap);
        memcpy(str + sz, other.str, other.cap);
        sz += other.sz;
        str[sz] = '\0';
        return *this;
    }

    String& operator+=(char c) {
        if (sz + 2 <= cap) {
            str[sz] = c;
            str[sz + 1] = '\0';
            ++sz;
            return *this;
        }
        cap *= 2;
        str = bigger(cap);
        str[sz] = c;
        str[sz + 1] = '\0';
        ++sz;
        return *this;
    }

    String operator+(const String& other) {
        String copy = *this;
        copy += other;
        return copy;
    }

    String operator+(char c) {
        String other(1, c);
        String copy = *this;
        copy += other;
        return copy;
    }

    friend bool operator==(const String& s, const String& other);

    friend bool operator<(const String& s, const String& other);

    char& operator[](int k) {
        return str[k];
    }

    const char& operator[](int k) const {
        return str[k];
    }

    size_t length() const {
        return sz;
    }

    size_t size() const {
        return sz;
    }

    size_t capacity() const {
        return cap - 1;
    }

    void clear() {
        str[0] = '\0';
        sz = 0;
    }

    bool empty() const {
        return (sz == 0);
    }

    void push_back(char c) {
        *this += c;
    }

    void pop_back() {
        if (sz >= 1) {
            --sz;
            str[sz] = '\0';
        }
    }

    char& front() {
        return str[0];
    }

    const char& front() const {
        return str[0];
    }

    char& back() {
        if (sz == 0) {
            return str[0];
        }
        return str[sz - 1];
    }

    const char& back() const {
        if (sz == 0) {
            return str[0];
        }
        return str[sz - 1];
    }

    char* data() const {
        return str;
    }

    void shrink_to_fit() {
        if (cap == sz + 1) {
            return;
        }
        char* str1 = new char[sz + 1];
        memcpy(str1, str, sz + 1);
        delete[] str;
        str = str1;
        cap = sz + 1;
    }

    size_t find(const String& substring) const {
        for (size_t i = 0; i < sz - substring.sz + 1; ++i) {
            if (std::memcmp(str + i, substring.str, substring.sz) == 0) {
                return i;
            }
        }
        return sz;
    }

    size_t rfind(const String& substring) const {
        for (long long i = sz - substring.sz; i > -1; --i) {
            if (std::memcmp(str + i, substring.str, substring.sz) == 0) {
                return i;
            }
        }
        return sz;
    }

    String substr(size_t start, size_t count) const {
        String copy(count);
        memcpy(copy.str, str + start, count);
        copy.str[count] = '\0';
        return copy;
    }

    ~String() {
        delete[] str;
    }
};

String operator+(char c, const String& other) {
    String copy(1, c);
    copy += other;
    return copy;
}

bool operator==(const String& s, const String& other) {
    return std::memcmp(s.data(), other.data(), std::max(s.size(), other.size()) + 1) == 0;
}

bool operator!=(const String& s, const String& other) {
    return !(s == other);
}

bool operator<(const String& s, const String& other) {
    return std::memcmp(s.data(), other.data(), std::max(s.size(), other.size()) + 1) < 0;
}

bool operator<=(const String& s, const String& other) {
    return !(other < s);
}

bool operator>(const String& s, const String& other) {
    return other < s;
}

bool operator>=(const String& s, const String& other) {
    return !(s < other);
}

std::istream& operator>>(std::istream& in, String& other) {
    other.clear();
    char c;
    in.get(c);
    while (std::isspace(c)) { c = in.get(); }
    while (!std::isspace(c) && c != EOF) {
        other.push_back(c);
        c = in.get();
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, const String& other) {
    out << other.data();
    return out;
}