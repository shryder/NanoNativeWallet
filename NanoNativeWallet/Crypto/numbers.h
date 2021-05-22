#pragma once

#include <boost/multiprecision/cpp_int.hpp>

using uint128_t = boost::multiprecision::uint128_t;
using uint256_t = boost::multiprecision::uint256_t;
uint128_t const raw_ratio = uint128_t("1000000000000000000000000000000");

namespace nano {
    class uint128_union {
        public:
            uint128_union () = default;
            /**
            * Decode from hex string
            * @warning Aborts at runtime if the input is invalid
            */
            uint128_union (std::string const &);
            uint128_union (uint64_t);
            uint128_union (uint128_t const &);
            bool operator== (uint128_union const &) const;
            bool operator!= (uint128_union const &) const;
            bool operator< (uint128_union const &) const;
            bool operator> (uint128_union const &) const;
            void encode_hex (std::string &) const;
            bool decode_hex (std::string const &);
            void encode_dec (std::string &) const;
            bool decode_dec (std::string const &, bool = false);
            bool decode_dec (std::string const &, uint128_t);
            std::string format_balance (uint128_t scale, int precision, bool group_digits) const;
            std::string format_balance (uint128_t scale, int precision, bool group_digits, const std::locale & locale) const;

            uint128_t number () const;
            void clear ();
            bool is_zero () const;
            std::string to_string () const;
            std::string to_string_dec () const;
            union
            {
                std::array<uint8_t, 16> bytes;
                std::array<char, 16> chars;
                std::array<uint32_t, 4> dwords;
                std::array<uint64_t, 2> qwords;
            };
    };

    class amount : public uint128_union {
        public:
            using uint128_union::uint128_union;
    };
};
