// Project: ATM Simulation
// Utilization: Linked List and OOP
// Created by: Kenzo Buan and Derette Puerto

// Libraries used
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <windows.h>

using namespace std;

// File and USB configuration
const string ACCOUNT_FILE = "Account.csv";
const string CARD_FILE_NAME = "ATM_CARD.txt";
const char FIRST_USB_DRIVE = 'D';
const char LAST_USB_DRIVE = 'Z';
const string ENCRYPTION_KEY = "TUP_ATM_2026";

// Account information section
struct Account {
    string num, name, bday, contact, PIN;
    long long bal;
};

// Linked list node section
struct Node {
    Account acc;
    Node *next;

    Node(Account a) {
        acc = a;
        next = NULL;
    }
};

// ATM class and function declarations
class ATM {
private:
    Node *head, *account;
    Node* LocAcc(string num);

    // Validation
    bool IsAccName(string name);
    bool IsAccNum(string num);
    bool IsAccBday(string bday);
    bool IsAccContact(string contact);
    bool IsAccPIN(string PIN);
    bool IsAccDeposit(long long amount);
    bool IsValidAmount(string str);

    // USB card helpers
    string FindUSBCardPath();
    bool ReadCardData(string path, string &cardNum, string &cardPIN);
    bool WriteCardData(string path, string cardNum, string encryptedPIN);

    // Login helpers
    bool VerifyPIN(string enteredPIN);

public:
    // Constructor
    ATM() : head(NULL), account(NULL) {}

    // Destructor
    ~ATM() {
        Node *p;

        while (head != NULL) {
            p = head;
            head = head->next;
            delete p;
        }
    }

    // Validation
    bool IsDigit(string str);

    // Registration
    void RegAcc();

    // Account management
    void InsertAcc(Account x);
    void DispAcc();

    // Login/authentication
    bool LogIn();
    void LogOut();

    // Transactions
    void BalInq();
    void WithdrawBal();
    void DepositAmount();
    void FundTrans();
    void ChangePIN();

    // File handling
    void SaveAcc();
    void RetAcc();
    void SaveCard();

    // Security
    string Encrypt(string text);
    string Decrypt(string text);

    // ATM menu
    void MainMenu();
};

// Security section - PIN encryption
string ATM::Encrypt(string text) {
    string result;

    for (int i = 0; i < (int)text.length(); i++) {
        unsigned char value =
            (unsigned char)text[i] ^
            (unsigned char)ENCRYPTION_KEY[i % ENCRYPTION_KEY.length()];

        const char hex[] = "0123456789ABCDEF";

        result += hex[(value >> 4) & 0x0F];
        result += hex[value & 0x0F];
    }

    return result;
}

// Security section - PIN decryption
string ATM::Decrypt(string text) {
    string result;

    if (text.length() % 2 != 0)
        return "";

    for (int i = 0; i < (int)text.length(); i += 2) {
        char first = text[i];
        char second = text[i + 1];

        int high, low;

        if (first >= '0' && first <= '9')
            high = first - '0';
        else if (first >= 'A' && first <= 'F')
            high = first - 'A' + 10;
        else
            return "";

        if (second >= '0' && second <= '9')
            low = second - '0';
        else if (second >= 'A' && second <= 'F')
            low = second - 'A' + 10;
        else
            return "";

        unsigned char value = (high << 4) | low;

        value ^= (unsigned char)
            ENCRYPTION_KEY[(i / 2) % ENCRYPTION_KEY.length()];

        result += (char)value;
    }

    return result;
}

// Account checking section - locate an account in the linked list
Node* ATM::LocAcc(string num) {
    Node *p = head;

    while (p != NULL) {
        if (p->acc.num == num)
            return p;

        p = p->next;
    }

    return NULL;
}

// Validation section - check if the input is a valid amount
bool ATM::IsValidAmount(string str) {
    if (!IsDigit(str))
        return false;

    try {
        stoll(str);
    }
    catch (...) {
        return false;
    }

    return true;
}

