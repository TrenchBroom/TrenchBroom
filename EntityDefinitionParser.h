//
//  EntityDefinitionParser.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class EntityDefinition;
@class EntityDefinitionTokenizer;

@interface EntityDefinitionParser : NSObject {
    EntityDefinitionTokenizer* tokenizer;
}

- (id)initWithString:(NSString *)definitionString;
- (EntityDefinition *)nextDefinition;

@end
