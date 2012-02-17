/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    
    NSLog(@"Found %lu entity definitions", [definitions count]);

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
    return [self definitionsOfType:type sortCriterion:ES_NAME];
}

- (NSArray *)definitionsOfType:(EEntityDefinitionType)type sortCriterion:(EEntityDefinitionSortCriterion)criterion {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    for (EntityDefinition* definition in [definitionsByName objectEnumerator])
        if ([definition type] == type)
            [result addObject:definition];
    
    if (criterion == ES_USAGE)
        [result sortUsingSelector:@selector(compareByUsageCount:)];
    
    return [result autorelease];
}

- (void)dealloc {
    [definitions release];
    [definitionsByName release];
    [super dealloc];
}

@end
