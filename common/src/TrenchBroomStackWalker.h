/*
 Copyright (C) 2016 Eric Wasylishen
 
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

#ifndef TrenchBroomStackWalker_h
#define TrenchBroomStackWalker_h

#include "StringUtils.h"

#include <wx/wx.h>
#include <wx/stackwalk.h>

namespace TrenchBroom {
    
    class TrenchBroomStackTrace {
        friend class TrenchBroomStackWalker;
    private:
        std::vector<void *> m_frames;
        TrenchBroomStackTrace(std::vector<void *> frames) : m_frames(frames) {}
    public:
        String asString();
    };
    
    class TrenchBroomStackWalker {
    public:
        static TrenchBroomStackTrace getStackTrace();
    };
}

#endif
