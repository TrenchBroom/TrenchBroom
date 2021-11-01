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
#include "Logger.h"
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
#include "IO/GameConfigParser.h"
#include "IO/IOUtils.h"
#include "IO/ImageSpriteParser.h"
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
#include "IO/SprParser.h"
#include "IO/SystemPaths.h"
#include "IO/TextureLoader.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushError.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/ExportFormat.h"
#include "Model/GameConfig.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/vec_io.h>

#include <fstream>
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
            return m_config.name;
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

        const CompilationConfig& GameImpl::doCompilationConfig() {
            return m_config.compilationConfig;
        }

        size_t GameImpl::doMaxPropertyLength() const {
            return m_config.maxPropertyLength;
        }

        std::optional<vm::bbox3> GameImpl::doSoftMapBounds() const {
            return m_config.softMapBounds;
        }

        Game::SoftMapBounds GameImpl::doExtractSoftMapBounds(const Entity& entity) const {
            if (!entity.hasProperty(EntityPropertyKeys::SoftMapBounds)) {
                // Not set in map -> use Game value
                return {SoftMapBoundsType::Game, doSoftMapBounds()};
            }

            if (const auto* mapValue = entity.property(EntityPropertyKeys::SoftMapBounds); mapValue && *mapValue != EntityPropertyValues::NoSoftMapBounds) {
                return {SoftMapBoundsType::Map, IO::parseSoftMapBoundsString(*mapValue)};
            } else {
                return {SoftMapBoundsType::Map, std::nullopt};
            }
        }

        const std::vector<SmartTag>& GameImpl::doSmartTags() const {
            return m_config.smartTags;
        }

        std::unique_ptr<WorldNode> GameImpl::doNewMap(const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const {
            const auto initialMapFilePath = m_config.findInitialMap(formatName(format));
            if (!initialMapFilePath.isEmpty() && IO::Disk::fileExists(initialMapFilePath)) {
                return doLoadMap(format, worldBounds, initialMapFilePath, logger);
            } else {
                auto propertyConfig = entityPropertyConfig();
                auto worldEntity = Model::Entity{};
                if (format == MapFormat::Valve || format == MapFormat::Quake2_Valve || format == MapFormat::Quake3_Valve) {
                    worldEntity.addOrUpdateProperty(entityPropertyConfig(), EntityPropertyKeys::ValveVersion, "220");
                }

                auto worldNode = std::make_unique<WorldNode>(std::move(propertyConfig), std::move(worldEntity), format);

                const Model::BrushBuilder builder(worldNode->mapFormat(), worldBounds, defaultFaceAttribs());
                builder.createCuboid(vm::vec3(128.0, 128.0, 32.0), Model::BrushFaceAttributes::NoTextureName).
                    visit(kdl::overload(
                        [&](Brush&& b) {
                            worldNode->defaultLayer()->addChild(new BrushNode(std::move(b)));
                        },
                        [&](const Model::BrushError e) {
                            logger.error() << "Could not create default brush: " << e;
                        }
                    ));

                return worldNode;
            }
        }

        std::unique_ptr<WorldNode> GameImpl::doLoadMap(const MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const {
            const auto entityPropertyConfig = Model::EntityPropertyConfig{m_config.entityConfig.scaleExpression};
            IO::SimpleParserStatus parserStatus(logger);
            auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
            auto fileReader = file->reader().buffer();
            if (format == MapFormat::Unknown) {
                // Try all formats listed in the game config
                const auto possibleFormats = kdl::vec_transform(m_config.fileFormats, [](const MapFormatConfig& config) {
                    return Model::formatFromName(config.format);
                });
                return IO::WorldReader::tryRead(fileReader.stringView(), possibleFormats, worldBounds, entityPropertyConfig, parserStatus);
            } else {
                IO::WorldReader worldReader(fileReader.stringView(), format, entityPropertyConfig);
                return worldReader.read(worldBounds, parserStatus);
            }
        }

        void GameImpl::doWriteMap(WorldNode& world, const IO::Path& path, const bool exporting) const {
            const auto mapFormatName = formatName(world.mapFormat());

            std::ofstream file = openPathAsOutputStream(path);
            if (!file) {
                throw FileSystemException("Cannot open file: " + path.asString());
            }
            IO::writeGameComment(file, gameName(), mapFormatName);

            IO::NodeWriter writer(world, file);
            writer.setExporting(exporting);
            writer.writeMap();
        }

        void GameImpl::doWriteMap(WorldNode& world, const IO::Path& path) const {
            doWriteMap(world, path, false);
        }

        void GameImpl::doExportMap(WorldNode& world, const Model::ExportFormat format, const IO::Path& path) const {
            switch (format) {
                case Model::ExportFormat::WavefrontObj: {
                    std::ofstream objFile = openPathAsOutputStream(path);
                    if (!objFile) {
                        throw FileSystemException("Cannot open file: " + path.asString());
                    }

                    const auto mtlPath = path.replaceExtension("mtl");
                    std::ofstream mtlFile = openPathAsOutputStream(mtlPath);
                    if (!mtlFile) {
                        throw FileSystemException("Cannot open file: " + mtlPath.asString());
                    }

                    IO::NodeWriter writer(world, std::make_unique<IO::ObjSerializer>(objFile, mtlFile, mtlPath.filename()));
                    writer.setExporting(true);
                    writer.writeMap();
                    break;
                }
                case Model::ExportFormat::Map:
                    doWriteMap(world, path, true);
                    break;
            }
        }

        std::vector<Node*> GameImpl::doParseNodes(const std::string& str, const MapFormat mapFormat, const vm::bbox3& worldBounds, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            return IO::NodeReader::read(str, mapFormat, worldBounds, entityPropertyConfig(), parserStatus);
        }

        std::vector<BrushFace> GameImpl::doParseBrushFaces(const std::string& str, const MapFormat mapFormat, const vm::bbox3& worldBounds, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::BrushFaceReader reader(str, mapFormat);
            return reader.read(worldBounds, parserStatus);
        }

        void GameImpl::doWriteNodesToStream(WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }

        void GameImpl::doWriteBrushFacesToStream(WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }

        Game::TexturePackageType GameImpl::doTexturePackageType() const {
            using Model::GameConfig;
            return std::visit(kdl::overload(
                [](const TextureFilePackageConfig&) {
                    return TexturePackageType::File;
                },
                [](const TextureDirectoryPackageConfig&) {
                    return TexturePackageType::Directory;
                }
            ), m_config.textureConfig.package);
        }

        void GameImpl::doLoadTextureCollections(const Entity& entity, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const {
            const auto paths = extractTextureCollections(entity);

            const auto fileSearchPaths = textureCollectionSearchPaths(documentPath);
            IO::TextureLoader textureLoader(m_fs, fileSearchPaths, m_config.textureConfig, logger);
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
            return std::visit(kdl::overload(
                [&](const TextureFilePackageConfig& filePackageConfig) {
                    return path.hasExtension(filePackageConfig.fileFormat.extensions, false);
                },
                [](const TextureDirectoryPackageConfig&) {
                    return false;
                }
            ), m_config.textureConfig.package);
        }

        std::vector<IO::Path> GameImpl::doFindTextureCollections() const {
            try {
                const auto searchPath = getRootDirectory(m_config.textureConfig.package);
                if (!searchPath.isEmpty() && m_fs.directoryExists(searchPath)) {
                    return kdl::vec_concat(std::vector<IO::Path>({searchPath}), m_fs.findItemsRecursively(searchPath, IO::FileTypeMatcher(false, true)));
                }
                return std::vector<IO::Path>();
            } catch (FileSystemException& e) {
                throw GameException("Could not find texture collections: " + std::string(e.what()));
            }
        }

        std::vector<std::string> GameImpl::doFileTextureCollectionExtensions() const {
            return std::visit(kdl::overload(
                [](const TextureFilePackageConfig& filePackageConfig) {
                    return filePackageConfig.fileFormat.extensions;
                },
                [](const TextureDirectoryPackageConfig&) {
                    return std::vector<std::string>{};
                }
            ), m_config.textureConfig.package);
        }

        std::vector<IO::Path> GameImpl::doExtractTextureCollections(const Entity& entity) const {
            const auto& property = m_config.textureConfig.property;
            if (property.empty()) {
                return {};
            }

            const auto* pathsValue = entity.property(property);
            if (!pathsValue) {
                return {};
            }
            
            return IO::Path::asPaths(kdl::str_split(*pathsValue, ";"));
        }

        void GameImpl::doUpdateTextureCollections(Entity& entity, const std::vector<IO::Path>& paths) const {
            const auto& attribute = m_config.textureConfig.property;
            if (attribute.empty()) {
                return;
            }

            const auto value = kdl::str_join(IO::Path::asStrings(paths, "/"), ";");
            entity.addOrUpdateProperty(entityPropertyConfig(), attribute, value);
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
            const auto& defaultColor = m_config.entityConfig.defaultColor;

            if (kdl::ci::str_is_equal("fgd", extension)) {
                auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                auto reader = file->reader().buffer();
                IO::FgdParser parser(reader.stringView(), defaultColor, file->path());
                return parser.parseDefinitions(status);
            } else if (kdl::ci::str_is_equal("def", extension)) {
                auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                auto reader = file->reader().buffer();
                IO::DefParser parser(reader.stringView(), defaultColor);
                return parser.parseDefinitions(status);
            } else if (kdl::ci::str_is_equal("ent", extension)) {
                auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                auto reader = file->reader().buffer();
                IO::EntParser parser(reader.stringView(), defaultColor);
                return parser.parseDefinitions(status);
            } else {
                throw GameException("Unknown entity definition format: '" + path.asString() + "'");
            }
        }

        std::vector<Assets::EntityDefinitionFileSpec> GameImpl::doAllEntityDefinitionFiles() const {
            const auto paths = m_config.entityConfig.defFilePaths;
            const auto count = paths.size();

            std::vector<Assets::EntityDefinitionFileSpec> result;
            result.reserve(count);

            for (const auto& path : paths) {
                result.push_back(Assets::EntityDefinitionFileSpec::builtin(path));
            }

            return result;
        }

        Assets::EntityDefinitionFileSpec GameImpl::doExtractEntityDefinitionFile(const Entity& entity) const {
            if (const auto* defValue = entity.property(EntityPropertyKeys::EntityDefinitions)) {
                return Assets::EntityDefinitionFileSpec::parse(*defValue);
            } else {
                return defaultEntityDefinitionFile();
            }
        }

        Assets::EntityDefinitionFileSpec GameImpl::defaultEntityDefinitionFile() const {
            const auto paths = m_config.entityConfig.defFilePaths;
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

        static bool isModelFormat(const std::string& candidate, const std::string& extension, const std::vector<std::string>& supported) {
            return candidate == extension && kdl::vec_contains(supported, candidate);
        }

        std::unique_ptr<Assets::EntityModel> GameImpl::doInitializeModel(const IO::Path& path, Logger& logger) const {
            try {
                auto file = m_fs.openFile(path);
                ensure(file != nullptr, "file is null");

                const auto modelName = path.lastComponent().asString();
                const auto extension = kdl::str_to_lower(path.extension());
                const auto supported = m_config.entityConfig.modelFormats;

                if (isModelFormat("mdl", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::MdlParser parser(modelName, std::begin(reader), std::end(reader), palette);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("md2", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Md2Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("md3", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::Md3Parser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("mdx", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::MdxParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("bsp", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Bsp29Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("dkm", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::DkmParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("ase", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::AseParser parser(modelName, reader.stringView(), m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("obj", extension, supported)) {
                    auto reader = file->reader().buffer();
                    // has to be the whole path for implicit textures!
                    IO::NvObjParser parser(path, std::begin(reader), std::end(reader), m_fs);
                    return parser.initializeModel(logger);
                } else if (isModelFormat("png", extension, supported)) {
                    IO::ImageSpriteParser parser{modelName, file, m_fs};
                    return parser.initializeModel(logger);
                } else if (isModelFormat("spr", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::SprParser parser{modelName, std::begin(reader), std::end(reader), palette};
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
                const auto supported = m_config.entityConfig.modelFormats;

                if (isModelFormat("mdl", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::MdlParser parser(modelName, std::begin(reader), std::end(reader), palette);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("md2", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Md2Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("md3", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::Md3Parser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("mdx", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::MdxParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("bsp", extension, supported)) {
                    const auto palette = loadTexturePalette();
                    auto reader = file->reader().buffer();
                    IO::Bsp29Parser parser(modelName, std::begin(reader), std::end(reader), palette, m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("dkm", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::DkmParser parser(modelName, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("ase", extension, supported)) {
                    auto reader = file->reader().buffer();
                    IO::AseParser parser(modelName, reader.stringView(), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("obj", extension, supported)) {
                    auto reader = file->reader().buffer();
                    // has to be the whole path for implicit textures!
                    IO::NvObjParser parser(path, std::begin(reader), std::end(reader), m_fs);
                    parser.loadFrame(frameIndex, model, logger);
                } else if (isModelFormat("png", extension, supported)) {
                    IO::ImageSpriteParser parser{modelName, file, m_fs};
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
            const auto& path = m_config.textureConfig.palette;
            return Assets::Palette::loadFile(m_fs, path);
        }

        std::vector<std::string> GameImpl::doAvailableMods() const {
            std::vector<std::string> result;
            if (m_gamePath.isEmpty() || !IO::Disk::directoryExists(m_gamePath)) {
                return result;
            }

            const auto& defaultMod = m_config.fileSystemConfig.searchPath.lastComponent().asString();
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

        std::vector<std::string> GameImpl::doExtractEnabledMods(const Entity& entity) const {
            if (const auto* modStr = entity.property(EntityPropertyKeys::Mods)) {
                return kdl::str_split(*modStr, ";");
            } else {
                return {};
            }
        }

        std::string GameImpl::doDefaultMod() const {
            return m_config.fileSystemConfig.searchPath.asString();
        }

        const FlagsConfig& GameImpl::doSurfaceFlags() const {
            return m_config.faceAttribsConfig.surfaceFlags;
        }

        const FlagsConfig& GameImpl::doContentFlags() const {
            return m_config.faceAttribsConfig.contentFlags;
        }

        const BrushFaceAttributes& GameImpl::doDefaultFaceAttribs() const {
            return m_config.faceAttribsConfig.defaults;
        }

        const std::vector<CompilationTool>& GameImpl::doCompilationTools() const {
            return m_config.compilationTools;
        }

        EntityPropertyConfig GameImpl::entityPropertyConfig() const {
            return EntityPropertyConfig{m_config.entityConfig.scaleExpression};
        }

        void GameImpl::writeLongAttribute(EntityNodeBase& node, const std::string& baseName, const std::string& value, const size_t maxLength) const {
            auto entity = node.entity();
            entity.removeNumberedProperty(entityPropertyConfig(), baseName);

            std::stringstream nameStr;
            for (size_t i = 0; i <= value.size() / maxLength; ++i) {
                nameStr.str("");
                nameStr << baseName << i+1;
                entity.addOrUpdateProperty(entityPropertyConfig(), nameStr.str(), value.substr(i * maxLength, maxLength));
            }

            node.setEntity(std::move(entity));
        }

        std::string GameImpl::readLongAttribute(const EntityNodeBase& node, const std::string& baseName) const {
            size_t index = 1;
            std::stringstream nameStr;
            std::stringstream valueStr;
            nameStr << baseName << index;

            const auto& entity = node.entity();
            while (entity.hasProperty(nameStr.str())) {
                if (const auto* value = entity.property(nameStr.str())) {
                    valueStr << *value;
                }
                nameStr.str("");
                nameStr << baseName << ++index;
            }

            return valueStr.str();
        }
    }
}
