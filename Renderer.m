//
//  Renderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Renderer.h"
#import "GeometryLayer.h"
#import "VBOBuffer.h"
#import "RenderContext.h"
#import "SelectionManager.h"

@implementation Renderer

- (id)initWithVbo:(VBOBuffer *)theVbo {
    if (theVbo == nil)
        [NSException raise:NSInvalidArgumentException format:@"vbo must not be nil"];
    
    if (self = [super init]) {
        vbo = [theVbo retain];
        geometryLayer = [[GeometryLayer alloc] initWithVbo:vbo];
    }
    
    return self;
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
}

- (void)selectionRemoved:(NSNotification *)notification {
}

- (void)setSelectionManager:(SelectionManager *)theSelectionManager {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    if (selectionManager != nil) {
        [center removeObserver:self name:SelectionAdded object:selectionManager];
        [center removeObserver:self name:SelectionRemoved object:selectionManager];
        [selectionManager release];
        selectionManager = nil;
    }
    
    if (theSelectionManager != nil) {
        selectionManager = [theSelectionManager retain];
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
    }
}

- (void)render:(RenderContext *)renderContext {
    [geometryLayer render:renderContext];
    [selectionLayer render:renderContext];
    [handleLayer render:renderContext];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [geometryLayer release];
    [selectionLayer release];
    [handleLayer release];
    [vbo release];
    [selectionManager release];
    [super dealloc];
}

@end
