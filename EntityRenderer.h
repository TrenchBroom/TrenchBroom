//
//  EntityRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol Entity;

@protocol EntityRenderer <NSObject>

- (void)renderWithEntity:(id <Entity>)theEntity;

@end
