#pragma once

#include <nlohmann/json.hpp>
#include "../Crypto/crypto_utils.h"
#include "../Crypto/numbers.h"

class Account {
public:
	std::string address;
	std::string representative = "";

	nano::amount balance = 0;
	nano::amount pending = 0;

	uint8_t block_count = 0;

	size_t index;

	nlohmann::json account_history;


	bool hidden = false; // Maybe this shouldn't be here but w/e for now
	bool isAccountOpen = false;

	Account(size_t index, std::string accountAddress);

	std::string getNANOBalance();
	void SetBalance(nano::amount newBalance);
	void UpdateAccountInfo();

	void hide();
	void show();
};