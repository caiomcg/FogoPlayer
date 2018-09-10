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


#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <memory>

#include "Receiver.hpp"
#include "Utils/ArgumentParser.hpp"
#include "InputMedia/OpenCV/CameraInput.hpp"
#include "InputMedia/LibAV/FileInput.hpp"
#include "Synchronizer/Synchronizer.hpp"

void usage() {
    std::cout << "\033[1;37mNAME\033[0m" << std::endl;
    std::cout << "        FogoPlayer - A complete distributed video player" << std::endl;
	std::cout << "\033[1;37mSYNOPSIS\033[0m" << std::endl;
    std::cout << "        FogoPlayer [OPTIONS]... [OUTPUTS]..." << std::endl;
    std::cout << "\033[1;37mDESCRIPTION\033[0m" << std::endl;
    std::cout << "        -i or --input" << std::endl;
    std::cout << "            Path to the input device or input file. Depends on the mode of operation, for more information refer to the README file." << std::endl;
    std::cout << "        -b or --border_offset" << std::endl;
    std::cout << "            Border offset in pixels." << std::endl;
    std::cout << "        -s or --stream" << std::endl;
    std::cout << "            Enables the streming mode" << std::endl;
    std::cout << "        -r or --receiver" << std::endl;
    std::cout << "            Enables the receiving mode" << std::endl;
    std::cout << "        -c or --cutter" << std::endl;
    std::cout << "            Enables the video cutter" << std::endl;
    std::cout << "\033[1;37mEXIT STATUS\033[0m" << std::endl;
    std::cout << "        0 - Exited normally" << std::endl;
    std::cout << "        1 - An error occured" << std::endl;
    std::cout << "\033[1;37mUSE EXAMPLE\033[0m" << std::endl;
    std::cout << "        \033[0;35m\033[0m" << std::endl;
}

int main(int argc, char** argv) {  
    int niceness = 0;
    std::string current_arg;
    std::string input_file;

    std::map<char, std::string>::iterator argument;

    struct option long_options[] = {
        {"input",     required_argument, 0,  'i' },
        {"border_offset", optional_argument, 0, 'b'},
        {"stream",     no_argument, 0,  's' },
        {"receiver",     no_argument, 0,  'r' },
        {"cutter",     no_argument, 0,  'c' },
        {0,                           0, 0,   0  }
    };

    auto args = ArgumentParser("i:b:src", long_options).parse(argc, &argv);
    
    if (args.size() < 1) {
        usage();
        return 1;
    }
    
    if ((niceness = nice(1)) == -1) {
        std::cerr << "Could not set higher priority to the process " << strerror(errno) << std::endl;
    }

    if ((argument = args.find('i')) != args.end()) {
        input_file = argument->second;
    } else {
        std::cerr << "An input is required" << std::endl;
        return 1;
    }

    if ((argument = args.find('s')) != args.end()) {
        std::cout << "Initializing streaming mode ";
    } else if ((argument = args.find('r')) != args.end()) {
        std::cout << "Initializing receiver mode " << std::endl;
        Receiver receiver{};

        if ((argument = args.find('b')) != args.end()) {
            receiver.setBorderOffset(atoi(argument->second.c_str()));
        }

        receiver.spawn(input_file).join();
    } else if ((argument = args.find('c')) != args.end()) {
        std::cout << "Initializing video cutter ";
    } else {
        Synchronizer synchronizer{};
        synchronizer.setInputMedia(std::unique_ptr<OpencvInputMedia>(new CameraInput()))
            .spawn(input_file).join();
    }

    return 0;
}