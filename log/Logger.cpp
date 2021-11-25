/*****************************************************************
 *  @file       Logger.cpp
 *  @brief      Logger class implementation
 *  @author     Kalmykov Dmitry
 *  @date       28.04.2021
 *  @modified   19.08.2021
 *  @version    0.1
 */

/* std C++ lib headers */
#include <iostream>
#include <chrono>

/* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/date_time.hpp>

#include <spdlog/spdlog.h>

#include "Logger.h"

uint64_t ConsoleLogger::GetCurrTimeMs() noexcept {
    const auto systick_now = std::chrono::system_clock::now();
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
    return nowMs.count();
}

void ConsoleLogger::Info(const std::string&& log) noexcept {

    spdlog::info(log);
}

void ConsoleLogger::Debug(const std::string&& log) noexcept {
#ifdef DEBUG_ENABLE
    spdlog::info(log);
#endif /* DEBUG_ENABLE */
}

void ConsoleLogger::Error(const std::string&& log) noexcept {
    spdlog::error(log);
}