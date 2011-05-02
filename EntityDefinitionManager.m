//
//  EntitiyDefinitionManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionManager.h"
#import "EntityDefinitionParser.h"

@implementation EntityDefinitionManager

- (id)initWithDefinitionFile:(NSString *)thePath {
    NSAssert(thePath != nil, @"path must not be nil");
    NSString* definitionsString = [NSString stringWithContentsOfFile:thePath encoding:NSASCIIStringEncoding error:NULL];
    NSAssert(definitionsString != nil, @"definitions file must be readable");
    
    NSLog(@"Loading entity definitions from '%@'", thePath);
    
    if (self = [self init]) {
        definitions = [[NSMutableDictionary alloc] init];
        definitionsByName = [[NSMutableArray alloc] init];
        
        EntityDefinitionParser* parser = [[EntityDefinitionParser alloc] initWithString:definitionsString];
        EntityDefinition* definition;
        while ((definition = [parser nextDefinition]) != nil) {
            [definitions setObject:definition forKey:[definition name]];
            [definitionsByName addObject:definition];
        }
        
        [definitionsByName sortUsingSelector:@selector(compareByName:)];
        [parser release];
    }
    
    NSLog(@"Found %i entity definitions", [definitions count]);

    return self;
}

- (EntityDefinition *)definitionForName:(NSString *)name {
    NSAssert(name != nil, @"name must not be nil");
    return [definitions objectForKey:name];
}

- (NSArray *)definitions {
    return definitionsByName;
}

- (NSArray *)definitionsOfType:(EEntityDefinitionType)type {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    NSEnumerator* definitionEn = [definitionsByName objectEnumerator];
    EntityDefinition* definition;
    while ((definition = [definitionEn nextObject])) {
        if ([definition type] == type)
            [result addObject:definition];
    }
    
    return [result autorelease];
}

- (void)dealloc {
    [definitions release];
    [definitionsByName release];
    [super dealloc];
}

@end