// Validation section - check if account name contains letters and spaces only
bool ATM::IsAccName(string name) {
    if (name.empty())
        return false;

    for (int i = 0; i < name.length(); i++) {
        if (!((name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= 'a' && name[i] <= 'z') ||
              name[i] == ' ')) {
            return false;
        }
    }

    return true;
}

// Validation section - check if input contains digits only
bool ATM::IsDigit(string str) {
    if (str.empty())
        return false;

    for (int i = 0; i < str.length(); i++) {
        if (str[i] >= '0' && str[i] <= '9')
            continue;
        else
            return false;
    }

    return true;
}

// Validation section - check account number
bool ATM::IsAccNum(string num) {
    return num.length() == 5 &&
           IsDigit(num) &&
           LocAcc(num) == NULL;
}

// Validation section - check birthday
bool ATM::IsAccBday(string bday) {
    if (bday.length() != 10 ||
        bday[2] != '/' ||
        bday[5] != '/')
        return false;

    for (int i = 0; i < bday.length(); i++) {
        if (i == 2 || i == 5)
            continue;

        if (bday[i] < '0' || bday[i] > '9')
            return false;
    }

    int month = stoi(bday.substr(0, 2));
    int day = stoi(bday.substr(3, 2));
    int year = stoi(bday.substr(6, 4));

    if (month < 1 || month > 12)
        return false;

    if (year < 1926 || year > 2026)
        return false;

    int days;

    if (month == 2) {
        if ((year % 400 == 0) ||
            (year % 4 == 0 && year % 100 != 0))
            days = 29;
        else
            days = 28;
    }
    else if (month == 4 ||
             month == 6 ||
             month == 9 ||
             month == 11)
        days = 30;
    else
        days = 31;

    if (day < 1 || day > days)
        return false;

    return true;
}

// Validation section - check contact number
bool ATM::IsAccContact(string contact) {
    return contact.length() == 11 &&
           contact[0] == '0' &&
           contact[1] == '9' &&
           IsDigit(contact);
}

// Validation section - check PIN
bool ATM::IsAccPIN(string PIN) {
    return PIN.length() >= 4 &&
           PIN.length() <= 6 &&
           IsDigit(PIN);
}

// Validation section - check initial deposit
bool ATM::IsAccDeposit(long long amount) {
    return amount >= 5000;
}

// Linked list section - insert account in sorted order
void ATM::InsertAcc(Account x) {
    Node *p, *q, *newNode;

    p = q = head;
    newNode = new Node(x);

    while (p != NULL && x.name > p->acc.name) {
        q = p;
        p = p->next;
    }

    if (head == NULL || x.name < head->acc.name) {
        newNode->next = head;
        head = newNode;
    }
    else {
        q->next = newNode;
        newNode->next = p;
    }
}

