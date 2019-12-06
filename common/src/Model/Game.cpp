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

#include "Model/GameFactory.h"
#include "Model/World.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
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

        const std::vector<SmartTag>& Game::smartTags() const {
            return doSmartTags();
        }

        std::unique_ptr<World> Game::newMap(const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const {
            return doNewMap(format, worldBounds, logger);
        }

        std::unique_ptr<World> Game::loadMap(const MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const {
            return doLoadMap(format, worldBounds, path, logger);
        }

        void Game::writeMap(World& world, const IO::Path& path) const {
            doWriteMap(world, path);
        }

        void Game::exportMap(World& world, const Model::ExportFormat format, const IO::Path& path) const {
            doExportMap(world, format, path);
        }

        std::vector<Node*> Game::parseNodes(const String& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const {
            return doParseNodes(str, world, worldBounds, logger);
        }

        std::vector<BrushFace*> Game::parseBrushFaces(const String& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const {
            return doParseBrushFaces(str, world, worldBounds, logger);
        }

        void Game::writeNodesToStream(World& world, const std::vector<Node*>& nodes, std::ostream& stream) const {
            doWriteNodesToStream(world, nodes, stream);
        }

        void Game::writeBrushFacesToStream(World& world, const std::vector<BrushFace*>& faces, std::ostream& stream) const {
            doWriteBrushFacesToStream(world, faces, stream);
        }

        Game::TexturePackageType Game::texturePackageType() const {
            return doTexturePackageType();
        }

        void Game::loadTextureCollections(AttributableNode& node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const {
            doLoadTextureCollections(node, documentPath, textureManager, logger);
        }

        bool Game::isTextureCollection(const IO::Path& path) const {
            return doIsTextureCollection(path);
        }

        IO::Path::List Game::findTextureCollections() const {
            return doFindTextureCollections();
        }

        IO::Path::List Game::extractTextureCollections(const AttributableNode& node) const {
            return doExtractTextureCollections(node);
        }

        void Game::updateTextureCollections(AttributableNode& node, const IO::Path::List& paths) const {
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

        Assets::EntityDefinitionFileSpec Game::extractEntityDefinitionFile(const AttributableNode& node) const {
            return doExtractEntityDefinitionFile(node);
        }

        IO::Path Game::findEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const {
            return doFindEntityDefinitionFile(spec, searchPaths);
        }

        std::vector<std::string> Game::availableMods() const {
            return doAvailableMods();
        }

        std::vector<std::string> Game::extractEnabledMods(const AttributableNode& node) const {
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
