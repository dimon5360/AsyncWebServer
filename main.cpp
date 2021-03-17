#include <iostream>
#include <pqxx/pqxx>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "db/PostgresProcessor.h"

const uint32_t PATCH = 0;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

int main()
{
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    std::unique_ptr<Postgres::PostgresProcessor> db = std::make_unique<Postgres::PostgresProcessor>();
    std::cout << (int)db->InitializeDatabaseConnection() << std::endl;
    return 0;
}