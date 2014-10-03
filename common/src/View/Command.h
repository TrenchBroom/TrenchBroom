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

#ifndef __TrenchBroom__Command__
#define __TrenchBroom__Command__

#include "StringUtils.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        class Command {
        public:
            typedef size_t CommandType;
            
            typedef enum {
                CommandState_Default,
                CommandState_Doing,
                CommandState_Done,
                CommandState_Undoing
            } CommandState;
        protected:
            CommandType m_type;
            CommandState m_state;
            String m_name;
        public:
            static CommandType freeType();
            
            Command(CommandType type, const String& name);
            virtual ~Command();

            CommandType type() const;
            CommandState state() const;
            const String& name() const;
            
            bool performDo(MapDocumentCommandFacade* document);
        private:
            virtual bool doPerformDo(MapDocumentCommandFacade* document) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Command__) */
