/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "EntityDefinitionParser.h"
#import "EntityDefinition.h"
#import "EntityDefinitionTokenizer.h"
#import "EntityDefinitionToken.h"
#import "EntityDefinitionProperty.h"
#import "ChoiceProperty.h"
#import "ChoiceArgument.h"
#import "ModelProperty.h"
#import "DefaultProperty.h"
#import "BaseProperty.h"
#import "SpawnFlag.h"

static NSString* InvalidTokenException = @"InvalidTokenException";

@implementation EntityDefinitionParser

- (id)initWithString:(NSString *)definitionString {
    if ((self = [self init])) {
        tokenizer = [[EntityDefinitionTokenizer alloc] initWithDefinitionString:definitionString];
    }
    
    return self;
}

- (void)expect:(int)expectedType actual:(EntityDefinitionToken *)actualToken {
    if (!actualToken || ([actualToken type] & expectedType) == 0)
        [NSException raise:InvalidTokenException format:@"invalid token: %@, expected %@", actualToken, [EntityDefinitionToken typeName:expectedType]];
}

- (TVector4f)parseColor {
    TVector4f color;
    
    EntityDefinitionToken* token = [tokenizer nextToken];
    [self expect:TT_B_O actual:token];

    token = [tokenizer nextToken];
    [self expect:TT_FRAC actual:token];
    color.x = [[token data] floatValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_FRAC actual:token];
    color.y = [[token data] floatValue];

    token = [tokenizer nextToken];
    [self expect:TT_FRAC actual:token];
    color.z = [[token data] floatValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_B_C actual:token];

    color.w = 1;
    return color;
}

- (TBoundingBox *)parseBounds {
    EntityDefinitionToken* token = [tokenizer nextToken];
    [self expect:TT_B_O | TT_QM actual:token];
    if ([token type] == TT_QM)
        return NULL;

    TBoundingBox* bounds = malloc(sizeof(TBoundingBox));
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    bounds->min.x = [[token data] intValue];

    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    bounds->min.y = [[token data] intValue];

    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    bounds->min.z = [[token data] intValue];

    token = [tokenizer nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [tokenizer nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    bounds->max.x = [[token data] intValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    bounds->max.y = [[token data] intValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    bounds->max.z = [[token data] intValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_B_C actual:token];
    
    return bounds;
}

- (NSDictionary *)parseFlags {
    EntityDefinitionToken* token = [tokenizer peekToken];
    if ([token type] != TT_WORD)
        return nil;
        
    NSMutableDictionary* flags = [[NSMutableDictionary alloc] init];
    while ([token type] == TT_WORD) {
        token = [tokenizer nextToken];
        [self expect:TT_WORD actual:token];
        
        NSString* name = [token data];
        int value = 1 << [flags count];
        SpawnFlag* flag = [[SpawnFlag alloc] initWithName:name flag:value];
        [flags setObject:flag forKey:name];
        
        token = [tokenizer peekToken];
    }
    
    return [flags autorelease];
}

- (EntityDefinitionToken *)nextTokenIgnoringNewlines {
    EntityDefinitionToken* token = [tokenizer nextToken];
    while ([token type] == TT_NL)
        token = [tokenizer nextToken];
    
    return token;
}

- (id <EntityDefinitionProperty>)parseProperty {
    EntityDefinitionToken* token = [self nextTokenIgnoringNewlines];
    if ([token type] != TT_WORD)
        return nil;

    id <EntityDefinitionProperty> property = nil;
    
    NSString* type = [[token data] retain];
    if ([type isEqualToString:@"choice"]) {
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_STR actual:token];
        NSString* name = [[token data] retain];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_O actual:token];
        
        NSMutableArray* arguments = [[NSMutableArray alloc] init];
        
        token = [self nextTokenIgnoringNewlines];
        while ([token type] == TT_B_O) {
            token = [self nextTokenIgnoringNewlines];
            [self expect:TT_DEC actual:token];
            int key = [[token data] intValue];
            
            token = [self nextTokenIgnoringNewlines];
            [self expect:TT_C actual:token];
            
            token = [self nextTokenIgnoringNewlines];
            [self expect:TT_STR actual:token];
            NSString* value = [[token data] retain];
            
            ChoiceArgument* argument = [[ChoiceArgument alloc] initWithKey:key value:value];
            [value release];
            
            [arguments addObject:argument];
            [argument release];
            
            token = [self nextTokenIgnoringNewlines];
            [self expect:TT_B_C actual:token];
            
            token = [self nextTokenIgnoringNewlines];
        }
        
        [self expect:TT_B_C actual:token];
        
        property = [[ChoiceProperty alloc] initWithName:name arguments:arguments];
        [name release];
        [arguments release];
    } else if ([type isEqualToString:@"model"]) {
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_O actual:token];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_STR actual:token];
        
        NSString* modelPath = [[token data] retain];
        int skinIndex = 0;
        
        NSCharacterSet* colonSet = [NSCharacterSet characterSetWithCharactersInString:@":"];
        NSRange range = [modelPath rangeOfCharacterFromSet:colonSet options:0 range:NSMakeRange(1, [modelPath length] - 1)];
        if (range.location != NSNotFound) {
            NSString* skinIndexStr = [modelPath substringFromIndex:range.location + 1];
            skinIndex = [skinIndexStr intValue];

            NSString* cleanModelPath = [modelPath substringToIndex:range.location];
            [modelPath release];
            modelPath = [cleanModelPath retain];
        }
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_C | TT_B_C actual:token];
        
        if ([token type] == TT_C) {
            token = [self nextTokenIgnoringNewlines];
            [self expect:TT_STR actual:token];

            NSString* flagName = [[token data] retain];
            property = [[ModelProperty alloc] initWithFlagName:flagName modelPath:modelPath skinIndex:skinIndex];

            [flagName release];
            [modelPath release];

            token = [self nextTokenIgnoringNewlines];
            [self expect:TT_B_C actual:token];
        } else {
            property = [[ModelProperty alloc] initWithModelPath:modelPath skinIndex:skinIndex];
            [modelPath release];
        }
    } else if ([type isEqualToString:@"default"]) {
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_O actual:token];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_STR actual:token];
        NSString* name = [[token data] retain];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_C actual:token];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_STR actual:token];
        NSString* value = [[token data] retain];
        
        property = [[DefaultProperty alloc] initWithName:name value:value];
        [name release];
        [value release];

        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_C actual:token];
    } else if ([type isEqualToString:@"base"]) {
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_O actual:token];

        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_STR actual:token];
        NSString* baseName = [[token data] retain];
        
        property = [[BaseProperty alloc] initWithBaseName:baseName];
        [baseName release];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_C actual:token];
    }
    [type release];
    
    token = [self nextTokenIgnoringNewlines];
    [self expect:TT_SC actual:token];
    
    return [property autorelease];
}

