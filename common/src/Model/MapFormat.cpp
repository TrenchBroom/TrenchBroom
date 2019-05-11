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

#include "Macros.h"

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
            } else if (formatName == "Quake3 (legacy)") {
                return MapFormat::Quake3_Legacy;
            } else if (formatName == "Quake3") {
                return MapFormat::Quake3;
            } else {
                return MapFormat::Unknown;
            }
        }

        String formatName(const MapFormat format) {
            switch (format) {
                case MapFormat::Standard:
                    return "Standard";
                case MapFormat::Quake2:
                    return "Quake2";
                case MapFormat::Valve:
                    return "Valve";
                case MapFormat::Hexen2:
                    return "Hexen2";
                case MapFormat::Daikatana:
                    return "Daikatana";
                case MapFormat::Quake3_Legacy:
                    return "Quake3 (legacy)";
                case MapFormat::Quake3:
                    return "Quake3";
                case MapFormat::Unknown:
                    return "Unknown";
                switchDefault()
            }
        }
    }
}
