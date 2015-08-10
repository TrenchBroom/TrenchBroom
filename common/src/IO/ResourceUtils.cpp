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

#include "ResourceUtils.h"

#include "IO/Path.h"
#include "IO/SystemPaths.h"

#include <wx/log.h>

namespace TrenchBroom {
    namespace IO {
        wxBitmap loadImageResource(const String& name) {
            return loadImageResource(IO::Path(name));
        }

        wxBitmap loadImageResource(const IO::Path& imagePath) {
            wxLogNull logNull; // need this to suppress errors when loading PNG files, see http://trac.wxwidgets.org/ticket/15331
            const IO::Path fullPath = imagePath.isAbsolute() ? imagePath : IO::SystemPaths::resourceDirectory() + IO::Path("images") + imagePath;
            if (!::wxFileExists(fullPath.asString()))
                return wxNullBitmap;
            return wxBitmap(fullPath.asString(), wxBITMAP_TYPE_PNG);
        }
    }
}