- (NSArray *)parseProperties {
    EntityDefinitionToken* token = [tokenizer peekToken];
    if ([token type] != TT_CB_O)
        return nil;

    NSMutableArray* properties = [[NSMutableArray alloc] init];
    
    token = [tokenizer nextToken];
    id <EntityDefinitionProperty> property;
    while ((property = [self parseProperty]) != nil)
        [properties addObject:property];
    
    [self expect:TT_CB_C actual:token];
    return [properties autorelease];
}

- (NSString *)parseDescription {
    EntityDefinitionToken* token = [tokenizer peekToken];
    if ([token type] == TT_ED_C)
        return nil;
    
    return [tokenizer remainderAsDescription];
}

- (EntityDefinition *)nextDefinition {
    EntityDefinitionToken* token = [tokenizer nextToken];
    if (token == nil)
        return nil;

    [self expect:TT_ED_O actual:token];

    NSString* name = nil;
    TVector4f color;
    BOOL hasColor = NO;
    TBoundingBox* bounds = NULL;
    NSDictionary* flags = nil;
    NSArray* properties = nil;
    NSString* description = nil;
    
    token = [tokenizer nextToken];
    [self expect:TT_WORD actual:token];
    
    name = [[token data] retain];
    
    token = [tokenizer peekToken];
    [self expect:TT_B_O | TT_NL actual:token];
    
    if ([token type] == TT_B_O) {
        color = [self parseColor];
        hasColor = YES;
        bounds = [self parseBounds];
        flags = [self parseFlags];
    }
    
    token = [tokenizer nextToken];
    [self expect:TT_NL actual:token];
    
    properties = [self parseProperties];
    description = [self parseDescription];
    
    token = [tokenizer nextToken];
    [self expect:TT_ED_C actual:token];

    EntityDefinition* definition;
    if (!hasColor)
        definition = [[EntityDefinition alloc] initBaseDefinitionWithName:name flags:flags properties:properties];
    else if (bounds != NULL)
        definition = [[EntityDefinition alloc] initPointDefinitionWithName:name color:&color bounds:bounds flags:flags properties:properties description:description];
    else
        definition = [[EntityDefinition alloc] initBrushDefinitionWithName:name color:&color flags:flags properties:properties description:description];
    
    [name release];
    if (bounds != NULL)
        free(bounds);
    
    return [definition autorelease];
}

- (void)dealloc {
    [tokenizer release];
    [super dealloc];
}

@end
