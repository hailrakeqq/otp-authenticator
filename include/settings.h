#include "crypt.h"
#include "otp.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <nlohmann/json.hpp>

#define SECRET_KEY_FILE_PATH ".secret_key"
#define JSON_STORAGE_PATH "storage.json"

using json = nlohmann::json;

class settings {
public:
  settings();

  std::string secret;
  json jsonFile;

  void printMenu();
  void saveSettings();
  void createEmptyJsonFile(std::string path);
  void saveSecretKey(std::string path);

  void addNewOTPItem(otp *optItem);
  void changeOTPItem(std::string currentName, std::string newName);
  void deleteOTPItem(std::string name);
};