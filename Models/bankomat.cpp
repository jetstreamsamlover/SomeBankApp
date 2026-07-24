#include "Bankomat.h"
#include "..\Helpers\Printer.h"
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace Tom::Models
{
    Bankomat::Bankomat()
        : account_(0.0), database_("bankomat.db"), current_account_id_(-1)
    {
        database_.initialize();
    }

    void Bankomat::run()
    {
        if (!startSession())
        {
            return;
        }

        bool running = true;
        while (running)
        {
            displayMenu();
            running = selectOption();
        }
    }

    bool Bankomat::startSession()
    {
        while (true)
        {
            Tom::Helpers::displayText("\n--- Account ---");
            Tom::Helpers::displayText("1: Create new account");
            Tom::Helpers::displayText("2: Log in to existing account");
            Tom::Helpers::displayText("0: Exit");
            Tom::Helpers::displayText("Choose an option: ", false);

            int choice = 0;
            std::cin >> choice;

            if (std::cin.fail())
            {
                Tom::Helpers::displayText("Please enter a valid number.");
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (choice == 1)
            {
                std::string username;
                std::string password;
                double initial_balance = 0.0;

                Tom::Helpers::displayText("Username: ", false);
                std::cin >> username;
                Tom::Helpers::displayText("Password: ", false);
                std::cin >> password;
                Tom::Helpers::displayText("Initial balance: ", false);
                std::cin >> initial_balance;

                if (database_.createAccount(username, password, initial_balance))
                {
                    double balance = 0.0;
                    if (database_.loginAccount(username, password, balance, current_account_id_))
                    {
                        account_ = Bankowosc(balance);
                        Tom::Helpers::displayText("Account created and logged in.");
                        return true;
                    }
                }

                Tom::Helpers::displayText("Could not create account. Username may already exist.");
                continue;
            }

            if (choice == 2)
            {
                std::string username;
                std::string password;
                double balance = 0.0;

                Tom::Helpers::displayText("Username: ", false);
                std::cin >> username;
                Tom::Helpers::displayText("Password: ", false);
                std::cin >> password;

                if (database_.loginAccount(username, password, balance, current_account_id_))
                {
                    account_ = Bankowosc(balance);
                    Tom::Helpers::displayText("Logged in successfully.");
                    return true;
                }

                Tom::Helpers::displayText("Invalid username or password.");
                continue;
            }

            if (choice == 0)
            {
                Tom::Helpers::displayText("Goodbye!");
                return false;
            }

            Tom::Helpers::displayText("Invalid option.");
        }
    }

    void Bankomat::saveCurrentBalance()
    {
        if (current_account_id_ != -1)
        {
            database_.updateBalance(current_account_id_, account_.getBalance());
        }
    }

    void Bankomat::displayMenu() const
    {
        Tom::Helpers::displayText("\n--- Bankomat ---");
        Tom::Helpers::displayText("1: Deposit money");
        Tom::Helpers::displayText("2: Withdraw money");
        Tom::Helpers::displayText("3: Show account balance");
        Tom::Helpers::displayText("4: Show branches");
        Tom::Helpers::displayText("5: Show users");
        Tom::Helpers::displayText("6: Search by name");
        Tom::Helpers::displayText("7: Search by PersonID");
        Tom::Helpers::displayText("8: Search accounts");
        Tom::Helpers::displayText("0: Exit");
        Tom::Helpers::displayText("Choose an option: ", false);
    }

    bool Bankomat::selectOption()
    {
        int choice = 0;
        std::cin >> choice;

        if (std::cin.fail())
        {
            Tom::Helpers::displayText("Please enter a valid number.");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return true;
        }

        switch (choice)
        {
            case 1:
                deposit();
                return true;

            case 2:
                withdraw();
                return true;

            case 3:
            {
                std::ostringstream balance_stream;
                balance_stream << std::fixed << std::setprecision(2) << account_.getBalance();
                Tom::Helpers::displayText("Current balance: " + balance_stream.str());
                return true;
            }

            case 4:
                showBranches();
                return true;

            case 5:
                showUsers();
                return true;

            case 6:
                searchByName();
                return true;

            case 7:
                searchByPersonId();
                return true;

            case 8:
                searchAccounts();
                return true;

            case 0:
                Tom::Helpers::displayText("Goodbye!");
                return false;

            default:
                Tom::Helpers::displayText("Invalid option.");
                return true;
        }
    }

    void Bankomat::deposit()
    {
        Tom::Helpers::displayText("Enter amount to deposit: ", false);
        double amount = 0.0;
        std::cin >> amount;

        if (std::cin.fail() || amount <= 0.0)
        {
            Tom::Helpers::displayText("Please enter a positive number.");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return;
        }

        account_.deposit(amount);
        saveCurrentBalance();

        std::ostringstream amount_stream;
        amount_stream << std::fixed << std::setprecision(2) << amount;
        std::ostringstream balance_stream;
        balance_stream << std::fixed << std::setprecision(2) << account_.getBalance();

        Tom::Helpers::displayText("Deposited: " + amount_stream.str());
        Tom::Helpers::displayText("New balance: " + balance_stream.str());
    }

    void Bankomat::withdraw()
    {
        Tom::Helpers::displayText("Enter amount to withdraw: ", false);
        double amount = 0.0;
        std::cin >> amount;

        if (std::cin.fail() || amount <= 0.0)
        {
            Tom::Helpers::displayText("Please enter a positive number.");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return;
        }

        if (account_.withdraw(amount))
        {
            saveCurrentBalance();

            std::ostringstream amount_stream;
            amount_stream << std::fixed << std::setprecision(2) << amount;
            std::ostringstream balance_stream;
            balance_stream << std::fixed << std::setprecision(2) << account_.getBalance();

            Tom::Helpers::displayText("Withdrew: " + amount_stream.str());
            Tom::Helpers::displayText("New balance: " + balance_stream.str());
        }
        else
        {
            Tom::Helpers::displayText("Insufficient balance.");
        }
    }

    void Bankomat::showBranches()
    {
        const auto branches = database_.listBranches();
        Tom::Helpers::displayText("Branches:");
        for (const auto& branch : branches)
        {
            Tom::Helpers::displayText(branch);
        }
    }

    void Bankomat::showUsers()
    {
        const auto users = database_.listUsers();
        Tom::Helpers::displayText("Users:");
        for (const auto& user : users)
        {
            Tom::Helpers::displayText(user);
        }
    }

    void Bankomat::searchByName()
    {
        std::string name;
        Tom::Helpers::displayText("Enter name to search: ", false);
        std::cin >> name;

        const auto users = database_.searchUsersByName(name);
        if (users.empty())
        {
            Tom::Helpers::displayText("No users found.");
            return;
        }

        Tom::Helpers::displayText("Matching users:");
        for (const auto& user : users)
        {
            Tom::Helpers::displayText(user);
        }
    }

    void Bankomat::searchByPersonId()
    {
        int person_id = 0;
        Tom::Helpers::displayText("Enter PersonID: ", false);
        std::cin >> person_id;

        const auto users = database_.searchUsersByPersonId(person_id);
        if (users.empty())
        {
            Tom::Helpers::displayText("No user found with that PersonID.");
            return;
        }

        Tom::Helpers::displayText("Matching users:");
        for (const auto& user : users)
        {
            Tom::Helpers::displayText(user);
        }
    }

    void Bankomat::searchAccounts()
    {
        std::string keyword;
        Tom::Helpers::displayText("Enter account name to search: ", false);
        std::cin >> keyword;

        const auto accounts = database_.searchAccounts(keyword);
        if (accounts.empty())
        {
            Tom::Helpers::displayText("No accounts found.");
            return;
        }

        Tom::Helpers::displayText("Matching accounts:");
        for (const auto& account : accounts)
        {
            Tom::Helpers::displayText(account);
        }
    }
}
