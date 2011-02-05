//
//  RenderMap.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const RenderMapChanged;

typedef enum {
    RM_TEXTURED,
    RM_FLAT,
    RM_WIREFRAME
} ERenderMode;

@class Map;
@class VBOBuffer;
@class Camera;
@class TextureManager;
@class SelectionManager;

@interface RenderMap : NSObject {
    Map* map;
    VBOBuffer* faceVBO;
    NSMutableDictionary* renderEntities;
    Camera* camera;
    TextureManager* textureManager;
    SelectionManager* selectionManager;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    NSMutableDictionary* selIndexBuffers;
    NSMutableDictionary* selCountBuffers;
    BOOL buffersValid;
    
    NSMutableArray* faceTools;
}

- (id)initWithMap:(Map *)theMap faceVBO:(VBOBuffer *)theFaceVBO camera:(Camera *)theCamera textureManager:(TextureManager *)theTextureManager selectionManager:(SelectionManager *)theSelectionManager;

- (NSArray *)renderEntities;

- (void)render;
- (void)updateView:(NSRect)bounds;

- (void)entityChanged;

@end
