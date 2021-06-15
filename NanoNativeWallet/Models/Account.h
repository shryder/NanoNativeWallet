#pragma once

#include <nlohmann/json.hpp>
#include "../Crypto/crypto_utils.h"
#include "../Crypto/numbers.h"
#include "Transaction.h"
#include "UnclaimedTransaction.h"

class Account {
public:
	std::string address;
	std::string representative;

	// Just so we don't do it every frame
	std::string ui_name;
	std::string balance_formatted = "0";
	std::string unclaimed_formatted = "0";

	nano::amount balance = 0;
	nano::amount unclaimed = 0;

	uint8_t block_count = 0;

	size_t index;

	std::vector<Transaction> account_history;
	std::vector<UnclaimedTransaction> unclaimed_transactions;

	bool hidden = false; // Maybe this shouldn't be here but w/e for now
	bool isAccountOpen = false;
	bool isAccountLoaded = false;

	Account(size_t index, const std::string &accountAddress);

	void SetBalance(const nano::amount &newBalance);
	void SetPending(const nano::amount &newPendingAmount);

	void UpdateAccountInfo();
	void UpdateUIName();

	void hide();
	void show();
};