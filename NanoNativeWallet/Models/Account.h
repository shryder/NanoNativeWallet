#pragma once

#include <nlohmann/json.hpp>
#include "../Crypto/crypto_utils.h"

class Account {
public:
	std::string address;
	std::string representative = "";

	uint256_t balance = 0;
	uint256_t pending = 0;
	uint8_t block_count = 0;

	size_t index;

	nlohmann::json account_history;

	bool hidden = false; // Maybe this shouldn't be here but w/e for now
	bool isAccountOpen = false;

	Account(size_t index, std::string accountAddress);

	double getNANOBalance();
	void SetBalance(uint256_t newBalance);
	void UpdateAccountInfo();

	void hide();
	void show();
};