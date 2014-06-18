/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Assets/ModelDefinition.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        Assets::EntityModel* safeGetModel(Assets::EntityModelManager& manager, const Assets::ModelSpecification& spec, Logger& logger) {
            try {
                return manager.model(spec.path);
            } catch (const GameException& e) {
                logger.error(String(e.what()));
                return NULL;
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

        bool loadTextureCollection(MapDocumentWPtr document, ControllerWPtr controller, wxWindow* parent, const wxString& wxPath) {
            wxArrayString wxPaths;
            wxPaths.Add(wxPath);
            return loadTextureCollections(document, controller, parent, wxPaths) == 1;
        }

        size_t loadTextureCollections(MapDocumentWPtr i_document, ControllerWPtr i_controller, wxWindow* parent, const wxArrayString& wxPaths) {
            if (wxPaths.empty())
                return 0;
            
            size_t count = 0;
            
            MapDocumentSPtr document = lock(i_document);
            Model::GamePtr game = document->game();
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(game->gameName());
            const IO::Path docPath = document->path();

            ControllerSPtr controller = lock(i_controller);
            UndoableCommandGroup commandGroup(controller);
            try {
                for (size_t i = 0; i < wxPaths.size(); ++i) {
                    const wxString& wxPath = wxPaths[i];
                    const IO::Path absPath(wxPath.ToStdString());
                    if (game->isTextureCollection(absPath)) {
                        ChoosePathTypeDialog pathDialog(wxGetTopLevelParent(parent), absPath, docPath, gamePath);
                        if (pathDialog.ShowModal() == wxID_OK) {
                            const IO::Path collectionPath = pathDialog.path();
                            controller->addTextureCollection(collectionPath.asString());
                            ++count;
                        }
                    }
                }
            } catch (...) {
                commandGroup.rollback();
                throw;
            }
            
            return count;
        }

        bool loadEntityDefinitionFile(MapDocumentWPtr document, ControllerWPtr controller, wxWindow* parent, const wxString& wxPath) {
            wxArrayString wxPaths;
            wxPaths.Add(wxPath);
            return loadEntityDefinitionFile(document, controller, parent, wxPaths) == 0;
        }
        
        size_t loadEntityDefinitionFile(MapDocumentWPtr i_document, ControllerWPtr i_controller, wxWindow* parent, const wxArrayString& wxPaths) {
            if (wxPaths.empty())
                return 0;
            
            MapDocumentSPtr document = lock(i_document);
            Model::GamePtr game = document->game();
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(game->gameName());
            const IO::Path docPath = document->path();
            
            ControllerSPtr controller = lock(i_controller);
            try {
                for (size_t i = 0; i < wxPaths.size(); ++i) {
                    const wxString& wxPath = wxPaths[i];
                    const IO::Path absPath(wxPath.ToStdString());
                    if (game->isEntityDefinitionFile(absPath)) {
                        ChoosePathTypeDialog pathDialog(wxGetTopLevelParent(parent), absPath, docPath, gamePath);
                        if (pathDialog.ShowModal() == wxID_OK) {
                            const Model::EntityDefinitionFileSpec spec = Model::EntityDefinitionFileSpec::external(pathDialog.path());
                            controller->setEntityDefinitionFile(spec);
                            return i;
                        }
                    }
                }
            } catch (...) {
                throw;
            }
            
            return wxPaths.size();
        }
    }
}
