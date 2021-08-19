/*****************************************************************
 *  @file       json.cpp
 *  @brief      JSON handler class implementation
 *
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @modified   19.08.2021
 *  @version    0.1
 */

 /* local C++ headers */
#include "json.h"
#include "../log/Logger.h"

/* external C++ libs headers -------------------------------- */
/* boost C++ lib headers */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>

/* spdlog C++ lib */
#include <spdlog/spdlog.h>

/* public method implementation ----------------------------- */

/* TEST JSON data
 * 
 {
    "height" : 320,
    "some" :
    {
        "complex" :
        {
            "path" : "hello"
        }
    },
    "animals" :
    {
        "rabbit" : "white",
        "dog" : "brown",
        "cat" : "grey"
    },
    "fruits" : ["apple", "raspberry", "orange"],
    "matrix" : [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
 }
 * 
 */

#include <fstream>
#include <iostream>

void JsonParser::handle() noexcept {

    namespace pt = boost::property_tree;
    const std::string sTempJson = "{ \"usermail\" : \"test@test.com\", \"username\" : \"test\", \"password\" : \"testPASS2#$\" }\r\n";
    try
    {

        pt::ptree tree;
        std::stringstream req{ sTempJson };
        
        pt::read_json(req, tree);
    
        PrintTree(tree);
    }
    catch (const std::exception& ex)
    {
        ConsoleLogger::Error(boost::str(boost::format("JSON parse exception : %1%\n") % ex.what()));
    }

}


void JsonParser::PrintTree(boost::property_tree::ptree& tree) {

    /* print separate parameters */
    std::string msg;

    msg = tree.get<std::string>("usermail");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));
    msg = tree.get<std::string>("username");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));
    msg = tree.get<std::string>("password");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));

    /* traversal the tree through 'foreach' */
    for (auto& item : tree) {
        auto key = item.first;
        auto value = item.second.data();
        ConsoleLogger::Debug(boost::str(boost::format("%1% : %2%") % key % value));
    }
}




JsonParser::JsonParser() {

}

JsonParser::~JsonParser() {

}