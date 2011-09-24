//
//  GroupManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.09.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

static NSString* const GroupsChanged = @"GroupsChanged";


@class MapDocument;
@protocol Entity;
@protocol Brush;

@interface GroupManager : NSObject {
    MapDocument* map;
    NSMutableArray* groups;
    int visibleGroupCount;
}

- (id)initWithMap:(MapDocument *)theMap;

- (NSArray *)groups;
- (void)setGroup:(id <Entity>)theGroup name:(NSString *)theName;
- (void)setGroup:(id <Entity>)theGroup visibility:(BOOL)theVisibility;
- (BOOL)isVisible:(id <Entity>)theGroup;
- (BOOL)allGroupsInvisible;

@end
