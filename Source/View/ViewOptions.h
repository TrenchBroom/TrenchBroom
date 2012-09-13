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

#ifndef TrenchBroom_ViewOptions_h
#define TrenchBroom_ViewOptions_h

#include "Utilities/String.h"

namespace TrenchBroom {
    namespace View {
        class ViewOptions {
        private:
            bool m_showEntities;
            bool m_showBrushes;
            String m_filterPattern;
            bool m_showEntityModels;
            bool m_showEntityBounds;
            bool m_showEntityClassnames;
        public:
            ViewOptions() :
            m_showEntities(true),
            m_showBrushes(true),
            m_filterPattern(""),
            m_showEntityModels(true),
            m_showEntityBounds(true),
            m_showEntityClassnames(true) {}
            
            inline bool showEntities() const {
                return m_showEntities;
            }
            
            inline void setShowEntities(bool showEntities) {
                m_showEntities = showEntities;
            }
            
            inline bool showBrushes() const {
                return m_showBrushes;
            }
            
            inline void setShowBrushes(bool showBrushes) {
                m_showBrushes = showBrushes;
            }
            
            inline const String& filterPattern() const {
                return m_filterPattern;
            }
            
            inline void setFilterPattern(const String& filterPattern) const {
                m_filterPattern = Utility::trim(filterPattern);
            }
            
            inline bool showEntityModels() const {
                return m_showEntityModels;
            }
            
            inline void setShowEntityModels(bool showEntityModels) {
                m_showEntityModels = showEntityModels;
            }
            
            inline bool showEntityBounds() const {
                return m_showEntityBounds;
            }
            
            inline void setShowEntityBounds(bool showEntityBounds) {
                m_showEntityBounds = showEntityBounds;
            }
            
            inline bool showEntityClassnames() const {
                return m_showEntityClassnames;
            }
            
            inline void setShowEntityClassnames(bool showEntityClassnames) {
                m_showEntityClassnames = showEntityClassnames;
            }
        };
    }
}

#endif