// Registration flow - create account and connect it to the ATM card
void ATM::RegAcc() {
    // Registration flow: check existing account -> collect information -> save account -> create/update USB card
    Account x;
    long long deposit;
    string input;

    system("cls");

    cout << "========================================\n";
    cout << "          ATM REGISTRATION\n";
    cout << "========================================\n\n";

    while (true) {
        cout << "Enter the account number (Must be 5 digits): ";
        cin >> x.num;

        if (IsAccNum(x.num))
            break;

        cout << "\nInvalid account number. Try again.\n\n";
        system("pause");
        system("cls");
    }

    while (true) {
        cout << "Enter the account name: ";
        getline(cin >> ws, x.name);

        if (IsAccName(x.name))
            break;

        cout << "\nInvalid account name. Letters and spaces only.\n\n";
        system("pause");
        system("cls");
    }

    while (true) {
        cout << "Enter the account birthday (Format: XX/XX/XXXX): ";
        cin >> x.bday;

        if (IsAccBday(x.bday))
            break;

        cout << "\nInvalid account birthday. Try again.\n\n";
        system("pause");
        system("cls");
    }

    while (true) {
        cout << "Enter the account contact number (Format: 09XXXXXXXXX): ";
        cin >> x.contact;

        if (IsAccContact(x.contact))
            break;

        cout << "\nInvalid account contact number. Try again.\n\n";
        system("pause");
        system("cls");
    }

    while (true) {
        cout << "Enter the account PIN code (4-6 digits): ";
        cin >> x.PIN;

        if (IsAccPIN(x.PIN))
            break;

        cout << "\nInvalid account PIN code. Try again.\n\n";
        system("pause");
        system("cls");
    }

    while (true) {
        cout << "Insert the initial balance (Min. 5000): ";
        cin >> input;

        if (!IsValidAmount(input)) {
            cout << "\nInvalid initial balance.\n";
            cout << "Please enter whole numbers only. No letters or decimals.\n\n";
            system("pause");
            system("cls");
            continue;
        }

        deposit = stoll(input);

        if (IsAccDeposit(deposit))
            break;

        cout << "\nInsufficient initial balance.\n";
        cout << "The minimum initial balance is 5000. Try again.\n\n";
        system("pause");
        system("cls");
    }

    x.bal = deposit;

    InsertAcc(x);

    account = LocAcc(x.num);

    SaveAcc();

    cout << "\nAccount successfully created.\n";

    SaveCard();

    cout << "\nRegistration complete.\n";
    cout << "If no USB was detected, insert a USB and create the card file before login.\n\n";

    account = NULL;

    system("pause");
}

// Transaction section - balance inquiry
void ATM::BalInq() {
    if (account == NULL)
        return;

    cout << "\nCurrent Balance: "
         << account->acc.bal
         << "\n\n";

    system("pause");
}

// Transaction section - withdrawal
void ATM::WithdrawBal() {
    // Withdrawal flow: validate whole number -> check multiples of 100 -> check balance -> deduct amount
    if (account == NULL)
        return;

    string amountInput;
    long long amount;

    cout << "Enter the withdrawal amount "
         << "(whole numbers only: 100, 500, 1000, etc.): ";

    cin >> amountInput;

    if (!IsValidAmount(amountInput)) {
        cout << "\nInvalid withdrawal amount.\n";
        cout << "Please enter whole numbers only. No letters or decimals.\n\n";
        system("pause");
        return;
    }

    amount = stoll(amountInput);

    if (amount <= 0 || amount % 100 != 0) {
        cout << "\nInvalid withdrawal amount.\n";
        cout << "Withdrawal must be a positive amount divisible by 100.\n";
        cout << "Example: 100, 500, 1000, 1700.\n\n";
        system("pause");
    }
    else if (amount > account->acc.bal) {
        cout << "\nInsufficient balance. Try again.\n\n";
        system("pause");
    }
    else {
        account->acc.bal -= amount;

        cout << "\nAmount successfully withdrawn.\n\n";
        cout << "New balance: "
             << account->acc.bal
             << "\n\n";

        SaveAcc();

        system("pause");
    }
}

// Transaction section - deposit
void ATM::DepositAmount() {
    // Deposit flow: validate whole number -> add amount -> save updated balance
    if (account == NULL)
        return;

    string amountInput;
    long long amount;

    cout << "Enter the amount to deposit (whole numbers only): ";
    cin >> amountInput;

    if (!IsValidAmount(amountInput)) {
        cout << "\nInvalid deposit amount.\n";
        cout << "Please enter whole numbers only. No letters or decimals.\n\n";
        system("pause");
        return;
    }

    amount = stoll(amountInput);

    if (amount <= 0) {
        cout << "\nInvalid deposit amount. Try again.\n\n";
        system("pause");
    }
    else {
        account->acc.bal += amount;

        cout << "\nAmount successfully deposited.\n";
        cout << "New balance: "
             << account->acc.bal
             << "\n\n";

        SaveAcc();

        system("pause");
    }
}

