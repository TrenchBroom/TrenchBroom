/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Command__
#define __TrenchBroom__Command__

#include "SharedPointer.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace Controller {
        class Command {
        public:
            typedef std::tr1::shared_ptr<Command> Ptr;
        private:
            String m_name;
            bool m_canRollback;
        public:
            Command(const String& name, const bool canRollback);
            
            bool canRollback() const;
            const String& name() const;
            bool execute();
            bool rollback();
        private:
            virtual bool doExecute() = 0;
            virtual bool doRollback() = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Command__) */
