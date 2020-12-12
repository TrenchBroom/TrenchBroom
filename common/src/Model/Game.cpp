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

#include "Assets/EntityDefinitionFileSpec.h"
#include "Model/BrushFace.h"
#include "Model/GameFactory.h"
#include "Model/WorldNode.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        const std::string& Game::gameName() const {
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

        void Game::setAdditionalSearchPaths(const std::vector<IO::Path>& searchPaths, Logger& logger) {
            doSetAdditionalSearchPaths(searchPaths, logger);
        }

        Game::PathErrors Game::checkAdditionalSearchPaths(const std::vector<IO::Path>& searchPaths) const {
            return doCheckAdditionalSearchPaths(searchPaths);
        }

        const CompilationConfig& Game::compilationConfig() {
            return doCompilationConfig();
        }

        size_t Game::maxPropertyLength() const {
            return doMaxPropertyLength();
        }

        const std::vector<SmartTag>& Game::smartTags() const {
            return doSmartTags();
        }

        std::optional<vm::bbox3> Game::softMapBounds() const {
            return doSoftMapBounds();
        }

        Game::SoftMapBounds Game::extractSoftMapBounds(const Entity& entity) const {
            return doExtractSoftMapBounds(entity);
        }

        std::unique_ptr<WorldNode> Game::newMap(const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const {
            return doNewMap(format, worldBounds, logger);
        }

        std::unique_ptr<WorldNode> Game::loadMap(const MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const {
            return doLoadMap(format, worldBounds, path, logger);
        }

        void Game::writeMap(WorldNode& world, const IO::Path& path) const {
            doWriteMap(world, path);
        }

        void Game::exportMap(WorldNode& world, const Model::ExportFormat format, const IO::Path& path) const {
            doExportMap(world, format, path);
        }

        std::vector<Node*> Game::parseNodes(const std::string& str, WorldNode& world, const vm::bbox3& worldBounds, Logger& logger) const {
            return doParseNodes(str, world, worldBounds, logger);
        }

        std::vector<BrushFace> Game::parseBrushFaces(const std::string& str, WorldNode& world, const vm::bbox3& worldBounds, Logger& logger) const {
            return doParseBrushFaces(str, world, worldBounds, logger);
        }

        void Game::writeNodesToStream(WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const {
            doWriteNodesToStream(world, nodes, stream);
        }

        void Game::writeBrushFacesToStream(WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const {
            doWriteBrushFacesToStream(world, faces, stream);
        }

        Game::TexturePackageType Game::texturePackageType() const {
            return doTexturePackageType();
        }

        void Game::loadTextureCollections(const Entity& entity, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const {
            doLoadTextureCollections(entity, documentPath, textureManager, logger);
        }

        bool Game::isTextureCollection(const IO::Path& path) const {
            return doIsTextureCollection(path);
        }

        std::vector<std::string> Game::fileTextureCollectionExtensions() const {
            return doFileTextureCollectionExtensions();
        }

        std::vector<IO::Path> Game::findTextureCollections() const {
            return doFindTextureCollections();
        }

        std::vector<IO::Path> Game::extractTextureCollections(const Entity& entity) const {
            return doExtractTextureCollections(entity);
        }

        void Game::updateTextureCollections(Entity& entity, const std::vector<IO::Path>& paths) const {
            doUpdateTextureCollections(entity, paths);
        }

        void Game::reloadShaders() {
            doReloadShaders();
        }

        bool Game::isEntityDefinitionFile(const IO::Path& path) const {
            return doIsEntityDefinitionFile(path);
        }

        std::vector<Assets::EntityDefinitionFileSpec> Game::allEntityDefinitionFiles() const {
            return doAllEntityDefinitionFiles();
        }

        Assets::EntityDefinitionFileSpec Game::extractEntityDefinitionFile(const Entity& entity) const {
            return doExtractEntityDefinitionFile(entity);
        }

        IO::Path Game::findEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const std::vector<IO::Path>& searchPaths) const {
            return doFindEntityDefinitionFile(spec, searchPaths);
        }

        std::vector<std::string> Game::availableMods() const {
            return doAvailableMods();
        }

        std::vector<std::string> Game::extractEnabledMods(const Entity& entity) const {
            return doExtractEnabledMods(entity);
        }

        std::string Game::defaultMod() const {
            return doDefaultMod();
        }

        const FlagsConfig& Game::surfaceFlags() const {
            return doSurfaceFlags();
        }

        const FlagsConfig& Game::contentFlags() const {
            return doContentFlags();
        }

        const BrushFaceAttributes& Game::defaultFaceAttribs() const {
            return doDefaultFaceAttribs();
        }

        const std::vector<CompilationTool>& Game::compilationTools() const {
            return doCompilationTools();
        }
    }
}
