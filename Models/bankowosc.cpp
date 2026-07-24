
#include "Bankowosc.h"

namespace Tom::Models
{
    Bankowosc::Bankowosc(double initial_balance)
        : balance_(initial_balance)
    {
    }

    void Bankowosc::deposit(double amount)
    {
        if (amount > 0.0)
        {
            balance_ += amount;
        }
    }

    bool Bankowosc::withdraw(double amount)
    {
        if (amount > 0.0 && amount <= balance_)
        {
            balance_ -= amount;
            return true;
        }
        return false;
    }

    double Bankowosc::getBalance() const
    {
        return balance_;
    }
}
