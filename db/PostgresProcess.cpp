/*****************************************************************
 *  @file       PostgresProcessor.cpp
 *  @brief      Postgres connection handler class implementation
 *  @author     Kalmykov Dmitry
 *  @date       26.03.2021
 *  @modified   19.08.2021
 *  @version    0.1
 */

#include <iostream>
#include <ctime>
#include <cstdint>
#include <fstream>

#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/container_hash/hash.hpp>

#include <openssl/sha.h>

#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

#include "PostgresProcessor.h"

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

const std::string createtableScript {
    "CREATE TABLE IF NOT EXISTS userstable$167 (\
            id SERIAL PRIMARY KEY,\
            username VARCHAR UNIQUE NOT NULL,\
            email VARCHAR UNIQUE NOT NULL,\
            password VARCHAR NOT NULL,\
            update_at timestamp NOT NULL,\
            created_at timestamp NOT NULL,\
            active BOOL\
        )"
};

/*********************************************************
 *  @brief  Set connection to PostgreSQL database
 */
void PostgresProcessor::InitializeDatabaseConnection() {
        
    try
    {
        /* open db config file */
        auto dbcfg = std::make_shared<IConfig>();
        dbcfg->Open("postgres.ini");
        std::string connection_string{dbcfg->GetConfigValueByKey("postgres_connection_string")};

        pqxx::connection C{ connection_string };
        if (C.is_open()) {
            std::cout << "Opened database successfully: " << C.dbname() << std::endl;
        } 
        pqxx::work W{ C };


        pqxx::result R{ W.exec(createtableScript) };

        R = W.exec(boost::str(boost::format("SELECT * FROM %1% where email = \'%2%\';\n") % 
                dbcfg->GetConfigValueByKey("dbusertable") % "vasiliy@test.com"));

        if (R.size()) {
            spdlog::info(boost::str(boost::format("Found %1% users:") % R.size()));
            for (auto row : R) {
                for (auto const& v : row) {
                    std::cout << v << ' ';
                }
                std::cout << '\n';
            }
            spdlog::info("OK.\n");
        }
        else {
            spdlog::info("Users not found\n");

            std::stringstream req;

            using namespace boost::posix_time;
            using namespace boost::gregorian;

            spdlog::info("Add new user\n");
            ptime now = second_clock::local_time();

            unsigned char obuf[20];
            std::string pass = "vasyapassword";

            std::string insert_request{
                boost::str(boost::format("INSERT INTO %1% (username, email, password, update_at, created_at, active) VALUES(\'%2%\',\'%3%\',\'%4%\',\'%5%\',\'%6%\',\'%7%\');")
                % dbcfg->GetConfigValueByKey("dbusertable")
                % "vasya123"
                % "vasiliy@test.com"
                % pass
                % boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time())
                % boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time())
                % true)};

            pqxx::result R{ W.exec(insert_request) };
        }
        W.commit();
        C.disconnect();
    }
    catch (std::exception const& e)
    {
        spdlog::error(e.what(), "\n");
    }
}