#include "Account.h"
#include "../Crypto/crypto_utils.h"
#include "../NodeRPC/NodeRPC.h"
#include "../Logger/Logger.h"
#include "Transaction.h"

#include <future>
#include <nlohmann/json.hpp>

Account::Account(size_t i, const std::string &accountAddress) {
    address = accountAddress;
    index = i;

    UpdateUIName();
    UpdateAccountInfo();
}

void Account::UpdateUIName() {
    ui_name = address + " - #" + std::to_string(index) + "\n" + balance_formatted + " NANO##" + std::to_string(index);
}

void Account::UpdateAccountInfo() {
    Logger::info("Loading account data: " + address);

    auto accountInfo = NodeRPC::GetAccountInfo(address);
    isAccountOpen = accountInfo.contains("frontier");

    // Check if account is open
    if (isAccountOpen) {
        SetBalance(decode_raw_str(accountInfo["balance"]));
        SetPending(decode_raw_str(accountInfo["pending"]));

        block_count = std::stoi((std::string) accountInfo["block_count"]); // to be fixed

        representative = accountInfo["representative"];

        auto accountHistoryJSON = NodeRPC::GetAccountHistory(address)["history"];

        account_history.clear();
        for (int i = 0; i < accountHistoryJSON.size(); i++) {
            account_history.push_back(Transaction(accountHistoryJSON.at(i)));
        }
    }

    UpdateUIName();

    Logger::info("Successfully loaded account data: " + address);
    isAccountLoaded = true;
}

void Account::SetPending (const nano::amount &newPending) {
    unclaimed = newPending;
    unclaimed_formatted = unclaimed.format_balance(raw_ratio, 6, true);
}

void Account::SetBalance(const nano::amount &newBalance) {
    balance = newBalance;
    balance_formatted = balance.format_balance(raw_ratio, 6, true);

    UpdateUIName();
}

void Account::hide() {
    hidden = true;
}

void Account::show() {
    hidden = false;
}