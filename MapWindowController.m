//
//  MapWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapWindowController.h"
#import "MapView3D.h"
#import "TextureView.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "MapDocument.h"
#import "WadLoader.h"
#import "Wad.h"
#import "TextureManager.h"
#import "InputManager.h"
#import "VBOBuffer.h"
#import "Octree.h"
#import "Picker.h"
#import "SelectionManager.h"
#import "GLFontManager.h"
#import "InspectorViewController.h"
#import "InspectorWindowController.h"
#import "Options.h"
#import "Grid.h"
#import "PrefabManager.h"
#import "PrefabNameSheetController.h"
#import "Prefab.h"
#import "MapWriter.h"
#import "CameraAnimation.h"
#import "CursorManager.h"
#import "ClipTool.h"
#import "EntityDefinitionManager.h"
#import "EntityDefinition.h"
#import "math.h"
#import "ControllerUtils.h"
#import "PreferencesController.h"
#import "PreferencesManager.h"
#import "MapCompiler.h"
#import "Console.h"

@interface MapWindowController (private)

- (TBoundingBox)boundsOf:(NSSet *)theBrushes entities:(NSSet *)theEntities;

@end

@implementation MapWindowController (private)

- (TBoundingBox)boundsOf:(NSSet *)theBrushes entities:(NSSet *)theEntities {
    TBoundingBox bounds;
    BOOL initialized = NO;
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity = nil;
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush = nil;
    
    while ((entity = [entityEn nextObject]) || (brush = [brushEn nextObject])) {
        if (entity != nil && [entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT) {
            if (!initialized) {
                bounds = *[entity bounds];
                initialized = YES;
            } else {
                mergeBoundsWithBounds(&bounds, [entity bounds], &bounds);
            }
        }
        
        if (brush != nil) {
            if (!initialized) {
                bounds = *[brush bounds];
                initialized = YES;
            } else {
                mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
            }
        }
    }
    
    return bounds;
}

@end

@implementation MapWindowController

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self document] undoManager];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    if ([[PreferencesManager sharedManager] inspectorSeparate]) {
        InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
        [inspector setMapWindowController:self];
    }
    [view3D becomeFirstResponder];
}

- (void)windowDidResignKey:(NSNotification *)notification {
    if ([[PreferencesManager sharedManager] inspectorSeparate]) {
        InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
        [inspector setMapWindowController:nil];
    }
    [view3D resignFirstResponder];
}

- (void)windowWillClose:(NSNotification *)notification {
    if ([[PreferencesManager sharedManager] inspectorSeparate]) {
        InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
        [inspector setMapWindowController:nil];
    }
}

- (CGFloat)splitView:(NSSplitView *)theSplitView constrainMaxCoordinate:(CGFloat)theProposedMax ofSubviewAt:(NSInteger)theDividerIndex {
    float width = [splitView frame].size.width;
    return width - 402;
}

- (void)preferencesDidChange:(NSNotification *)notification {
    PreferencesManager* preferences = [PreferencesManager sharedManager];

    [camera setFieldOfVision:[preferences cameraFov]];
    [camera setNearClippingPlane:[preferences cameraNear]];
    [camera setFarClippingPlane:[preferences cameraFar]];

    BOOL inspectorVisible = [preferences inspectorVisible];
    BOOL inspectorSeparate = [preferences inspectorSeparate];

    if (inspectorSeparate) {
        if (inspectorViewController != nil) {
            NSView* inspectorView = [inspectorViewController view];
            [inspectorView removeFromSuperview];
            [inspectorViewController release];
            inspectorViewController = nil;
            [preferences setInspectorVisible:YES];

            InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
            [inspector setMapWindowController:self];
            [[inspector window] makeKeyAndOrderFront:self];
        } else {
            InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
            [inspector setMapWindowController:self];
            
            if (inspectorVisible)
                [[inspector window] makeKeyAndOrderFront:self];
            else
                [[inspector window] orderOut:self];
        }
    } else {
        if (inspectorViewController == nil) {
            InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
            [inspector setMapWindowController:nil];
            NSWindow* inspectorWindow = [inspector window];
            [inspectorWindow close];

            inspectorViewController = [[InspectorViewController alloc] initWithNibName:@"InspectorView" bundle:nil];
            [inspectorViewController setMapWindowController:self];
            [preferences setInspectorVisible:YES];
        }

        if (inspectorVisible) {
            NSView* inspectorView = [inspectorViewController view];
            [splitView addSubview:inspectorView];
            [splitView adjustSubviews];
            
            float width = [splitView frame].size.width;
            float pos = width - 402;
            [splitView setPosition:pos ofDividerAtIndex:0];
        } else {
            NSView* inspectorView = [inspectorViewController view];
            [inspectorView removeFromSuperview];
        }
    }
}

