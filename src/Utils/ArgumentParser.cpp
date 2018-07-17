/**
 * @file ArgumentParser.cpp

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

#include "ArgumentParser.h"

ArgumentParser::ArgumentParser(const std::string& opt_list, struct option* options) {
    this->opt_list_ = opt_list;
    this->options_  = options;
}

std::map<char, char*> ArgumentParser::parse(int& argc, char*** argv) {
    char option = 0;
    int optindex = 0;

    std::map<char, char*> args;

    while ((option = getopt_long(argc, *argv, this->opt_list_.c_str(), this->options_, &optindex)) != -1) {
        if (option == '?') {
            continue;
        }
        args.insert(std::pair<char, char*>(option, optarg));
    }
    
    *argv += optind;
    argc -= optind;

    return args; 
}