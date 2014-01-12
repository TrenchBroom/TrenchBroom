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

#ifndef __TrenchBroom__ControllerFacade__
#define __TrenchBroom__ControllerFacade__

#include "Notifier.h"
#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Controller/Command.h"
#include "Controller/CommandProcessor.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class Game;
    }
    
    namespace Controller {
        class CommandProcessor;
    }
    
    namespace View {
        class ControllerFacade {
        public:
            struct MoveVerticesResult {
                bool success;
                bool hasRemainingVertices;
                MoveVerticesResult(bool i_success, bool i_hasRemainingVertices);
            };
        private:
            MapDocumentWPtr m_document;
            Controller::CommandProcessor m_commandProcessor;
        public:
            ControllerFacade(MapDocumentWPtr document);
            
            Notifier1<Controller::Command::Ptr>& commandDoNotifier;
            Notifier1<Controller::Command::Ptr>& commandDoneNotifier;
            Notifier1<Controller::Command::Ptr>& commandDoFailedNotifier;
            Notifier1<Controller::Command::Ptr>& commandUndoNotifier;
            Notifier1<Controller::Command::Ptr>& commandUndoneNotifier;
            Notifier1<Controller::Command::Ptr>& commandUndoFailedNotifier;
            
            bool hasLastCommand() const;
            bool hasNextCommand() const;
            const String& lastCommandName() const;
            const String& nextCommandName() const;

            bool newDocument(const BBox3& worldBounds, Model::GamePtr game);
            bool openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            
            void beginUndoableGroup(const String& name);
            void beginOneShotGroup(const String& name);
            void closeGroup();
            void rollbackGroup();
            
            bool undoLastCommand();
            bool redoNextCommand();

            bool selectObjects(const Model::ObjectList& objects);
            bool selectObject(Model::Object& object);
            bool selectAllObjects();
            bool deselectAllAndSelectObjects(const Model::ObjectList& objects);
            bool deselectAllAndSelectObject(Model::Object& object);
            bool deselectObject(Model::Object& object);
            bool selectFace(Model::BrushFace& face);
            bool deselectAllAndSelectFace(Model::BrushFace& face);
            bool deselectFace(Model::BrushFace& face);
            bool deselectAll();
            
            bool addObjects(const Model::ObjectList& objects);
            bool addObjects(const Model::ObjectParentList& objects);
            bool addObject(Model::Object& object);
            bool removeObjects(const Model::ObjectList& objects);
            bool removeObjects(const Model::ObjectParentList& objects);
            bool removeObject(Model::Object& object);
            Model::ObjectList duplicateObjects(const Model::ObjectList& objects, const BBox3& worldBounds);

            bool moveBrushesToWorldspawn(const Model::BrushList& brushes);
            bool reparentBrushes(const Model::BrushList& brushes, Model::Entity* newParent);
            
            bool renameEntityProperty(const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, bool force = false);
            bool setEntityProperty(const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyValue& newValue, bool force = false);
            bool setEntityProperty(Model::Entity& entity, const Model::PropertyKey& key, const Model::PropertyValue& newValue, bool force = false);
            bool removeEntityProperty(const Model::EntityList& entities, const Model::PropertyKey& key, bool force = false);
            
            bool setMods(const StringList& mods);
            bool setEntityDefinitionFile(const IO::Path& file);

            bool addTextureCollection(const String& name);
            bool removeTextureCollections(const StringList& names);
            bool moveTextureCollectionUp(const String& name);
            bool moveTextureCollectionDown(const String& name);
            
            bool moveObjects(const Model::ObjectList& objects, const Vec3& delta, const bool lockTextures);
            bool rotateObjects(const Model::ObjectList& objects, const Vec3& center, const Vec3& axis, const FloatType angle, const bool lockTextures);
            bool resizeBrushes(const Model::BrushFaceList& faces, const Vec3& delta, const bool lockTextures);

            bool snapPlanePoints(Model::Brush& brush);
            bool findPlanePoints(Model::Brush& brush);

            MoveVerticesResult moveVertices(const Model::VertexToBrushesMap& vertices, const Vec3& delta);
            bool moveEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta);
            bool moveFaces(const Model::VertexToFacesMap& faces, const Vec3& delta);
            bool splitEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta);
            bool splitFaces(const Model::VertexToFacesMap& faces, const Vec3& delta);
            
            bool setTexture(const Model::BrushFaceList& faces, Assets::Texture* texture);
            bool setFaceXOffset(const Model::BrushFaceList& faces, float xOffset, bool add);
            bool setFaceYOffset(const Model::BrushFaceList& faces, float yOffset, bool add);
            bool setFaceRotation(const Model::BrushFaceList& faces, float rotation, bool add);
            bool setFaceXScale(const Model::BrushFaceList& faces, float xScale, bool add);
            bool setFaceYScale(const Model::BrushFaceList& faces, float yScale, bool add);
            bool setSurfaceFlag(const Model::BrushFaceList& faces, size_t index, bool set);
            bool setContentFlag(const Model::BrushFaceList& faces, size_t index, bool set);
            bool setContentFlags(const Model::BrushFaceList& faces, int flags);
            bool setSurfaceValue(const Model::BrushFaceList& faces, float value, bool add);
            bool setFaceAttributes(const Model::BrushFaceList& faces, const Model::BrushFace& source);
        };
    }
}

#endif /* defined(__TrenchBroom__ControllerFacade__) */
