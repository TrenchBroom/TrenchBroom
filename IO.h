/*
 *  IO.h
 *  TrenchBroom
 *
 *  Created by Kristian Duske on 15.05.11.
 *  Copyright 2011 TU Berlin. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import "Math.h"

char readChar(NSData* data, int location);
unsigned char readUChar(NSData* data, int location);
NSString* readString(NSData* data, NSRange range);
unsigned int readULong(NSData* data, int location);
int readLong(NSData* data, int location);
unsigned int readUShort(NSData* data, int location);
int readShort(NSData* data, int location);
float readFloat(NSData* data, int location);
TVector3f readVector3f(NSData* data, int location);
