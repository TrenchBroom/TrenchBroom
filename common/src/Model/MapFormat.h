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

#ifndef TrenchBroom_MapFormat
#define TrenchBroom_MapFormat

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        enum class MapFormat {
            /**
             * Unknown map format.
             */
            Unknown,
            /**
             * Standard Quake 1 map format.
             */
            Standard,
            /**
             * Quake 2 map format.
             */
            Quake2,
            /**
             * Quake 2 with Valve 220 format texturing, supported by https://github.com/qbism/q2tools-220
             */
            Quake2_Valve,
            /**
             * Valve 220 map format.
             */
            Valve,
            /**
             * Hexen 2 map format.
             */
            Hexen2,
            /**
             * Daikatana map format.
             */
            Daikatana,
            /**
             * Quake 3 legacy format (like Quake 2, no brush primitives)
             */
            Quake3_Legacy,
            /**
             * Quake 3 with Valve 220 format texturing, supported by https://github.com/Garux/netradiant-custom/tree/master/tools/quake3/q3map2
             */
            Quake3_Valve,
            /**
             * Quake 3 with brush primitives, also allows Quake 2 brushes
             */
            Quake3
        };

        /**
         * Returns the map format enum value with the given name. If the given name is not recognized, MapFormat::Unknown
         * is returned.
         *
         * @param formatName the name
         * @return the enum value
         */
        MapFormat mapFormat(const std::string& formatName);

        /**
         * Returns the name of the given map format enum value.
         *
         * @param format the enum value
         * @return the name
         */
        std::string formatName(MapFormat format);
        std::vector<MapFormat> compatibleFormats(MapFormat format);
        bool isParallelTexCoordSystem(MapFormat format);
    }
}

#endif /* defined(TrenchBroom_MapFormat) */
