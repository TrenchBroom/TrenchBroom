//
//  DefaultProperty.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityDefinitionProperty.h"

@interface DefaultProperty : NSObject <EntityDefinitionProperty> {
    NSString* name;
    NSString* value;
}

- (id)initWithName:(NSString *)theName value:(NSString *)theValue;

- (NSString *)name;
- (NSString *)value;

@end
