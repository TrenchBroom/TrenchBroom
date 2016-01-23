/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
        MapFormat::Type mapFormat(const String& formatName) {
            if (formatName == "Standard")
                return MapFormat::Standard;
            if (formatName == "Quake2")
                return MapFormat::Quake2;
            if (formatName == "Valve")
                return MapFormat::Valve;
            if (formatName == "Hexen2")
                return MapFormat::Hexen2;
            return MapFormat::Unknown;
        }

        String formatName(const MapFormat::Type format) {
            if (format == MapFormat::Standard)
                return "Standard";
            if (format == MapFormat::Quake2)
                return "Quake2";
            if (format == MapFormat::Valve)
                return "Valve";
            if (format == MapFormat::Hexen2)
                return "Hexen2";
            return "Unknown";
        }
    }
}
