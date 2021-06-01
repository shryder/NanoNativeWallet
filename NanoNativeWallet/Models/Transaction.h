#pragma once

#include <nlohmann/json.hpp>
#include "../Crypto/numbers.h"

class Transaction {
public:
	std::string type;
	
	std::string hash;

	std::string account;

	std::string amount;
	std::string balance;
	
	std::string representative;
	std::string local_timestamp;

	Transaction(nlohmann::json transaction);
};