//
//  EntityDefinitionProperty.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    EDP_CHOICE,
    EDP_MODEL,
    EDP_DEFAULT,
    EDP_BASE
} EEntityDefinitionPropertyType;

@protocol EntityDefinitionProperty <NSObject>

- (EEntityDefinitionPropertyType)type;

@end
