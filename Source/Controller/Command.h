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

#ifndef __TrenchBroom__Command__
#define __TrenchBroom__Command__

#include <wx/cmdproc.h>

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Controller {
        class Command : public wxCommand {
        public:
            typedef enum {
                LoadMap,
                ClearMap,
                ChangeEditState,
                InvalidateRendererEntityState,
                InvalidateRendererBrushState,
                InvalidateRendererState,
                InvalidateEntityModelRendererCache,
                SetFaceAttribute,
                AddTextureCollection,
                RemoveTextureCollection,
                CreateEntity,
                SetEntityPropertyValue,
                SetEntityPropertyKey,
                RemoveEntityProperty,
                MoveObjects,
                DeleteObjects,
                UpdateFigures
            } Type;
            
            typedef enum {
                None,
                Doing,
                Done,
                Undoing,
                Undone
            } State;
        private:
            Type m_type;
            State m_state;
        protected:
            virtual bool performDo() { return true; }
            virtual bool performUndo() { return true; }
        public:
            Command(Type type) :
            wxCommand(false, ""),
            m_type(type),
            m_state(None) {}

            Command(Type type, bool undoable, const wxString& name) :
            wxCommand(undoable, name),
            m_type(type),
            m_state(None) {}
            
            inline Type type() const {
                return m_type;
            }
            
            inline State state() const {
                return m_state;
            }
            
            bool Do() {
                State previous = m_state;
                m_state = Doing;
                bool result = performDo();
                if (result)
                    m_state = Done;
                else
                    m_state = previous;
                return result;
            }
            
            bool Undo() {
                State previous = m_state;
                m_state = Undoing;
                bool result = performUndo();
                if (result)
                    m_state = Undone;
                else
                    m_state = previous;
                return result;
            }
        };
        
        class DocumentCommand : public Command {
            Model::MapDocument& m_document;
        protected:
            inline Model::MapDocument& document() const {
                return m_document;
            }
        public:
            DocumentCommand(Type type, Model::MapDocument& document, bool undoable, const wxString& name) :
            Command(type, undoable, name),
            m_document(document) {}
            
        };
    }
}

#endif /* defined(__TrenchBroom__Command__) */
