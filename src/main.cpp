#include "../include/crypt.h"
#include "../include/settings.h"
#include <chrono>
// #include <future>
// #include <iomanip>
// #include <iostream>
#include <ncurses.h>

#include <thread>

// TODO: add frame update , and make progress bar

std::vector<otp> optVector;

bool isMenuOpen = false;

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
  initscr();
  keypad(stdscr, TRUE);

  auto s = new settings();
  optVector = getOtpVector(&s->jsonFile);

  std::thread timerThread([]() { // update topt every 30 seconds
    while (true) {
      if (!optVector.empty()) {
        for (auto &item : optVector) {
          item.totp = crypt::generateTOTP(item.secret);
        }
      }

      std::this_thread::sleep_for(std::chrono::seconds(30));
    }
  });

  int ch;

  while (true) {
    clear();

    if (isMenuOpen) {
      s->printMenu();
    } else {
      printMainScreen();
    }

    refresh();

    ch = getch();

    if (ch == 'm' || ch == 'M') {
      isMenuOpen = !isMenuOpen;
    } else if (ch == 27) {
      break;
    }
  }

  timerThread.join();
  endwin();
  return 0;
}