#pragma once

#include <iostream>
#include <cryptopp/modes.h>

using byte = CryptoPP::byte;

#define MAX_WALLET_PASSWORD_LENGTH 1024
#define MAX_WALLET_NAME_LENGTH 256

#define IV_SIZE 16
#define SEED_SIZE 64