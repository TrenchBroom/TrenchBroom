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

#ifndef TrenchBroom_FreeType_h
#define TrenchBroom_FreeType_h

// FreeType defines SIZEOF_LONG, which is either already defined by
// wxWidgets or which will later be defined by wxWidgets. In either
// case, this leads to a warning.

#ifdef SIZEOF_LONG
    #define SIZEOF_LONG_OLD SIZEOF_LONG
    #undef SIZEOF_LONG
    #include <ft2build.h>
    #include FT_FREETYPE_H
    #undef SIZEOF_LONG
    #define SIZEOF_LONG SIZEOF_LONG_OLD
    #undef SIZEOF_LONG_OLD
#else
    #include <ft2build.h>
    #include FT_FREETYPE_H
    #undef SIZEOF_LONG
#endif


#endif
