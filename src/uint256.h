#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <array>
#include <optional>
#include <span>
#include <cassert>
#include <algorithm>
#include <compare>

/*
Implementation of a unsigned 256 bit integer intended to store a SHA256 hash code.
The uint256 is meant solely for storage and unable to have arithmetic performed on it.
*/

// Builds a decode table to translate into a hex number from an ASCII value
constexpr std::array<signed char, 256> MakeHexDigitTable(){
    std::array<signed char, 256> table{};
    for(auto& v : table) v = -1;
    for(char c = '0'; c <= '9'; ++c) table[(unsigned char)c] = c - '0';
    for(char c = 'a'; c <= 'f'; ++c) table[(unsigned char)c] = c - 'a' + 10;
    for(char c = 'A'; c <= 'F'; ++c) table[(unsigned char)c] = c - 'A' + 10;
    return table;
}
constexpr auto p_util_hexdigit = MakeHexDigitTable();
// Converts a character to its hex value
inline signed char HexDigit(char c){
    return p_util_hexdigit[(unsigned char)c];
}

// Namespace to hold helper functions for string encoding and decoding
namespace util {
    // Remmoves the "0x" prefix on inputted hex numbers
    [[nodiscard]] inline std::string_view RemovePrefixView(std::string_view str, std::string_view prefix){
        if (str.starts_with(prefix)){
            return str.substr(prefix.size());
        }
        return str;
    }

    // Returns true if a string is a valid hex number, false otherwise.
    inline bool IsHex(std::string_view str)
    {
        for (char c : str) {
            if (HexDigit(c) < 0) return false;
        }
        return (str.size() > 0) && (str.size()%2 == 0);
    }

    template <class uintN_t>
    std::optional<uintN_t> FromHex(std::string_view str)
    {
        if (uintN_t::size() * 2 != str.size() || !IsHex(str)) return std::nullopt;
        uintN_t rv;
        unsigned char* p1 = rv.begin();
        unsigned char* pend = rv.end();
        size_t digits = str.size();
        while (digits > 0 && p1 < pend) {
            *p1 = ::HexDigit(str[--digits]);
            if (digits > 0) {
                *p1 |= ((unsigned char)::HexDigit(str[--digits]) << 4);
                p1++;
            }
        }
        return rv;
    }

    template <class uintN_t>
    std::optional<uintN_t> FromUserHex(std::string_view input)
    {
        input = util::RemovePrefixView(input, "0x");
        constexpr auto expected_size{uintN_t::size() * 2};
        if (input.size() < expected_size) {
            auto padded = std::string(expected_size, '0');
            std::copy(input.begin(), input.end(), padded.begin() + expected_size - input.size());
            return FromHex<uintN_t>(padded);
        }
        return FromHex<uintN_t>(input);
    }
}

// Parent class to create a 256 bit contiguous space in memory.
// Used by uint256 to reserve space to store int value.
class buffer256 {
    public:
        constexpr buffer256() : b_data{} {}
        constexpr explicit buffer256(uint8_t v) : b_data{v}{}
        constexpr explicit buffer256(std::span<const unsigned char> vch){
            assert(vch.size() == WIDTH);
            std::copy(vch.begin(), vch.end(), b_data.begin());
        }
        consteval buffer256(std::string_view hex_str) : b_data{}{
            assert(hex_str.size() == WIDTH * 2);
            for (int i = 0; i < WIDTH; ++i){
                signed char hi = HexDigit(hex_str[i * 2]);
                signed char lo = HexDigit(hex_str[i * 2 + 1]);
                assert(hi >= 0 && lo >= 0);
                b_data[WIDTH - 1 - i] = static_cast<uint8_t>((hi << 4) | lo);
            }
        }
        constexpr bool isNull() const{
            return std::all_of(b_data.begin(), b_data.end(), [](uint8_t val){return val == 0;});
        }
        constexpr void setNull(){
            std::fill(b_data.begin(), b_data.end(), 0);
        }
        constexpr bool operator==(const buffer256&) const  = default;
        constexpr std::strong_ordering operator<=>(const buffer256&) const = default;

        static constexpr size_t size() { return WIDTH; }
        unsigned char* begin() { return b_data.data(); }
        unsigned char* end() { return b_data.data() + WIDTH; }
        const unsigned char* begin() const { return b_data.data(); }
        const unsigned char* end() const { return b_data.data() + WIDTH; }

        std::string GetHex() const;
        std::string ToString() const;

    protected:
        static constexpr int WIDTH = 32;
        std::array<uint8_t, WIDTH> b_data;
        static_assert(WIDTH == sizeof(b_data), "Sanity check");
};

// Reads the byte hex value in s and converts it to a string.
// Used in buffer256::GetHex() definition.
inline std::string HexStr(const std::span<const uint8_t> s){
    std::string rv(s.size() * 2, '\0');
    static constexpr char hexmap[] = "0123456789abcdef";
    char* p = rv.data();
    for (uint8_t v : s){
        *p++ = hexmap[v >> 4];
        *p++ = hexmap[v & 0xf];
    }
    return rv;
}

// Unsigned 256 bit integer implementation
class uint256 : public buffer256 {
    public:
        static std::optional<uint256> FromHex(std::string_view str){
            return util::FromHex<uint256>(str);
        }
        static std::optional<uint256> FromUserHex(std::string_view str){
            return util::FromUserHex<uint256>(str);
        }
        constexpr uint256() = default;
        consteval explicit uint256(std::string_view hex_str) : buffer256(hex_str){}
        constexpr explicit uint256(uint8_t v) : buffer256(v){}
        constexpr explicit uint256(std::span<const unsigned char> vch) : buffer256(vch){}
        static const uint256 ZERO;
        static const uint256 ONE;
};

inline std::string buffer256::GetHex() const{
    uint8_t b_data_rev[WIDTH];
    for(int i = 0; i < WIDTH; ++i){
        b_data_rev[i] = b_data[WIDTH - i - 1];
    }
    return HexStr(b_data_rev);
}

inline std::string buffer256::ToString() const {
    return GetHex();
}
inline const uint256 uint256::ZERO(0);
inline const uint256 uint256::ONE(1);