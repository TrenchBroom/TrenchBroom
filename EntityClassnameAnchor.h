//
//  EntityClassnameAnchor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TextAnchor.h"

@protocol Entity;

@interface EntityClassnameAnchor : NSObject <TextAnchor> {
@private
    id <Entity> entity;
}

- (id)initWithEntity:(id <Entity>)theEntity;

@end
