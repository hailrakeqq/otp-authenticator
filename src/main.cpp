#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <string>
#include <thread>

// fix: cin block thread
// TODO: add frame update , and make progress bar
// TODO: implement crud function for topt

using json = nlohmann::json;

#define SECRET_KEY_FILE_PATH ".secret_key"
#define JSON_STORAGE "storage.json"

typedef struct {
  std::string name;
  std::string secret;
  std::string totp;
} otp;

std::vector<otp> optVector;

#pragma region crypt

std::string generateSecretKey() {
  std::string key;
  const char charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  for (int8_t i = 0; i < 16; i++)
    key += charset[rand() % (sizeof(charset) - 1)];

  return key;
}

std::string getSecretKey(std::string pathToSecretKeyFile) {
  std::ifstream keyFile(pathToSecretKeyFile);
  if (keyFile.is_open()) {
    std::string key;
    keyFile >> key;
    keyFile.close();

    return key;
  }
  return "";
}

std::string encodeString(const std::string &data, const std::string &key) {
  std::string encodedData = data;

  for (size_t i = 0; i < data.size(); ++i) {
    encodedData[i] = data[i] ^ key[i % key.size()];
  }

  return encodedData;
}

std::string decodeString(const std::string &encodedData,
                         const std::string &key) {
  return encodeString(encodedData, key);
}

#pragma endregion

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

void addNewOTPItem(otp *otpItem) {
  if (std::filesystem::is_regular_file(JSON_STORAGE)) {
    json otpFile;
    std::ifstream iStream(JSON_STORAGE);
    iStream >> otpFile;
    iStream.close();

    otpFile[otpItem->name] = otpItem->secret;

    std::ofstream oStream(JSON_STORAGE);
    oStream << otpFile.dump(4);
    oStream.close();
  }
}

void changeOTPItem(std::string currentName, std::string newName) {
  nlohmann::json otpFile;
  std::ifstream iStream(JSON_STORAGE);
  iStream >> otpFile;
  iStream.close();

  auto it = otpFile.find(currentName);
  if (it != otpFile.end()) {
    std::string itemSecret = it.value();

    otpFile.erase(it);

    otpFile[newName] = itemSecret;

    std::ofstream oStream(JSON_STORAGE);
    oStream << otpFile.dump(4);
    oStream.close();
  } else {
    std::cout << "OTP item with name '" << currentName << "' not found.\n";
  }
}

void deleteOTPItem(std::string name) {
  json otpFile;
  std::ifstream iStream(JSON_STORAGE);
  iStream >> otpFile;
  iStream.close();

  if (otpFile.find(name) != otpFile.end()) {
    otpFile.erase(name);

    std::ofstream oStream(JSON_STORAGE);
    oStream << otpFile.dump(4);
    oStream.close();
  } else {
    std::cout << "OTP item '" << name << "' not found.\n";
  }
}

std::vector<otp> getOtpVector(json *jsonData) {
  std::vector<otp> otpArray;

  for (auto it = jsonData->begin(); it != jsonData->end(); ++it) {
    otp entry;
    entry.name = it.key();
    entry.secret = it.value().get<std::string>();
    otpArray.push_back(entry);
  }

  return otpArray;
}

void printMainScreen() {
  const int fieldWidth = 31;

  for (auto &item : optVector) {
    std::cout << "+-------------------------------+" << std::endl;
    std::cout << '|' << std::setfill(' ') << std::setw(fieldWidth) << std::left
              << ' ' + item.name + ": " + item.totp << '|' << std::endl;
    std::cout << "+-------------------------------+" << std::endl;
  }
}

int main() {

  std::string key = getSecretKey(SECRET_KEY_FILE_PATH);

  if (key.empty()) {
    std::string newKey = generateSecretKey();
    std::ofstream newKeyFile(SECRET_KEY_FILE_PATH);
    newKeyFile << newKey;
    newKeyFile.close();

    key = newKey;
  }

  std::ifstream storageFile(JSON_STORAGE);
  if (storageFile.is_open()) {
    json data;

    storageFile >> data;
    storageFile.close();

    optVector = getOtpVector(&data);
  } else {
    std::ofstream storageFile(JSON_STORAGE);
    json data = json::object();

    storageFile << data;

    storageFile.close();
  }

  std::thread timerThread([]() {
    while (true) {
      if (!optVector.empty()) {
        for (auto &item : optVector) {
          item.totp = generateTOTP(item.secret);
        }
      }

      std::this_thread::sleep_for(std::chrono::seconds(30));
    }
  });

  char input;
  while (true) {
    printMainScreen();

    std::cout << "Press 'm' to open the menu: ";
    std::cin >> input;

    if (input == 'm') {
      std::cout << "Menu is not implemented yet." << std::endl;
    }
  }

  timerThread.join();

  return 0;
}