// Transaction section - fund transfer
void ATM::FundTrans() {
    if (account == NULL)
        return;

    string amountInput, receiverNum;
    long long amount;

    cout << "Enter the receiver's account number: ";
    cin >> receiverNum;

    Node *receiver = LocAcc(receiverNum);

    if (receiver == NULL) {
        cout << "\nThe account was not found.\n\n";
        system("pause");
    }
    else if (receiver == account) {
        cout << "\nYou cannot transfer to the same account.\n\n";
        system("pause");
    }
    else {
        while (true) {
            cout << "Enter the amount to transfer: ";
            cin >> amountInput;

            if (!IsValidAmount(amountInput)) {
                cout << "\nInvalid transfer amount.\n";
                cout << "Please enter whole numbers only. No letters or decimals.\n\n";
                system("pause");
                continue;
            }

            amount = stoll(amountInput);

            if (amount <= 0) {
                cout << "\nInvalid transfer amount. Try again.\n\n";
                system("pause");
            }
            else if (amount > account->acc.bal) {
                cout << "\nInsufficient balance. Try again.\n\n";
                system("pause");
            }
            else {
                cout << "\nReceiver: "
                     << receiver->acc.name
                     << "\n";

                cout << "Amount: "
                     << amount
                     << "\n";

                char confirm;

                cout << "Confirm transfer? (Y/N): ";
                cin >> confirm;

                if (confirm == 'Y' || confirm == 'y') {
                    account->acc.bal -= amount;
                    receiver->acc.bal += amount;

                    cout << "\nAmount successfully transferred.\n\n";
                    cout << "New balance: "
                         << account->acc.bal
                         << "\n\n";

                    SaveAcc();

                    system("pause");
                    break;
                }
                else {
                    cout << "\nTransfer cancelled.\n\n";
                    system("pause");
                    break;
                }
            }
        }
    }
}

// Account management section - change PIN
void ATM::ChangePIN() {
    if (account == NULL)
        return;

    string currentPIN, newPIN, confirmPIN;

    cout << "Enter your current PIN: ";
    cin >> currentPIN;

    if (currentPIN != account->acc.PIN) {
        cout << "\nIncorrect current PIN.\n\n";
        system("pause");
        return;
    }

    while (true) {
        cout << "Enter the new PIN code (4-6 digits): ";
        cin >> newPIN;

        if (!IsAccPIN(newPIN)) {
            cout << "\nInvalid account PIN code. Try again.\n\n";
            system("pause");
            continue;
        }

        cout << "Confirm the new PIN: ";
        cin >> confirmPIN;

        if (newPIN != confirmPIN) {
            cout << "\nPIN codes do not match. Try again.\n\n";
            system("pause");
            continue;
        }

        if (newPIN == account->acc.PIN) {
            cout << "\nNew PIN must be different from the current PIN.\n\n";
            system("pause");
            continue;
        }

        account->acc.PIN = newPIN;
        break;
    }

    SaveAcc();
    SaveCard();

    cout << "\nPIN code successfully changed.\n";
    cout << "The account file and ATM card file were updated.\n\n";

    system("pause");
}

// File section - save account data
void ATM::SaveAcc() {
    Node *p = head;

    ofstream file(ACCOUNT_FILE);

    if (!file) {
        cout << "\nError opening account database.\n\n";
        system("pause");
        return;
    }

    while (p != NULL) {
        file << p->acc.num
             << ","
             << p->acc.name
             << ","
             << p->acc.bday
             << ","
             << p->acc.contact
             << ","
             << Encrypt(p->acc.PIN)
             << ","
             << p->acc.bal
             << "\n";

        p = p->next;
    }

    file.close();
}

// File section - retrieve account data
void ATM::RetAcc() {
    string num, name, bday, contact, PIN, balance;
    string line;

    Account x;

    ifstream file(ACCOUNT_FILE);

    if (!file)
        return;

    while (getline(file, line)) {
        if (line.empty())
            continue;

        stringstream ss(line);

        getline(ss, num, ',');
        getline(ss, name, ',');
        getline(ss, bday, ',');
        getline(ss, contact, ',');
        getline(ss, PIN, ',');
        getline(ss, balance, ',');

        x.num = num;
        x.name = name;
        x.bday = bday;
        x.contact = contact;
        x.PIN = Decrypt(PIN);
        x.bal = stoll(balance);

        InsertAcc(x);
    }

    file.close();
}

