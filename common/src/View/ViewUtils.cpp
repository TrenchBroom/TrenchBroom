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

#include "ViewUtils.h"

#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/PathQt.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>

#include <memory>

#include <QWidget>
#include <QString>
#include <QInputDialog>
#include <QMessageBox>

namespace TrenchBroom {
    namespace View {
        void combineFlags(const size_t numFlags, const int newFlagValue, int& setFlags, int& mixedFlags) {
            for (size_t i = 0; i < numFlags; ++i) {
                const bool alreadySet = (newFlagValue & (1 << i)) != 0;
                const bool willBeSet = (setFlags & (1 << i)) != 0;
                if (alreadySet == willBeSet)
                    continue;

                setFlags &= ~(1 << i);
                mixedFlags |= (1 << i);
            }
        }

        bool loadTextureCollection(std::weak_ptr<MapDocument> document, QWidget* parent, const QString& path) {
            return loadTextureCollections(document, parent, QStringList { path }) == 1;
        }

        size_t loadTextureCollections(std::weak_ptr<MapDocument> i_document, QWidget* parent, const QStringList& pathStrs) {
            if (pathStrs.empty()) {
                return 0;
            }

            size_t count = 0;

            auto document = kdl::mem_lock(i_document);
            std::vector<IO::Path> collections = document->enabledTextureCollections();

            auto game = document->game();
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(game->gameName());
            const IO::Path docPath = document->path();

            for (int i = 0; i < pathStrs.size(); ++i) {
                const QString& pathStr = pathStrs[i];
                const IO::Path absPath = IO::pathFromQString(pathStr);
                if (game->isTextureCollection(absPath)) {
                    ChoosePathTypeDialog pathDialog(parent->window(), absPath, docPath, gamePath);
                    const int result = pathDialog.exec();
                    if (result == QDialog::Rejected) {
                        return 0;
                    } else if (result == QDialog::Accepted) {
                        collections.push_back(pathDialog.path());
                        ++count;
                    }
                }
            }

            document->setEnabledTextureCollections(collections);

            return count;
        }

        bool loadEntityDefinitionFile(std::weak_ptr<MapDocument> document, QWidget* parent, const QString& path) {
            return loadEntityDefinitionFile(document, parent, QStringList { path }) == 0;
        }

        size_t loadEntityDefinitionFile(std::weak_ptr<MapDocument> i_document, QWidget* parent, const QStringList& pathStrs) {
            if (pathStrs.empty()) {
                return 0;
            }

            auto document = kdl::mem_lock(i_document);
            auto game = document->game();
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(game->gameName());
            const IO::Path docPath = document->path();

            try {
                for (int i = 0; i < pathStrs.size(); ++i) {
                    const QString& pathStr = pathStrs[i];
                    const IO::Path absPath = IO::pathFromQString(pathStr);
                    if (game->isEntityDefinitionFile(absPath)) {
                        ChoosePathTypeDialog pathDialog(parent->window(), absPath, docPath, gamePath);
                        if (pathDialog.exec() == QDialog::Accepted) {
                            const Assets::EntityDefinitionFileSpec spec = Assets::EntityDefinitionFileSpec::external(pathDialog.path());
                            document->setEntityDefinitionFile(spec);
                            return static_cast<size_t>(i);
                        }
                    }
                }
            } catch (...) {
                throw;
            }

            return static_cast<size_t>(pathStrs.size());
        }

        std::string queryGroupName(QWidget* parent) {
            while (true) {
                bool ok;
                QString text = QInputDialog::getText(parent, "Enter a name", "Group Name", QLineEdit::Normal, "Unnamed", &ok);

                if (!ok) {
                    return "";
                }

                const auto name = text.toStdString();
                if (kdl::str_is_blank(name)) {
                    if (QMessageBox::warning(parent, "Error", "Group names cannot be blank.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)
                        return "";
                } else if (kdl::ci::str_contains(name, "\"")) {
                    if (QMessageBox::warning(parent, "Error", "Group names cannot contain double quotes.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)
                        return "";
                } else {
                    return name;
                }
            }
        }
    }
}
