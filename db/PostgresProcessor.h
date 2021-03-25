/*********************************************
 *
 *
 */
#pragma once

/* std C++ lib headers */
#include <iostream>
#include <ctime>
#include <cstdint>

#include <boost/date_time.hpp>


/* extern C++ lib pqxx headers */
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

enum class postres_err_t {
    POSTGRES_ERROR_OK = 0,
    POSTGRES_ERROR_FAILED_INIT_DB = 1,
};


class PostgresProcessor {
public:
    PostgresProcessor() {}

    ~PostgresProcessor() {}


    postres_err_t InitializeDatabaseConnection();

private:
    uint32_t id_;
};