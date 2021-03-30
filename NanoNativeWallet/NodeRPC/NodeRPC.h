#pragma once

#include "../NanoNativeWallet.h"
#include <nlohmann/json.hpp>

namespace NodeRPC {
	using nlohmann::json;

	json GetAccountInfo(std::string account);
	json GetAccountHistory(std::string account);
}
