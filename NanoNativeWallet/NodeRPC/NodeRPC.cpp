#include "../NanoNativeWallet.h"

#include <sstream>
#include <cstdlib>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "NodeRPC.h"
#include <nlohmann/json.hpp>

using nlohmann::json;

json GET(const std::string &url) {
    std::ostringstream response;

    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::options::WriteStream(&response));
        request.setOpt(new curlpp::options::Url(url.c_str()));

        request.perform();
    } catch (curlpp::LogicError& e) {
        std::cout << "[ERROR] CURLPP::LogicError" << e.what() << std::endl;
    } catch (curlpp::RuntimeError& e) {
        std::cout << "[ERROR] CURLPP::RuntimeError" << e.what() << std::endl;
    }

    return json::parse(std::string(response.str()));
}

json POST(json body) {
    std::ostringstream response;
    std::string jsonBody = body.dump();

    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        std::list<std::string> header;
        header.push_back("Content-Type: application/json");

        request.setOpt(new curlpp::options::HttpHeader(header));
        request.setOpt(new curlpp::options::WriteStream(&response));
        request.setOpt(new curlpp::options::Url(gNodeRPC));
        request.setOpt(new curlpp::options::PostFields(jsonBody));
        request.setOpt(new curlpp::options::PostFieldSize(jsonBody.size()));

        request.perform();
    } catch (curlpp::LogicError& e) {
        std::cout << "[ERROR] CURLPP::LogicError" << e.what() << std::endl;
    } catch (curlpp::RuntimeError& e) {
        std::cout << "[ERROR] CURLPP::RuntimeError" << e.what() << std::endl;
    }

    std::string response_str = response.str();
    return json::parse(response_str);
}

json NodeRPC::GetAccountInfo(const std::string &account) {
    return POST({
        { "action", "account_info" },
        { "account", account },

        { "representative", true },
        { "weight", true },
        { "pending", true }
    });
}

json NodeRPC::GetAccountHistory(const std::string &account) {
    return POST({
        { "action", "account_history" },
        { "account", account },

        { "raw", true },

        { "count", 20 }
    });
}

json NodeRPC::GetUnclaimedTransactions (const std::string& account) {
    return POST ({
        { "action", "pending" },
        { "account", account },

        { "source", true },
        { "include_only_confirmed", true },

        { "count", 20 }
    });
}