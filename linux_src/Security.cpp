#include <iostream>
#include "Security.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

std::string sha256_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Unable to open file.");

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
        throw std::runtime_error("Failed to create EVP context.");

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("DigestInit failed.");

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)))
        EVP_DigestUpdate(ctx, buffer, file.gcount());
    EVP_DigestUpdate(ctx, buffer, file.gcount()); // final chunk

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(ctx, hash, &lengthOfHash) != 1)
        throw std::runtime_error("DigestFinal failed.");

    EVP_MD_CTX_free(ctx);

    std::ostringstream result;
    for (unsigned int i = 0; i < lengthOfHash; ++i)
        result << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return result.str();
}