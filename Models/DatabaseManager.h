#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <sqlite3.h>

namespace Tom::Models
{
    class DatabaseManager
    {
    public:
        explicit DatabaseManager(const std::string& database_path = "bankomat.db");

        bool initialize();
        std::vector<std::string> listBranches();
        std::vector<std::string> listUsers();
        std::vector<std::string> searchUsersByName(const std::string& name);
        std::vector<std::string> searchUsersByPersonId(int person_id);
        bool createAccount(const std::string& username, const std::string& password, double initial_balance);
        bool loginAccount(const std::string& username, const std::string& password, double& balance, int& account_id);
        bool updateBalance(int account_id, double balance);
        std::vector<std::string> searchAccounts(const std::string& keyword);

    private:
        bool openConnection();
        void closeConnection();

        std::string database_path_;
        sqlite3* connection_;
    };
}

#endif
