/*********************************************
 *
 *
 */
#pragma once

class ConsoleLogger {

private:

    /* get time code */
    uint64_t GetCurrTimeMs() noexcept;

public:

    /* write log string */
    void write(std::string log) noexcept;

    /* destructor */
    ConsoleLogger() {
        std::cout << "Construct logger class\n";
    }

    /* destructor */
    ~ConsoleLogger() {
        std::cout << "Destruct logger class\n";
    }
};
