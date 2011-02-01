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
}

- (id)initWithEntity:(Entity *)theEntity faceVBO:(VBOBuffer *)theFaceVBO;

- (NSArray *)renderBrushes;
@end
