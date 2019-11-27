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

#ifndef TrenchBroom_GameImpl
#define TrenchBroom_GameImpl

#include "TrenchBroom.h"
#include "Assets/AssetTypes.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameFileSystem.h"
#include "Model/Model_Forward.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class GameImpl : public Game {
        private:
            GameConfig& m_config;
            GameFileSystem m_fs;
            IO::Path m_gamePath;
            IO::Path::List m_additionalSearchPaths;
        public:
            GameImpl(GameConfig& config, const IO::Path& gamePath, Logger& logger);
        private:
            void initializeFileSystem(Logger& logger);
        private:
            const String& doGameName() const override;
            IO::Path doGamePath() const override;
            void doSetGamePath(const IO::Path& gamePath, Logger& logger) override;
            void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger& logger) override;
            PathErrors doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const override;

            CompilationConfig& doCompilationConfig() override;

            size_t doMaxPropertyLength() const override;

            const std::vector<SmartTag>& doSmartTags() const override;

            std::unique_ptr<World> doNewMap(MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const override;
            std::unique_ptr<World> doLoadMap(MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const override;
            void doWriteMap(World& world, const IO::Path& path) const override;
            void doExportMap(World& world, Model::ExportFormat format, const IO::Path& path) const override;

            std::vector<Node*> doParseNodes(const String& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const override;
            std::vector<BrushFace*> doParseBrushFaces(const String& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const override;

            void doWriteNodesToStream(World& world, const std::vector<Node*>& nodes, std::ostream& stream) const override;
            void doWriteBrushFacesToStream(World& world, const std::vector<BrushFace*>& faces, std::ostream& stream) const override;

            TexturePackageType doTexturePackageType() const override;
            void doLoadTextureCollections(AttributableNode& node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const override;
            IO::Path::List textureCollectionSearchPaths(const IO::Path& documentPath) const;

            bool doIsTextureCollection(const IO::Path& path) const override;
            IO::Path::List doFindTextureCollections() const override;
            IO::Path::List doExtractTextureCollections(const AttributableNode& node) const override;
            void doUpdateTextureCollections(AttributableNode& node, const IO::Path::List& paths) const override;
            void doReloadShaders() override;

            bool doIsEntityDefinitionFile(const IO::Path& path) const override;
            Assets::EntityDefinitionList doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const override;
            Assets::EntityDefinitionFileSpec::List doAllEntityDefinitionFiles() const override;
            Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const AttributableNode& node) const override;
            Assets::EntityDefinitionFileSpec defaultEntityDefinitionFile() const;
            IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const override;

            std::unique_ptr<Assets::EntityModel> doInitializeModel(const IO::Path& path, Logger& logger) const override;
            void doLoadFrame(const IO::Path& path, size_t frameIndex, Assets::EntityModel& model, Logger& logger) const override;

            Assets::Palette loadTexturePalette() const;

            StringList doAvailableMods() const override;
            StringList doExtractEnabledMods(const AttributableNode& node) const override;
            String doDefaultMod() const override;

            const GameConfig::FlagsConfig& doSurfaceFlags() const override;
            const GameConfig::FlagsConfig& doContentFlags() const override;
        private:
            void writeLongAttribute(AttributableNode& node, const AttributeName& baseName, const AttributeValue& value, size_t maxLength) const;
            String readLongAttribute(const AttributableNode& node, const AttributeName& baseName) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameImpl) */
