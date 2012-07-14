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

#ifndef TrenchBroom_TransientOptions_h
#define TrenchBroom_TransientOptions_h

#include "Utilities/Event.h"

namespace TrenchBroom {
    namespace Controller {

        typedef enum {
            TB_RM_TEXTURED,
            TB_RM_FLAT,
            TB_RM_WIREFRAME
        } ERenderMode;
        
        typedef enum {
            IM_NONE, // no isolation
            IM_WIREFRAME, // render unselected geometry as wireframe, ignore while picking
            IM_DISCARD // do not render unselected geometry, ignore while picking
        } EIsolationMode;

        class TransientOptions {
        private:
            ERenderMode m_renderMode;
            EIsolationMode m_isolationMode;
            bool m_renderEntities;
            bool m_renderEntityClassnames;
            bool m_renderBrushes;
            bool m_renderOrigin;
            float m_originAxisLength;
            bool m_renderSizeGuides;
            bool m_lockTextures;
        public:
            typedef Event<const TransientOptions&> OptionsEvent;
            OptionsEvent optionsDidChange;

            TransientOptions() : m_renderMode(TB_RM_TEXTURED), m_isolationMode(IM_NONE), m_renderEntities(true), m_renderEntityClassnames(true), m_renderBrushes(true), m_renderOrigin(true), m_originAxisLength(64), m_renderSizeGuides(true), m_lockTextures(true) {}
            
            ERenderMode renderMode() const {
                return m_renderMode;
            }
            
            void setRenderMode(ERenderMode renderMode) {
                if (renderMode == m_renderMode)
                    return;
                
                m_renderMode = renderMode;
                optionsDidChange(*this);
            }

            EIsolationMode isolationMode() const {
                return m_isolationMode;
            }
            
            void setIsolationMode(EIsolationMode isolationMode) {
                if (m_isolationMode == isolationMode)
                    return;
                
                m_isolationMode = isolationMode;
                optionsDidChange(*this);
            }
            
            bool renderEntities() const {
                return m_renderEntities;
            }
            
            void setRenderEntities(bool renderEntities) {
                if (renderEntities == m_renderEntities)
                    return;
                
                m_renderEntities = renderEntities;
                optionsDidChange(*this);
            }
            
            bool renderEntityClassnames() const {
                return m_renderEntityClassnames;
            }
            
            void setRenderEntityClassnames(bool renderEntityClassnames) {
                if (renderEntityClassnames == m_renderEntityClassnames)
                    return;
                
                m_renderEntityClassnames = renderEntityClassnames;
                optionsDidChange(*this);
            }
            
            bool renderBrushes() const {
                return m_renderBrushes;
            }
            
            void setRenderBrushes(bool renderBrushes) {
                if (renderBrushes == m_renderBrushes)
                    return;
                
                m_renderBrushes = renderBrushes;
                optionsDidChange(*this);
            }
            
            bool renderOrigin() const {
                return m_renderOrigin;
            }
            
            void setRenderOrigin(bool renderOrigin) {
                if (renderOrigin == m_renderOrigin)
                    return;
                
                m_renderOrigin = renderOrigin;
                optionsDidChange(*this);
            }
            
            float originAxisLength() const {
                return m_originAxisLength;
            }
            
            void setOriginAxisLength(float originAxisLength) {
                if (originAxisLength == m_originAxisLength)
                    return;
                
                m_originAxisLength = originAxisLength;
                optionsDidChange(*this);
            }
            
            bool renderSizeGuides() const {
                return m_renderSizeGuides;
            }
            
            void setRenderSizeGuides(bool renderSizeGuides) {
                if (renderSizeGuides == m_renderSizeGuides)
                    return;
                
                m_renderSizeGuides = renderSizeGuides;
                optionsDidChange(*this);
            }
            
            bool lockTextures() const {
                return m_lockTextures;
            }
            
            void setLockTextures(bool lockTextures) {
                if (lockTextures == m_lockTextures)
                    return;
                
                m_lockTextures = lockTextures;
                optionsDidChange(*this);
            }
        };
    }
}

#endif
