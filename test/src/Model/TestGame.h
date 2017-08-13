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

#ifndef TestGame_h
#define TestGame_h

#include "Model/Game.h"

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class ParserStatus;
    }
    
    namespace Model {
        class TestGame : public Game {
        public:
            TestGame();
        private:
            const String& doGameName() const;
            IO::Path doGamePath() const;
            void doSetGamePath(const IO::Path& gamePath, Logger* logger);
            void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger* logger);
            PathErrors doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const;

            CompilationConfig& doCompilationConfig();
            size_t doMaxPropertyLength() const;
            
            World* doNewMap(MapFormat::Type format, const BBox3& worldBounds) const;
            World* doLoadMap(MapFormat::Type format, const BBox3& worldBounds, const IO::Path& path, Logger* logger) const;
            void doWriteMap(World* world, const IO::Path& path) const;
            void doExportMap(World* world, Model::ExportFormat format, const IO::Path& path) const;
            
            NodeList doParseNodes(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const;
            BrushFaceList doParseBrushFaces(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const;
            void doWriteNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const;
            void doWriteBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const;
            
            TexturePackageType doTexturePackageType() const;
            void doLoadTextureCollections(AttributableNode* node, const IO::Path& documentPath, Assets::TextureManager& textureManager) const;
            bool doIsTextureCollection(const IO::Path& path) const;
            IO::Path::List doFindTextureCollections() const;
            IO::Path::List doExtractTextureCollections(const AttributableNode* node) const;
            void doUpdateTextureCollections(AttributableNode* node, const IO::Path::List& paths) const;
            
            bool doIsEntityDefinitionFile(const IO::Path& path) const;
            Assets::EntityDefinitionFileSpec::List doAllEntityDefinitionFiles() const;
            Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const AttributableNode* node) const;
            IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const;
            
            const BrushContentType::List& doBrushContentTypes() const;
            
            StringList doAvailableMods() const;
            StringList doExtractEnabledMods(const AttributableNode* node) const;
            String doDefaultMod() const;
            
            const GameConfig::FlagsConfig& doSurfaceFlags() const;
            const GameConfig::FlagsConfig& doContentFlags() const;

            Assets::EntityDefinitionList doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const;
            Assets::EntityModel* doLoadEntityModel(const IO::Path& path) const;
        };
    }
}

#endif /* TestGame_h */
