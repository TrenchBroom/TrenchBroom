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

#include "IO/FileManager.h"
#include "IO/MapParser.h"
#include "IO/Wad.h"
#include "IO/mmapped_fstream.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "Model/Palette.h"
#include "Model/TextureManager.h"
#include "Utility/Console.h"
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

        std::istream& MapDocument::LoadObject(std::istream& stream) {
//            wxDocument::LoadObject(stream);

            View::ProgressIndicatorDialog progressIndicator;
            progressIndicator.setText("Loading map file...");
            
            wxStopWatch watch;
            IO::MapParser parser(stream, Console());
            parser.parseMap(*m_map, &progressIndicator);
            stream.clear(); // everything went well, prevent wx from displaying an error dialog
            Console().info("Loaded map file in %f seconds", watch.Time() / 1000.0f);

            // load palette, in the future, we might get the path from the map's config
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            String palettePath = fileManager.appendPath(resourcePath, "QuakePalette.lmp");
            if (m_palette != NULL)
                delete m_palette;
            
            Console().info("Loading palette file %s", palettePath.c_str());
            m_palette = new Palette(palettePath);
            
            const String* wads = m_map->worldspawn(true)->propertyForKey(Entity::WadKey);
            if (wads != NULL) {
                StringList wadPaths = Utility::split(*wads, ';');
                for (unsigned int i = 0; i < wadPaths.size(); i++) {
                    String wadPath = Utility::trim(wadPaths[i]);
                    LoadTextureWad(wadPath);
                }
            }
            
            UpdateFaceTextures();
            
            return stream;
        }
        
        std::ostream& MapDocument::SaveObject(std::ostream& stream) {
            return wxDocument::SaveObject(stream);
        }
        
        MapDocument::MapDocument() : m_map(NULL), m_palette(NULL), m_textureManager(NULL) {}
        
        MapDocument::~MapDocument() {
            if (m_map != NULL) {
                delete m_map;
                m_map = NULL;
            }
            if (m_palette != NULL) {
                delete m_palette;
                m_palette = NULL;
            }
            if (m_textureManager != NULL) {
                delete m_textureManager;
                m_textureManager = NULL;
            }
        }
        
        Model::Map& MapDocument::Map() const {
            return *m_map;
        }
        
        Utility::Console& MapDocument::Console() const {
            View::EditorView* editorView = dynamic_cast<View::EditorView*>(GetFirstView());
            assert(editorView != NULL);
            return editorView->Console();
        }

        bool MapDocument::OnCreate(const wxString& path, long flags) {
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));
            m_map = new Model::Map(worldBounds);
            m_textureManager = new TextureManager();
            
            // initialize here
            
            return wxDocument::OnCreate(path, flags);
        }
    }
}