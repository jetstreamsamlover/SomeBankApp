#ifndef BANKOWOSC_H
#define BANKOWOSC_H

namespace Tom::Models
{
    class Bankowosc
    {
    public:
        Bankowosc(double initial_balance = 0.0);
        void deposit(double amount);
        bool withdraw(double amount);
        double getBalance() const;

    private:
        double balance_;
    };
}

#endif 

        