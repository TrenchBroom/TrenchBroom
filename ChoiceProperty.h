//
//  ChoiceProperty.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityDefinitionProperty.h"

@interface ChoiceProperty : NSObject <EntityDefinitionProperty> {
    NSString* name;
    NSArray* arguments;
}

- (id)initWithName:(NSString*)theName arguments:(NSArray *)theArguments;

- (NSString *)name;
- (NSArray *)arguments;

@end
