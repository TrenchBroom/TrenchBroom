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

- (void)addEntity:(MutableEntity *)theEntity;
- (void)removeEntity:(MutableEntity *)theEntity;

- (id <Entity>)worldspawn;
- (NSArray *)entities;

@end
