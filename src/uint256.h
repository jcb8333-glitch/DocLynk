#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <array>
#include <optional>
#include <span>
#include <cassert>

namespace util {
    [[nodiscard]] inline std::string_view RemovePrefixView(std::string_view str, std::string_view prefix){
        if (str.starts_with(prefix)){
            return str.substr(prefix.size());
        }
        return str;
    }
}

template <unsigned int BITS>
class buffer256 {
    public:
        constexpr buffer256() : b_data {}
        constexpr explicit buffer256(uint8_t v) : b_data{v}{}
        constexpr explicit buffer256(std::span<const unsigned char> vch){
            assert(vch.size() == WIDTH);
            std::copy(vch.begin(), vch.end(), b_data.begin());
        }
        consteval explicit buffer256(std::string_view hex_str);
        constexpr bool isNull() const{
            return std::all_of(b_data.begin(), b_data.end(), [](uint8_t val){return val == 0;});
        }
        constexpr void setNull(){
            std::fill(b_data.begin(), b_data.end(), 0);
        }
        constexpr bool operator==(const buffer256&) const default;
        constexpr std::strong_ordering operator<=>(const buffer256 other) const = default;
        std::string GetHex() const;
        std::string ToString() const;

    protected:
        static constexpr int WIDTH = BITS / 8;
        std::array<uint8_t, WIDTH> b_data;
        static_assert(WIDTH == sizeof(b_data, "Sanity check"));
};

template <class uintN_t>
std::optional<uintN_t> FromHex(std::string_view str);
std::string HexStr(const std::span<const uint8_t> s);
// std::string GetHex() const;

namespace detail {
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

class uint256 : public buffer256<256> {
    public:
        static std::optional<uint256> FromHex(std::string_view str){
            return detail::FromHex<uint256>(str);
        }
        static std::optional<uint256> FromUserHex(std::string_view str){
            return detail::FromUserHex<uint256>(str);
        }
        constexpr uint256() = default;
        consteval explicit uint256(std::string_view hex_str) : buffer256<256>(hex_str){}
        constexpr explicit uint256(uint8_t v) : buffer256<256>(v){}
        constexpr explicit uint256(std::span<const unsigned char> vch) : buffer256<256>(vch){}
        static const uint256 ZERO;
        static const uint256 ONE;
};
// ref
template <unsigned int BITS>
std::string buffer256<BITS>::GetHex() const{
    uint8_t b_data_rev[WIDTH];
    for(int i = 0; i < WIDTH; ++i){
        b_data_rev[i] = b_data[WIDTH - i - 1];
    }
    return HexStr(b_data_rev);
}

template std::string buffer256<256>::GetHex() const;
template std::string buffer256<256>::ToString() const;
const uint256 uint256::ZERO(0);
const uint256 uint256::ONE(1);
// fin