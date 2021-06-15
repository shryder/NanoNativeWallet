#pragma once

#include <nlohmann/json.hpp>
#include "../Crypto/numbers.h"

struct UnclaimedTransaction {
	std::string hash;
	nano::amount amount;
	std::string source;
};