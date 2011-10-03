/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "QuickBarWindowController.h"
#import "Math.h"
#import "Camera.h"
#import "CompassView.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Face.h"

@interface QuickBarWindowController (private)

- (void)updateSelectionBoundsInspector;
- (void)updateEntityInspector;
- (void)updateFaceInspector;
- (void)facesDidChange:(NSNotification *)notification;
- (void)selectionChanged:(NSNotification *)notification;
- (void)propertiesDidChange:(NSNotification *)notification;
- (void)brushesDidChange:(NSNotification *)notification;
- (void)updateCameraInspector;
- (void)cameraChanged:(NSNotification *)notification;
- (void)mapWindowDidResize:(NSNotification *)notification;

@end

@implementation QuickBarWindowController (private)

- (void)updateSelectionBoundsInspector {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    TBoundingBox bounds;
    [selectionManager selectionBounds:&bounds];
    
    TVector3f size;
    sizeOfBounds(&bounds, &size);
    
    [selectionOriginXField setFloatValue:bounds.min.x];
    [selectionOriginYField setFloatValue:bounds.min.y];
    [selectionOriginZField setFloatValue:bounds.min.z];
    [selectionSizeXField setFloatValue:size.x];
    [selectionSizeYField setFloatValue:size.y];
    [selectionSizeZField setFloatValue:size.z];
}

- (void)updateEntityInspector {
    SelectionManager* selectionManager = [mapWindowController selectionManager];

    if ([selectionManager mode] == SM_ENTITIES || [selectionManager mode] == SM_BRUSHES_ENTITIES) {
        NSEnumerator* entityEn = [[selectionManager selectedEntities] objectEnumerator];
        id <Entity> entity = [entityEn nextObject];
        
        NSString* classname = [entity classname];
        BOOL classnameMultiple = NO;
        NSString* spawnFlags = [entity spawnFlagsString];
        BOOL spawnFlagsMultiple = NO;
        
        while ((entity = [entityEn nextObject]) && !classnameMultiple && !spawnFlagsMultiple) {
            classnameMultiple |= ![classname isEqualToString:[entity classname]];
            spawnFlagsMultiple |= ![spawnFlags isEqualToString:[entity spawnFlagsString]];
        }
        
        if (classnameMultiple) {
            [[entityClassnameField cell] setPlaceholderString:@"<m>"];
            [entityClassnameField setStringValue:@""];
        } else {
            [entityClassnameField setStringValue:classname];
        }
        
        if (spawnFlagsMultiple) {
            [[entitySpawnFlagsField cell] setPlaceholderString:@"<m>"];
            [entitySpawnFlagsField setStringValue:@""];
        } else {
            [entitySpawnFlagsField setStringValue:spawnFlags];
        }
    } else {
        [[entityClassnameField cell] setPlaceholderString:@""];
        [entityClassnameField setStringValue:@""];
        [[entitySpawnFlagsField cell] setPlaceholderString:@""];
        [entitySpawnFlagsField setStringValue:@""];
    }
}

- (void)updateFaceInspector {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    if ([selectionManager mode] == SM_FACES) {
        NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
        id <Face> face = [faceEn nextObject];
        
        Texture* texture = [face texture];
        BOOL multipleTextures = NO;
        int xOffset = [face xOffset];
        BOOL multipleXOffsets = NO;
        int yOffset = [face yOffset];
        BOOL multipleYOffsets = NO;
        float xScale = [face xScale];
        BOOL multipleXScales = NO;
        float yScale = [face yScale];
        BOOL multipleYScales = NO;
        float rotation = [face rotation];
        BOOL multipleRotations = NO;
        
        while ((face = [faceEn nextObject]) && !multipleTextures && !multipleXOffsets && !multipleYOffsets && !multipleXScales && !multipleYScales && !multipleRotations) {
            multipleTextures = texture != [face texture];
            multipleXOffsets = xOffset != [face xOffset];
            multipleYOffsets = yOffset != [face yOffset];
            multipleXScales = xScale != [face xScale];
            multipleYScales = yScale != [face yScale];
            multipleRotations = rotation != [face rotation];
        }

        if (multipleTextures) {
            [[faceTextureField cell] setPlaceholderString:@"<multiple>"];
            [faceTextureField setStringValue:@""];
        } else {
            [faceTextureField setStringValue:[texture name]];
        }
        
        if (multipleXOffsets) {
            [[faceXOffsetField cell] setPlaceholderString:@"<m>"];
            [faceXOffsetField setStringValue:@""];
        } else {
            [faceXOffsetField setIntValue:xOffset];
        }
        
        if (multipleYOffsets) {
            [[faceYOffsetField cell] setPlaceholderString:@"<m>"];
            [faceYOffsetField setStringValue:@""];
        } else {
            [faceYOffsetField setIntValue:yOffset];
        }
        
        if (multipleXScales) {
            [[faceXScaleField cell] setPlaceholderString:@"<m>"];
            [faceXScaleField setStringValue:@""];
        } else {
            [faceXScaleField setFloatValue:xScale];
        }
        
        if (multipleYScales) {
            [[faceYScaleField cell] setPlaceholderString:@"<m>"];
            [faceYScaleField setStringValue:@""];
        } else {
            [faceYScaleField setFloatValue:yScale];
        }
        
        if (multipleRotations) {
            [[faceRotationField cell] setPlaceholderString:@"<m>"];
            [faceRotationField setStringValue:@""];
        } else {
            [faceRotationField setFloatValue:rotation];
        }
    } else {
        [[faceTextureField cell] setPlaceholderString:@"<Texture>"];
        [faceTextureField setStringValue:@""];
        [[faceXOffsetField cell] setPlaceholderString:@""];
        [faceXOffsetField setIntValue:0];
        [[faceYOffsetField cell] setPlaceholderString:@""];
        [faceYOffsetField setIntValue:0];
        [[faceXScaleField cell] setPlaceholderString:@""];
        [faceXScaleField setFloatValue:0];
        [[faceYScaleField cell] setPlaceholderString:@""];
        [faceYScaleField setFloatValue:0];
        [[faceRotationField cell] setPlaceholderString:@""];
        [faceRotationField setFloatValue:0];
    }
}

