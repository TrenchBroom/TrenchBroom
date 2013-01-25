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

#ifndef __TrenchBroom__SmartPropertyEditor__
#define __TrenchBroom__SmartPropertyEditor__

#include "Model/EntityProperty.h"
#include "Model/EntityTypes.h"
#include "Utility/String.h"

#include <map>

class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace View {
        class SmartPropertyEditorManager;
        
        class SmartPropertyEditor {
        private:
            bool m_active;
            const Model::PropertyKey m_property;
            SmartPropertyEditorManager& m_manager;
        protected:
            virtual wxWindow* createVisual(wxWindow* parent) = 0;
            virtual void destroyVisual() = 0;
            virtual void updateVisual() = 0;
            
            Model::MapDocument& document() const;
            const Model::EntityList& selectedEntities() const;
        public:
            SmartPropertyEditor(SmartPropertyEditorManager& manager);
            virtual ~SmartPropertyEditor() {}
            
            void activate(wxWindow* parent);
            void deactivate();
            void update();
        };
        
        class DefaultPropertyEditor : public SmartPropertyEditor {
        private:
            wxStaticText* m_text;
        protected:
            virtual wxWindow* createVisual(wxWindow* parent);
            virtual void destroyVisual();
            virtual void updateVisual();
        public:
            DefaultPropertyEditor(SmartPropertyEditorManager& manager);
        };
        
        class SmartPropertyEditorManager {
        private:
            typedef std::map<Model::PropertyKey, SmartPropertyEditor*> EditorMap;
            
            Model::MapDocument& m_document;
            wxWindow* m_panel;

            EditorMap m_editors;
            SmartPropertyEditor* m_defaultEditor;
            SmartPropertyEditor* m_activeEditor;
            
            void activateEditor(SmartPropertyEditor* editor);
            void deactivateEditor();
        public:
            SmartPropertyEditorManager(wxWindow* parent, Model::MapDocument& document);
            ~SmartPropertyEditorManager();
            
            void selectEditor(const Model::PropertyKey& key);
            void update();
        
            inline Model::MapDocument& document() const {
                return m_document;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditor__) */
