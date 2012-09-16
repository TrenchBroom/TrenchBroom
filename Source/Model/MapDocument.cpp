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
#include "Model/Palette.h"
#include "Model/Picker.h"
#include "Model/TextureManager.h"
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
            Console().info("Unloading existing map file and textures...");
            Clear();

            Console().info("Loading file %s", file.mbc_str().data());
            mmapped_fstream stream(file.mbc_str().data(), std::ios::in);
            if (!stream.is_open() || !stream.good())
                return false;
            
            LoadObject(stream);
            return true;
        }
        
        bool MapDocument::DoSaveDocument(const wxString& file) {
            return wxDocument::DoSaveDocument(file);
        }

        void MapDocument::UpdateFaceTextures() {
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
        }
        
        void MapDocument::UpdateEntityDefinitions() {
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

        void MapDocument::Clear() {
            Controller::Command clearCommand(Controller::Command::ClearMap);
            UpdateAllViews(NULL, &clearCommand);

            m_editStateManager->clear();
            m_map->clear();
            m_octree->clear();
            m_textureManager->clear();
            m_definitionManager->clear();
        }
        
        void MapDocument::LoadPalette() {
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            String palettePath = fileManager.appendPath(resourcePath, "QuakePalette.lmp");
            if (m_palette != NULL)
                delete m_palette;
            m_palette = new Model::Palette(palettePath);
        }
        
        void MapDocument::LoadMap(std::istream& stream, Utility::ProgressIndicator& progressIndicator) {
			progressIndicator.setText("Loading map file...");

            wxStopWatch watch;
            IO::MapParser parser(stream, Console());
            parser.parseMap(*m_map, &progressIndicator);
            m_octree->loadMap();
            stream.clear(); // everything went well, prevent wx from displaying an error dialog
            
            Console().info("Loaded map file in %f seconds", watch.Time() / 1000.0f);
        }

        void MapDocument::LoadTextureWad(const String& path) {
            IO::FileManager fileManager;
            
            String wadPath = path;
            String mapPath = GetFilename().ToStdString();
            if (!fileManager.exists(wadPath) && !mapPath.empty()) {
                String folderPath = fileManager.deleteLastPathComponent(mapPath);
                wadPath = fileManager.appendPath(folderPath, wadPath);
            }
            
            if (fileManager.exists(wadPath)) {
                wxStopWatch watch;
                IO::Wad wad(wadPath);
                Model::TextureCollection* collection = new Model::TextureCollection(wadPath, wad, *m_palette);
                unsigned int index = static_cast<unsigned int>(m_textureManager->collections().size());
                m_textureManager->addCollection(collection, index);
                Console().info("Loaded %s in %f seconds", wadPath.c_str(), watch.Time() / 1000.0f);
            } else {
                Console().error("Could not open texture wad %s", path.c_str());
            }
        }
        
        void MapDocument::LoadTextures(Utility::ProgressIndicator& progressIndicator) {
            progressIndicator.setText("Loading textures...");
            
            const String* wads = m_map->worldspawn(true)->propertyForKey(Entity::WadKey);
            if (wads != NULL) {
                StringList wadPaths = Utility::split(*wads, ';');
                for (unsigned int i = 0; i < wadPaths.size(); i++) {
                    String wadPath = Utility::trim(wadPaths[i]);
                    LoadTextureWad(wadPath);
                }
            }
        }
        
        void MapDocument::LoadEntityDefinitions(Utility::ProgressIndicator& progressIndicator) {
            progressIndicator.setText("Loading entity definitions...");

            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            String defPath = fileManager.appendPath(resourcePath, "Quake.def");
            Console().info("Loading entity definition file %s", defPath.c_str());
            
            m_definitionManager->load(defPath);
        }
        
        std::istream& MapDocument::LoadObject(std::istream& stream) {
//            wxDocument::LoadObject(stream);

            View::ProgressIndicatorDialog progressIndicator;
            LoadMap(stream, progressIndicator);
            LoadTextures(progressIndicator);
            LoadEntityDefinitions(progressIndicator);

            UpdateFaceTextures();
            UpdateEntityDefinitions();
            
            return stream;
        }
        
        std::ostream& MapDocument::SaveObject(std::ostream& stream) {
            return wxDocument::SaveObject(stream);
        }
        
        MapDocument::MapDocument() :
        m_map(NULL),
        m_editStateManager(NULL),
        m_octree(NULL),
        m_picker(NULL),
        m_palette(NULL),
        m_textureManager(NULL),
        m_definitionManager(NULL),
        m_grid(new Utility::Grid(4)) {}
        
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
            
            if (m_palette != NULL) {
                delete m_palette;
                m_palette = NULL;
            }
            
            if (m_textureManager != NULL) {
                delete m_textureManager;
                m_textureManager = NULL;
            }
            
            if (m_grid != NULL) {
                delete m_grid;
                m_grid = NULL;
            }
        }
        
        Map& MapDocument::Map() const {
            return *m_map;
        }
        
        EditStateManager& MapDocument::EditStateManager() const {
            return *m_editStateManager;
        }

        Picker& MapDocument::Picker() const {
            return *m_picker;
        }

        Utility::Grid& MapDocument::Grid() const {
            return *m_grid;
        }
        
        Utility::Console& MapDocument::Console() const {
            View::EditorView* editorView = dynamic_cast<View::EditorView*>(GetFirstView());
            assert(editorView != NULL);
            return editorView->console();
        }

        const StringList& MapDocument::Mods() const {
            return m_mods;
        }
        
        const Palette& MapDocument::Palette() const {
            return *m_palette;
        }

        bool MapDocument::OnCreate(const wxString& path, long flags) {
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));

            m_map = new Model::Map(worldBounds);
            m_editStateManager = new Model::EditStateManager();
            m_octree = new Octree(*m_map);
            m_picker = new Model::Picker(*m_octree);
            m_textureManager = new TextureManager();
            m_definitionManager = new EntityDefinitionManager();
            m_mods.push_back("id1");

            LoadPalette();
            
            return wxDocument::OnCreate(path, flags);
        }
 
		bool MapDocument::OnNewDocument() {
			if (wxDocument::OnNewDocument()) {
				// prompt for initial stuff like world bounds, mods, palette, def here
                Clear();
                
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