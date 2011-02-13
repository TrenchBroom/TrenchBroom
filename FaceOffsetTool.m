//
//  FaceOffsetTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceOffsetTool.h"
#import "Face.h"
#import "Vector2f.h"
#import "Vector3f.h"
#import "Math.h"
#import "SelectionManager.h"
#import "Ray3D.h"
#import "HalfSpace3D.h"
#import "Plane3D.h"
#import "FaceOffsetFigure.h"
#import "Options.h"

static NSString* FaceDefaults = @"Face Flags";
static NSString* MoveLeftKey = @"Move Texture Left";
static NSString* MoveRightKey = @"Move Texture Right";
static NSString* MoveUpKey = @"Move Texture Up";
static NSString* MoveDownKey = @"Move Texture Down";

@implementation FaceOffsetTool

- (void)userDefaultsChanged:(NSNotification *)notification {
    NSDictionary* defaults = [[NSUserDefaults standardUserDefaults] dictionaryForKey:FaceDefaults];
    if (defaults == nil)
        return;
    
    moveLeftKey = [[defaults objectForKey:MoveLeftKey] intValue];
    moveRightKey = [[defaults objectForKey:MoveRightKey] intValue];
    moveUpKey = [[defaults objectForKey:MoveUpKey] intValue];
    moveDownKey = [[defaults objectForKey:MoveDownKey] intValue];
}

- (id)init {
    if (self = [super init]) {
        faces = [[NSMutableArray alloc] init];
        figures = [[NSMutableArray alloc] init];
        lastSurfacePos = [[Vector3f alloc] init];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self 
                   selector:@selector(userDefaultsChanged:) 
                       name:NSUserDefaultsDidChangeNotification 
                     object:[NSUserDefaults standardUserDefaults]];
        
        [self userDefaultsChanged:nil];
    }
    
    return self;
}

- (id)initWithOptions:(Options *)theOptions {
    if (theOptions == nil)
        [NSException raise:NSInvalidArgumentException format:@"options must not be nil"];
    
    if (self = [self init]) {
        options = [theOptions retain];
    }
    
    return self;
}


- (Face *)faceHitByRay:(Ray3D *)theRay {
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        if ([face pickWithRay:theRay] != nil)
            return face;
    
    return nil;
}

- (BOOL)hitByRay:(Ray3D *)theRay {
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject])) {
        Plane3D* plane = [[face halfSpace] boundary];
        Vector3f* is = [plane intersectWithRay:theRay];
        if (is != nil) {
            [is sub:[face center]];
            if (flte([is lengthSquared], 11 * 11))
                return YES;
        }
    }

    return NO;
}

- (void)startDrag:(Ray3D *)theRay {
    draggedFace = [self faceHitByRay:theRay];
    
    Plane3D* plane = [[draggedFace halfSpace] boundary];
    Vector3f* is = [plane intersectWithRay:theRay];
    if (is == nil)
        return;
    
    [lastSurfacePos setFloat:[draggedFace surfaceCoordsOf:is]];
}

- (void)drag:(Ray3D *)theRay {
    Plane3D* plane = [[draggedFace halfSpace] boundary];
    Vector3f* is = [plane intersectWithRay:theRay];
    if (is == nil)
        return;
    
    int d = [options gridSize];
    if (([NSEvent modifierFlags] & NSShiftKeyMask) != 0)
        d = 1;
    
    Vector3f* surfacePos = [draggedFace surfaceCoordsOf:is];
    int dx = ((int)([surfacePos x] - [lastSurfacePos x])) / d;
    int dy = ((int)([surfacePos y] - [lastSurfacePos y])) / d;

    if (dx != 0 || dy != 0) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject]))
            [face translateOffsetsX:dx * d y:dy * d];
    }

    if (dx != 0)
        [lastSurfacePos setX:[surfacePos x]];
    
    if (dy != 0)
        [lastSurfacePos setY:[surfacePos y]];
    
}

- (void)endDrag:(Ray3D *)theRay {
    draggedFace = nil;
}

- (void)keyDown:(NSEvent *)theEvent {
    int keyCode = [theEvent keyCode];
    int d = [options gridSize];

    if (([theEvent modifierFlags] & NSShiftKeyMask) != 0)
        d = 1;
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    if (keyCode == moveLeftKey)
        while ((face = [faceEn nextObject]))
            [face translateOffsetsX:-d y:0];
    else if (keyCode == moveRightKey)
        while ((face = [faceEn nextObject]))
            [face translateOffsetsX:d y:0];
    else if (keyCode == moveUpKey)
        while ((face = [faceEn nextObject]))
            [face translateOffsetsX:0 y:d];
    else if (keyCode == moveDownKey)
        while ((face = [faceEn nextObject]))
            [face translateOffsetsX:0 y:-d];
    else
        NSLog(@"unknown key code: %i", [theEvent keyCode]);
}

- (id)addObject:(id)object {
    Face* face = (Face *)object;
    FaceOffsetFigure* figure = [[FaceOffsetFigure alloc] initWithFace:face];
    [faces addObject:face];
    [figures addObject:figure];
    
    return [figure autorelease];
}

- (id)removeObject:(id)object {
    Face* face = (Face *)object;
    int index = [faces indexOfObject:face];
    [faces removeObjectAtIndex:index];
    
    FaceOffsetFigure* figure = [figures objectAtIndex:index];
    [figure retain];
    [figures removeObjectAtIndex:index];
    
    return [figure autorelease];
}

- (void)dealloc {
    [lastSurfacePos release];
    [faces release];
    [figures release];
    [options release];
    [super dealloc];
}

@end
