//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class MutableEntity;
@protocol Entity;

@protocol Map <NSObject>

- (void)addEntities:(NSArray *)theEntities;
- (void)addEntity:(MutableEntity *)theEntity;
- (void)removeEntities:(NSArray *)theEntities;
- (void)removeEntity:(MutableEntity *)theEntity;

- (TBoundingBox *)worldBounds;
- (id <Entity>)worldspawn:(BOOL)create;
- (NSArray *)entities;

@end