- (void)selectionRemoved:(NSNotification *)notification {
    if ([selectionManager mode] == SM_UNDEFINED)
        [options setIsolationMode:IM_NONE];
}

- (void)windowDidLoad {
    console = [[Console alloc] initWithTextView:consoleTextView];
    
    GLResources* glResources = [[self document] glResources];
    NSOpenGLContext* sharedContext = [glResources openGLContext];
    NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[view3D pixelFormat] shareContext:sharedContext];
    [view3D setOpenGLContext:sharingContext];
    [sharingContext release];
    
    options = [[Options alloc] init];
    camera = [[Camera alloc] init];
    [self preferencesDidChange:nil];
    
    selectionManager = [[SelectionManager alloc] initWithUndoManager:[[self document] undoManager]];
    inputManager = [[InputManager alloc] initWithWindowController:self];
    cursorManager = [[CursorManager alloc] init];
    
    [view3D setup];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
    [center addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:[self window]];
    [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:[PreferencesManager sharedManager]];
    [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
    
    [[self window] setAcceptsMouseMovedEvents:YES];
    [[self window] makeKeyAndOrderFront:nil];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    if (action == @selector(selectAll:)) {
        return YES;
    } else if (action == @selector(selectNone:)) {
        return [selectionManager hasSelection];
    } else if (action == @selector(selectEntity:)) {
        id <Entity> entity = [selectionManager brushSelectionEntity];
        return ![entity isWorldspawn] && [[selectionManager selectedBrushes] count] < [[entity brushes] count];
    } else if (action == @selector(copySelection:)) {
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes];
    } else if (action == @selector(cutSelection:)) {
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes];
    } else if (action == @selector(pasteClipboard:)) {
        return NO;
    } else if (action == @selector(deleteSelection:)) {
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes] || ([[inputManager clipTool] active] && [[inputManager clipTool] numPoints] > 0);
    } else if (action == @selector(moveTextureLeft:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureLeft:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureRight:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureUp:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureDown:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(stretchTextureHorizontally:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(shrinkTextureHorizontally:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(stretchTextureVertically:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(shrinkTextureVertically:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(rotateTextureLeft:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(rotateTextureRight:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(duplicateSelection:)) {
        return ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) && ![[inputManager clipTool] active];
    } else if (action == @selector(createPrefabFromSelection:)) {
        return [selectionManager hasSelectedBrushes];
    } else if (action == @selector(showInspector:)) {
        return YES;
    } else if (action == @selector(switchToXYView:)) {
        return YES;
    } else if (action == @selector(switchToXZView:)) {
        return YES;
    } else if (action == @selector(switchToYZView:)) {
        return YES;
    } else if (action == @selector(isolateSelection:)) {
        return YES;
    } else if (action == @selector(toggleProjection:)) {
        return YES;
    } else if (action == @selector(toggleGrid:)) {
        return YES;
    } else if (action == @selector(toggleSnap:)) {
        return YES;
    } else if (action == @selector(setGridSize:)) {
        return YES;
    } else if (action == @selector(toggleClipTool:)) {
        return [selectionManager hasSelectedBrushes] || [[inputManager clipTool] active];
    } else if (action == @selector(toggleClipMode:)) {
        return [[inputManager clipTool] active];
    } else if (action == @selector(performClip:)) {
        return [[inputManager clipTool] active] && [[inputManager clipTool] numPoints] > 1;
    } else if (action == @selector(rotateZ90CW:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities];
    } else if (action == @selector(rotateZ90CCW:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities];
    } else if (action == @selector(createPointEntity:)) {
        return YES;
    } else if (action == @selector(createBrushEntity:)) {
        id <Entity> entity = [selectionManager brushSelectionEntity];
        return [entity isWorldspawn];
    } else if (action == @selector(compile:)) {
        return YES;
    } else if (action == @selector(run:)) {
        return YES;
    }

    return NO;
}

- (void)prefabNameSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo {
    PrefabNameSheetController* pns = [sheet windowController];
    if (returnCode == NSOKButton) {
        NSString* prefabName = [pns prefabName];
        NSString* prefabGroupName = [pns prefabGroup];
        
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        id <PrefabGroup> prefabGroup = [prefabManager prefabGroupWithName:prefabGroupName create:YES];
        [prefabManager createPrefabFromBrushTemplates:[selectionManager selectedBrushes] name:prefabName group:prefabGroup];
    }
    [pns release];
}

- (id)retain {
    return [super retain];
}

- (oneway void)release {
    [super release];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [options release];
    [selectionManager release];
    [inputManager release];
    [cursorManager release];
    [camera release];
    [inspectorViewController release];
    [console release];
    [super dealloc];
}

#pragma mark Entity related actions

- (IBAction)createPointEntity:(id)sender {
    MapDocument* map = [self document];
    EntityDefinitionManager* entityDefinitionManager = [map entityDefinitionManager];
    
    NSArray* pointDefinitions = [entityDefinitionManager definitionsOfType:EDT_POINT];
    EntityDefinition* definition = [pointDefinitions objectAtIndex:[sender tag]];

    NSPoint mousePos = [inputManager menuPosition];
    PickingHitList* hits = [inputManager currentHitList];
    
    TVector3i insertPoint;
    calculateEntityOrigin(definition, hits, mousePos, camera, &insertPoint);

    Grid* grid = [options grid];
    [grid snapToGridV3i:&insertPoint result:&insertPoint];
    
    NSString* origin = [NSString stringWithFormat:@"%i %i %i", insertPoint.x, insertPoint.y, insertPoint.z];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [selectionManager removeAll:YES];
    
    id <Entity> entity = [map createEntityWithClassname:[definition name]];
    [map setEntity:entity propertyKey:OriginKey value:origin];
    
    [selectionManager addEntity:entity record:YES];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Create Entity"];
}

- (IBAction)createBrushEntity:(id)sender {
}


#pragma mark Brush related actions

- (IBAction)rotateZ90CW:(id)sender {
    NSSet* brushes = [selectionManager selectedBrushes];
    NSSet* entities = [selectionManager selectedEntities];
    
    TVector3f centerf;
    TVector3i centeri;
    
    TBoundingBox bounds = [self boundsOf:brushes entities:entities];
    centerOfBounds(&bounds, &centerf);
    roundV3f(&centerf, &centeri);
    
    MapDocument* map = [self document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    [map rotateBrushesZ90CW:brushes center:centeri];
    [map rotateEntitiesZ90CW:entities center:centeri];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Rotate Objects"];
}

- (IBAction)rotateZ90CCW:(id)sender {
    NSSet* brushes = [selectionManager selectedBrushes];
    NSSet* entities = [selectionManager selectedEntities];
    
    TVector3f centerf;
    TVector3i centeri;
    
    TBoundingBox bounds = [self boundsOf:brushes entities:entities];
    centerOfBounds(&bounds, &centerf);
    roundV3f(&centerf, &centeri);
    
    MapDocument* map = [self document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [map rotateBrushesZ90CCW:brushes center:centeri];
    [map rotateEntitiesZ90CCW:entities center:centeri];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Rotate Objects"];
}

- (IBAction)toggleClipTool:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active])
        [clipTool deactivate];
    else
        [clipTool activate];
}

- (IBAction)toggleClipMode:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active])
        [clipTool toggleClipMode];
}

- (IBAction)performClip:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active]) {
        [clipTool performClip:[self document]];
        [clipTool deactivate];
    }
}

