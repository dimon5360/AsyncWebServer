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

#include <spdlog/spdlog.h>

/* local C++ headers */
#include "Logger.h"

/* write log string */
void ConsoleLogger::Write(std::string log) noexcept {

    spdlog::info(log);
}