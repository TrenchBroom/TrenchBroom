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

#ifndef TrenchBroom_UpdateEntitySpawnflagCommand
#define TrenchBroom_UpdateEntitySpawnflagCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        class UpdateEntitySpawnflagCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<UpdateEntitySpawnflagCommand> Ptr;
        private:
            bool m_setFlag;
            Model::AttributeName m_name;
            size_t m_flagIndex;
        public:
            static Ptr update(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag);
        private:
            UpdateEntitySpawnflagCommand(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag);
            static String makeName(const bool setFlag);

            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);

            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(UpdateEntitySpawnflagCommand) */
