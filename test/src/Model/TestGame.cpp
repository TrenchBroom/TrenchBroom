/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "IO/BrushFaceReader.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
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
        
        void TestGame::doSetGamePath(const IO::Path& gamePath) {}
        void TestGame::doSetAdditionalSearchPaths(const IO::Path::List& searchPaths) {}
        
        CompilationConfig& TestGame::doCompilationConfig() {
            static CompilationConfig config;
            return config;
        }
        
        size_t TestGame::doMaxPropertyLength() const {
            return 1024;
        }
        
        World* TestGame::doNewMap(const MapFormat::Type format, const BBox3& worldBounds) const {
            return new World(format, brushContentTypeBuilder(), worldBounds);
        }
        
        World* TestGame::doLoadMap(const MapFormat::Type format, const BBox3& worldBounds, const IO::Path& path, Logger* logger) const {
            return new World(format, brushContentTypeBuilder(), worldBounds);
        }
        
        void TestGame::doWriteMap(World* world, const IO::Path& path) const {}
        void TestGame::doExportMap(World* world, Model::ExportFormat format, const IO::Path& path) const {}
        
        NodeList TestGame::doParseNodes(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const {
            IO::NodeReader reader(str, world, logger);
            return reader.read(worldBounds);
        }
        
        BrushFaceList TestGame::doParseBrushFaces(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const {
            IO::BrushFaceReader reader(str, world, logger);
            return reader.read(worldBounds);
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
        
        void TestGame::doLoadTextureCollections(World* world, const IO::Path& documentPath, Assets::TextureManager& textureManager) const {}
        
        bool TestGame::doIsTextureCollection(const IO::Path& path) const {
            return false;
        }
        
        IO::Path::List TestGame::doFindTextureCollections() const {
            return IO::Path::List();
        }
        
        IO::Path::List TestGame::doExtractTextureCollections(const World* world) const {
            return IO::Path::List();
        }
        
        void TestGame::doUpdateTextureCollections(World* world, const IO::Path::List& paths) const {}
        
        bool TestGame::doIsEntityDefinitionFile(const IO::Path& path) const {
            return false;
        }
        
        Assets::EntityDefinitionFileSpec::List TestGame::doAllEntityDefinitionFiles() const {
            return Assets::EntityDefinitionFileSpec::List();
        }
        
        Assets::EntityDefinitionFileSpec TestGame::doExtractEntityDefinitionFile(const World* world) const {
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
        
        StringList TestGame::doExtractEnabledMods(const World* world) const {
            return EmptyStringList;
        }
        
        String TestGame::doDefaultMod() const {
            return "";
        }
        
        ::StringMap TestGame::doExtractGameEngineParameterSpecs(const World* world) const {
            return ::StringMap();
        }
        
        void TestGame::doSetGameEngineParameterSpecs(World* world, const ::StringMap& specs) const {}
        
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
            return NULL;
        }
    }
}
