/*********************************************
 *
 *
 */
#pragma once

/* std C++ lib headers */
#include <iostream>
#include <ctime>
#include <cstdint>

 /* boost C++ lib headers */
#include <boost/date_time.hpp>

/* extern C++ lib pqxx headers */
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

/* local C++ headers */
#include "../config/config.h"

class PostgresProcessor {

private:

    /*********************************************************
     *  @brief  Set connection to PostgreSQL database
     */
    void InitializeDatabaseConnection();

public:

    /* constructor */
    PostgresProcessor() {
        std::cout << "Postgres processor class constructor\n";
        /* try to connect to database */
        InitializeDatabaseConnection();
    }

    /* destructor */
    ~PostgresProcessor() {
        std::cout << "Postgres processor class destructor\n";
    }

};