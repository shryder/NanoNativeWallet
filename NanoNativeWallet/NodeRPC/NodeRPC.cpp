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

#define RPC_URL "https://node.shrynode.me/api"

json GET(std::string url) {
    std::ostringstream response;

    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::options::WriteStream(&response));
        request.setOpt(new curlpp::options::Url(url.c_str()));

        request.perform();
    }
    catch (curlpp::LogicError& e) {
        std::cout << e.what() << std::endl;
    }
    catch (curlpp::RuntimeError& e) {
        std::cout << e.what() << std::endl;
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

        request.setOpt(curlpp::options::HttpHeader(header));
        request.setOpt(new curlpp::options::WriteStream(&response));
        request.setOpt(new curlpp::options::Url(RPC_URL));
        request.setOpt(new curlpp::options::PostFields(jsonBody));
        request.setOpt(new curlpp::options::PostFieldSize(jsonBody.size()));

        request.perform();
    } catch (curlpp::LogicError& e) {
        std::cout << e.what() << std::endl;
    } catch (curlpp::RuntimeError& e) {
        std::cout << e.what() << std::endl;
    }

    return json::parse(std::string(response.str()));
}

json NodeRPC::GetAccountInfo(std::string account) {
    return POST({
        { "action", "account_info" },
        { "account", account },
        { "representative", true },
        { "weight", true },
        { "pending", true }
    });
}

json NodeRPC::GetAccountHistory(std::string account) {
    return POST({
        { "action", "account_history" },
        { "account", account },
        { "count", 20 }
    });
}