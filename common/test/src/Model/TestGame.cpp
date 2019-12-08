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

#include "TestGame.h"

#include "Assets/EntityModel.h"
#include "IO/BrushFaceReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/TestParserStatus.h"
#include "IO/TextureLoader.h"
#include "Model/GameConfig.h"
#include "Model/World.h"

#include <kdl/string_utils.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        TestGame::TestGame() = default;

        void TestGame::setSmartTags(std::vector<SmartTag> smartTags) {
            m_smartTags = std::move(smartTags);
        }

        const std::string& TestGame::doGameName() const {
            static const std::string name("Test");
            return name;
        }

        IO::Path TestGame::doGamePath() const {
            return IO::Path(".");
        }

        void TestGame::doSetGamePath(const IO::Path& /* gamePath */, Logger& /* logger */) {}
        void TestGame::doSetAdditionalSearchPaths(const std::vector<IO::Path>& /* searchPaths */, Logger& /* logger */) {}
        Game::PathErrors TestGame::doCheckAdditionalSearchPaths(const std::vector<IO::Path>& /* searchPaths */) const { return PathErrors(); }

        CompilationConfig& TestGame::doCompilationConfig() {
            static CompilationConfig config;
            return config;
        }

        size_t TestGame::doMaxPropertyLength() const {
            return 1024;
        }

        const std::vector<SmartTag>& TestGame::doSmartTags() const {
            return m_smartTags;
        }

        std::unique_ptr<World> TestGame::doNewMap(const MapFormat format, const vm::bbox3& /* worldBounds */, Logger& /* logger */) const {
            return std::make_unique<World>(format);
        }

        std::unique_ptr<World> TestGame::doLoadMap(const MapFormat format, const vm::bbox3& /* worldBounds */, const IO::Path& /* path */, Logger& /* logger */) const {
            return std::make_unique<World>(format);
        }

        void TestGame::doWriteMap(World& world, const IO::Path& path) const {
            const auto mapFormatName = formatName(world.format());

            IO::OpenFile open(path, true);
            IO::writeGameComment(open.file, gameName(), mapFormatName);

            IO::NodeWriter writer(world, open.file);
            writer.writeMap();
        }

        void TestGame::doExportMap(World& /* world */, const Model::ExportFormat /* format */, const IO::Path& /* path */) const {}

        std::vector<Node*> TestGame::doParseNodes(const std::string& str, World& world, const vm::bbox3& worldBounds, Logger& /* logger */) const {
            IO::TestParserStatus status;
            IO::NodeReader reader(str, world);
            return reader.read(worldBounds, status);
        }

        std::vector<BrushFace*> TestGame::doParseBrushFaces(const std::string& str, World& world, const vm::bbox3& worldBounds, Logger& /* logger */) const {
            IO::TestParserStatus status;
            IO::BrushFaceReader reader(str, world);
            return reader.read(worldBounds, status);
        }

        void TestGame::doWriteNodesToStream(World& world, const std::vector<Node*>& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }

        void TestGame::doWriteBrushFacesToStream(World& world, const std::vector<BrushFace*>& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }

        TestGame::TexturePackageType TestGame::doTexturePackageType() const {
            return TexturePackageType::File;
        }

        void TestGame::doLoadTextureCollections(AttributableNode& node, const IO::Path& /* documentPath */, Assets::TextureManager& textureManager, Logger& logger) const {
            const std::vector<IO::Path> paths = extractTextureCollections(node);

            const IO::Path root = IO::Disk::getCurrentWorkingDir();
            const std::vector<IO::Path> fileSearchPaths{ root };
            const IO::DiskFileSystem fileSystem(root, true);

            const GameConfig::TextureConfig textureConfig(GameConfig::TexturePackageConfig(GameConfig::PackageFormatConfig("wad", "idmip")),
                                                          GameConfig::PackageFormatConfig("D", "idmip"),
                                                          IO::Path("data/palette.lmp"),
                                                          "wad",
                                                          IO::Path());

            IO::TextureLoader textureLoader(fileSystem, fileSearchPaths, textureConfig, logger);
            textureLoader.loadTextures(paths, textureManager);
        }

        bool TestGame::doIsTextureCollection(const IO::Path& /* path */) const {
            return false;
        }

        std::vector<IO::Path> TestGame::doFindTextureCollections() const {
            return std::vector<IO::Path>();
        }

        std::vector<IO::Path> TestGame::doExtractTextureCollections(const AttributableNode& node) const {
            const AttributeValue& pathsValue = node.attribute("wad");
            if (pathsValue.empty()) {
                return std::vector<IO::Path>(0);
            }

            return IO::Path::asPaths(kdl::str_split(pathsValue, ";"));
        }

        void TestGame::doUpdateTextureCollections(AttributableNode& node, const std::vector<IO::Path>& paths) const {
            const std::string value = kdl::str_join(IO::Path::asStrings(paths, "/"), ";");
            node.addOrUpdateAttribute("wad", value);
        }

        void TestGame::doReloadShaders() {}

        bool TestGame::doIsEntityDefinitionFile(const IO::Path& /* path */) const {
            return false;
        }

        Assets::EntityDefinitionFileSpec::List TestGame::doAllEntityDefinitionFiles() const {
            return Assets::EntityDefinitionFileSpec::List();
        }

        Assets::EntityDefinitionFileSpec TestGame::doExtractEntityDefinitionFile(const AttributableNode& /* node */) const {
            return Assets::EntityDefinitionFileSpec();
        }

        IO::Path TestGame::doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& /* spec */, const std::vector<IO::Path>& /* searchPaths */) const {
            return IO::Path();
        }

        std::vector<std::string> TestGame::doAvailableMods() const {
            return {};
        }

        std::vector<std::string> TestGame::doExtractEnabledMods(const AttributableNode& /* node */) const {
            return {};
        }

        std::string TestGame::doDefaultMod() const {
            return "";
        }

        const GameConfig::FlagsConfig& TestGame::doSurfaceFlags() const {
            static const GameConfig::FlagsConfig config;
            return config;
        }

        const GameConfig::FlagsConfig& TestGame::doContentFlags() const {
            static const GameConfig::FlagsConfig config;
            return config;
        }

        std::vector<Assets::EntityDefinition*> TestGame::doLoadEntityDefinitions(IO::ParserStatus& /* status */, const IO::Path& /* path */) const {
            return {};
        }

        std::unique_ptr<Assets::EntityModel> TestGame::doInitializeModel(const IO::Path& /* path */, Logger& /* logger */) const { return nullptr; }
        void TestGame::doLoadFrame(const IO::Path& /* path */, size_t /* frameIndex */, Assets::EntityModel& /* model */, Logger& /* logger */) const {}
    }
}
