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

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Controller {
        class Command {
        public:
            typedef std::tr1::shared_ptr<Command> Ptr;
            typedef std::vector<Ptr> List;
            typedef size_t CommandType;
            
            typedef enum {
                CommandState_Default,
                CommandState_Doing,
                CommandState_Done,
                CommandState_Undoing
            } CommandState;
        private:
            CommandType m_type;
            CommandState m_state;
            String m_name;
            bool m_undoable;
            bool m_modifiesDocument;
        public:
            static CommandType freeType();
            
            Command(CommandType type, const String& name, bool undoable, bool modifiesDocument);
            virtual ~Command();

            CommandType type() const;
            CommandState state() const;
            const String& name() const;
            bool undoable() const;
            
            bool performDo();
            bool performUndo();
            
            bool modifiesDocument() const;
            
            bool isRepeatDelimiter() const;
            bool isRepeatable(View::MapDocumentSPtr document) const;
            Ptr repeat(View::MapDocumentSPtr document) const;
            bool collateWith(Ptr command);
            
            template <class T>
            static std::tr1::shared_ptr<T> cast(Ptr& command) {
                return std::tr1::static_pointer_cast<T>(command);
            }
        private:
            virtual bool doPerformDo() = 0;
            virtual bool doPerformUndo();
            
            virtual bool doIsRepeatDelimiter() const;
            virtual bool doIsRepeatable(View::MapDocumentSPtr document) const = 0;
            virtual Command* doRepeat(View::MapDocumentSPtr document) const;
            virtual bool doCollateWith(Ptr command) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Command__) */
