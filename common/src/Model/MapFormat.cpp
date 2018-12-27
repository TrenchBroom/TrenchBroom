/*
 Copyright (C) 2010-2017 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapFormat.h"

namespace TrenchBroom {
    namespace Model {
        MapFormat mapFormat(const String& formatName) {
            if (formatName == "Standard") {
                return MapFormat::Standard;
            } else if (formatName == "Quake2") {
                return MapFormat::Quake2;
            } else if (formatName == "Valve") {
                return MapFormat::Valve;
            } else if (formatName == "Hexen2") {
                return MapFormat::Hexen2;
            } else if (formatName == "Daikatana") {
                return MapFormat::Daikatana;
            } else {
                return MapFormat::Unknown;
            }
        }

        String formatName(const MapFormat format) {
            if (format == MapFormat::Standard) {
                return "Standard";
            } else if (format == MapFormat::Quake2) {
                return "Quake2";
            } else if (format == MapFormat::Valve) {
                return "Valve";
            } else if (format == MapFormat::Hexen2) {
                return "Hexen2";
            } else if (format == MapFormat::Daikatana) {
                return "Daikatana";
            } else {
                return "Unknown";
            }
        }
    }
}
