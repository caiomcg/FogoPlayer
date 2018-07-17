/**
 * @file main.cpp
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

#include <iostream>

#include "Utils/ArgumentParser.h"

void usage() {
    // Fill in software usage
}

int main(int argc, char** argv) {   
    struct option long_options[] = {
        {"input",     required_argument, 0,  'i' },
        {0,                           0, 0,   0  }
    };

    auto args = ArgumentParser("i:", long_options).parse(argc, &argv);
    
    if (args.size() < 1) {
        usage();
        return 1;
    }

    return 0;
}