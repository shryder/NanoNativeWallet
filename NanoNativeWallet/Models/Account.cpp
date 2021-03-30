#include "Account.h"
#include "../Crypto/crypto_utils.h"
#include "../NodeRPC/NodeRPC.h"

Account::Account(size_t i, std::string accountAddress, uint256_t accountBalance) {
    address = accountAddress;
    index = i;

    UpdateAccountInfo();
}

void Account::UpdateAccountInfo() {
    // This currently blocks the whole app for a while until the HTTP requests are done, need to do this on another thread or something
    auto accountInfo = NodeRPC::GetAccountInfo(address);

    // Check if account is open
    if (accountInfo.contains("frontier")) {
        isAccountOpen = true;

        balance = decode_dec(accountInfo["balance"]);
        pending = decode_dec(accountInfo["pending"]);

        block_count = decode_dec(accountInfo["block_count"]).convert_to<uint8_t>();

        representative = accountInfo["representative"];

        auto accountHistory = NodeRPC::GetAccountHistory(address);
        account_history = accountHistory["history"];
    }
}

void Account::SetBalance(uint256_t newBalance) {
    balance = newBalance;
}

double Account::getNANOBalance() {
    return rawToNano(balance);
}

void Account::hide() {
    hidden = true;
}

void Account::show() {
    hidden = false;
}