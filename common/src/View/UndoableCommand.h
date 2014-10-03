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

#ifndef __TrenchBroom__UndoableCommand__
#define __TrenchBroom__UndoableCommand__

#include "View/Command.h"

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        class UndoableCommand : public Command {
        public:
            UndoableCommand(CommandType type, const String& name);
            virtual ~UndoableCommand();

            bool performUndo(MapDocumentCommandFacade* document);

            bool isRepeatDelimiter() const;
            bool isRepeatable(View::MapDocumentSPtr document) const;
            UndoableCommand* repeat(View::MapDocumentSPtr document) const;
            
            bool collateWith(UndoableCommand* command);
        private:
            virtual bool doPerformUndo(MapDocumentCommandFacade* document) = 0;
            
            virtual bool doIsRepeatDelimiter() const;
            virtual bool doIsRepeatable(View::MapDocumentSPtr document) const = 0;
            virtual UndoableCommand* doRepeat(View::MapDocumentSPtr document) const;
            
            virtual bool doCollateWith(UndoableCommand* command) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__UndoableCommand__) */