// Account management section - display account information
void ATM::DispAcc() {
    if (account == NULL)
        return;

    cout << "Info\n";
    cout << "Number: "
         << account->acc.num
         << "\n";

    cout << "Name: "
         << account->acc.name
         << "\n";

    cout << "Birthday: "
         << account->acc.bday
         << "\n";

    cout << "Contact Number: "
         << account->acc.contact
         << "\n";

    cout << "\n";

    system("pause");
}

// USB section - find the inserted removable drive
string ATM::FindUSBCardPath() {
    DWORD drives = GetLogicalDrives();

    if (drives == 0)
        return "";

    for (char drive = FIRST_USB_DRIVE;
         drive <= LAST_USB_DRIVE;
         drive++) {

        int index = drive - 'A';

        if (!(drives & (1 << index)))
            continue;

        string root;

        root += drive;
        root += ":\\";

        UINT driveType = GetDriveTypeA(root.c_str());

        if (driveType == DRIVE_REMOVABLE) {
            string path = root + CARD_FILE_NAME;
            return path;
        }
    }

    return "";
}

// USB section - read ATM card information
bool ATM::ReadCardData(
    string path,
    string &cardNum,
    string &cardPIN) {

    ifstream file(path);

    if (!file)
        return false;

    string line;

    while (getline(file, line)) {
        size_t separator = line.find('=');

        if (separator == string::npos)
            continue;

        string key = line.substr(0, separator);
        string value = line.substr(separator + 1);

        if (key == "ACCOUNT_NO")
            cardNum = value;

        else if (key == "ENCRYPTED_PIN")
            cardPIN = value;
    }

    file.close();

    return !cardNum.empty() && !cardPIN.empty();
}

// USB section - write ATM card information
bool ATM::WriteCardData(
    string path,
    string cardNum,
    string encryptedPIN) {

    ofstream file(path);

    if (!file)
        return false;

    file << "ATM_CARD\n";
    file << "ACCOUNT_NO=" << cardNum << "\n";
    file << "ENCRYPTED_PIN=" << encryptedPIN << "\n";

    file.close();

    return true;
}

// USB section - create or update the ATM card file
void ATM::SaveCard() {
    if (account == NULL)
        return;

    string path = FindUSBCardPath();

    if (path.empty()) {
        cout << "\nNo ATM USB card detected.\n";
        return;
    }

    if (WriteCardData(
            path,
            account->acc.num,
            Encrypt(account->acc.PIN))) {

        cout << "\nATM card file successfully updated.\n";
    }
    else {
        cout << "\nUnable to update the ATM card file.\n";
    }
}

// Login security section - verify the entered PIN
bool ATM::VerifyPIN(string enteredPIN) {
    if (account == NULL)
        return false;

    return enteredPIN == account->acc.PIN;
}

// Login flow - detect USB, check account, and verify PIN
bool ATM::LogIn() {
    // Login flow: detect USB -> read card -> find account -> verify card data -> verify PIN
    string path = FindUSBCardPath();

    if (path.empty()) {
        cout << "\n========================================\n";
        cout << "           USB CARD NOT FOUND\n";
        cout << "========================================\n\n";

        cout << "Please insert the ATM USB card.\n\n";

        system("pause");

        return false;
    }

    string cardNum, encryptedCardPIN;

    if (!ReadCardData(path, cardNum, encryptedCardPIN)) {
        cout << "\nInvalid ATM card file.\n\n";

        system("pause");

        return false;
    }

    Node *found = LocAcc(cardNum);

    if (found == NULL) {
        cout << "\nAccount connected to this card was not found.\n\n";

        system("pause");

        return false;
    }

    account = found;

    if (Decrypt(encryptedCardPIN) != account->acc.PIN) {
        cout << "\nATM card PIN data does not match the account database.\n";
        cout << "Please contact the bank.\n\n";

        account = NULL;

        system("pause");

        return false;
    }

    system("cls");

    cout << "========================================\n";
    cout << "              USB DETECTED\n";
    cout << "========================================\n\n";

    cout << "Account Number: "
         << account->acc.num
         << "\n\n";

    string enteredPIN;

    cout << "Enter PIN: ";
    cin >> enteredPIN;

    if (VerifyPIN(enteredPIN)) {
        cout << "\nPIN verified.\n";
        cout << "Access granted.\n\n";

        system("pause");

        return true;
    }

    cout << "\nIncorrect PIN.\n";
    cout << "Access denied.\n\n";

    account = NULL;

    system("pause");

    return false;
}

