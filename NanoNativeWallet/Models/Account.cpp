#include "Account.h"
#include "../Crypto/crypto_utils.h"
#include "../NodeRPC/NodeRPC.h"
#include <future>

Account::Account(size_t i, std::string accountAddress) {
    address = accountAddress;
    index = i;

    UpdateAccountInfo();
}

void Account::UpdateAccountInfo() {
    // This currently blocks the whole app for a while until the HTTP requests are done, need to do this asynchronously
    auto accountInfo = NodeRPC::GetAccountInfo(address);
    // auto accountInfo = std::async(std::launch::async, NodeRPC::GetAccountInfo, address).get();

    // Check if account is open
    if (accountInfo.contains("frontier")) {
        isAccountOpen = true;

        balance = decode_raw_str(accountInfo["balance"]);
        pending = decode_raw_str(accountInfo["pending"]);

        block_count = std::stoi((std::string) accountInfo["block_count"]); // to be fixed

        representative = accountInfo["representative"];

        auto accountHistory = NodeRPC::GetAccountHistory(address);
        account_history = accountHistory["history"];
    }
}

void Account::SetBalance(nano::amount newBalance) {
    balance = newBalance;
}

std::string Account::getNANOBalance() {
    return balance.format_balance(raw_ratio, 6, true);
}

void Account::hide() {
    hidden = true;
}

void Account::show() {
    hidden = false;
}