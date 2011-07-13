//
//  TextAnchor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@protocol TextAnchor <NSObject>

- (void)position:(TVector3f *)thePosition;

@end
