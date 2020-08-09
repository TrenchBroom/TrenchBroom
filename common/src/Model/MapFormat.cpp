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

#include <string>

namespace TrenchBroom {
    namespace Model {
        MapFormat mapFormat(const std::string& formatName) {
            if (formatName == "Standard") {
                return MapFormat::Standard;
            } else if (formatName == "Quake2") {
                return MapFormat::Quake2;
            } else if (formatName == "Quake2 (Valve)") {
                return MapFormat::Quake2_Valve;
            } else if (formatName == "Valve") {
                return MapFormat::Valve;
            } else if (formatName == "Hexen2") {
                return MapFormat::Hexen2;
            } else if (formatName == "Daikatana") {
                return MapFormat::Daikatana;
            } else if (formatName == "Quake3 (legacy)") {
                return MapFormat::Quake3_Legacy;
            } else if (formatName == "Quake3 (Valve)") {
                return MapFormat::Quake3_Valve;
            } else if (formatName == "Quake3") {
                return MapFormat::Quake3;
            } else {
                return MapFormat::Unknown;
            }
        }

        std::string formatName(const MapFormat format) {
            switch (format) {
                case MapFormat::Standard:
                    return "Standard";
                case MapFormat::Quake2:
                    return "Quake2";
                case MapFormat::Quake2_Valve:
                    return "Quake2 (Valve)";
                case MapFormat::Valve:
                    return "Valve";
                case MapFormat::Hexen2:
                    return "Hexen2";
                case MapFormat::Daikatana:
                    return "Daikatana";
                case MapFormat::Quake3_Legacy:
                    return "Quake3 (legacy)";
                case MapFormat::Quake3_Valve:
                    return "Quake3 (Valve)";
                case MapFormat::Quake3:
                    return "Quake3";
                case MapFormat::Unknown:
                    return "Unknown";
                switchDefault()
            }
        }

        std::vector<MapFormat> compatibleFormats(const MapFormat format) {
            switch (format) {
                case MapFormat::Standard:
                case MapFormat::Valve:
                    return { MapFormat::Standard, MapFormat::Valve};
                case MapFormat::Quake2:
                case MapFormat::Quake2_Valve:
                    return { MapFormat::Quake2, MapFormat::Quake2_Valve};
                case MapFormat::Hexen2:
                    return { MapFormat::Hexen2 };
                case MapFormat::Daikatana:
                    return { MapFormat::Daikatana };
                case MapFormat::Quake3_Legacy:
                case MapFormat::Quake3_Valve:
                case MapFormat::Quake3:
                    return { MapFormat::Quake3_Legacy, MapFormat::Quake3_Valve, MapFormat::Quake3 };
                case MapFormat::Unknown:
                    return { MapFormat::Unknown };
                switchDefault()
            }
        }

        bool isParallelTexCoordSystem(const MapFormat format) {
            switch (format) {
                case MapFormat::Valve:
                case MapFormat::Quake2_Valve:
                case MapFormat::Quake3_Valve:
                    return true;
                case MapFormat::Standard:
                case MapFormat::Quake2:
                case MapFormat::Hexen2:
                case MapFormat::Daikatana:
                case MapFormat::Quake3_Legacy:
                case MapFormat::Quake3:
                case MapFormat::Unknown:
                    return false;
                switchDefault()
            }
        }
    }
}
