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

#include "EL/VariableStore.h"
#include "IO/BrushFaceReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/TestParserStatus.h"
#include "IO/TextureLoader.h"
#include "Model/GameConfig.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        TestGame::TestGame() {}

        const String& TestGame::doGameName() const {
            static const String name("Test");
            return name;
        }
        
        IO::Path TestGame::doGamePath() const {
            return IO::Path(".");
        }
        
        void TestGame::doSetGamePath(const IO::Path& gamePath, Logger* logger) {}
        void TestGame::doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger* logger) {}
        Game::PathErrors TestGame::doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const { return PathErrors(); }
        
        CompilationConfig& TestGame::doCompilationConfig() {
            static CompilationConfig config;
            return config;
        }
        
        size_t TestGame::doMaxPropertyLength() const {
            return 1024;
        }
        
        World* TestGame::doNewMap(const MapFormat::Type format, const vm::bbox3& worldBounds, Logger* logger) const {
            return new World(format, brushContentTypeBuilder(), worldBounds);
        }
        
        World* TestGame::doLoadMap(const MapFormat::Type format, const vm::bbox3& worldBounds, const IO::Path& path, Logger* logger) const {
            return new World(format, brushContentTypeBuilder(), worldBounds);
        }
        
        void TestGame::doWriteMap(World* world, const IO::Path& path) const {}
        void TestGame::doExportMap(World* world, Model::ExportFormat format, const IO::Path& path) const {}
        
        NodeList TestGame::doParseNodes(const String& str, World* world, const vm::bbox3& worldBounds, Logger* logger) const {
            IO::TestParserStatus status;
            IO::NodeReader reader(str, world);
            return reader.read(worldBounds, status);
        }
        
        BrushFaceList TestGame::doParseBrushFaces(const String& str, World* world, const vm::bbox3& worldBounds, Logger* logger) const {
            IO::TestParserStatus status;
            IO::BrushFaceReader reader(str, world);
            return reader.read(worldBounds, status);
        }
        
        void TestGame::doWriteNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }
        
        void TestGame::doWriteBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }
        
        TestGame::TexturePackageType TestGame::doTexturePackageType() const {
            return TP_File;
        }
        
        void TestGame::doLoadTextureCollections(AttributableNode* node, const IO::Path& documentPath, Assets::TextureManager& textureManager) const {
            const EL::NullVariableStore variables;
            const IO::Path::List paths = extractTextureCollections(node);
            
            const IO::Path root = IO::Disk::getCurrentWorkingDir();
            const IO::Path::List fileSearchPaths{ root };
            const IO::DiskFileSystem fileSystem(root, true);
            
            const GameConfig::TextureConfig textureConfig(GameConfig::TexturePackageConfig(GameConfig::PackageFormatConfig("wad", "idmip")),
                                                          GameConfig::PackageFormatConfig("D", "idmip"),
                                                          IO::Path("data/palette.lmp"),
                                                          "wad");
            
            IO::TextureLoader textureLoader(variables, fileSystem, fileSearchPaths, textureConfig);
            textureLoader.loadTextures(paths, textureManager);
        }
        
        bool TestGame::doIsTextureCollection(const IO::Path& path) const {
            return false;
        }
        
        IO::Path::List TestGame::doFindTextureCollections() const {
            return IO::Path::List();
        }
        
        IO::Path::List TestGame::doExtractTextureCollections(const AttributableNode* node) const {
            const AttributeValue& pathsValue = node->attribute("wad");
            if (pathsValue.empty())
                return IO::Path::List(0);
            
            return IO::Path::asPaths(StringUtils::splitAndTrim(pathsValue, ';'));
        }
        
        void TestGame::doUpdateTextureCollections(AttributableNode* node, const IO::Path::List& paths) const {
            const String value = StringUtils::join(IO::Path::asStrings(paths, '/'), ';');
            node->addOrUpdateAttribute("wad", value);
        }
        
        bool TestGame::doIsEntityDefinitionFile(const IO::Path& path) const {
            return false;
        }
        
        Assets::EntityDefinitionFileSpec::List TestGame::doAllEntityDefinitionFiles() const {
            return Assets::EntityDefinitionFileSpec::List();
        }
        
        Assets::EntityDefinitionFileSpec TestGame::doExtractEntityDefinitionFile(const AttributableNode* node) const {
            return Assets::EntityDefinitionFileSpec();
        }
        
        IO::Path TestGame::doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const {
            return IO::Path();
        }
        
        const BrushContentType::List& TestGame::doBrushContentTypes() const {
            static const BrushContentType::List result;
            return result;
        }
        
        StringList TestGame::doAvailableMods() const {
            return EmptyStringList;
        }
        
        StringList TestGame::doExtractEnabledMods(const AttributableNode* node) const {
            return EmptyStringList;
        }
        
        String TestGame::doDefaultMod() const {
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

        Assets::EntityDefinitionList TestGame::doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const {
            return Assets::EntityDefinitionList();
        }
        
        Assets::EntityModel* TestGame::doLoadEntityModel(const IO::Path& path) const {
            return nullptr;
        }
    }
}
