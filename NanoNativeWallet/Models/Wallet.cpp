#include "Wallet.h"
#include "Account.h"

#include <future>
#include <vector>
#include <mutex>

#include "../Crypto/crypto_utils.h"

Wallet::Wallet(std::string walletUuid, std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV) {
    uuid = walletUuid;
    name = walletName;
    encryptedSeed = walletEncryptedSeed;

    iv = IV;
    accounts = { };
    isEncrypted = true;
    
    ui_name = name + "##" + walletUuid;
}

Wallet::Wallet(std::string walletUuid, std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV, std::string walletSeed) {
    uuid = walletUuid;
    name = walletName;
    encryptedSeed = walletEncryptedSeed;

    iv = IV;
    accounts = { };
    isEncrypted = false;
    
    ui_name = name + "##" + walletUuid;

    seed = walletSeed;
    
    loadAccounts();
}

std::vector<std::future<void>> f_AccountsInfo;
std::mutex m_AccountsLock;

bool OrderByIndex (Account a, Account b) {
    return a.index < b.index;
}

void Wallet::addAccount(size_t index) {
    // Check if account already exists
    for (Account account : accounts) {
        if(account.index == index) return;
    }

    auto accountSecret = deriveSecretKey(seed, index);
    auto derivedPublicAddress = derivePublicAddressFromSecret(accountSecret);

    f_AccountsInfo.push_back(std::async(std::launch::async, [accounts = &accounts, index, derivedPublicAddress] () {
        auto account = Account(index, derivedPublicAddress);

        std::lock_guard<std::mutex> lock(m_AccountsLock);
        accounts->push_back(account);
        std::sort(accounts->begin(), accounts->end(), OrderByIndex);
    }));
}

void Wallet::addAccount() {
    addAccount(accounts.size());
}

void Wallet::unlock(std::string password) {
    isEncrypted = false;
    seed = decryptAES(encryptedSeed, iv, password);

    loadAccounts();
}

void Wallet::lock() {
    isEncrypted = true;

    accounts.clear();
    seed.clear();
}

void Wallet::loadAccounts() {
    for (size_t i = 0; i < 5; i++) {
        addAccount(i);
    }
}

void Wallet::setName(char* newName) {
    name = std::string(newName);
    ui_name = name + "##" + uuid;
}

void Wallet::updatePassword(char* newPassword) {
    iv = generateIV();
    auto vEncryptedSeed = encryptAES(seed, std::string(newPassword), iv);

    encryptedSeed = std::vector<byte>(vEncryptedSeed.begin(), vEncryptedSeed.end());

    lock();
}