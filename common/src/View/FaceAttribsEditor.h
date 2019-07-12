/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_FaceAttribsEditor
#define TrenchBroom_FaceAttribsEditor

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QGridLayout;

namespace TrenchBroom {
    namespace View {
        class FlagChangedCommand;
        class FlagsPopupEditor;
        class GLContextManager;
        class Selection;
        class SpinControl;
        class UVEditor;

        class FaceAttribsEditor : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            Model::BrushFaceList m_faces;

            UVEditor* m_uvEditor;
            QLabel* m_textureName;
            QLabel* m_textureSize;
            SpinControl* m_xOffsetEditor;
            SpinControl* m_yOffsetEditor;
            SpinControl* m_xScaleEditor;
            SpinControl* m_yScaleEditor;
            SpinControl* m_rotationEditor;
            QLabel* m_surfaceValueLabel;
            SpinControl* m_surfaceValueEditor;
            QGridLayout* m_faceAttribsLayout;

            QLabel* m_surfaceFlagsLabel;
            FlagsPopupEditor* m_surfaceFlagsEditor;
            QLabel* m_contentFlagsLabel;
            FlagsPopupEditor* m_contentFlagsEditor;

            QLabel* m_colorLabel;
            QLineEdit* m_colorEditor;
        public:
            FaceAttribsEditor(MapDocumentWPtr document, GLContextManager& contextManager, QWidget* parent = nullptr);
            ~FaceAttribsEditor() override;

            bool cancelMouseDrag();
        private:
            void xOffsetChanged(double value);
            void yOffsetChanged(double value);
            void rotationChanged(double value);
            void xScaleChanged(double value);
            void yScaleChanged(double value);
            void surfaceFlagChanged(size_t index, int setFlag, int mixedFlag);
            void contentFlagChanged(size_t index, int setFlag, int mixedFlag);
            void surfaceValueChanged(double value);
            void colorValueChanged(const QString& text);
            void gridDidChange();
        private:
            void createGui(GLContextManager& contextManager);
            void bindEvents();

            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void brushFacesDidChange(const Model::BrushFaceList& faces);
            void selectionDidChange(const Selection& selection);
            void textureCollectionsDidChange();

            void updateControls();

            bool hasSurfaceAttribs() const;
            void showSurfaceAttribEditors();
            void hideSurfaceAttribEditors();

            bool hasColorAttribs() const;
            void showColorAttribEditor();
            void hideColorAttribEditor();

            void getSurfaceFlags(QStringList& names, QStringList& descriptions) const;
            void getContentFlags(QStringList& names, QStringList& descriptions) const;
        };
    }
}

#endif /* defined(TrenchBroom_FaceAttribsEditor) */