#pragma mark Face related actions

- (IBAction)stretchTextureHorizontally:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    [[self document] scaleFaces:[selectionManager selectedFaces] xFactor:0.1f yFactor:0];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Stretch Texture Horizontally"];
}

- (IBAction)shrinkTextureHorizontally:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    [[self document] scaleFaces:[selectionManager selectedFaces] xFactor:-0.1f yFactor:0];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Shrink Texture Horizontally"];
}

- (IBAction)stretchTextureVertically:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    [[self document] scaleFaces:[selectionManager selectedFaces] xFactor:0 yFactor:0.1f];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Stretch Texture Vertically"];

}

- (IBAction)shrinkTextureVertically:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    [[self document] scaleFaces:[selectionManager selectedFaces] xFactor:0 yFactor:0.1f];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Shrink Texture Vertically"];
}

- (IBAction)rotateTextureLeft:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    [[self document] rotateFaces:[selectionManager selectedFaces] angle:-15];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Rotate Texture Left"];
}

- (IBAction)rotateTextureRight:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    [[self document] rotateFaces:[selectionManager selectedFaces] angle:15];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Rotate Texture Right"];
}

#pragma mark Shared actions

- (void)moveLeft:(id)sender {
    MapDocument* map = [self document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    const TVector3f* direction = [camera right];
    float delta = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces])
        [map translateFaceOffsets:[selectionManager selectedFaces] xDelta:delta yDelta:0];
    
    if ([selectionManager hasSelectedBrushes])
        [map translateBrushes:[selectionManager selectedBrushes] direction:*direction delta:-delta];
    
    if ([selectionManager hasSelectedEntities])
        [map translateEntities:[selectionManager selectedEntities] direction:*direction delta:-delta];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

