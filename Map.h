//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MutableEntity;
@protocol Entity;

@protocol Map <NSObject>

- (void)addEntities:(NSSet *)theEntities;
- (void)addEntity:(MutableEntity *)theEntity;
- (void)removeEntities:(NSSet *)theEntities;
- (void)removeEntity:(MutableEntity *)theEntity;

- (id <Entity>)worldspawn:(BOOL)create;
- (NSArray *)entities;

@end
