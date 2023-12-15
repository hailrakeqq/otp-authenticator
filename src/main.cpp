#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <openssl/hmac.h>
#include <openssl/sha.h>

std::string decodeQRcode(std::string path) {
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

std::string generateTOTP(const std::string &key, uint64_t timeStep = 30,
                         size_t codeLength = 6) {
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

int main(int argc, char *argv[]) {
  std::cout << decodeQRcode("../qr-code.png") << std::endl;
  return 0;
}