- (void)moveRight:(id)sender {
    MapDocument* map = [self document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    const TVector3f* direction = [camera right];
    float delta = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces])
        [map translateFaceOffsets:[selectionManager selectedFaces] xDelta:-delta yDelta:0];
    
    if ([selectionManager hasSelectedBrushes])
        [map translateBrushes:[selectionManager selectedBrushes] direction:*direction delta:delta];
    
    if ([selectionManager hasSelectedEntities])
        [map translateEntities:[selectionManager selectedEntities] direction:*direction delta:delta];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];}

- (void)moveUp:(id)sender {
    MapDocument* map = [self document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    const TVector3f* direction = [camera up];
    float delta = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces])
        [map translateFaceOffsets:[selectionManager selectedFaces] xDelta:0 yDelta:delta];
    
    if ([selectionManager hasSelectedBrushes])
        [map translateBrushes:[selectionManager selectedBrushes] direction:*direction delta:delta];
    
    if ([selectionManager hasSelectedEntities])
        [map translateEntities:[selectionManager selectedEntities] direction:*direction delta:delta];

    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

- (void)moveDown:(id)sender {
    MapDocument* map = [self document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    const TVector3f* direction = [camera up];
    float delta = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces])
        [map translateFaceOffsets:[selectionManager selectedFaces] xDelta:0 yDelta:-delta];
    
    if ([selectionManager hasSelectedBrushes])
        [map translateBrushes:[selectionManager selectedBrushes] direction:*direction delta:-delta];
    
    if ([selectionManager hasSelectedEntities])
        [map translateEntities:[selectionManager selectedEntities] direction:*direction delta:-delta];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

#pragma mark View related actions

- (IBAction)showInspector:(id)sender {
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    [preferences setInspectorVisible:![preferences inspectorVisible]];
}

- (IBAction)toggleGrid:(id)sender {
    [[options grid] toggleDraw];
}

- (IBAction)toggleSnap:(id)sender {
    [[options grid] toggleSnap];
}

- (IBAction)setGridSize:(id)sender {
    [[options grid] setSize:[sender tag]];
}

- (IBAction)isolateSelection:(id)sender {
    EIsolationMode isolationMode = [options isolationMode];
    [options setIsolationMode:(isolationMode + 1) % 3];
}

- (IBAction)toggleProjection:(id)sender {
    ECameraMode cameraMode = [camera mode];
    [camera setMode:(cameraMode + 1) % 2];
}

