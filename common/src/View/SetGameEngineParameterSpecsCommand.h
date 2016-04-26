/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef SetGameEngineParameterSpecsCommand_h
#define SetGameEngineParameterSpecsCommand_h

#include "Macros.h"
#include "StringUtils.h"
#include "View/Command.h"

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        class SetGameEngineParameterSpecsCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<SetGameEngineParameterSpecsCommand> Ptr;
        private:
            ::StringMap m_specs;
        public:
            static Ptr set(const ::StringMap& specs);
        private:
            SetGameEngineParameterSpecsCommand(const String& name, const ::StringMap& specs);
            
            bool doPerformDo(MapDocumentCommandFacade* document);
            
            deleteCopyAndAssignment(SetGameEngineParameterSpecsCommand)
        };
    }
}

#endif /* SetGameEngineParameterSpecsCommand_h */