// Logout flow - end the current account session
void ATM::LogOut() {
    account = NULL;

    cout << "\nYou have been logged out.\n";
    cout << "Please remove your ATM USB card.\n\n";

    system("pause");
}

// Main menu flow - show ATM transactions and account options
void ATM::MainMenu() {
    // Main menu flow: choose transaction -> perform action -> return to menu until logout
    int choice;
    string input;

    while (account != NULL) {
        system("cls");

        cout << "========================================\n";
        cout << "             ATM MAIN MENU\n";
        cout << "========================================\n";

        cout << "Account: "
             << account->acc.num
             << "\n";

        cout << "Name: "
             << account->acc.name
             << "\n\n";

        cout << "1. Balance Inquiry\n";
        cout << "2. Withdraw\n";
        cout << "3. Deposit\n";
        cout << "4. Fund Transfer\n";
        cout << "5. Change PIN\n";
        cout << "6. Account Information\n";
        cout << "7. Eject Card / Logout\n";

        cout << "\nEnter your choice: ";
        cin >> input;

        if (!IsDigit(input)) {
            cout << "\nInvalid choice. Please enter a number.\n\n";
            system("pause");
            continue;
        }

        try {
            choice = stoi(input);
        }
        catch (...) {
            cout << "\nInvalid choice. Please enter a number.\n\n";
            system("pause");
            continue;
        }

        system("cls");

        switch (choice) {
        case 1:
            BalInq();
            break;

        case 2:
            WithdrawBal();
            break;

        case 3:
            DepositAmount();
            break;

        case 4:
            FundTrans();
            break;

        case 5:
            ChangePIN();
            break;

        case 6:
            DispAcc();
            break;

        case 7:
            LogOut();
            break;

        default:
            cout << "\nInvalid choice. Please choose from the menu.\n\n";
            system("pause");
        }
    }
}

// Program flow - start ATM, load accounts, and show registration/login menu
int main() {
    // Program flow: load saved account -> registration or login -> enter main menu -> exit
    ATM atm;

    atm.RetAcc();

    int choice;
    string input;

    while (true) {
        system("cls");

        cout << "========================================\n";
        cout << "             ATM SIMULATION\n";
        cout << "========================================\n\n";

        cout << "1. Register New Account\n";
        cout << "2. Insert ATM Card / Login\n";
        cout << "3. Exit\n";

        cout << "\nEnter your choice: ";
        cin >> input;

        if (!atm.IsDigit(input)) {
            cout << "\nInvalid choice. Please enter a number.\n\n";
            system("pause");
            continue;
        }

        try {
            choice = stoi(input);
        }
        catch (...) {
            cout << "\nInvalid choice. Please enter a number.\n\n";
            system("pause");
            continue;
        }

        switch (choice) {
        case 1:
            atm.RegAcc();
            break;

        case 2:
            if (atm.LogIn())
                atm.MainMenu();

            break;

        case 3:
            atm.SaveAcc();

            cout << "\nThank you for using the ATM Simulation.\n";

            return 0;

        default:
            cout << "\nInvalid choice. Please choose from the menu.\n\n";
            system("pause");
        }
    }

    return 0;
}