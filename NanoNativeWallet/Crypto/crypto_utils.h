#pragma once

#include <cryptopp/modes.h>
#include <boost/multiprecision/cpp_int.hpp>
using namespace CryptoPP;

using uint128_t = boost::multiprecision::uint128_t;
using uint256_t = boost::multiprecision::uint256_t;

std::string rawToNanoStr(uint256_t amount);

double rawToNano(uint256_t amount);
uint256_t decode_dec(std::string const& text);

std::string encryptAES(const std::vector<byte>& vMessage, const std::vector<byte>& vPassword, const std::vector<byte>& vIV);
std::string encryptAES(const std::string& sMessage, const std::string& sPassword, const std::vector<byte>& vIV);

std::string decryptAES(const std::vector<byte>& encryptedData, const std::vector<byte>& vIV, const std::string& password);

std::vector<byte> generateIV();

std::string encodeBase32(std::vector<byte> bytes);
std::string derivePublicAddressFromSecret(std::vector<byte> secretKey);

std::vector<byte> deriveSecretKey(const std::string& seed, size_t index);

std::vector<byte> HexToBytes(const std::string& hex);
std::string BytesToHex(std::vector<byte> bytes);


struct IVKeyPair {
    std::vector<byte> IV;
    std::vector<byte> key;
};