- (IBAction)switchToXYView:(id)sender {
    TVector3f center, diff, position;
    
    if (![selectionManager selectionCenter:&center]) {
        center = *[camera direction];
        scaleV3f(&center, 256, &center);
        addV3f(&center, [camera position], &center);
    }
    
    subV3f(&center, [camera position], &diff);
    position = center;
    position.z += lengthV3f(&diff);
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

- (IBAction)switchToXZView:(id)sender {
    TVector3f center, diff, position;
    
    if (![selectionManager selectionCenter:&center]) {
        center = *[camera direction];
        scaleV3f(&center, 256, &center);
        addV3f(&center, [camera position], &center);
    }
    
    subV3f(&center, [camera position], &diff);
    position = center;
    position.y -= lengthV3f(&diff);

    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

- (IBAction)switchToYZView:(id)sender {
    TVector3f center, diff, position;
    
    if (![selectionManager selectionCenter:&center]) {
        center = *[camera direction];
        scaleV3f(&center, 256, &center);
        addV3f(&center, [camera position], &center);
    }
    
    subV3f(&center, [camera position], &diff);
    position = center;
    position.x += lengthV3f(&diff);
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

#pragma mark Structure and selection

- (IBAction)selectAll:(id)sender {
    [selectionManager removeAll:NO];
    
    NSMutableSet* entities = [[NSMutableSet alloc] init];
    NSMutableSet* brushes = [[NSMutableSet alloc] init];
    
    NSEnumerator* entityEn = [[[self document] entities] objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [entities addObject:entity];
        else
            [brushes addObjectsFromArray:[entity brushes]];
    }
    
    [selectionManager addEntities:entities record:NO];
    [selectionManager addBrushes:brushes record:NO];
    
    [entities release];
    [brushes release];
}

- (IBAction)selectNone:(id)sender {
    [selectionManager removeAll:NO];
}

- (IBAction)selectEntity:(id)sender {
    id <Brush> brush = [[[selectionManager selectedBrushes] objectEnumerator] nextObject];
    id <Entity> entity = [brush entity];
    
    [selectionManager removeAll:NO];
    [selectionManager addEntity:entity record:NO];
    [selectionManager addBrushes:[NSSet setWithArray:[entity brushes]] record:NO];
}

- (IBAction)copySelection:(id)sender {}
- (IBAction)cutSelection:(id)sender {}
- (IBAction)pasteClipboard:(id)sender {}

- (IBAction)deleteSelection:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active] && [clipTool numPoints] > 0) {
        [clipTool deleteLastPoint];
    } else {
        NSUndoManager* undoManager = [[self document] undoManager];
        [undoManager beginUndoGrouping];
        
        if ([selectionManager hasSelectedEntities]) {
            NSSet* deletedEntities = [[NSSet alloc] initWithSet:[selectionManager selectedEntities]];
            [selectionManager removeEntities:deletedEntities record:YES];
            [[self document] deleteEntities:deletedEntities];
            [deletedEntities release];
        }
        
        if ([selectionManager hasSelectedBrushes]) {
            NSSet* deletedBrushes = [[NSSet alloc] initWithSet:[selectionManager selectedBrushes]];
            [selectionManager removeBrushes:deletedBrushes record:YES];
            [[self document] deleteBrushes:deletedBrushes];
            [deletedBrushes release];
        }
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Delete Selection"];
    }
}

- (IBAction)duplicateSelection:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    NSMutableSet* newEntities = [[NSMutableSet alloc] init];
    NSMutableSet* newBrushes = [[NSMutableSet alloc] init];

    TVector3i delta = {[[options grid] actualSize], [[options grid] actualSize], 0};
    
    if ([selectionManager hasSelectedEntities]) {
        NSEnumerator* entityEn = [[selectionManager selectedEntities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            id <Entity> newEntity = [[self document] createEntityWithProperties:[entity properties]];
            
            if ([[entity entityDefinition] type] != EDT_POINT) {
                NSArray* brushes = [entity brushes];
                if ([brushes count] > 0) {
                    NSEnumerator* brushEn = [brushes objectEnumerator];
                    id <Brush> brush;
                    while ((brush = [brushEn nextObject])) {
                        id <Brush> newBrush = [[self document] createBrushInEntity:newEntity fromTemplate:brush];
                        [newBrushes addObject:newBrush];
                    }
                }
            }
            [newEntities addObject:newEntity];
        }
    }
    
    if ([selectionManager hasSelectedBrushes]) {
        id <Entity> worldspawn = [[self document] worldspawn:YES];
        NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            id <Brush> newBrush = [[self document] createBrushInEntity:worldspawn fromTemplate:brush];
            [newBrushes addObject:newBrush];
        }
        
    }
    
    [[self document] translateEntities:newEntities delta:delta];
    [[self document] translateBrushes:newBrushes delta:delta];

    [selectionManager removeAll:YES];
    [selectionManager addEntities:newEntities record:YES];
    [selectionManager addBrushes:newBrushes record:YES];

    [newEntities release];
    [newBrushes release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Duplicate Selection"];
}


