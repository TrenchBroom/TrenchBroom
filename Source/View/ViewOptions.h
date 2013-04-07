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

#include "Utility/String.h"

namespace TrenchBroom {
    namespace View {
        class ViewOptions {
        public:
            typedef enum {
                Textured,
                Flat,
                Discard
            } FaceRenderMode;
        private:
            String m_filterPattern;
            bool m_showEntities;
            bool m_showEntityModels;
            bool m_showEntityBounds;
            bool m_showEntityClassnames;
            bool m_showBrushes;
            bool m_showClipBrushes;
            bool m_showSkipBrushes;
            FaceRenderMode m_faceRenderMode;
            bool m_renderEdges;
            bool m_renderSelection;
            bool m_shadeFaces;
            bool m_useFog;
        public:
            ViewOptions() :
            m_filterPattern(""),
            m_showEntities(true),
            m_showEntityModels(true),
            m_showEntityBounds(true),
            m_showEntityClassnames(true),
            m_showBrushes(true),
            m_showClipBrushes(true),
            m_showSkipBrushes(true),
            m_faceRenderMode(Textured),
            m_renderEdges(true),
            m_renderSelection(true),
            m_shadeFaces(true),
            m_useFog(false) {}

            inline const String& filterPattern() const {
                return m_filterPattern;
            }

            inline void setFilterPattern(const String& filterPattern) {
                m_filterPattern = Utility::trim(filterPattern);
            }

            inline bool showEntities() const {
                return m_showEntities;
            }

            inline void setShowEntities(bool showEntities) {
                m_showEntities = showEntities;
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

            inline bool showBrushes() const {
                return m_showBrushes;
            }

            inline void setShowBrushes(bool showBrushes) {
                m_showBrushes = showBrushes;
            }

            inline bool showClipBrushes() const {
                return m_showClipBrushes;
            }

            inline void setShowClipBrushes(bool showClipBrushes) {
                m_showClipBrushes = showClipBrushes;
            }

            inline bool showSkipBrushes() const {
                return m_showSkipBrushes;
            }

            inline void setShowSkipBrushes(bool showSkipBrushes) {
                m_showSkipBrushes = showSkipBrushes;
            }

            inline FaceRenderMode faceRenderMode() const {
                return m_faceRenderMode;
            }

            inline void setFaceRenderMode(FaceRenderMode faceRenderMode) {
                m_faceRenderMode = faceRenderMode;
            }

            inline bool renderEdges() const {
                return m_renderEdges;
            }

            inline void setRenderEdges(bool renderEdges) {
                m_renderEdges = renderEdges;
            }

            inline bool renderSelection() const {
                return m_renderSelection;
            }

            inline void setRenderSelection(bool renderSelection) {
                m_renderSelection = renderSelection;
            }

            inline bool shadeFaces() const {
                return m_shadeFaces;
            }

            inline void setShadeFaces(bool shadeFaces) {
                m_shadeFaces = shadeFaces;
            }

            inline bool useFog() const {
                return m_useFog;
            }

            inline void setUseFog(bool useFog) {
                m_useFog = useFog;
            }
        };
    }
}

#endif
