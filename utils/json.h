/*****************************************************************
 *  @file       json.h
 *  @brief      JSON handler class declaration
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @version    0.1
 */
#pragma once

 /* boost C++ lib headers */
#include <boost/property_tree/ptree.hpp>

class JsonParser {

private:

    void PrintTree(boost::property_tree::ptree& tree);

public:
    void handle(void) noexcept;

    JsonParser(void);
    ~JsonParser(void);
};