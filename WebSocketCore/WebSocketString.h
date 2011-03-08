//
// WebSocketString.h
// WebSocketCore
//
// Created by Mirek Rusin on 07/03/2011.
// Copyright 2011 Inteliv Ltd. All rights reserved.
//

#ifndef __CORE_WEB_SOCKET_STRING__
#define __CORE_WEB_SOCKET_STRING__ 1

#include <CoreFoundation/CoreFoundation.h>

#pragma Internal string helper for fast CFStringRef buffer access

// Internal helper structure supporting fast and easy access to CFStringRef's
// string buffer. Whenever possible getting the buffer is O(1) without copying
// CFStringRef's buffer. If not possible buffer is being copied.
//
// Provided CFStringRef has to be valid for the lifetime of the struct.
// 
// When finished invoke __JSONUTF8StringDestroy(...) to deallocate internal
// data properly. Internal members of this struct should not be accessed directly.
// Use provided functions instead.
//
// This structure is not opaque and is not intended to be passed as function
// argument. CFStringRef should be passed instead. The structure should be
// used inside the function to get access to UTF8 buffer of CFStringRef.
typedef struct {
  CFAllocatorRef allocator;
  CFStringRef string;
  CFStringEncoding encoding;
  CFIndex maximumSize; // ...for encoding
  const unsigned char *pointer;
  const unsigned char *buffer;
} __WebSocketString;

typedef struct __WebSocketString *__WebSocketStringRef;

// Internal function, use it to instantiate __JSONUTF8String structure to get
// fast and easy access to CFStringRef's UTF8 buffer.
//
// The function takes advantage of Core Foundation CFStringRef and tries to get buffer
// pointer in 0(1). If not possible, the buffer is being copied to internally allocated
// storage.
//
// Returns properly initialized __JSONUTF8String struct. Use __JSONUTF8StringGetBuffer
// to get the UTF8 buffer. Invoke __JSONUTF8StringDestroy to deallocate this struct properly.
__WebSocketString     __WebSocketStringMake           (CFAllocatorRef allocator, CFStringRef string, CFStringEncoding encoding);

// Internal function, get the internal buffer of associated CFStringRef.
const unsigned char *__WebSocketStringGetCString      (__WebSocketString string);

// Get the size of the internal buffer
CFIndex              __WebSocketStringGetMaximumSizeForEncoding (__WebSocketString string);

// Deallocate internal members of __JSONUTF8String stuct.
void                 __WebSocketStringDestroy        (__WebSocketString string);

#endif

