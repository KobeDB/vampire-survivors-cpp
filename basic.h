#ifndef BASIC_H
#define BASIC_H

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

//
// Primitive type aliases
//
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef float     f32;
typedef double    f64;


//
// Pair types
//
template< typename T0, typename T1 >
struct Pair {
    T0 first;
    T1 second;
};

template< typename T >
struct Result {
    T value;
    bool ok;
};


//
// Math types
//
struct Vec2 {
    union {
        struct {
            float x, y;
        };
        float v[2];
    };

    Vec2() { x = 0; y = 0; }

    Vec2(float px, float py) {
        x = px;
        y = py;
    }

    float& operator[](size_t index) { return v[index]; }

    const float& operator[](size_t index) const { return v[index]; }

    Vec2 operator+(const Vec2 &w) const {
        return { x + w.x, y + w.y };
    }

    Vec2 operator-(const Vec2 &w) const {
        return { x - w.x, y - w.y };
    }

    Vec2 operator*(float s) const {
        return { x * s, y * s };
    }

    Vec2 operator/(float s) const {
        return { x / s, y / s };
    }

    Vec2& operator+=(const Vec2& other) {
        *this = *this + other;
        return *this;
    }

    Vec2& operator-=(const Vec2& other) {
        *this = *this - other;
        return *this;
    }

    Vec2& operator*=(float s) {
        *this = *this * s;
        return *this;
    }

    Vec2& operator/=(float s) {
        *this = *this / s;
        return *this;
    }
};

Vec2 operator*(float s, const Vec2 &v) {
    return v * s;
}

float length(const Vec2 &v) {
    return sqrtf(v.x*v.x + v.y*v.y);
}

Vec2 normalize(const Vec2 &v) {
    return v * 1.0f/length(v);
}

float dot(const Vec2 &v, const Vec2 &w) {
    return v.x * w.x + v.y * w.y;
}

// generates random float between -1 and 1
float random_float() {
    return (rand()/float(RAND_MAX)) * 2.0f - 1.0f;
}

Vec2 random_unit_vec() {
    while (true) {
        Vec2 v = {random_float(), random_float()};
        float len = length(v);
        if (len <= 1.0f && len > 0.001) {
            return normalize(v);
        }
    }
}

template <int N>
struct Vec {
    float v[N] {};

    Vec() {
        for (int i = 0; i < N; ++i) {
            v[i] = 0.0f;
        }
    }

    float& operator[](int index) {
        assert(index < N);
        return v[index];
    }

    const float& operator[](int index) const {
        assert(index < N);
        return v[index];
    }

    Vec operator+(const Vec& other) const {
        Vec result;
        for (int i = 0; i < N; ++i) {
            result.v[i] = v[i] + other.v[i];
        }
        return result;
    }

    Vec operator-(const Vec& other) const {
        Vec result;
        for (int i = 0; i < N; ++i) {
            result.v[i] = v[i] - other.v[i];
        }
        return result;
    }

    Vec operator*(float s) const {
        Vec result;
        for (int i = 0; i < N; ++i) {
            result[i] = v[i] * s;
        }
        return result;
    }

    Vec operator/(float s) const {
        Vec result;
        for (int i = 0; i < N; ++i) {
            result[i] = v[i] / s;
        }
        return result;
    }

    Vec& operator+=(const Vec& other) {
        *this = *this + other;
        return *this;
    }

    Vec& operator-=(const Vec& other) {
        *this = *this - other;
        return *this;
    }

    Vec& operator*=(float s) {
        *this = *this * s;
        return *this;
    }

    Vec& operator/=(float s) {
        *this = *this / s;
        return *this;
    }
};

template< int N >
Vec<N> operator*(float s, const Vec<N> &v) {
    return v * s;
}

template< int N >
float dot(const Vec<N> &v, const Vec<N> &w) {
    float result = 0;
    for (int i = 0; i < N; ++i) {
        result += v[i] * w[i];
    }
    return result;
}

template< int N >
float length(const Vec<N> &v) {
    return sqrtf(dot(v, v));
}

template< int N >
Vec<N> normalize(const Vec<N> &v) {
    float len = length(v);
    assert(len > 1e-6f && "Cannot normalize a near-zero vector");
    return v * 1.0f/length(v);
}

template< int N >
Vec<N> random_unit_vec() {
    while (true) {
        Vec<N> v;
        for (int i = 0; i < N; ++i) {
            v[i] = random_float();
        }
        float len = length(v);
        if (len <= 1.0f && len > 0.01) {
            return normalize(v);
        }
    }
}

// TODO
// struct Vec2 : public Vec<2> {
//     Vec2(float x, float y) : Vec<2>{} {
//         v[0] = x;
//         v[1] = y;
//     }
// };

struct Vec3 : public Vec<3> {
    Vec3(float x, float y, float z) : Vec<3>{} {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }

    Vec3(const Vec<3> &other) : Vec<3>{other} {}
    Vec3 &operator=(const Vec<3> &other) {
        if (this != &other) { // check for self-assignment.
            Vec<3>::operator=(other);
        }
        return *this;
    }

    float &x() { return this->v[0]; }
    const float &x() const { return this->v[0]; }

    float &y() { return this->v[1]; }
    const float &y() const { return this->v[1]; }

    float &z() { return this->v[2]; }
    const float &z() const { return this->v[2]; }
};

//
// Defer
//
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})


#endif