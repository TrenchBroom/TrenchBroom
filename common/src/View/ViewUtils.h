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

#ifndef TrenchBroom_ViewUtils
#define TrenchBroom_ViewUtils

#include "View/ViewTypes.h"
#include "StringUtils.h"

class QWidget;
class QString;
class QStringList;

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class EntityModel;
        class EntityModelManager;
        struct ModelSpecification;
    }

    namespace View {
        void combineFlags(size_t numFlags, int newFlagValue, int& setFlags, int& mixedFlags);

        bool loadTextureCollection(MapDocumentWPtr document, QWidget* parent, const QString& wxPath);
        size_t loadTextureCollections(MapDocumentWPtr document, QWidget* parent, const QStringList& wxPaths);

        bool loadEntityDefinitionFile(MapDocumentWPtr document, QWidget* parent, const QString& wxPath);
        size_t loadEntityDefinitionFile(MapDocumentWPtr document, QWidget* parent, const QStringList& wxPaths);

        String queryGroupName(QWidget* parent);
    }
}

#endif /* defined(TrenchBroom_ViewUtils) */
