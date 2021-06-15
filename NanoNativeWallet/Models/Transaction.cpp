#include "Transaction.h"
#include <nlohmann/json.hpp>
#include "../Crypto/crypto_utils.h"

using nlohmann::json;

Transaction::Transaction(json transaction) {
	type = transaction["subtype"];
	hash = transaction["hash"];
	representative = transaction["representative"];
	balance = decode_raw_str(transaction["balance"]).pretty_format();

	local_timestamp = transaction["local_timestamp"];

	// Change blocks don't have the fields "account" and "amount".
	if (type != "change") {
		account = transaction["account"];
		amount = decode_raw_str(transaction["amount"]).pretty_format();
	}
}
