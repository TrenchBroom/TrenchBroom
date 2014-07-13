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

#include "Game.h"

#include "Model/GameFactory.h"

namespace TrenchBroom {
    namespace Model {
        Game::~Game() {}

        const String& Game::gameName() const {
            return doGameName();
        }

        bool Game::isGamePathPreference(const IO::Path& prefPath) const {
            const GameFactory& gameFactory = GameFactory::instance();
            return gameFactory.isGamePathPreference(gameName(), prefPath);
        }

        IO::Path Game::gamePath() const {
            return doGamePath();
        }

        void Game::setGamePath(const IO::Path& gamePath) {
            doSetGamePath(gamePath);
        }

        void Game::setAdditionalSearchPaths(const IO::Path::List& searchPaths) {
            doSetAdditionalSearchPaths(searchPaths);
        }

        Map* Game::newMap(const MapFormat::Type format) const {
            return doNewMap(format);
        }
        
        Map* Game::loadMap(const BBox3& worldBounds, const IO::Path& path) const {
            return doLoadMap(worldBounds, path);
        }
        
        Model::EntityList Game::parseEntities(const BBox3& worldBounds, MapFormat::Type format, const String& str) const {
            return doParseEntities(worldBounds, format, str);
        }
        
        Model::BrushList Game::parseBrushes(const BBox3& worldBounds, MapFormat::Type format, const String& str) const {
            return doParseBrushes(worldBounds, format, str);
        }
        
        Model::BrushFaceList Game::parseFaces(const BBox3& worldBounds, MapFormat::Type format, const String& str) const {
            return doParseFaces(worldBounds, format, str);
        }
        
        void Game::writeMap(Map& map, const IO::Path& path) const {
            doWriteMap(map, path);
        }
        
        void Game::writeObjectsToStream(const MapFormat::Type format, const Model::ObjectList& objects, std::ostream& stream) const {
            doWriteObjectsToStream(format, objects, stream);
        }
        
        void Game::writeFacesToStream(const MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream) const {
            doWriteFacesToStream(format, faces, stream);
        }
        
        bool Game::isTextureCollection(const IO::Path& path) const {
            return doIsTextureCollection(path);
        }

        IO::Path::List Game::findBuiltinTextureCollections() const {
            return doFindBuiltinTextureCollections();
        }
        
        StringList Game::extractExternalTextureCollections(const Map* map) const {
            return doExtractExternalTextureCollections(map);
        }
        
        void Game::updateExternalTextureCollections(Map* map, const StringList& collections) const {
            doUpdateExternalTextureCollections(map, collections);
        }

        Assets::TextureCollection* Game::loadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            return doLoadTextureCollection(spec);
        }
        
        bool Game::isEntityDefinitionFile(const IO::Path& path) const {
            return doIsEntityDefinitionFile(path);
        }

        Assets::EntityDefinitionList Game::loadEntityDefinitions(const IO::Path& path) const {
            return doLoadEntityDefinitions(path);
        }
        
        EntityDefinitionFileSpec::List Game::allEntityDefinitionFiles() const {
            return doAllEntityDefinitionFiles();
        }

        EntityDefinitionFileSpec Game::extractEntityDefinitionFile(const Map* map) const {
            return doExtractEntityDefinitionFile(map);
        }
        
        IO::Path Game::findEntityDefinitionFile(const EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const {
            return doFindEntityDefinitionFile(spec, searchPaths);
        }
        
        Assets::EntityModel* Game::loadModel(const IO::Path& path) const {
            return doLoadModel(path);
        }

        StringList Game::availableMods() const {
            return doAvailableMods();
        }

        StringList Game::extractEnabledMods(const Map* map) const {
            return doExtractEnabledMods(map);
        }
        
        const GameConfig::FlagsConfig& Game::surfaceFlags() const {
            return doSurfaceFlags();
        }
        
        const GameConfig::FlagsConfig& Game::contentFlags() const {
            return doContentFlags();
        }
    }
}
