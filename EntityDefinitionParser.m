//
//  EntityDefinitionParser.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
#import "BoundingBox.h"
#import "Vector3f.h"

static NSString* InvalidTokenException = @"InvalidTokenException";

@implementation EntityDefinitionParser

- (id)initWithString:(NSString *)definitionString {
    if (self = [self init]) {
        tokenizer = [[EntityDefinitionTokenizer alloc] initWithDefinitionString:definitionString];
    }
    
    return self;
}

- (void)expect:(int)expectedType actual:(EntityDefinitionToken *)actualToken {
    if (!actualToken || ([actualToken type] & expectedType) == 0)
        [NSException raise:InvalidTokenException format:@"invalid token: %@, expected %@", actualToken, [EntityDefinitionToken typeName:expectedType]];
}

- (float *)parseColor {
    float* color = malloc(3 * sizeof(float));
    
    EntityDefinitionToken* token = [tokenizer nextToken];
    [self expect:TT_B_O actual:token];

    token = [tokenizer nextToken];
    [self expect:TT_FRAC actual:token];
    color[0] = [[token data] floatValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_FRAC actual:token];
    color[1] = [[token data] floatValue];

    token = [tokenizer nextToken];
    [self expect:TT_FRAC actual:token];
    color[2] = [[token data] floatValue];
    
    token = [tokenizer nextToken];
    [self expect:TT_B_C actual:token];
    
    return color;
}

- (BoundingBox *)parseBounds {
    EntityDefinitionToken* token = [tokenizer nextToken];
    [self expect:TT_B_O | TT_QM actual:token];
    if ([token type] == TT_QM)
        return nil;
    
    Vector3f* min = [[Vector3f alloc] init];
    Vector3f* max = [[Vector3f alloc] init];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    [min setX:[[token data] intValue]];

    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    [min setY:[[token data] intValue]];

    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    [min setZ:[[token data] intValue]];

    token = [tokenizer nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [tokenizer nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    [max setX:[[token data] intValue]];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    [max setY:[[token data] intValue]];
    
    token = [tokenizer nextToken];
    [self expect:TT_DEC actual:token];
    [max setZ:[[token data] intValue]];
    
    token = [tokenizer nextToken];
    [self expect:TT_B_C actual:token];
    
    [max add:min];
    BoundingBox* bounds = [[BoundingBox alloc] initWithMin:min max:max];
    [min release];
    [max release];
    return [bounds autorelease];
}

- (NSArray *)parseFlags {
    EntityDefinitionToken* token = [tokenizer peekToken];
    if ([token type] != TT_WORD)
        return nil;
        
    NSMutableArray* flags = [[NSMutableArray alloc] init];
    while ([token type] == TT_WORD) {
        token = [tokenizer nextToken];
        [self expect:TT_WORD actual:token];
        
        NSString* flag = [token data];
        [flags addObject:flag];
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
        property = [[ModelProperty alloc] initWithModelPath:modelPath];
        [modelPath release];
        
        token = [self nextTokenIgnoringNewlines];
        [self expect:TT_B_C actual:token];
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
    float* color = NULL;
    BoundingBox* bounds = nil;
    NSArray* flags = nil;
    NSArray* properties = nil;
    NSString* description = nil;
    
    token = [tokenizer nextToken];
    [self expect:TT_WORD actual:token];
    
    name = [[token data] retain];
    
    token = [tokenizer peekToken];
    [self expect:TT_B_O | TT_NL actual:token];
    
    if ([token type] == TT_B_O) {
        color = [self parseColor];
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
    if (color == NULL)
        definition = [[EntityDefinition alloc] initBaseDefinitionWithName:name flags:flags properties:properties];
    else if (bounds == nil)
        definition = [[EntityDefinition alloc] initBrushDefinitionWithName:name color:color flags:flags properties:properties description:description];
    else
        definition = [[EntityDefinition alloc] initPointDefinitionWithName:name color:color bounds:bounds flags:flags properties:properties description:description];
    
    [name release];
    free(color);
    
    return [definition autorelease];
}

- (void)dealloc {
    [tokenizer release];
    [super dealloc];
}

@end
