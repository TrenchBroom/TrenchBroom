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

#ifndef __TrenchBroom__DocumentViewHolder__
#define __TrenchBroom__DocumentViewHolder__

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace View {
        class EditorView;
        
        class DocumentViewHolder {
        private:
            bool m_valid;
            Model::MapDocument* m_document;
            EditorView* m_view;
        public:
            DocumentViewHolder(Model::MapDocument* document, EditorView* view) :
            m_valid(true),
            m_document(document),
            m_view(view) {}
            
            ~DocumentViewHolder() {
                m_valid = false;
                m_document = NULL;
                m_view = NULL;
            }
            
            inline Model::MapDocument& document() {
                assert(m_valid);
                return *m_document;
            }
            
            inline EditorView& view() {
                assert(m_valid);
                return *m_view;
            }

            inline bool valid() {
                return m_valid;
            }
            
            inline void invalidate() {
                m_valid = false;
                m_document = NULL;
                m_view = NULL;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__DocumentViewHolder__) */
