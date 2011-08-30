//
//  CompilerTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@interface CompilerTool : NSObject {
    NSString* name;
    NSString* path;
    NSMutableArray* parameters;
}

- (id)initWithName:(NSString *)theName path:(NSString *)thePath parameters:(NSArray *)theParameters;

- (NSString *)name;
- (void)setName:(NSString *)theName;

- (NSString *)path;
- (void)setPath:(NSString *)thePath;

- (NSArray *)parameters;
- (void)insertObject:(NSString *)theParameter inParametersAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromParametersAtIndex:(NSUInteger)theIndex;

@end
