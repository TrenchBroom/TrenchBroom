//
//  PrefabViewTarget.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol Prefab;

@protocol PrefabViewTarget <NSObject>

- (void)prefabSelected:(id <Prefab>)thePrefab;

@end
