#include "DatabaseManager.h"

#include <iostream>
#include <sstream>

namespace Tom::Models
{
    namespace
    {
        int callbackStringList(void* data, int argc, char** argv, char** azColName)
        {
            auto* results = static_cast<std::vector<std::string>*>(data);
            if (argc > 0 && argv[0] != nullptr)
            {
                results->emplace_back(argv[0]);
            }
            (void)azColName;
            return 0;
        }
    }

    DatabaseManager::DatabaseManager(const std::string& database_path)
        : database_path_(database_path), connection_(nullptr)
    {
    }

    bool DatabaseManager::initialize()
    {
        if (!openConnection())
        {
            return false;
        }

        const char* create_branches_sql = R"(
            CREATE TABLE IF NOT EXISTS branches (
                BranchID INTEGER PRIMARY KEY AUTOINCREMENT,
                BranchName TEXT NOT NULL
            );
        )";

        const char* create_people_sql = R"(
            CREATE TABLE IF NOT EXISTS people (
                PersonID INTEGER PRIMARY KEY,
                FullName TEXT NOT NULL,
                BranchID INTEGER NOT NULL,
                FOREIGN KEY (BranchID) REFERENCES branches(BranchID)
            );
        )";

        const char* create_accounts_sql = R"(
            CREATE TABLE IF NOT EXISTS accounts (
                AccountID INTEGER PRIMARY KEY AUTOINCREMENT,
                Username TEXT NOT NULL UNIQUE,
                Password TEXT NOT NULL,
                Balance REAL NOT NULL DEFAULT 0.0
            );
        )";

        sqlite3_exec(connection_, create_branches_sql, nullptr, nullptr, nullptr);
        sqlite3_exec(connection_, create_people_sql, nullptr, nullptr, nullptr);
        sqlite3_exec(connection_, create_accounts_sql, nullptr, nullptr, nullptr);

        sqlite3_exec(connection_, "INSERT OR IGNORE INTO branches (BranchID, BranchName) VALUES (1, 'Main Branch');", nullptr, nullptr, nullptr);
        sqlite3_exec(connection_, "INSERT OR IGNORE INTO branches (BranchID, BranchName) VALUES (2, 'West Branch');", nullptr, nullptr, nullptr);

        sqlite3_exec(connection_, "INSERT OR IGNORE INTO people (PersonID, FullName, BranchID) VALUES (1001, 'Anna Kowalska', 1);", nullptr, nullptr, nullptr);
        sqlite3_exec(connection_, "INSERT OR IGNORE INTO people (PersonID, FullName, BranchID) VALUES (1002, 'Piotr Nowak', 2);", nullptr, nullptr, nullptr);
        sqlite3_exec(connection_, "INSERT OR IGNORE INTO people (PersonID, FullName, BranchID) VALUES (1003, 'Marta Zielinska', 1);", nullptr, nullptr, nullptr);
        sqlite3_exec(connection_, "INSERT OR IGNORE INTO accounts (Username, Password, Balance) VALUES ('admin', 'admin', 100.0);", nullptr, nullptr, nullptr);

        closeConnection();
        return true;
    }

    std::vector<std::string> DatabaseManager::listBranches()
    {
        if (!openConnection())
        {
            return {};
        }

        std::vector<std::string> results;
        sqlite3_exec(connection_, "SELECT BranchName FROM branches ORDER BY BranchID;", callbackStringList, &results, nullptr);
        closeConnection();
        return results;
    }

    std::vector<std::string> DatabaseManager::listUsers()
    {
        if (!openConnection())
        {
            return {};
        }

        std::vector<std::string> results;
        sqlite3_exec(connection_, "SELECT PersonID || ' - ' || FullName || ' (' || (SELECT BranchName FROM branches WHERE BranchID = people.BranchID) || ')' FROM people ORDER BY PersonID;", callbackStringList, &results, nullptr);
        closeConnection();
        return results;
    }

    std::vector<std::string> DatabaseManager::searchUsersByName(const std::string& name)
    {
        if (!openConnection())
        {
            return {};
        }

        std::vector<std::string> results;
        std::ostringstream sql;
        sql << "SELECT PersonID || ' - ' || FullName || ' (' || (SELECT BranchName FROM branches WHERE BranchID = people.BranchID) || ')' FROM people WHERE FullName LIKE '%" << name << "%' ORDER BY PersonID;";
        sqlite3_exec(connection_, sql.str().c_str(), callbackStringList, &results, nullptr);
        closeConnection();
        return results;
    }

    std::vector<std::string> DatabaseManager::searchUsersByPersonId(int person_id)
    {
        if (!openConnection())
        {
            return {};
        }

        std::vector<std::string> results;
        std::ostringstream sql;
        sql << "SELECT PersonID || ' - ' || FullName || ' (' || (SELECT BranchName FROM branches WHERE BranchID = people.BranchID) || ')' FROM people WHERE PersonID = " << person_id << ";";
        sqlite3_exec(connection_, sql.str().c_str(), callbackStringList, &results, nullptr);
        closeConnection();
        return results;
    }

    bool DatabaseManager::createAccount(const std::string& username, const std::string& password, double initial_balance)
    {
        if (!openConnection())
        {
            return false;
        }

        sqlite3_stmt* stmt = nullptr;
        const char* check_sql = "SELECT AccountID FROM accounts WHERE Username = ?1;";
        int rc = sqlite3_prepare_v2(connection_, check_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            closeConnection();
            return false;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        const bool exists = rc == SQLITE_ROW;
        sqlite3_finalize(stmt);

        if (exists)
        {
            closeConnection();
            return false;
        }

        const char* insert_sql = "INSERT INTO accounts (Username, Password, Balance) VALUES (?1, ?2, ?3);";
        rc = sqlite3_prepare_v2(connection_, insert_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            closeConnection();
            return false;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, initial_balance);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        closeConnection();
        return rc == SQLITE_DONE;
    }

    bool DatabaseManager::loginAccount(const std::string& username, const std::string& password, double& balance, int& account_id)
    {
        if (!openConnection())
        {
            return false;
        }

        sqlite3_stmt* stmt = nullptr;
        const char* query_sql = "SELECT AccountID, Balance FROM accounts WHERE Username = ?1 AND Password = ?2;";
        int rc = sqlite3_prepare_v2(connection_, query_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            closeConnection();
            return false;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            account_id = sqlite3_column_int(stmt, 0);
            balance = sqlite3_column_double(stmt, 1);
            sqlite3_finalize(stmt);
            closeConnection();
            return true;
        }

        sqlite3_finalize(stmt);
        closeConnection();
        return false;
    }

    bool DatabaseManager::updateBalance(int account_id, double balance)
    {
        if (!openConnection())
        {
            return false;
        }

        sqlite3_stmt* stmt = nullptr;
        const char* update_sql = "UPDATE accounts SET Balance = ?1 WHERE AccountID = ?2;";
        int rc = sqlite3_prepare_v2(connection_, update_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            closeConnection();
            return false;
        }

        sqlite3_bind_double(stmt, 1, balance);
        sqlite3_bind_int(stmt, 2, account_id);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        closeConnection();
        return rc == SQLITE_DONE;
    }

    std::vector<std::string> DatabaseManager::searchAccounts(const std::string& keyword)
    {
        if (!openConnection())
        {
            return {};
        }

        std::vector<std::string> results;
        std::ostringstream sql;
        sql << "SELECT Username || ' - Balance: ' || printf('%.2f', Balance) FROM accounts WHERE Username LIKE '%" << keyword << "%' ORDER BY Username;";
        sqlite3_exec(connection_, sql.str().c_str(), callbackStringList, &results, nullptr);
        closeConnection();
        return results;
    }

    bool DatabaseManager::openConnection()
    {
        if (connection_ != nullptr)
        {
            return true;
        }

        const int rc = sqlite3_open(database_path_.c_str(), &connection_);
        return rc == SQLITE_OK;
    }

    void DatabaseManager::closeConnection()
    {
        if (connection_ != nullptr)
        {
            sqlite3_close(connection_);
            connection_ = nullptr;
        }
    }
}
