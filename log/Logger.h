/*********************************************
 *
 *
 */
#pragma once

class ILogger {

private:

    /* get time code */
    virtual uint64_t GetCurrTimeMs() = 0;

public:

    /* open log file */
    virtual void open() noexcept = 0;

    /* close log file */
    virtual void close() noexcept = 0;

    /* write log string */
    virtual void write(std::string log) noexcept = 0;

    /* destructor */
    virtual ~ILogger() {
        std::cout << "Destructor of concole logger\n";
    }
};