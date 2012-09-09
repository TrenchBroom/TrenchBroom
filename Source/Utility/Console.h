/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Console__
#define __TrenchBroom__Console__

#include "Utility/String.h"

#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace Utility {
        class Console {
        protected:
            StringStream m_buffer;
            wxTextCtrl* m_textCtrl;
            
            void formatMessage(const char* format, va_list arguments, String& result);
        public:
            Console() : m_textCtrl(NULL) {}
            
            void setTextCtrl(wxTextCtrl* textCtrl);
            
            void log(const String& message, bool setDefaultColor = true);
            
            void debug(const String& message);
            void debug(const char* format, ...);
            void info(const String& message);
            void info(const char* format, ...);
            void warn(const String& message);
            void warn(const char* format, ...);
            void error(const String& message);
            void error(const char* format, ...);
        };
    }
}

#endif /* defined(__TrenchBroom__Console__) */
