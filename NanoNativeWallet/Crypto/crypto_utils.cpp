#include "cryptopp/osrng.h"
#include "cryptopp/rijndael.h"
#include "cryptopp/aes.h"
#include "cryptopp/randpool.h"
#include "cryptopp/filters.h"
#include "cryptopp/hkdf.h"
#include "cryptopp/sha.h"

#include <cryptopp/blake2.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#include "Ed25519/blake2.h"
#include "Ed25519/ed25519.h"

#include "crypto_utils.h"

#include <filesystem>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string base32_characters = "13456789abcdefghijkmnopqrstuwxyz";
uint128_t const raw_ratio = uint128_t("1000000000000000000000000000000");

uint256_t decode_dec(std::string const& text) {
    if (text.size() > 78 || (text.size() > 1 && text.front() == '0') || (!text.empty() && text.front() == '-')) {
        throw "Invalid value";
    }

    std::stringstream stream(text);
    stream << std::dec << std::noshowbase;

    uint256_t number_l;
    stream >> number_l;

    if (!stream.eof()) {
        throw "!Stream EOF";
    }

    return number_l;
}

double rawToNano(uint256_t amount) {
    return (amount / raw_ratio).convert_to<double>();
}

std::string encryptAES(std::vector<byte> vMessage, std::vector<byte> vPassword, std::vector<byte> vIV) {
    SecByteBlock key(AES::MAX_KEYLENGTH + AES::BLOCKSIZE);
    std::string encrypted;

    HKDF<SHA256> hkdf;
    hkdf.DeriveKey(key, key.size(), (const byte*) vPassword.data(), vPassword.size(), (const byte*) vIV.data(), vIV.size(), NULL, 0);

    CTR_Mode<AES>::Encryption encryption;
    encryption.SetKeyWithIV(key, AES::MAX_KEYLENGTH, key + AES::MAX_KEYLENGTH);

    StringSource encryptor(std::string(vMessage.begin(), vMessage.end()), true, new StreamTransformationFilter(encryption, new StringSink(encrypted)));

    return encrypted;
}

std::string encryptAES(std::string sMessage, std::string sPassword, std::vector<byte> vIV) {
    return encryptAES(std::vector<byte>(sMessage.begin(), sMessage.end()), std::vector<byte>(sPassword.begin(), sPassword.end()), vIV);
}

std::string decryptAES(std::vector<byte> encryptedData, std::vector<byte> vIV, std::string password) {
    SecByteBlock key(AES::MAX_KEYLENGTH + AES::BLOCKSIZE);

    HKDF<SHA256> hkdf;
    hkdf.DeriveKey(key, key.size(), (const byte*)password.data(), password.size(), (const byte*)vIV.data(), vIV.size(), NULL, 0);

    CTR_Mode<AES>::Decryption decryption;
    decryption.SetKeyWithIV(key, AES::MAX_KEYLENGTH, key + AES::MAX_KEYLENGTH);

    std::string decrypted;
    VectorSource decryptor(encryptedData, true, new StreamTransformationFilter(decryption, new StringSink(decrypted)));

    return decrypted;
}

std::string generateUUID(){
    std::stringstream ss;
    ss << boost::uuids::random_generator()();

    return ss.str();
}

std::vector<byte> generateIV() {
    CryptoPP::AutoSeededRandomPool rnd;

    CryptoPP::SecByteBlock iv(16);
    rnd.GenerateBlock(iv, iv.size());

    return std::vector<byte>(iv.begin(), iv.end());
}

std::vector<byte> deriveSecretKey(std::string seed, size_t index) {
    std::vector<byte> bytes = HexToBytes(seed);
    std::vector<byte> digest;

    BLAKE2b hash((unsigned int)32);

    hash.Update((const byte*) bytes.data(), bytes.size());
    hash.Update((const byte*) &index, 4);
    digest.resize(hash.DigestSize());
    hash.Final((byte*) &digest[0]);

    HexEncoder encoder(new FileSink(std::cout));
    VectorSource digesting(digest, true, new Redirector(encoder));

    return digest;
}

std::string encodeBase32(byte* bytes, size_t length) {
    int leftover = (length * 8) % 5;
    int offset = leftover == 0 ? 0 : 5 - leftover;

    int value = 0, bits = 0;

    std::string output = "";

    for (size_t i = 0; i < length; i++) {
        value = (value << 8) | bytes[i];
        bits += 8;
        while (bits >= 5) {
            output.push_back(base32_characters[(value >> (bits + offset - 5)) & 31]);
            bits -= 5;
        }
    }

    if (bits > 0) {
        output.push_back(base32_characters[(value << (5 - (bits + offset))) & 31]);
    }

    return output;
}

std::string encodeBase32(std::vector<byte> bytes) {
    return encodeBase32(bytes.data(), bytes.size());
}

std::string derivePublicAddressFromSecret(std::vector<byte> accountSecretKey) {
    unsigned char pubkey[32];
    ed25519_publickey(accountSecretKey.data(), pubkey);

    // create checksum
    HexEncoder encoder(new FileSink(std::cout));
    std::vector<byte> checksum;

    BLAKE2b hash((unsigned int) 5);

    hash.Update((const byte*)pubkey, 32);
    checksum.resize(hash.DigestSize());
    hash.Final((byte*)&checksum[0]);

    VectorSource digesting(checksum, true, new Redirector(encoder));
    std::reverse(checksum.begin(), checksum.end());

    return "nano_" + encodeBase32((byte*)pubkey, 32) + encodeBase32(checksum);
}

std::string BytesToHex(std::vector<byte> bytes) {
    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < bytes.size(); ++i) {
        ss << std::setw(2) << std::uppercase << std::setfill('0') << (int)bytes[i];
    }

    return ss.str();
}

std::vector<byte> HexToBytes(const std::string& hex) {
    std::vector<byte> bytes;

    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        const long byte = strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }

    return bytes;
}
