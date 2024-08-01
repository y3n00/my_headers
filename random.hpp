#pragma once

#include <array>
#include <concepts>
#include <execution>
#include <limits>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <vector>

#ifdef RANDOM_STATIC
#define IF_STATIC static
#define IF_STATIC_VAR thread_local static inline
#else
#define IF_STATIC
#define IF_STATIC_VAR
#endif

template <typename T>
concept Numeric_Type = std::is_arithmetic_v<T>;

#define MAX_LIMIT(T) T max_val = (std::numeric_limits<T>::max())
#define MIN_LIMIT(T) T min_val = (std::numeric_limits<T>::min())

#define PURE_AUTO decltype(auto)

class Random_t {
    template <std::integral I>
    using int_dist = std::uniform_int_distribution<I>;

    template <std::floating_point R>
    using real_dist = std::uniform_real_distribution<R>;

   private:
    IF_STATIC_VAR std::mt19937 gen{std::random_device{}()};

    template <Numeric_Type Num_Type>
    [[nodiscard]] IF_STATIC inline PURE_AUTO get_distribution(MIN_LIMIT(Num_Type), MAX_LIMIT(Num_Type)) noexcept {
        if constexpr (std::is_floating_point_v<Num_Type>) {
            return real_dist<Num_Type>{min_val, max_val};
        } else {
            if constexpr (sizeof(Num_Type) == 1)             // cant use 1 byte types on msvc
                return int_dist<int16_t>{min_val, max_val};  // int16_t covers all numeric limits of 1 byte types
            else
                return int_dist<Num_Type>{min_val, max_val};
        }
    }

   public:
    Random_t() = default;
    Random_t(Random_t&&) = delete;
    Random_t(const Random_t&) = delete;
    Random_t& operator=(const Random_t&) = delete;

    template <Numeric_Type Num_Type>
    [[nodiscard]] IF_STATIC inline PURE_AUTO from_range(MIN_LIMIT(Num_Type), MAX_LIMIT(Num_Type)) noexcept {
        return get_distribution<Num_Type>(min_val, max_val)(gen);
    }

    template <Numeric_Type Num_Type>
    [[nodiscard]] IF_STATIC inline PURE_AUTO from_zero_to(MAX_LIMIT(Num_Type)) noexcept {
        return from_range<Num_Type>(Num_Type{}, max_val);
    }

    [[nodiscard]] IF_STATIC inline PURE_AUTO get_elem(const std::ranges::random_access_range auto& range) noexcept {
        return range[from_zero_to<std::size_t>(std::ranges::size(range) - 1)];
    }

    template <std::ranges::random_access_range R, typename Num_t = std::ranges::range_value_t<R>>
    [[noreturn]] IF_STATIC inline void fill_range(R& range, MIN_LIMIT(Num_t), MAX_LIMIT(Num_t)) noexcept {
        std::ranges::for_each(range, [&](auto& elem) { elem = from_range<Num_t>(min_val, max_val); });
    }

    [[noreturn]] IF_STATIC inline void fill_range_from(std::ranges::random_access_range auto& range, const std::ranges::random_access_range auto& from) noexcept {
        if (not std::ranges::empty(from))
            std::ranges::for_each(range, [&](auto& elem) { elem = get_elem(from); });
    }

    [[nodiscard]] IF_STATIC inline PURE_AUTO get_string(size_t str_len, const std::string& extra_chars = "") noexcept {
        constexpr std::string_view basic_symbols =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "1234567890";
        std::vector<char> result(str_len);
        const auto final_range = extra_chars + basic_symbols.data();
        fill_range_from(result, final_range);
        return std::string(result.begin(), result.end());
    }

    template <Numeric_Type Num_Type, size_t SZ>
    [[nodiscard]] IF_STATIC inline PURE_AUTO get_array(MIN_LIMIT(Num_Type), MAX_LIMIT(Num_Type)) noexcept {
        std::array<Num_Type, SZ> arr;
        fill_range(arr, min_val, max_val);
        return arr;
    }

    template <Numeric_Type Num_Type>
    [[nodiscard]] IF_STATIC inline PURE_AUTO get_vector(size_t size, MIN_LIMIT(Num_Type), MAX_LIMIT(Num_Type)) noexcept {
        std::vector<Num_Type> vec(size);
        fill_range(vec, min_val, max_val);
        return vec;
    }
};

#undef IF_STATIC
#undef IF_STATIC_VAR
#undef MAX_LIMIT
#undef MIN_LIMIT
#undef PURE_AUTO
