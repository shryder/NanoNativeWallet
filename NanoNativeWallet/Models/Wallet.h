#pragma once

#include "../NanoNativeWallet.h"
#include "Account.h"

#include <vector>

class Wallet {
public:
    std::string uuid;
    std::string name;
    std::vector<byte> encryptedSeed;
    std::vector<byte> iv;
    std::string seed;

    bool isEncrypted;
    std::vector<Account> accounts;

    Wallet(std::string walletUuid, std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV);
    Wallet(std::string walletUuid, std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV, std::string walletSeed);

    void addAccount(size_t i);
    void addAccount();

    void unlock(std::string decryptedSeed);
    void lock();

    void loadAccounts();

    void setName(char* newName);
    void updatePassword(char* newPassword);
};