#include "../include/settings.h"

settings::settings() {
  if (!std::filesystem::exists(JSON_STORAGE_PATH)) {
    createEmptyJsonFile(JSON_STORAGE_PATH);
  }

  std::ifstream storageFile(JSON_STORAGE_PATH);
  storageFile >> jsonFile;

  storageFile.close();

  secret = crypt::getSecretKey(SECRET_KEY_FILE_PATH);

  if (secret.empty()) {
    secret = crypt::generateSecretKey();
    saveSecretKey(SECRET_KEY_FILE_PATH);
  }
};

void settings::saveSettings() {
  std::ofstream oStream(JSON_STORAGE_PATH);
  oStream << jsonFile.dump(4);
  oStream.close();
}

void settings::saveSecretKey(std::string path) {
  std::ofstream oStream(path);
  oStream << secret;
  oStream.close();
}

void settings::createEmptyJsonFile(std::string path) {
  std::ofstream storageFile(path);
  json data = json::object();

  storageFile << data;

  storageFile.close();
}

void settings::printMenu() {
  printw("Settings:\n");
  printw("1. add otp\n");
  printw("2. delete otp\n");
  printw("3. change otp\n");
  printw("4. show otp list\n");
  printw("5. exit\n");
}

void settings::addNewOTPItem(otp *otpItem) {
  if (std::filesystem::is_regular_file(JSON_STORAGE_PATH)) {
    json otpFile;
    std::ifstream iStream(JSON_STORAGE_PATH);
    iStream >> otpFile;
    iStream.close();

    otpFile[otpItem->name] = otpItem->secret;

    std::ofstream oStream(JSON_STORAGE_PATH);
    oStream << otpFile.dump(4);
    oStream.close();
  }
}

void settings::changeOTPItem(std::string currentName, std::string newName) {
  nlohmann::json otpFile;
  std::ifstream iStream(JSON_STORAGE_PATH);
  iStream >> otpFile;
  iStream.close();

  auto it = otpFile.find(currentName);
  if (it != otpFile.end()) {
    std::string itemSecret = it.value();

    otpFile.erase(it);

    otpFile[newName] = itemSecret;

    std::ofstream oStream(JSON_STORAGE_PATH);
    oStream << otpFile.dump(4);
    oStream.close();
  } else {
    std::cout << "OTP item with name '" << currentName << "' not found.\n";
  }
}

void settings::deleteOTPItem(std::string name) {
  json otpFile;
  std::ifstream iStream(JSON_STORAGE_PATH);
  iStream >> otpFile;
  iStream.close();

  if (otpFile.find(name) != otpFile.end()) {
    otpFile.erase(name);

    std::ofstream oStream(JSON_STORAGE_PATH);
    oStream << otpFile.dump(4);
    oStream.close();
  } else {
    std::cout << "OTP item '" << name << "' not found.\n";
  }
}