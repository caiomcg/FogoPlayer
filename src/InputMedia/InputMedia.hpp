/**
 * @file InputMedia.hpp
 *
 * @class InputMedia
 *
 * @brief Define the interface for media input
 *
 * Defines a interface that should be implemented in order to change the 
 * default behaviour of the streamer/player
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

#include <string>

class InputMedia {
public:
    virtual ~InputMedia() {}
    virtual void open(const std::string& file_name) = 0;
};