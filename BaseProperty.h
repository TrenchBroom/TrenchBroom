//
//  BaseProperty.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityDefinitionProperty.h"

@interface BaseProperty : NSObject <EntityDefinitionProperty> {
    NSString* baseName;
}

- (id)initWithBaseName:(NSString *)theBaseName;

- (NSString *)baseName;

@end
