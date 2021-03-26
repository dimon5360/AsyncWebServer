/*********************************************
 *
 *  @version    0.1
 *  @date       26.03.2021
 */

/* std C++ lib headers */
#include <iostream>
#include <ctime>
#include <cstdint>
#include <fstream>

/* boost C++ lib headers */
#include <boost/date_time.hpp>

/* extern C++ lib pqxx headers */
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

/* local C++ headers */
#include "PostgresProcessor.h"

static IConfig dbcfg;

/*********************************************************
 *  @brief  Set connection to PostgreSQL database
 */
void PostgresProcessor::InitializeDatabaseConnection() {
        
    try
    {
        /* open db config file */
        dbcfg.Open("db.ini");

        std::stringstream connection_string;
        connection_string << "dbname = " << dbcfg.GetRecordByKey("dbname")
            << " user = " << dbcfg.GetRecordByKey("admin")
            << " password = " << dbcfg.GetRecordByKey("password");

        pqxx::connection C{ connection_string.str() };
        spdlog::info("Connected to ", C.dbname(), "\n");
        pqxx::work W{ C };

        pqxx::result R{ W.exec("SELECT id FROM UsersTable;") };

        if (!R.size()) {
            std::stringstream res;
            res << R.size();
            spdlog::info("Found " + res.str() + " users:\n");
            for (auto row : R)
                std::cout << row[0].c_str() << '\n';

            spdlog::info("Doubling all employees' salaries...\n");
            W.exec0("UPDATE employee SET salary = salary*2");


            spdlog::info("Making changes definite: ");
            W.commit();

            spdlog::info("OK.\n");
        }
        else {
            spdlog::info("Users not found\n");

            std::stringstream req;

            using namespace boost::posix_time;
            using namespace boost::gregorian;

            ptime now = second_clock::local_time();
            req << "INSERT INTO UsersTable (fname, email, date) VALUES ('Vasiliy','vasiliy@test.com','" << to_iso_string(now) << "');";
            pqxx::result R{ W.exec(req.str()) };
            W.commit();
        }
        C.close();
    }
    catch (std::exception const& e)
    {
        spdlog::error(e.what(), "\n");
    }
}