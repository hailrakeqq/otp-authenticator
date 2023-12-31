#include "../include/crypt.h"
#include "../include/settings.h"
#include <chrono>
#include <ncurses.h>
#include <thread>

#define SET_CURSOR_POSITION(x, y)                                              \
  std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H"

// TODO: add frame update , and make progress bar

std::vector<otp> otpVector;
int progress;
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

void printProgressBar(int progress, int width) {
  printw("[");
  int pos = width * progress / 100;
  for (int i = 0; i < width; ++i) {
    if (i < pos)
      printw("=");
    else if (i == pos)
      printw(">");
    else
      printw(" ");
  }
  printw("]");
  printw("\n");
}

void printOtpList() {
  for (int i = 0; i < otpVector.size(); i++) {
    printw("%d. %s: %s\n", i, otpVector[i].name, otpVector[i].secret);
  }
}

void printMainScreen() {
  const int fieldWidth = 30;
  const int progressBarWidth = fieldWidth;

  printProgressBar(progress, progressBarWidth);

  for (auto &item : otpVector) {
    printw("+-------------------------------+\n");
    printw("| %s: %-*s|", item.name.c_str(),
           fieldWidth - (int)item.name.length() - 2, item.totp.c_str());
    printw("\n+-------------------------------+\n");
  }
}

int main() {
  initscr();
  timeout(0);
  noecho();
  curs_set(FALSE);

  auto s = new settings();
  otpVector = getOtpVector(&s->jsonFile);

  std::thread timerThread([]() { // update topt every 30 seconds
    while (true) {
      if (!otpVector.empty()) {
        for (auto &item : otpVector) {
          item.totp = crypt::generateTOTP(item.secret);
        }
      }

      for (int i = 0; i < 30; i++) {
        progress = static_cast<int>((static_cast<double>(i) / 30.0) * 100.0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      progress = 0;
    }
  });

  int ch;

  while (true) {
    clear();
    SET_CURSOR_POSITION(0, 0);

    if (isMenuOpen)
      s->printMenu();
    else
      printMainScreen();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    refresh();

    ch = getch();

    if (ch == 'm' || ch == 'M') {
      isMenuOpen = !isMenuOpen;
    } else if (ch == '1' && isMenuOpen) {
    addItem:
      printw("Enter path to qr-code image: ");
      std::string input;
      std::getline(std::cin, input);

      if (!input.empty() && std::filesystem::is_regular_file("input")) {
        auto secret = crypt::decodeQRcode(input);

        printw("Enter name for this otp: ");
        std::string name;
        std::getline(std::cin, name);

        otp newOtpItem;
        newOtpItem.name = name;
        newOtpItem.secret = secret;

        otpVector.push_back(newOtpItem);
        s->addNewOTPItem(&newOtpItem);
      } else {
        printw("You enter path to qr-code image or image does not exist. Try "
               "again...\n");
        input.clear();
        goto addItem;
      }
    } else if (ch == '2' && isMenuOpen) {
    deleteItem:
      printOtpList();
      printw("Enter index otp item to delete: ");

      std::string input;
      std::getline(std::cin, input);
      int index = std::stoi(input);

      if (index <= otpVector.size() && index >= 0) {
        s->deleteOTPItem(otpVector[index].name);
      } else {
        printw("You enter wrong item. Try again...\n");
        input.clear();
        goto deleteItem;
      }
    } else if (ch == '3' && isMenuOpen) {
    changeItemName:
      printOtpList();
      printw("Enter item index to change name");
      std::string input;
      std::getline(std::cin, input);
      int index = std::stoi(input);

      if (index <= otpVector.size() && index >= 0) {
        printw("Enter new item name: ");
        std::string newItemName;
        std::getline(std::cin, newItemName);
        s->changeOTPItem(otpVector[index].name, newItemName);
      } else {
        printw("You enter wrong item. Try again...\n");
        input.clear();
        goto changeItemName;
      }
    } else if (ch == '4' && isMenuOpen) {
      printOtpList();
    } else if (ch == '5' && isMenuOpen) {
      isMenuOpen = false;
    } else if (ch == 27) {
      break;
    }
  }

  timerThread.join();
  endwin();
  return 0;
}