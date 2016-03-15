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


#include "TrenchBroomStackWalker.h"

namespace TrenchBroom {
    static String stackFrameToString(const wxStackFrame &frame) {
        StringStream ss;
        ss << frame.GetLevel() << "\t";
        ss << frame.GetModule() << "\t";
        ss << frame.GetAddress() << " ";
        ss << frame.GetName() << " ";
        if (frame.HasSourceLocation()) {
            ss << "(" << frame.GetFileName() << ":" << frame.GetLine() << ") ";
        }
        return ss.str();
    }

    String TrenchBroomStackTrace::asString() {
        StringStream ss;
        std::vector<wxStackFrame>::const_iterator it;
        for (it = m_frames.begin(); it != m_frames.end(); it++) {
            String frameStr = stackFrameToString(*it);
            ss << frameStr << std::endl;
        }
        return ss.str();
    }
    
    void TrenchBroomStackWalker::OnStackFrame(const wxStackFrame &frame) {
        m_frames.push_back(frame);
    }

    TrenchBroomStackTrace TrenchBroomStackWalker::getStackTrace() {
        TrenchBroomStackWalker w;
        w.Walk();
        
        TrenchBroomStackTrace trace(w.m_frames);
        return trace;
    }

    TrenchBroomStackTrace TrenchBroomStackWalker::getStackTraceFromOnFatalException() {
        TrenchBroomStackWalker w;
        w.WalkFromException();
        
        TrenchBroomStackTrace trace(w.m_frames);
        return trace;
    }
}
