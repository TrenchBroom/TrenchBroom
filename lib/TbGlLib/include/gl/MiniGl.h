/*
 Copyright (C) 2026 Kristian Duske

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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// IWYU pragma: always_keep

// This header provides the subset of OpenGL symbols used by TbGlLib and TbRenderLib
// without depending on Qt headers.

#include <cstdint>

#if !defined(GL_VERSION_1_1)
using GLvoid = void;
using GLboolean = unsigned char;
using GLbyte = signed char;
using GLubyte = unsigned char;
using GLshort = short;
using GLushort = unsigned short;
using GLint = int;
using GLuint = unsigned int;
using GLsizei = int;
using GLenum = unsigned int;
using GLbitfield = unsigned int;
using GLfloat = float;
using GLdouble = double;
using GLclampd = double;
#endif

#if !defined(GL_VERSION_2_0)
using GLchar = char;
#endif

using GLintptr = std::intptr_t;
using GLsizeiptr = std::intptr_t;

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_BACK
#define GL_BACK 0x0405
#endif
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_BLEND
#define GL_BLEND 0x0BE2
#endif
#ifndef GL_BYTE
#define GL_BYTE 0x1400
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif
#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_CONTEXT_LOST
#define GL_CONTEXT_LOST 0x0507
#endif
#ifndef GL_CULL_FACE
#define GL_CULL_FACE 0x0B44
#endif
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#endif
#ifndef GL_CW
#define GL_CW 0x0900
#endif
#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT 0x00000100
#endif
#ifndef GL_DEPTH_TEST
#define GL_DEPTH_TEST 0x0B71
#endif
#ifndef GL_DOUBLE
#define GL_DOUBLE 0x140A
#endif
#ifndef GL_DST_ALPHA
#define GL_DST_ALPHA 0x0304
#endif
#ifndef GL_DST_COLOR
#define GL_DST_COLOR 0x0306
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_FALSE
#define GL_FALSE 0
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif
#ifndef GL_FLOAT
#define GL_FLOAT 0x1406
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_FRONT
#define GL_FRONT 0x0404
#endif
#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK 0x0408
#endif
#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_INT
#define GL_INT 0x1404
#endif
#ifndef GL_INVALID_ENUM
#define GL_INVALID_ENUM 0x0500
#endif
#ifndef GL_INVALID_FRAMEBUFFER_OPERATION
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#endif
#ifndef GL_INVALID_OPERATION
#define GL_INVALID_OPERATION 0x0502
#endif
#ifndef GL_INVALID_VALUE
#define GL_INVALID_VALUE 0x0501
#endif
#ifndef GL_LEQUAL
#define GL_LEQUAL 0x0203
#endif
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_LINEAR
#define GL_LINEAR 0x2601
#endif
#ifndef GL_LINEAR_MIPMAP_NEAREST
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#endif
#ifndef GL_LINEAR_MIPMAP_LINEAR
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#endif
#ifndef GL_LINES
#define GL_LINES 0x0001
#endif
#ifndef GL_LINE_LOOP
#define GL_LINE_LOOP 0x0002
#endif
#ifndef GL_LINE_STRIP
#define GL_LINE_STRIP 0x0003
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_LUMINANCE
#define GL_LUMINANCE 0x1909
#endif
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif
#ifndef GL_NEAREST_MIPMAP_NEAREST
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#endif
#ifndef GL_NEAREST_MIPMAP_LINEAR
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif
#ifndef GL_NO_ERROR
#define GL_NO_ERROR 0
#endif
#ifndef GL_ONE
#define GL_ONE 1
#endif
#ifndef GL_ONE_MINUS_DST_ALPHA
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#endif
#ifndef GL_ONE_MINUS_DST_COLOR
#define GL_ONE_MINUS_DST_COLOR 0x0307
#endif
#ifndef GL_ONE_MINUS_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#endif
#ifndef GL_ONE_MINUS_SRC_COLOR
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#endif
#ifndef GL_OUT_OF_MEMORY
#define GL_OUT_OF_MEMORY 0x0505
#endif
#ifndef GL_POINTS
#define GL_POINTS 0x0000
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_POLYGON_BIT
#define GL_POLYGON_BIT 0x00000008
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif
#ifndef GL_RENDERER
#define GL_RENDERER 0x1F01
#endif
#ifndef GL_REPEAT
#define GL_REPEAT 0x2901
#endif
#ifndef GL_RGB
#define GL_RGB 0x1907
#endif
#ifndef GL_RGBA
#define GL_RGBA 0x1908
#endif
#ifndef GL_SHORT
#define GL_SHORT 0x1402
#endif
#ifndef GL_SRC_ALPHA
#define GL_SRC_ALPHA 0x0302
#endif
#ifndef GL_SRC_ALPHA_SATURATE
#define GL_SRC_ALPHA_SATURATE 0x0308
#endif
#ifndef GL_SRC_COLOR
#define GL_SRC_COLOR 0x0300
#endif
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_TABLE_TOO_LARGE
#define GL_TABLE_TOO_LARGE 0x8031
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MAG_FILTER 0x2800
#endif
#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif
#ifndef GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_MIN_FILTER 0x2801
#endif
#ifndef GL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_S 0x2802
#endif
#ifndef GL_TEXTURE_WRAP_T
#define GL_TEXTURE_WRAP_T 0x2803
#endif
#ifndef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#endif
#ifndef GL_TRIANGLE_FAN
#define GL_TRIANGLE_FAN 0x0006
#endif
#ifndef GL_TRIANGLE_STRIP
#define GL_TRIANGLE_STRIP 0x0005
#endif
#ifndef GL_TRUE
#define GL_TRUE 1
#endif
#ifndef GL_UNPACK_ALIGNMENT
#define GL_UNPACK_ALIGNMENT 0x0CF5
#endif
#ifndef GL_UNPACK_LSB_FIRST
#define GL_UNPACK_LSB_FIRST 0x0CF1
#endif
#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#endif
#ifndef GL_UNPACK_SKIP_PIXELS
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#endif
#ifndef GL_UNPACK_SKIP_ROWS
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#endif
#ifndef GL_UNPACK_SWAP_BYTES
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#endif
#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE 0x1401
#endif
#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT 0x1405
#endif
#ifndef GL_UNSIGNED_SHORT
#define GL_UNSIGNED_SHORT 0x1403
#endif
#ifndef GL_VENDOR
#define GL_VENDOR 0x1F00
#endif
#ifndef GL_VERSION
#define GL_VERSION 0x1F02
#endif
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_ZERO
#define GL_ZERO 0
#endif
