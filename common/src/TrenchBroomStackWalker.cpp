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

#include <execinfo.h>
#include "TrenchBroomStackWalker.h"

namespace TrenchBroom {
    
    String TrenchBroomStackTrace::asString() {
        if (m_frames.empty())
            return "";
        
        StringStream ss;
        char **strs = backtrace_symbols(&m_frames.front(), static_cast<int>(m_frames.size()));
        for (size_t i = 0; i < m_frames.size(); i++) {
            ss << strs[i] << std::endl;
        }
        free(strs);
        return ss.str();
    }
    
    TrenchBroomStackTrace TrenchBroomStackWalker::getStackTrace() {
        const int MaxDepth = 256;
        void *callstack[MaxDepth];
        const int frames = backtrace(callstack, MaxDepth);

        // copy into a vector
        std::vector<void *> framesVec(callstack, callstack + frames);
        TrenchBroomStackTrace trace(framesVec);
        return trace;
    }
}