- (IBAction)createPrefabFromSelection:(id)sender {
    PrefabNameSheetController* pns = [[PrefabNameSheetController alloc] init];
    NSWindow* prefabNameSheet = [pns window];
    
    NSApplication* app = [NSApplication sharedApplication];
    [app beginSheet:prefabNameSheet modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(prefabNameSheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)insertPrefab:(id <Prefab>)prefab {
    [selectionManager removeAll:YES];
    
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    TVector3f insertPos = [camera defaultPoint];
    [[options grid] snapToGridV3f:&insertPos result:&insertPos];

    TVector3f offset;
    [[options grid] gridOffsetV3f:[prefab center] result:&offset];
    
    addV3f(&insertPos, &offset, &insertPos);

    TVector3f dist;
    subV3f(&insertPos, [prefab center], &dist);
    
    TVector3i delta;
    roundV3f(&dist, &delta);
    
    NSMutableSet* newEntities = [[NSMutableSet alloc] init];
    NSMutableSet* newBrushes = [[NSMutableSet alloc] init];
    MapDocument* map = [self document];
    
    NSEnumerator* entityEn = [[prefab entities] objectEnumerator];
    id <Entity> prefabEntity;
    while ((prefabEntity = [entityEn nextObject])) {
        id <Entity> mapEntity;
        if ([prefabEntity isWorldspawn]) {
            mapEntity = [map worldspawn:YES];
        } else {
            mapEntity = [map createEntityWithProperties:[prefabEntity properties]];
            [map setEntityDefinition:mapEntity];
            [newEntities addObject:mapEntity];
        }
        
        NSEnumerator* prefabBrushEn = [[prefabEntity brushes] objectEnumerator];
        id <Brush> prefabBrush;
        while ((prefabBrush = [prefabBrushEn nextObject])) {
            id <Brush> mapBrush = [map createBrushInEntity:mapEntity fromTemplate:prefabBrush];
            [newBrushes addObject:mapBrush];
        }
    }
    
    [[self document] translateEntities:newEntities delta:delta];
    [[self document] translateBrushes:newBrushes delta:delta];
    
    [selectionManager removeAll:YES];
    [selectionManager addEntities:newEntities record:YES];
    [selectionManager addBrushes:newBrushes record:YES];
    
    [newEntities release];
    [newBrushes release];

    [undoManager endUndoGrouping];
    [undoManager setActionName:[NSString stringWithFormat:@"Insert Prefab '%@'", [prefab name]]];
}

#pragma mark -
#pragma mark Compile & Run

- (IBAction)compile:(id)sender {
    MapDocument* map = [self document];
    [map saveDocument:self];
    
    NSURL* mapFileUrl = [map fileURL];
    
    MapCompiler* compiler = [[MapCompiler alloc] initWithMapFileUrl:mapFileUrl console:console];
    [compiler compile];
}

- (IBAction)run:(id)sender {
    MapDocument* map = [self document];
    NSURL* mapFileUrl = [map fileURL];
    if (mapFileUrl == nil) {
        [console log:@"Map must be saved and compiled first"];
        return;
    }

    NSError* error;
    
    NSString* mapFilePath = [mapFileUrl path];
    NSString* mapDirPath = [mapFilePath stringByDeletingLastPathComponent];
    NSString* mapFileName = [mapFilePath lastPathComponent];
    NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
    NSString* bspFileName = [baseFileName stringByAppendingPathExtension:@"bsp"];
    NSString* bspFilePath = [mapDirPath stringByAppendingPathComponent:bspFileName];
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:bspFilePath]) {
        [console logBold:[NSString stringWithFormat:@"Could not find BSP file at '%@'\n", bspFilePath]];
        return;
    }

    NSString* quakePath = [[PreferencesManager sharedManager] quakePath];
    if (quakePath == nil) {
        [console logBold:@"Quake path not defined"];
        return;
    }
    
    NSArray* modList = modListFromWorldspawn([map worldspawn:YES]);
    NSString* modPath = [quakePath stringByAppendingPathComponent:[modList lastObject]];
    
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:modPath isDirectory:&directory];
    if (!exists || !directory) {
        [console logBold:[NSString stringWithFormat:@"Mod path '%@' does not exist or is not a directory\n", modPath]];
        return;
    }
        
    NSString* modMapsDirPath = [modPath stringByAppendingPathComponent:@"maps"];
    exists = [fileManager fileExistsAtPath:modMapsDirPath isDirectory:&directory];
    if (exists && !directory) {
        [console logBold:[NSString stringWithFormat:@"Mod maps path '%@' is not a directory\n", modMapsDirPath]];
        return;
    }
    
    if (!exists) {
        [console log:[NSString stringWithFormat:@"creating mod maps directory at '%@'\n", modMapsDirPath]];
        if (![fileManager createDirectoryAtPath:modMapsDirPath withIntermediateDirectories:NO attributes:nil error:&error]) {
            [console logBold:[NSString stringWithFormat:@"Failed to create mod maps directory at '%@': %@\n", modMapsDirPath, [error localizedDescription]]];
            return;
        }
    }
    
    NSString* targetBspPath = [modMapsDirPath stringByAppendingPathComponent:bspFileName];
    if ([fileManager fileExistsAtPath:targetBspPath]) {
        [console log:[NSString stringWithFormat:@"Removing existing BSP file '%@'\n", targetBspPath]];
        if (![fileManager removeItemAtPath:targetBspPath error:&error]) {
            [console logBold:[NSString stringWithFormat:@"Failed to remove existing BSP file from '%@': %@\n", targetBspPath, [error localizedDescription]]];
            return;
        }
    }
    
    if (![fileManager copyItemAtPath:bspFilePath toPath:targetBspPath error:&error]) {
        [console logBold:[NSString stringWithFormat:@"Failed to copy BSP file from '%@' to '%@': %@\n", bspFilePath, targetBspPath, [error localizedDescription]]];
        return;
    }
    
    NSString* quakeExecutable = [[PreferencesManager sharedManager] quakeExecutable];
    NSString* appPath = [quakePath stringByAppendingPathComponent:quakeExecutable];
    appPath = [appPath stringByAppendingPathExtension:@"app"];
    NSURL* appUrl = [NSURL URLWithString:appPath];
    
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    
    NSDictionary *config = [NSDictionary dictionaryWithObjectsAndKeys:[NSArray arrayWithObjects:@"-nolauncher", @"+map", baseFileName, nil], NSWorkspaceLaunchConfigurationArguments, nil];
    [workspace launchApplicationAtURL:appUrl options:NSWorkspaceLaunchNewInstance | NSWorkspaceLaunchDefault configuration:config error:&error];
}

