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

#include "Exceptions.h"
#include "Logger.h"
#include "Assets/EntityModelManager.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/ModelDefinition.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
//#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"

#include <QWidget>
#include <QString>
#include <QInputDialog>
#include <QMessageBox>

namespace TrenchBroom {
    namespace View {
        Assets::EntityModel* safeGetModel(Assets::EntityModelManager& manager, const Assets::ModelSpecification& spec, Logger& logger) {
            try {
                return manager.model(spec.path);
            } catch (const GameException& e) {
                logger.error(String(e.what()));
                return nullptr;
            }
        }

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

        // FIXME:
#if 0
        size_t loadDroppedFiles(MapDocumentWPtr document, QWidget* parent, const wxArrayString& wxPaths) {
            size_t count = 0;
            count += loadTextureCollections(document, parent, wxPaths);
            if (loadEntityDefinitionFile(document, parent, wxPaths) < wxPaths.size())
                ++count;
            return count > 0;
        }

        bool loadTextureCollection(MapDocumentWPtr document, QWidget* parent, const QString& wxPath) {
            wxArrayString wxPaths;
            wxPaths.Add(wxPath);
            return loadTextureCollections(document, parent, wxPaths) == 1;
        }
        
        size_t loadTextureCollections(MapDocumentWPtr i_document, QWidget* parent, const wxArrayString& wxPaths) {
            if (wxPaths.empty())
                return 0;
            
            size_t count = 0;
            
            MapDocumentSPtr document = lock(i_document);
            IO::Path::List collections = document->enabledTextureCollections();
            
            Model::GameSPtr game = document->game();
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(game->gameName());
            const IO::Path docPath = document->path();

            for (size_t i = 0; i < wxPaths.size(); ++i) {
                const QString& wxPath = wxPaths[i];
                const IO::Path absPath(wxPath.ToStdString());
                if (game->isTextureCollection(absPath)) {
                    assert(0); // FIXME:
#if 0
                    ChoosePathTypeDialog pathDialog(wxGetTopLevelParent(parent), absPath, docPath, gamePath);
                    const int result = pathDialog.ShowModal();
                    if (result == wxID_CANCEL) {
                        return 0;
                    } else if (result == wxID_OK) {
                        collections.push_back(pathDialog.path());
                        ++count;
                    }
#endif
                }
            }

            document->setEnabledTextureCollections(collections);
            
            return count;
        }

        bool loadEntityDefinitionFile(MapDocumentWPtr document, QWidget* parent, const QString& wxPath) {
            wxArrayString wxPaths;
            wxPaths.Add(wxPath);
            return loadEntityDefinitionFile(document, parent, wxPaths) == 0;
        }

        size_t loadEntityDefinitionFile(MapDocumentWPtr i_document, QWidget* parent, const wxArrayString& wxPaths) {
            if (wxPaths.empty())
                return 0;
            
            MapDocumentSPtr document = lock(i_document);
            Model::GameSPtr game = document->game();
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(game->gameName());
            const IO::Path docPath = document->path();
            
            try {
                for (size_t i = 0; i < wxPaths.size(); ++i) {
                    const QString& wxPath = wxPaths[i];
                    const IO::Path absPath(wxPath.ToStdString());
                    if (game->isEntityDefinitionFile(absPath)) {
                        // FIXME:
                        assert(0);
#if 0
                        ChoosePathTypeDialog pathDialog(wxGetTopLevelParent(parent), absPath, docPath, gamePath);
                        if (pathDialog.ShowModal() == wxID_OK) {
                            const Assets::EntityDefinitionFileSpec spec = Assets::EntityDefinitionFileSpec::external(pathDialog.path());
                            document->setEntityDefinitionFile(spec);
                            return i;
                        }
#endif
                    }
                }
            } catch (...) {
                throw;
            }
            
            return wxPaths.size();
        }
#endif

        String queryGroupName(QWidget* parent) {
            while (true) {
                bool ok;
                QString text = QInputDialog::getText(parent, "Enter a name", "Group Name", QLineEdit::Normal, "Unnamed", &ok);

                if (!ok) {
                    return "";
                }

                const auto name = text.toStdString();
                if (StringUtils::isBlank(name)) {
                    if (QMessageBox::warning(parent, "Error", "Group names cannot be blank.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)
                        return "";
                } else if (StringUtils::containsCaseInsensitive(name, "\"")) {
                    if (QMessageBox::warning(parent, "Error", "Group names cannot contain double quotes.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)
                        return "";
                } else {
                    return name;
                }
            }
        }
    }
}
