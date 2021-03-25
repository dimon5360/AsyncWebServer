/*********************************************
 *
 *
 */

 /* std C++ lib headers */
#include <iostream>
#include <chrono>

/* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/date_time.hpp>

 /* local C++ headers */
#include "Logger.h"

/* open log file */
int ILogger::open() noexcept {
#if FILE_LOGGER
    log_file.open(filename.str());
    if (!log_file.is_open())
    {
        std::cout << "Log file opening is failed\n\n";
        return 1;
    }
    write(boost::str(boost::format("Log file \'%1%\' is opened.\n") % filename.str()));
#endif /* FILE_LOGGER */
}

/* close log file */
void ILogger::close() noexcept {
#if FILE_LOGGER
    std::lock_guard<std::mutex> lk(this->_m);
    if (log_file.is_open()) {
        log_file.close();
    }
#endif /* FILE_LOGGER */
}

/* get time code */
uint64_t ILogger::GetCurrTimeMs() {
    const auto systick_now = std::chrono::system_clock::now();
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
    return nowMs.count();
}

/* write log string */
void ILogger::write(std::string log) noexcept {
    using namespace boost::posix_time;
    using namespace boost::gregorian;

    ptime now = second_clock::local_time();
    std::string time = boost::str(boost::format("%1%: ") % to_simple_string(now));
#if CONSOLE_LOGGER
    std::cout << time << log;
#endif /* CONSOLE_LOGGER */
#if FILE_LOGGER
    std::lock_guard<std::mutex> lk(this->_m);
    log_file << time << log;
#endif /* FILE_LOGGER */
}