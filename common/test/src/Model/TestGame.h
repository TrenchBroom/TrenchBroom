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

#pragma once

#include "Model/BrushFaceAttributes.h"
#include "Model/Game.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }

    namespace Model {
        class TestGame : public Game {
        private:
            std::vector<SmartTag> m_smartTags;
            Model::BrushFaceAttributes m_defaultFaceAttributes;
            std::vector<CompilationTool> m_compilationTools;
        public:
            TestGame();
        public:
            void setSmartTags(std::vector<SmartTag> smartTags);
            void setDefaultFaceAttributes(const Model::BrushFaceAttributes& newDefaults);
        private:
            const std::string& doGameName() const override;
            IO::Path doGamePath() const override;
            void doSetGamePath(const IO::Path& gamePath, Logger& logger) override;
            std::optional<vm::bbox3> doSoftMapBounds() const override;
            Game::SoftMapBounds doExtractSoftMapBounds(const Entity& entity) const override;
            void doSetAdditionalSearchPaths(const std::vector<IO::Path>& searchPaths, Logger& logger) override;
            PathErrors doCheckAdditionalSearchPaths(const std::vector<IO::Path>& searchPaths) const override;

            const CompilationConfig& doCompilationConfig() override;
            size_t doMaxPropertyLength() const override;

            const std::vector<SmartTag>& doSmartTags() const override;

            std::unique_ptr<WorldNode> doNewMap(MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const override;
            std::unique_ptr<WorldNode> doLoadMap(MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const override;
            void doWriteMap(WorldNode& world, const IO::Path& path) const override;
            void doExportMap(WorldNode& world, Model::ExportFormat format, const IO::Path& path) const override;

            std::vector<Node*> doParseNodes(const std::string& str, WorldNode& world, const vm::bbox3& worldBounds, Logger& logger) const override;
            std::vector<BrushFace> doParseBrushFaces(const std::string& str, WorldNode& world, const vm::bbox3& worldBounds, Logger& logger) const override;
            void doWriteNodesToStream(WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const override;
            void doWriteBrushFacesToStream(WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const override;

            TexturePackageType doTexturePackageType() const override;
            void doLoadTextureCollections(const Entity& entity, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const override;
            bool doIsTextureCollection(const IO::Path& path) const override;
            std::vector<std::string> doFileTextureCollectionExtensions() const override;
            std::vector<IO::Path> doFindTextureCollections() const override;
            std::vector<IO::Path> doExtractTextureCollections(const Entity& entity) const override;
            void doUpdateTextureCollections(Entity& entity, const std::vector<IO::Path>& paths) const override;
            void doReloadShaders() override;

            bool doIsEntityDefinitionFile(const IO::Path& path) const override;
            std::vector<Assets::EntityDefinitionFileSpec> doAllEntityDefinitionFiles() const override;
            Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const Entity& entity) const override;
            IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const std::vector<IO::Path>& searchPaths) const override;

            std::vector<std::string> doAvailableMods() const override;
            std::vector<std::string> doExtractEnabledMods(const Entity& entity) const override;
            std::string doDefaultMod() const override;

            const FlagsConfig& doSurfaceFlags() const override;
            const FlagsConfig& doContentFlags() const override;
            const BrushFaceAttributes& doDefaultFaceAttribs() const override;
            const std::vector<CompilationTool>& doCompilationTools() const override;

            std::vector<Assets::EntityDefinition*> doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const override;
            std::unique_ptr<Assets::EntityModel> doInitializeModel(const IO::Path& path, Logger& logger) const override;
            void doLoadFrame(const IO::Path& path, size_t frameIndex, Assets::EntityModel& model, Logger& logger) const override;
        };
    }
}

