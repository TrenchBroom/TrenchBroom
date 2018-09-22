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
            const String& doGameName() const override;
            IO::Path doGamePath() const override;
            void doSetGamePath(const IO::Path& gamePath, Logger* logger) override;
            void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger* logger) override;
            PathErrors doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const override;

            CompilationConfig& doCompilationConfig() override;
            size_t doMaxPropertyLength() const override;
            
            World* doNewMap(MapFormat::Type format, const vm::bbox3& worldBounds) const override;
            World* doLoadMap(MapFormat::Type format, const vm::bbox3& worldBounds, const IO::Path& path, Logger* logger) const override;
            void doWriteMap(World* world, const IO::Path& path) const override;
            void doExportMap(World* world, Model::ExportFormat format, const IO::Path& path) const override;
            
            NodeList doParseNodes(const String& str, World* world, const vm::bbox3& worldBounds, Logger* logger) const override;
            BrushFaceList doParseBrushFaces(const String& str, World* world, const vm::bbox3& worldBounds, Logger* logger) const override;
            void doWriteNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const override;
            void doWriteBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const override;
            
            TexturePackageType doTexturePackageType() const override;
            void doLoadTextureCollections(AttributableNode* node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger* logger) const override;
            bool doIsTextureCollection(const IO::Path& path) const override;
            IO::Path::List doFindTextureCollections() const override;
            IO::Path::List doExtractTextureCollections(const AttributableNode* node) const override;
            void doUpdateTextureCollections(AttributableNode* node, const IO::Path::List& paths) const override;
            
            bool doIsEntityDefinitionFile(const IO::Path& path) const override;
            Assets::EntityDefinitionFileSpec::List doAllEntityDefinitionFiles() const override;
            Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const AttributableNode* node) const override;
            IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const override;
            
            const BrushContentType::List& doBrushContentTypes() const override;
            
            StringList doAvailableMods() const override;
            StringList doExtractEnabledMods(const AttributableNode* node) const override;
            String doDefaultMod() const override;
            
            const GameConfig::FlagsConfig& doSurfaceFlags() const override;
            const GameConfig::FlagsConfig& doContentFlags() const override;

            Assets::EntityDefinitionList doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const override;
            Assets::EntityModel* doLoadEntityModel(const IO::Path& path) const override;
        };
    }
}

#endif /* TestGame_h */
