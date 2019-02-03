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

#include "Game.h"

#include "Model/BrushContentTypeBuilder.h"
#include "Model/GameFactory.h"

namespace TrenchBroom {
    namespace Model {
        Game::Game() :
        m_brushContentTypeBuilder(nullptr) {}
        
        Game::~Game() {
            delete m_brushContentTypeBuilder;
            m_brushContentTypeBuilder = nullptr;
        }

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

        void Game::setGamePath(const IO::Path& gamePath, Logger& logger) {
            doSetGamePath(gamePath, logger);
        }

        void Game::setAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger& logger) {
            doSetAdditionalSearchPaths(searchPaths, logger);
        }

        Game::PathErrors Game::checkAdditionalSearchPaths(const IO::Path::List& searchPaths) const {
            return doCheckAdditionalSearchPaths(searchPaths);
        }

        CompilationConfig& Game::compilationConfig() {
            return doCompilationConfig();
        }

        size_t Game::maxPropertyLength() const {
            return doMaxPropertyLength();
        }

        World* Game::newMap(const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const {
            return doNewMap(format, worldBounds, logger);
        }
        
        World* Game::loadMap(const MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const {
            return doLoadMap(format, worldBounds, path, logger);
        }

        void Game::writeMap(World* world, const IO::Path& path) const {
            ensure(world != nullptr, "world is null");
            doWriteMap(world, path);
        }

        void Game::exportMap(World* world, const Model::ExportFormat format, const IO::Path& path) const {
            ensure(world != nullptr, "world is null");
            doExportMap(world, format, path);
        }

        NodeList Game::parseNodes(const String& str, World* world, const vm::bbox3& worldBounds, Logger& logger) const {
            return doParseNodes(str, world, worldBounds, logger);
        }
        
        BrushFaceList Game::parseBrushFaces(const String& str, World* world, const vm::bbox3& worldBounds, Logger& logger) const {
            return doParseBrushFaces(str, world, worldBounds, logger);
        }

        void Game::writeNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const {
            doWriteNodesToStream(world, nodes, stream);
        }
    
        void Game::writeBrushFacesToStream(World* world, const Model::BrushFaceList& faces, std::ostream& stream) const {
            doWriteBrushFacesToStream(world, faces, stream);
        }
    
        Game::TexturePackageType Game::texturePackageType() const {
            return doTexturePackageType();
        }

        void Game::loadTextureCollections(AttributableNode* node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const {
            doLoadTextureCollections(node, documentPath, textureManager, logger);
        }

        bool Game::isTextureCollection(const IO::Path& path) const {
            return doIsTextureCollection(path);
        }

        IO::Path::List Game::findTextureCollections() const {
            return doFindTextureCollections();
        }
        
        IO::Path::List Game::extractTextureCollections(const AttributableNode* node) const {
            ensure(node != nullptr, "node is null");
            return doExtractTextureCollections(node);
        }
        
        void Game::updateTextureCollections(AttributableNode* node, const IO::Path::List& paths) const {
            ensure(node != nullptr, "node is null");
            doUpdateTextureCollections(node, paths);
        }

        void Game::reloadShaders() {
            doReloadShaders();
        }

        bool Game::isEntityDefinitionFile(const IO::Path& path) const {
            return doIsEntityDefinitionFile(path);
        }

        Assets::EntityDefinitionFileSpec::List Game::allEntityDefinitionFiles() const {
            return doAllEntityDefinitionFiles();
        }

        Assets::EntityDefinitionFileSpec Game::extractEntityDefinitionFile(const AttributableNode* node) const {
            ensure(node != nullptr, "node is null");
            return doExtractEntityDefinitionFile(node);
        }
        
        IO::Path Game::findEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const {
            return doFindEntityDefinitionFile(spec, searchPaths);
        }
        
        const BrushContentTypeBuilder* Game::brushContentTypeBuilder() const {
            if (m_brushContentTypeBuilder == nullptr)
                m_brushContentTypeBuilder = new BrushContentTypeBuilder(brushContentTypes());
            return m_brushContentTypeBuilder;
        }

        const BrushContentType::List& Game::brushContentTypes() const {
            return doBrushContentTypes();
        }

        StringList Game::availableMods() const {
            return doAvailableMods();
        }

        StringList Game::extractEnabledMods(const AttributableNode* node) const {
            ensure(node != nullptr, "node is null");
            return doExtractEnabledMods(node);
        }
        
        String Game::defaultMod() const {
            return doDefaultMod();
        }

        const GameConfig::FlagsConfig& Game::surfaceFlags() const {
            return doSurfaceFlags();
        }
        
        const GameConfig::FlagsConfig& Game::contentFlags() const {
            return doContentFlags();
        }
    }
}