- (void)makeEntityVisible:(id <Entity>)theEntity {
}

- (void)makeBrushVisible:(id <Brush>)theBrush {
    [selectionManager removeAll:NO];
    [selectionManager addBrush:theBrush record:NO];
    [options setIsolationMode:IM_WIREFRAME];
    
    TVector3f size;
    TBoundingBox* bounds = [theBrush bounds];
    sizeOfBounds(bounds, &size);
    float l = fmaxf(size.x, fmaxf(size.y, size.z));
    
    TVector3f* center = [theBrush center];
    TVector3f position = *center;
    position.x -= l;
    position.y -= l;
    position.z += l;
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:center duration:0.5];
    [animation startAnimation];
}

- (void)makeFaceVisible:(id <Face>)theFace {
    [selectionManager removeAll:NO];
    [selectionManager addBrush:[theFace brush] record:NO];
    [options setIsolationMode:IM_WIREFRAME];
    
    TVector3f position, direction;
    scaleV3f([theFace norm], 150, &position);
    addV3f([theFace center], &position, &position);
    scaleV3f([theFace norm], -1, &direction);
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:[theFace center] duration:0.5];
    [animation startAnimation];
}

#pragma mark -
#pragma mark Getters

- (Camera *)camera {
    return camera;
}

- (SelectionManager *)selectionManager {
    return selectionManager;
}

- (InputManager *)inputManager {
    return inputManager;
}

- (CursorManager *)cursorManager {
    return cursorManager;
}

- (Options *)options {
    return options;
}

- (Renderer *)renderer {
    return [view3D renderer];
}

- (MapView3D *)view3D {
    return view3D;
}

- (Console *)console {
    return console;
}

@end
