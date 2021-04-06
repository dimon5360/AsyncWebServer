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

/* write log string */
void ConsoleLogger::Write(std::string log) noexcept {
    using namespace boost::posix_time;
    using namespace boost::gregorian;

    ptime now = second_clock::local_time();
    std::string time = boost::str(boost::format("%1%: ") % to_simple_string(now));
    std::cout << time << log;
}