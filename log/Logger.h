/*********************************************
 *
 *
 */
#pragma once


class ConsoleLogger {

    /* get time code */
    static uint64_t GetCurrTimeMs() noexcept {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }

public:
        
    void Write(std::string log) noexcept;

    ConsoleLogger() {
        std::cout << "Construct logger class\n";
    }

    ~ConsoleLogger() {
        std::cout << "Destruct logger class\n";
    }
};
