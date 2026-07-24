
#include "Bankowosc.h"
#include "DatabaseManager.h"

namespace Tom::Models
{
    class Bankomat
    {
    public:
        Bankomat();
        void run();

    private:
        Bankowosc account_;
        DatabaseManager database_;
        int current_account_id_;
        bool startSession();
        void saveCurrentBalance();
        void displayMenu() const;
        bool selectOption();
        void deposit();
        void withdraw();
        void showBranches();
        void showUsers();
        void searchByName();
        void searchByPersonId();
        void searchAccounts();
    };
}

