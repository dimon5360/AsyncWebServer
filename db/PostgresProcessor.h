/*****************************************************************
 *  @file       PostgresProcessor.h
 *  @brief      Postgres connection handler class declaration
 *  @author     Kalmykov Dmitry
 *  @date       26.03.2021
 *  @modified   19.08.2021
 *  @version    0.1
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
        std::cout << "Construct Postgres processor class\n";
        /* try to connect to database */
        InitializeDatabaseConnection();
    }

    /* destructor */
    ~PostgresProcessor() {
        std::cout << "Destruct Postgres processor class\n";
    }

};