- (void)selectionChanged:(NSNotification *)notification {
    [self updateSelectionBoundsInspector];
    [self updateEntityInspector];
    [self updateFaceInspector];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    [self updateSelectionBoundsInspector];
    [self updateEntityInspector];
}

- (void)brushesDidChange:(NSNotification *)notification {
    [self updateSelectionBoundsInspector];
}

- (void)facesDidChange:(NSNotification *)notification {
    [self updateFaceInspector];
}

- (void)updateCameraInspector {
    Camera* camera = [mapWindowController camera];
    
    const TVector3f* pos = [camera position];
    const TVector3f* dir = [camera direction];
    
    [cameraPosXField setFloatValue:pos->x];
    [cameraPosYField setFloatValue:pos->y];
    [cameraPosZField setFloatValue:pos->z];
    [cameraDirXField setFloatValue:dir->x];
    [cameraDirYField setFloatValue:dir->y];
    [cameraDirZField setFloatValue:dir->z];
}

- (void)cameraChanged:(NSNotification *)notification {
    [self updateCameraInspector];
}

- (void)mapWindowDidResize:(NSNotification *)notification {
    NSRect mapWindowFrame = [[mapWindowController window] frame];
    NSPoint quickBarOrigin = NSMakePoint(NSMinX(mapWindowFrame), NSMinY(mapWindowFrame));
    [[self window] setFrameOrigin:quickBarOrigin];
}

@end

@implementation QuickBarWindowController

- (void)windowDidLoad {
    [super windowDidLoad];
    if (mapWindowController != nil)
        [compassView setCamera:[mapWindowController camera]];
    [[self window] setDelegate:self];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    NSAssert(theMapWindowController != nil, @"map window controller must not be nil");
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self];
    
    mapWindowController = theMapWindowController;
    
    NSWindow* mapWindow = [mapWindowController window];
    [center addObserver:self selector:@selector(mapWindowDidResize:) name:NSWindowDidResizeNotification object:mapWindow];
    
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    [center addObserver:self selector:@selector(selectionChanged:) name:SelectionAdded object:selectionManager];
    [center addObserver:self selector:@selector(selectionChanged:) name:SelectionRemoved object:selectionManager];
    
    Camera* camera = [mapWindowController camera];
    [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
    [compassView setCamera:camera];
    
    MapDocument* map = [mapWindowController document];
    [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesDidChange object:map];
    [center addObserver:self selector:@selector(brushesDidChange:) name:BrushesDidChange object:map];
    [center addObserver:self selector:@selector(facesDidChange:) name:FacesDidChange object:map];

    NSRect mapWindowFrame = [[mapWindowController window] frame];
    NSPoint quickBarOrigin = NSMakePoint(NSMinX(mapWindowFrame), NSMinY(mapWindowFrame));
    
    [[self window] setFrameOrigin:quickBarOrigin];
}

- (IBAction)switchToXYView:(id)sender {
    [mapWindowController switchToXYView:sender];
}

- (IBAction)switchToXZView:(id)sender {
    [mapWindowController switchToXZView:sender];
}
- (IBAction)switchToYZView:(id)sender {
    [mapWindowController switchToYZView:sender];
}

- (IBAction)selectAll:(id)sender {
    [mapWindowController selectAll:sender];
}

- (IBAction)selectNone:(id)sender {
    [mapWindowController selectNone:sender];
}

- (IBAction)selectEntity:(id)sender {
    [mapWindowController selectEntity:sender];
}

@end
