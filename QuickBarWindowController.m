//
//  QuickBarWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "QuickBarWindowController.h"
#import "Math.h"
#import "Camera.h"
#import "CompassView.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "MapDocument.h"

@interface QuickBarWindowController (private)

- (void)updateSelectionBoundsInspector:(SelectionManager *)selectionManager;
- (void)selectionChanged:(NSNotification *)notification;
- (void)propertiesDidChange:(NSNotification *)notification;
- (void)brushesDidChange:(NSNotification *)notification;
- (void)updateCameraInspector:(Camera *)camera;
- (void)cameraChanged:(NSNotification *)notification;

@end

@implementation QuickBarWindowController (private)

- (void)updateSelectionBoundsInspector:(SelectionManager *)selectionManager {
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

- (void)selectionChanged:(NSNotification *)notification {
    SelectionManager* selectionManager = [notification object];
    [self updateSelectionBoundsInspector:selectionManager];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    SelectionManager* selectionManager = [notification object];
    [self updateSelectionBoundsInspector:selectionManager];
}

- (void)brushesDidChange:(NSNotification *)notification {
    SelectionManager* selectionManager = [notification object];
    [self updateSelectionBoundsInspector:selectionManager];
}

- (void)updateCameraInspector:(Camera *)camera {
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
    Camera* camera = [notification object];
    [self updateCameraInspector:camera];
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
    
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    [center addObserver:self selector:@selector(selectionChanged:) name:SelectionAdded object:selectionManager];
    [center addObserver:self selector:@selector(selectionChanged:) name:SelectionRemoved object:selectionManager];
    
    Camera* camera = [mapWindowController camera];
    [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
    [compassView setCamera:camera];
    
    MapDocument* map = [mapWindowController document];
    [center addObserver:self selector:@selector(propertiesChanged:) name:PropertiesDidChange object:map];
    [center addObserver:self selector:@selector(brushesDidChange:) name:BrushesDidChange object:map];

    NSRect mapWindowFrame = [[mapWindowController window] frame];
    NSPoint quickBarOrigin = NSMakePoint(NSMinX(mapWindowFrame) + 10, NSMinY(mapWindowFrame) + 10);
    
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
