//
//  FaceTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceTool.h"
#import "Vector3f.h"
#import "Plane3D.h"
#import "HalfSpace3D.h"
#import "Ray3D.h"
#import "Face.h"

static NSString* FaceDefaults = @"Face Flags";

@implementation FaceTool
- (void)updateDefaults:(NSDictionary *)defaults {
}

- (void)userDefaultsChanged:(NSNotification *)notification {
    NSDictionary* defaults = [[NSUserDefaults standardUserDefaults] dictionaryForKey:FaceDefaults];
    if (defaults == nil)
        return;
    
    [self updateDefaults:(NSDictionary *)defaults];
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

- (BOOL)face:(Face *)theFace hitByRay:(Ray3D *)theRay {
    return NO;
}

- (BOOL)hitByRay:(Ray3D *)theRay {
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        if ([self face:face hitByRay:(Ray3D *)theRay])
             return YES;
    
    return NO;
}

- (id)createFigure:(Face *)face {
    return nil;
}

- (id)addObject:(id)object {
    Face* face = (Face *)object;
    id figure = [self createFigure:(Face *)face];
    [faces addObject:face];
    [figures addObject:figure];
    
    return figure;
}

- (id)removeObject:(id)object {
    Face* face = (Face *)object;
    int index = [faces indexOfObject:face];
    [faces removeObjectAtIndex:index];
    
    id figure = [figures objectAtIndex:index];
    [figure retain];
    [figures removeObjectAtIndex:index];
    
    return [figure autorelease];
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
}

- (void)endDrag:(Ray3D *)theRay {
    draggedFace = nil;
}

- (void)keyDown:(NSEvent *)theEvent {
}

- (void)dealloc {
    [lastSurfacePos release];
    [faces release];
    [figures release];
    [options release];
    [super dealloc];
}

@end
