module;


/* std C++ lib headers */
#include <iostream>
#include <ctime>
#include <cstdint>

#include <boost/date_time.hpp>


/* extern C++ lib pqxx headers */
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

export module MPostgresProcess;

export enum class postres_err_t  {
    POSTGRES_ERROR_OK = 0,
    POSTGRES_ERROR_FAILED_INIT_DB = 1,
};


export class PostgresProcessor {
public:
    PostgresProcessor() {}

    ~PostgresProcessor() {}

        
    postres_err_t InitializeDatabaseConnection() {
        id_ = 0;
        
        try
        {
            pqxx::connection C{ "dbname = postgres user = postgres password = adM1n34#184" };
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
            return postres_err_t::POSTGRES_ERROR_FAILED_INIT_DB;
        }
        return postres_err_t::POSTGRES_ERROR_OK;
    }

private:
    uint32_t id_;
};