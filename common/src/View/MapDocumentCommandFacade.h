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

#ifndef __TrenchBroom__MapDocumentCommandFacade__
#define __TrenchBroom__MapDocumentCommandFacade__

#include "View/CommandProcessor.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade : public MapDocument {
        private:
            CommandProcessor m_commandProcessor;
        public:
            static MapDocumentSPtr newMapDocument();
        private:
            MapDocumentCommandFacade();
        public: // selection modification
            void performSelect(const Model::NodeList& nodes);
            void performSelect(const Model::BrushFaceList& faces);
            void performSelectAllNodes();
            void performSelectAllBrushFaces();
            void performConvertToBrushFaceSelection();
            
            void performDeselect(const Model::NodeList& nodes);
            void performDeselect(const Model::BrushFaceList& faces);
            void performDeselectAll();
        private:
            void deselectAllNodes();
            void deselectAllBrushFaces();
        private: // implement MapDocument interface
            bool doCanUndoLastCommand() const;
            bool doCanRedoNextCommand() const;
            const String& doGetLastCommandName() const;
            const String& doGetNextCommandName() const;
            void doUndoLastCommand();
            void doRedoNextCommand();
            
            void doBeginTransaction(const String& name);
            void doEndTransaction();
            void doRollbackTransaction();

            bool doSubmit(UndoableCommand* command);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocumentCommandFacade__) */
