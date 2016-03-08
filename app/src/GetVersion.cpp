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

#include "View/GetVersion.h"
#include "Version.h"

namespace TrenchBroom {
    namespace View {
        wxString getBuildVersion() {
            wxString result;
            result << VERSION_STR;
            return result;
        }

        wxString getBuildChannel() {
            wxString result;
            result << BUILD_CHANNEL;
            return result;
        }

        wxString getBuildId() {
            wxString result;
            result << BUILD_ID;
            return result;
        }
        
        wxString getBuildType() {
            wxString result;
            result << BUILD_TYPE;
            return result;
        }
    }
}

