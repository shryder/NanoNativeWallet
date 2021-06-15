#pragma once

#include "../NanoNativeWallet.h"
#include <nlohmann/json.hpp>

namespace NodeRPC {
	using nlohmann::json;

	json GetAccountInfo(const std::string &account);
	json GetAccountHistory(const std::string &account);
	json GetUnclaimedTransactions(const std::string& account);
}
