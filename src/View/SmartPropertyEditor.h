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

#ifndef __TrenchBroom__SmartPropertyEditor__
#define __TrenchBroom__SmartPropertyEditor__

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class SelectionResult;
    }
    
    namespace View {
        class SmartPropertyEditor {
        private:
            View::MapDocumentPtr m_document;
            View::ControllerPtr m_controller;
            
            Model::PropertyKey m_key;
        public:
            SmartPropertyEditor(View::MapDocumentPtr document, View::ControllerPtr controller);
            virtual ~SmartPropertyEditor();
            
            wxWindow* activate(wxWindow* parent, const Model::PropertyKey& key);
            void deactivate();
        protected:
            void addOrUpdateProperty(const Model::PropertyValue& value);
        private:
            void bindObservers();
            void unbindObservers();
            
            void selectionDidChange(const Model::SelectionResult& result);
            void objectDidChange(Model::Object* object);
            
            wxWindow* createVisual(wxWindow* parent);
            void destroyVisual();
            void updateVisual();
            
            virtual wxWindow* doCreateVisual(wxWindow* parent) = 0;
            virtual void doDestroyVisual() = 0;
            virtual void doUpdateVisual(const Model::PropertyKey& key, const Model::EntityList& entities) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditor__) */
