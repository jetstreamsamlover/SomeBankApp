#include "Printer.h"

#include <iostream>

namespace Tom::Helpers
{
    void displayText(const std::string& text, bool with_new_line)
    {
        std::cout << text;
        if (with_new_line)
        {
            std::cout << std::endl;
        }
    }

    void readNumber(int& read_number)
    {
        std::cin >> read_number;
    }
}