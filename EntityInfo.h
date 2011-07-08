//
//  EntityInfo.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol Entity;
@class MutableEntity;

@interface EntityInfo : NSObject {
@private
    NSNumber* entityId;
    NSDictionary* properties;
}

+ (id)entityInfoFor:(id <Entity>)theEntity;

- (id)initWithEntity:(id <Entity>)theEntity;

- (void)updateEntity:(MutableEntity *)theEntity;

@end
