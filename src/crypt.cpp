#include "../include/crypt.h"

std::string crypt::generateSecretKey() {
  std::string key;
  const char charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  for (int8_t i = 0; i < 16; i++)
    key += charset[rand() % (sizeof(charset) - 1)];

  return key;
}

std::string crypt::getSecretKey(std::string pathToSecretKeyFile) {
  std::ifstream keyFile(pathToSecretKeyFile);
  if (keyFile.is_open()) {
    std::string key;
    keyFile >> key;
    keyFile.close();

    return key;
  }
  return "";
}

std::string crypt::encodeString(const std::string &data,
                                const std::string &key) {
  std::string encodedData = data;

  for (size_t i = 0; i < data.size(); ++i) {
    encodedData[i] = data[i] ^ key[i % key.size()];
  }

  return encodedData;
}

std::string crypt::decodeString(const std::string &encodedData,
                                const std::string &key) {
  return encodeString(encodedData, key);
}

std::string crypt::decodeQRcode(std::string path) {
  try {
    cv::QRCodeDetector qrDet;
    cv::Mat points, rectImage;

    cv::Mat qr = cv::imread(path);

    if (qr.empty()) {
      throw std::runtime_error("Error: Could not read the image.");
    }

    std::string data = qrDet.detectAndDecode(qr, points, rectImage);

    if (data.length() > 0) {
      return data;
    } else {
      throw std::runtime_error("Error: No QR code found in the image.");
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
    return "";
  }
}

std::string crypt::generateTOTP(const std::string &key, uint64_t timeStep,
                                size_t codeLength) {
  // Get the current time in seconds
  auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

  // Calculate the counter value
  uint64_t counter = currentTime / timeStep;

  // Convert the counter to a byte array
  unsigned char counterBytes[8];
  for (int i = 7; i >= 0; --i) {
    counterBytes[i] = static_cast<unsigned char>(counter & 0xFF);
    counter >>= 8;
  }

  // Calculate the HMAC-SHA1 hash
  unsigned char hash[SHA_DIGEST_LENGTH];
  HMAC(EVP_sha1(), key.c_str(), key.length(), counterBytes,
       sizeof(counterBytes), hash, nullptr);

  // Get the offset from the last nibble of the hash
  size_t offset = hash[SHA_DIGEST_LENGTH - 1] & 0x0F;

  // Calculate the 4-byte dynamic binary code (DT)
  uint32_t dynamicCode = ((static_cast<uint32_t>(hash[offset] & 0x7F) << 24) |
                          (static_cast<uint32_t>(hash[offset + 1]) << 16) |
                          (static_cast<uint32_t>(hash[offset + 2]) << 8) |
                          static_cast<uint32_t>(hash[offset + 3])) %
                         static_cast<uint32_t>(std::pow(10, codeLength));

  // Convert the dynamic code to a string
  std::ostringstream codeStream;
  codeStream << std::setw(codeLength) << std::setfill('0') << dynamicCode;

  return codeStream.str();
}