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
#include <boost/format.hpp>
#include <boost/container_hash/hash.hpp>

/* extern C++ lib pqxx headers */
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>
#include "cryptopp/base64.h"
#include "cryptopp/sha.h"
#include "cryptopp/hmac.h"

/* local C++ headers */
#include "PostgresProcessor.h"

static IConfig dbcfg;

std::string SHA256(std::string data)
{
    using namespace CryptoPP;
    byte const* pbData = (byte*)data.data();
    unsigned int nDataLen = data.size();
    byte abDigest[CryptoPP::SHA256::DIGESTSIZE];

    CryptoPP::SHA256().CalculateDigest(abDigest, pbData, nDataLen); 
    CryptoPP::Base64Encoder encoder;
    std::string output;
    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(abDigest, sizeof(abDigest));
    encoder.MessageEnd();
    return output;// std::string((char*)abDigest, CryptoPP::SHA256::DIGESTSIZE);
}

/*********************************************************
 *  @brief  Set connection to PostgreSQL database
 */
void PostgresProcessor::InitializeDatabaseConnection() {
        
    try
    {
        /* open db config file */
        dbcfg.Open("db.ini");

        std::string connection_string{
            boost::str(boost::format("dbname = %1% user = %2% password = %3% host = %4% port = %5%") 
            % dbcfg.GetRecordByKey("dbname") 
            % dbcfg.GetRecordByKey("admin")
            % dbcfg.GetRecordByKey("password")
            % dbcfg.GetRecordByKey("host")
            % dbcfg.GetRecordByKey("port"))};

        pqxx::connection C{ connection_string };
        pqxx::work W{ C };

        pqxx::result R{ W.exec(boost::str(boost::format("SELECT * FROM %1% where email = \'%2%\';\n") % dbcfg.GetRecordByKey("dbusertable") % "vasiliy@test.com")) };

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
            std::string hash = SHA256("password");
            
            std::string insert_request{
                boost::str(boost::format("INSERT INTO %1% (email, username, password) VALUES(\'%2%\',\'%3%\',\'%4%\');")
                % dbcfg.GetRecordByKey("dbusertable")
                % "vasiliy@test.com"
                % "Vasiok"
                % hash)};

            pqxx::result R{ W.exec(insert_request) };
            W.commit();
        }
        C.close();
    }
    catch (std::exception const& e)
    {
        spdlog::error(e.what(), "\n");
    }
}