#ifndef BASIC_H
#define BASIC_H

#include <stdint.h>
#include <math.h>

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

Vec2 operator/(float s, const Vec2 &v) {
    return v / s;
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