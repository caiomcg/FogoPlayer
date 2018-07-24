/**
 * @file ArgumentParser.hpp
 *
 * @class ArgumentParser
 *
 * @brief Parse command line arguments
 *
 * Receive argc and argv data and parse the arguments
 *
 * @license GNU GPL V3
 * 
 * Copyright (C) 2018 Caio Marcelo Campoy Guedes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Caio Marcelo Campoy Guedes <caiomcg@gmail.com>
 */

#pragma once

#include <map>
#include <string>
#include <getopt.h>

class ArgumentParser {
private:
    std::string opt_list_;
    struct option* options_;
public:
    /**
     * @brief Construct a new Argument Parser object
     * 
     * @param opt_list list of optios to be fetched - see getopt_long 
     * @param options long options
     */
    ArgumentParser(const std::string& opt_list, struct option* options);

    /**
     * @brief Parse the command line parameters
     * 
     * @param argc Amount of arguments passed through the command line
     * @param argv The arguments given to the process by the OS
     * 
     * @return std::map<char, char*> A map containing a pair of arguments extracted from argv
     */
    std::map<char, std::string> parse(int& argc, char*** argv);
};