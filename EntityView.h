/*
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

#import <Foundation/Foundation.h>
#import "EntityDefinitionManager.h"

@class GLResources;
@class EntityDefinitionLayout;
@class EntityDefinition;
@class DragImageWindowController;
@protocol EntityDefinitionFilter;
@protocol EntityDefinitionViewTarget;

@interface EntityView : NSOpenGLView {
    NSPoint dragDistance;
    EntityDefinition* draggedEntityDefinition;
    EntityDefinitionManager* entityDefinitionManager;
    GLResources* glResources;
    EntityDefinitionLayout* layout;
    IBOutlet id <EntityDefinitionViewTarget> target;
    id <EntityDefinitionFilter> filter;
    EEntityDefinitionSortCriterion sortCriterion;
    
    DragImageWindowController* dragImageWindowController;
    NSImage* dragPlaceholder;
    NSImage* dragImage;
    NSSize imageOffset;
    
    NSArray* mods;
}

- (void)setGLResources:(GLResources *)theGLResources entityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager;
- (void)setMods:(NSArray *)theMods;
- (void)setEntityDefinitionFilter:(id <EntityDefinitionFilter>)theFilter;
- (void)setSortCriterion:(EEntityDefinitionSortCriterion)criterion;

@end
