/*
 *  IO.h
 *  TrenchBroom
 *
 *  Created by Kristian Duske on 15.05.11.
 *  Copyright 2011 TU Berlin. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>

char readChar(NSData* data, int location);
NSString* readString(NSData* data, NSRange range);
unsigned int readInt(NSData* data, int location);
