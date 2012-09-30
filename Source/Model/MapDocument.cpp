/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapDocument.h"

#include "Controller/Command.h"
#include "IO/FileManager.h"
#include "IO/MapParser.h"
#include "IO/Wad.h"
#include "IO/mmapped_fstream.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "Model/Octree.h"
#include "Model/Picker.h"
#include "Model/TextureManager.h"
#include "Renderer/SharedResources.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/VecMath.h"
#include "View/EditorView.h"
#include "View/ProgressIndicatorDialog.h"

#include <cassert>

#include <wx/stopwatch.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
		IMPLEMENT_DYNAMIC_CLASS(MapDocument, wxDocument)
        
        bool MapDocument::DoOpenDocument(const wxString& file) {
            console().info("Unloading existing map file and textures...");
            clear();

            console().info("Loading file %s", file.mbc_str().data());
            mmapped_fstream stream(file.mbc_str().data(), std::ios::in);
            if (!stream.is_open() || !stream.good())
                return false;
            
            LoadObject(stream);
            return true;
        }
        
        bool MapDocument::DoSaveDocument(const wxString& file) {
            return wxDocument::DoSaveDocument(file);
        }

        void MapDocument::updateEntityDefinitions() {
            const EntityList& entities = m_map->entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity& entity = *entities[i];
                const PropertyValue* classname = entity.classname();
                
                if (classname != NULL) {
                    EntityDefinition* definition = m_definitionManager->definition(*classname);
                    entity.setDefinition(definition);
                }
            }
        }

        void MapDocument::clear() {
            Controller::Command clearCommand(Controller::Command::ClearMap);
            UpdateAllViews(NULL, &clearCommand);

            m_editStateManager->clear();
            m_map->clear();
            m_octree->clear();
            m_textureManager->clear();
            m_definitionManager->clear();
        }
        
        void MapDocument::loadPalette() {
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            String palettePath = fileManager.appendPath(resourcePath, "QuakePalette.lmp");
            m_sharedResources->loadPalette(palettePath);
        }
        
        void MapDocument::loadMap(std::istream& stream, Utility::ProgressIndicator& progressIndicator) {
			progressIndicator.setText("Loading map file...");

            wxStopWatch watch;
            IO::MapParser parser(stream, console());
            parser.parseMap(*m_map, &progressIndicator);
            m_octree->loadMap();
            stream.clear(); // everything went well, prevent wx from displaying an error dialog
            
            console().info("Loaded map file in %f seconds", watch.Time() / 1000.0f);
        }

        void MapDocument::loadTextures(Utility::ProgressIndicator& progressIndicator) {
            progressIndicator.setText("Loading textures...");
            
            const String* wads = m_map->worldspawn(true)->propertyForKey(Entity::WadKey);
            if (wads != NULL) {
                StringList wadPaths = Utility::split(*wads, ';');
                for (unsigned int i = 0; i < wadPaths.size(); i++) {
                    String wadPath = Utility::trim(wadPaths[i]);
                    loadTextureWad(wadPath);
                }
            }
        }
        
        void MapDocument::loadEntityDefinitions(Utility::ProgressIndicator& progressIndicator) {
            progressIndicator.setText("Loading entity definitions...");

            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            String defPath = fileManager.appendPath(resourcePath, "Quake.def");
            console().info("Loading entity definition file %s", defPath.c_str());
            
            m_definitionManager->load(defPath);
        }
        
        std::istream& MapDocument::LoadObject(std::istream& stream) {
//            wxDocument::LoadObject(stream);

            View::ProgressIndicatorDialog progressIndicator;
            loadMap(stream, progressIndicator);
            loadTextures(progressIndicator);
            loadEntityDefinitions(progressIndicator);

            updateAfterTextureManagerChanged();
            updateEntityDefinitions();
            
            return stream;
        }
        
        std::ostream& MapDocument::SaveObject(std::ostream& stream) {
            return wxDocument::SaveObject(stream);
        }
        
        MapDocument::MapDocument() :
        m_console(NULL),
        m_sharedResources(NULL),
        m_map(NULL),
        m_editStateManager(NULL),
        m_octree(NULL),
        m_picker(NULL),
        m_textureManager(NULL),
        m_definitionManager(NULL),
        m_grid(new Utility::Grid(4)),
        m_mruTexture(NULL),
        m_mruTextureName("") {}
        
        MapDocument::~MapDocument() {
            if (m_picker != NULL) {
                delete m_picker;
                m_picker = NULL;
            }
            
            if (m_octree != NULL) {
                delete m_octree;
                m_octree = NULL;
            }
            
            if (m_editStateManager != NULL) {
                delete m_editStateManager;
                m_editStateManager = NULL;
            }
            
            if (m_map != NULL) {
                delete m_map;
                m_map = NULL;
            }
            
            if (m_definitionManager != NULL) {
                delete m_definitionManager;
                m_definitionManager = NULL;
            }
            
            if (m_textureManager != NULL) {
                delete m_textureManager;
                m_textureManager = NULL;
            }
            
            if (m_grid != NULL) {
                delete m_grid;
                m_grid = NULL;
            }
            
            if (m_sharedResources != NULL) {
                m_sharedResources->Destroy(); // makes sure that the resources are deleted after the last frame
                m_sharedResources = NULL;
            }
            
            if (m_console != NULL) {
                delete m_console;
                m_console = NULL;
            }
        }
        
        Utility::Console& MapDocument::console() const {
            return *m_console;
        }
        
        Renderer::SharedResources& MapDocument::sharedResources() const {
            return *m_sharedResources;
        }

        Map& MapDocument::map() const {
            return *m_map;
        }
        
        EntityDefinitionManager& MapDocument::definitionManager() const {
            return *m_definitionManager;
        }

        EditStateManager& MapDocument::editStateManager() const {
            return *m_editStateManager;
        }

        TextureManager& MapDocument::textureManager() const {
            return *m_textureManager;
        }

        Picker& MapDocument::picker() const {
            return *m_picker;
        }

        Utility::Grid& MapDocument::grid() const {
            return *m_grid;
        }
        
        const StringList& MapDocument::mods() const {
            return m_mods;
        }
        
        Model::Texture* MapDocument::mruTexture() const {
            return m_mruTexture;
        }
        
        void MapDocument::setMruTexture(Model::Texture* texture) {
            if (texture == NULL)
                m_mruTextureName = "";
            else
                m_mruTextureName = texture->name();
            m_mruTexture = texture;
        }

        void MapDocument::updateAfterTextureManagerChanged() {
            Model::FaceList changedFaces;
            Model::TextureList newTextures;
            
            const Model::EntityList& entities = m_map->entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                const Model::BrushList& brushes = entities[i]->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    const Model::FaceList& faces = brushes[j]->faces();
                    for (unsigned int k = 0; k < faces.size(); k++) {
                        const String& textureName = faces[k]->textureName();
                        Model::Texture* oldTexture = faces[k]->texture();
                        Model::Texture* newTexture = m_textureManager->texture(textureName);
                        if (oldTexture != newTexture) {
                            changedFaces.push_back(faces[k]);
                            newTextures.push_back(newTexture);
                        }
                    }
                }
            }
            
            if (!changedFaces.empty()) {
                for (unsigned int i = 0; i < changedFaces.size(); i++)
                    changedFaces[i]->setTexture(newTextures[i]);
            }
            
            if (m_mruTexture != NULL && m_mruTexture != m_textureManager->texture(m_mruTextureName))
                setMruTexture(NULL);
        }
        
        void MapDocument::loadTextureWad(const String& path) {
            loadTextureWad(path, m_textureManager->collections().size());
        }
        
        void MapDocument::loadTextureWad(const String& path, size_t index) {
            IO::FileManager fileManager;
            
            String collectionName = path;
            String wadPath = path;
            String mapPath = GetFilename().ToStdString();
            if (!fileManager.exists(wadPath) && !mapPath.empty()) {
                String folderPath = fileManager.deleteLastPathComponent(mapPath);
                wadPath = fileManager.appendPath(folderPath, wadPath);
            }
            
            if (fileManager.exists(wadPath)) {
                wxStopWatch watch;
                Model::TextureCollection* collection = new Model::TextureCollection(collectionName, wadPath);
                m_textureManager->addCollection(collection, index);
                console().info("Loaded %s in %f seconds", wadPath.c_str(), watch.Time() / 1000.0f);
            } else {
                console().error("Could not open texture wad %s", path.c_str());
            }
        }

        bool MapDocument::OnCreate(const wxString& path, long flags) {
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));

            m_console = new Utility::Console();
            m_textureManager = new TextureManager();
            m_sharedResources = new Renderer::SharedResources(*m_textureManager, *m_console);
            m_map = new Model::Map(worldBounds);
            m_editStateManager = new Model::EditStateManager();
            m_octree = new Octree(*m_map);
            m_picker = new Model::Picker(*m_octree);
            m_definitionManager = new EntityDefinitionManager();
            m_mods.push_back("id1");
            m_mods.push_back("ID1");

            loadPalette();
            
            return wxDocument::OnCreate(path, flags);
        }
 
		bool MapDocument::OnNewDocument() {
			if (wxDocument::OnNewDocument()) {
				// prompt for initial stuff like world bounds, mods, palette, def here
                clear();
                
                Controller::Command loadCommand(Controller::Command::LoadMap);
                UpdateAllViews(NULL, &loadCommand);
				return true;
			}

			return false;
		}

        bool MapDocument::OnOpenDocument(const wxString& path) {
            if (wxDocument::OnOpenDocument(path)) {
                Controller::Command loadCommand(Controller::Command::LoadMap);
                UpdateAllViews(NULL, &loadCommand);
				return true;
            }
            
            return false;
        }
	}
}