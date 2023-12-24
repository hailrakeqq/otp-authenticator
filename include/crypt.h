#ifndef CRYPT_H
#define CRYPT_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <string>
#define SECRET_KEY_FILE_PATH ".secret_key"
#define JSON_STORAGE "storage.json"

namespace crypt {
std::string generateSecretKey();
std::string getSecretKey(std::string pathToSecretKeyFile);
std::string encodeString(const std::string &data, const std::string &key);
std::string decodeString(const std::string &encodedData,
                         const std::string &key);
std::string decodeQRcode(std::string path);
std::string generateTOTP(const std::string &key, uint64_t timeStep = 30,
                         size_t codeLength = 6);
} // namespace crypt

#endif // !CRYPT_H