//
//  RenderEntity.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Entity;
@class VBOBuffer;

@interface RenderEntity : NSObject {
    Entity* entity;
    NSMutableDictionary* renderBrushes;
    VBOBuffer* faceVBO;
    VBOBuffer* edgeVBO;
}

- (id)initWithEntity:(Entity *)theEntity faceVBO:(VBOBuffer *)theFaceVBO edgeVBO:(VBOBuffer *)theEdgeVBO;

- (NSArray *)renderBrushes;
@end
