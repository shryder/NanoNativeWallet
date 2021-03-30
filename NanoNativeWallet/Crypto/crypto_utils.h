#pragma once

#include <cryptopp/modes.h>
#include <boost/multiprecision/cpp_int.hpp>
using namespace CryptoPP;

using uint128_t = boost::multiprecision::uint128_t;
using uint256_t = boost::multiprecision::uint256_t;

double rawToNano(uint256_t amount);
uint256_t decode_dec(std::string const& text);

std::string encryptAES(std::vector<byte> vMessage, std::vector<byte> vPassword, std::vector<byte> vIV);
std::string encryptAES(std::string sMessage, std::string sPassword, std::vector<byte> vIV);

std::string decryptAES(std::vector<byte> encryptedData, std::vector<byte> vIV, std::string password);

std::vector<byte> generateIV();

std::string encodeBase32(std::vector<byte> bytes);
std::string derivePublicAddressFromSecret(std::vector<byte> secretKey);

std::vector<byte> deriveSecretKey(std::string seed, size_t index);

std::vector<byte> HexToBytes(const std::string& hex);
std::string BytesToHex(std::vector<byte> bytes);

struct IVKeyPair {
    std::vector<byte> IV;
    std::vector<byte> key;
};
