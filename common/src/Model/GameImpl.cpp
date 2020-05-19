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

#include "GameImpl.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "Macros.h"
#include "Assets/Palette.h"
#include "Assets/EntityModel.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/AseParser.h"
#include "IO/BrushFaceReader.h"
#include "IO/Bsp29Parser.h"
#include "IO/DefParser.h"
#include "IO/DiskIO.h"
#include "IO/DkmParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/EntParser.h"
#include "IO/FgdParser.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/IOUtils.h"
#include "IO/MdlParser.h"
#include "IO/Md2Parser.h"
#include "IO/Md3Parser.h"
#include "IO/MdxParser.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/ObjParser.h"
#include "IO/ObjSerializer.h"
#include "IO/WorldReader.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "IO/TextureLoader.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/EntityAttributes.h"
#include "Model/ExportFormat.h"
#include "Model/GameConfig.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        GameImpl::GameImpl(GameConfig& config, const IO::Path& gamePath, Logger& logger) :
        m_config(config),
        m_gamePath(gamePath) {
            initializeFileSystem(logger);
        }

        void GameImpl::initializeFileSystem(Logger& logger) {
            m_fs.initialize(m_config, m_gamePath, m_additionalSearchPaths, logger);
        }

        const std::string& GameImpl::doGameName() const {
            return m_config.name();
        }

        IO::Path GameImpl::doGamePath() const {
            return m_gamePath;
        }

        void GameImpl::doSetGamePath(const IO::Path& gamePath, Logger& logger) {
            if (gamePath != m_gamePath) {
                m_gamePath = gamePath;
                initializeFileSystem(logger);
            }
        }

        void GameImpl::doSetAdditionalSearchPaths(const std::vector<IO::Path>& searchPaths, Logger& logger) {
            if (searchPaths != m_additionalSearchPaths) {
                m_additionalSearchPaths = searchPaths;
                initializeFileSystem(logger);
            }
        }

        Game::PathErrors GameImpl::doCheckAdditionalSearchPaths(const std::vector<IO::Path>& searchPaths) const {
            PathErrors result;
            for (const auto& searchPath : searchPaths) {
                const auto absPath = m_gamePath + searchPath;
                if (!absPath.isAbsolute() || !IO::Disk::directoryExists(absPath)) {
                    result.insert(std::make_pair(searchPath, "Directory not found: '" + searchPath.asString() + "'"));
                }
            }
            return result;
        }

        CompilationConfig& GameImpl::doCompilationConfig() {
            return m_config.compilationConfig();
        }

        size_t GameImpl::doMaxPropertyLength() const {
            return m_config.maxPropertyLength();
        }

        const std::vector<SmartTag>& GameImpl::doSmartTags() const {
            return m_config.smartTags();
        }

        std::unique_ptr<WorldNode> GameImpl::doNewMap(const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const {
            const auto initialMapFilePath = m_config.findInitialMap(formatName(format));
            if (!initialMapFilePath.isEmpty() && IO::Disk::fileExists(initialMapFilePath)) {
                return doLoadMap(format, worldBounds, initialMapFilePath, logger);
            } else {
                auto world = std::make_unique<WorldNode>(format);

                const Model::BrushBuilder builder(world.get(), worldBounds, defaultFaceAttribs());
                auto* brush = world->createBrush(builder.createCuboid(vm::vec3(128.0, 128.0, 32.0), Model::BrushFaceAttributes::NoTextureName));
                world->defaultLayer()->addChild(brush);

                if (format == MapFormat::Valve || format == MapFormat::Quake2_Valve || format == MapFormat::Quake3_Valve) {
                    world->addOrUpdateAttribute(AttributeNames::ValveVersion, "220");
                }

                return world;
            }
        }

        std::unique_ptr<WorldNode> GameImpl::doLoadMap(const MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
            auto fileReader = file->reader().buffer();
            IO::WorldReader worldReader(std::begin(fileReader), std::end(fileReader));
            return worldReader.read(format, worldBounds, parserStatus);
        }

        void GameImpl::doWriteMap(WorldNode& world, const IO::Path& path) const {
            const auto mapFormatName = formatName(world.format());

            IO::OpenFile open(path, true);
            IO::writeGameComment(open.file, gameName(), mapFormatName);

            IO::NodeWriter writer(world, open.file);
            writer.writeMap();
        }

        void GameImpl::doExportMap(WorldNode& world, const Model::ExportFormat format, const IO::Path& path) const {
            switch (format) {
                case Model::ExportFormat::WavefrontObj:
                    IO::NodeWriter(world, new IO::ObjFileSerializer(path)).writeMap();
                    break;
            }
        }

        std::vector<Node*> GameImpl::doParseNodes(const std::string& str, WorldNode& world, const vm::bbox3& worldBounds, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::NodeReader reader(str, world);
            return reader.read(worldBounds, parserStatus);
        }

        std::vector<BrushFace*> GameImpl::doParseBrushFaces(const std::string& str, WorldNode& world, const vm::bbox3& worldBounds, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::BrushFaceReader reader(str, world);
            return reader.read(worldBounds, parserStatus);
        }

        void GameImpl::doWriteNodesToStream(WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }

        void GameImpl::doWriteBrushFacesToStream(WorldNode& world, const std::vector<const BrushFace*>& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }

        Game::TexturePackageType GameImpl::doTexturePackageType() const {
            using Model::GameConfig;
            switch (m_config.textureConfig().package.type) {
                case TexturePackageConfig::PT_File:
                    return TexturePackageType::File;
                case TexturePackageConfig::PT_Directory:
                    return TexturePackageType::Directory;
                case TexturePackageConfig::PT_Unset:
                    throw GameException("Texture package type is not set in game configuration");
                switchDefault()
            }
        }

        void GameImpl::doLoadTextureCollections(AttributableNode& node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const {
            const auto paths = extractTextureCollections(node);

            const auto fileSearchPaths = textureCollectionSearchPaths(documentPath);
            IO::TextureLoader textureLoader(m_fs, fileSearchPaths, m_config.textureConfig(), logger);
            textureLoader.loadTextures(paths, textureManager);
        }

        std::vector<IO::Path> GameImpl::textureCollectionSearchPaths(const IO::Path& documentPath) const {
            std::vector<IO::Path> result;

            // Search for assets relative to the map file.
            result.push_back(documentPath);

            // Search for assets relative to the location of the game.
            result.push_back(m_gamePath);

            // Search for assets relative to the application itself.
            result.push_back(IO::SystemPaths::appDirectory());

            return result;
        }

        bool GameImpl::doIsTextureCollection(const IO::Path& path) const {
            const auto& packageConfig = m_config.textureConfig().package;
            switch (packageConfig.type) {
                case TexturePackageConfig::PT_File:
                    return path.hasExtension(packageConfig.fileFormat.extensions, false);
                case TexturePackageConfig::PT_Directory:
                case TexturePackageConfig::PT_Unset:
                    return false;
                switchDefault()
            }
        }

        std::vector<IO::Path> GameImpl::doFindTextureCollections() const {
            try {
                const auto& searchPath = m_config.textureConfig().package.rootDirectory;
                if (!searchPath.isEmpty() && m_fs.directoryExists(searchPath)) {
                    return m_fs.findItems(searchPath, IO::FileTypeMatcher(false, true));
                }
                return std::vector<IO::Path>();
            } catch (FileSystemException& e) {
                throw GameException("Could not find texture collections: " + std::string(e.what()));
            }
        }

        std::vector<IO::Path> GameImpl::doExtractTextureCollections(const AttributableNode& node) const {
            const auto& property = m_config.textureConfig().attribute;
            if (property.empty()) {
                return std::vector<IO::Path>(0);
            }

            const auto& pathsValue = node.attribute(property);
            if (pathsValue.empty()) {
                return std::vector<IO::Path>(0);
            }

            return IO::Path::asPaths(kdl::str_split(pathsValue, ";"));
        }

        void GameImpl::doUpdateTextureCollections(AttributableNode& node, const std::vector<IO::Path>& paths) const {
            const auto& attribute = m_config.textureConfig().attribute;
            if (attribute.empty()) {
                return;
            }

            const auto value = kdl::str_join(IO::Path::asStrings(paths, "/"), ";");
            node.addOrUpdateAttribute(attribute, value);
        }

        void GameImpl::doReloadShaders() {
            m_fs.reloadShaders();
        }

        bool GameImpl::doIsEntityDefinitionFile(const IO::Path& path) const {
            const auto extension = path.extension();
            if (kdl::ci::str_is_equal("fgd", extension)) {
                return true;
            } else if (kdl::ci::str_is_equal("def", extension)) {
                return true;
            } else if (kdl::ci::str_is_equal("ent", extension)) {
                return true;
            } else {
                return false;
            }
        }

        std::vector<Assets::EntityDefinition*> GameImpl::doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const {
            const auto extension = path.extension();
            const auto& defaultColor = m_config.entityConfig().defaultColor;

            if (kdl::ci::str_is_equal("fgd", extension)) {
                auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                auto reader = file->reader().buffer();
                IO::FgdParser parser(std::begin(reader), std::end(reader), defaultColor, file->path());
                return parser.parseDefinitions(status);
            } else if (kdl::ci::str_is_equal("def", extension)) {
                auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                auto reader = file->reader().buffer();
                IO::DefParser parser(std::begin(reader), std::end(reader), defaultColor);
                return parser.parseDefinitions(status);
            } else if (kdl::ci::str_is_equal("ent", extension)) {
                auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                auto reader = file->reader().buffer();
                IO::EntParser parser(std::begin(reader), std::end(reader), defaultColor);
                return parser.parseDefinitions(status);
            } else {
                throw GameException("Unknown entity definition format: '" + path.asString() + "'");
            }
        }

        std::vector<Assets::EntityDefinitionFileSpec> GameImpl::doAllEntityDefinitionFiles() const {
            const auto paths = m_config.entityConfig().defFilePaths;
            const auto count = paths.size();

            std::vector<Assets::EntityDefinitionFileSpec> result;
            result.reserve(count);

            for (const auto& path : paths) {
                result.push_back(Assets::EntityDefinitionFileSpec::builtin(path));
            }

            return result;
        }

        Assets::EntityDefinitionFileSpec GameImpl::doExtractEntityDefinitionFile(const AttributableNode& node) const {
            const auto& defValue = node.attribute(AttributeNames::EntityDefinitions);
            if (defValue.empty()) {
                return defaultEntityDefinitionFile();
            }
            return Assets::EntityDefinitionFileSpec::parse(defValue);
        }

        Assets::EntityDefinitionFileSpec GameImpl::defaultEntityDefinitionFile() const {
            const auto paths = m_config.entityConfig().defFilePaths;
            if (paths.empty()) {
                throw GameException("No entity definition files found for game '" + gameName() + "'");
            }

            const auto& path = paths.front();
            return Assets::EntityDefinitionFileSpec::builtin(path);
        }

        IO::Path GameImpl::doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const std::vector<IO::Path>& searchPaths) const {
            if (!spec.valid()) {
                throw GameException("Invalid entity definition file spec");
            }

            const auto& path = spec.path();
            if (spec.builtin()) {
                return m_config.findConfigFile(path);
            } else {
                if (path.isAbsolute()) {
                    return path;
                } else {
                    return IO::Disk::resolvePath(searchPaths, path);
                }
            }
        }

        std::unique_ptr<Assets::EntityModel> GameImpl::doInitializeModel(const IO::Path& path, Logger& logger) const {
            try {
                auto file = m_fs.openFile(path);
                ensure(file != nullptr, "file is null");

                const auto modelName = path.lastComponent().asString();
                const auto extension = kdl::str_to_lower(path.extension());
                const auto supported = m_config.entityConfig().modelFormats;

                if (extension == "mdl" && kdl::vec_contains(supported, "mdl")) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::MdlParser parser(modelName, std::begin(reader), std::end(reader), palette);
                    return parser.initializeModel(logger);
                } else if (extension == "md2" && kdl::vec_contains(supported, "md2")) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Md2Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    return parser.initializeModel(logger);
                } else if (extension == "md3" && kdl::vec_contains(supported, "md3")) {
                    auto reader = file->reader().buffer();
                    IO::Md3Parser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (extension == "mdx" && kdl::vec_contains(supported, "mdx")) {
                    auto reader = file->reader().buffer();
                    IO::MdxParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (extension == "bsp" && kdl::vec_contains(supported, "bsp")) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Bsp29Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    return parser.initializeModel(logger);
                } else if (extension == "dkm" && kdl::vec_contains(supported, "dkm")) {
                    auto reader = file->reader().buffer();
                    IO::DkmParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (extension == "ase" && kdl::vec_contains(supported, "ase")) {
                    auto reader = file->reader().buffer();
                    IO::AseParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (extension == "obj" && kdl::vec_contains(supported, "obj_neverball")) {
                    auto reader = file->reader().buffer();
                    // has to be the whole path for implicit textures!
                    IO::NvObjParser parser(path, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else {
                    throw GameException("Unsupported model format '" + path.asString() + "'");
                }
            } catch (const FileSystemException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + std::string(e.what()));
            } catch (const AssetException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + std::string(e.what()));
            } catch (const ParserException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + std::string(e.what()));
            }
        }

        void GameImpl::doLoadFrame(const IO::Path& path, size_t frameIndex, Assets::EntityModel& model, Logger& logger) const {
            try {
                ensure(model.frame(frameIndex) != nullptr, "invalid frame index");
                ensure(!model.frame(frameIndex)->loaded(), "frame already loaded");

                const auto file = m_fs.openFile(path);
                ensure(file != nullptr, "file is null");

                const auto modelName = path.lastComponent().asString();
                const auto extension = kdl::str_to_lower(path.extension());
                const auto supported = m_config.entityConfig().modelFormats;

                if (extension == "mdl" && kdl::vec_contains(supported, "mdl")) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::MdlParser parser(modelName, std::begin(reader), std::end(reader), palette);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "md2" && kdl::vec_contains(supported, "md2")) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Md2Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "md3" && kdl::vec_contains(supported, "md3")) {
                    auto reader = file->reader().buffer();
                    IO::Md3Parser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "mdx" && kdl::vec_contains(supported, "mdx")) {
                    auto reader = file->reader().buffer();
                    IO::MdxParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "bsp" && kdl::vec_contains(supported, "bsp")) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Bsp29Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "dkm" && kdl::vec_contains(supported, "dkm")) {
                    auto reader = file->reader().buffer();
                    IO::DkmParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "ase" && kdl::vec_contains(supported, "ase")) {
                    auto reader = file->reader().buffer();
                    IO::AseParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (extension == "obj" && kdl::vec_contains(supported, "obj_neverball")) {
                    auto reader = file->reader().buffer();
                    // has to be the whole path for implicit textures!
                    IO::NvObjParser parser(path, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else {
                    throw GameException("Unsupported model format '" + path.asString() + "'");
                }
            } catch (FileSystemException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + std::string(e.what()));
            } catch (AssetException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + std::string(e.what()));
            }
        }

        Assets::Palette GameImpl::loadTexturePalette() const {
            const auto& path = m_config.textureConfig().palette;
            return Assets::Palette::loadFile(m_fs, path);
        }

        std::vector<std::string> GameImpl::doAvailableMods() const {
            std::vector<std::string> result;
            if (m_gamePath.isEmpty() || !IO::Disk::directoryExists(m_gamePath)) {
                return result;
            }

            const auto& defaultMod = m_config.fileSystemConfig().searchPath.lastComponent().asString();
            const IO::DiskFileSystem fs(m_gamePath);
            const auto subDirs = fs.findItems(IO::Path(""), IO::FileTypeMatcher(false, true));
            for (size_t i = 0; i < subDirs.size(); ++i) {
                const std::string mod = subDirs[i].lastComponent().asString();
                if (!kdl::ci::str_is_equal(mod, defaultMod)) {
                    result.push_back(mod);
                }
            }
            return result;
        }

        std::vector<std::string> GameImpl::doExtractEnabledMods(const AttributableNode& node) const {
            std::vector<std::string> result;
            const auto& modStr = node.attribute(AttributeNames::Mods);
            if (modStr.empty()) {
                return result;
            }

            return kdl::str_split(modStr, ";");
        }

        std::string GameImpl::doDefaultMod() const {
            return m_config.fileSystemConfig().searchPath.asString();
        }

        const FlagsConfig& GameImpl::doSurfaceFlags() const {
            return m_config.faceAttribsConfig().surfaceFlags;
        }

        const FlagsConfig& GameImpl::doContentFlags() const {
            return m_config.faceAttribsConfig().contentFlags;
        }

        const BrushFaceAttributes& GameImpl::doDefaultFaceAttribs() const {
            return m_config.faceAttribsConfig().defaults;
        }

        void GameImpl::writeLongAttribute(AttributableNode& node, const std::string& baseName, const std::string& value, const size_t maxLength) const {
            node.removeNumberedAttribute(baseName);

            std::stringstream nameStr;
            for (size_t i = 0; i <= value.size() / maxLength; ++i) {
                nameStr.str("");
                nameStr << baseName << i+1;
                node.addOrUpdateAttribute(nameStr.str(), value.substr(i * maxLength, maxLength));
            }
        }

        std::string GameImpl::readLongAttribute(const AttributableNode& node, const std::string& baseName) const {
            size_t index = 1;
            std::stringstream nameStr;
            std::stringstream valueStr;
            nameStr << baseName << index;
            while (node.hasAttribute(nameStr.str())) {
                valueStr << node.attribute(nameStr.str());
                nameStr.str("");
                nameStr << baseName << ++index;
            }

            return valueStr.str();
        }
    }
}
