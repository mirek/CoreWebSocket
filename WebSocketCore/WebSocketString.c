//
//  WebSocketString.c
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#include "WebSocketString.h"

inline __WebSocketString __WebSocketStringMake(CFAllocatorRef allocator, CFStringRef aString, CFStringEncoding encoding) {
  __WebSocketString string;
  string.allocator = allocator ? CFRetain(allocator) : NULL;
  string.string = aString ? CFRetain(aString) : NULL;
  string.encoding = encoding;
  string.maximumSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(aString), encoding) + 1;
  if ((string.pointer = (const unsigned char *)CFStringGetCStringPtr(aString, encoding))) {
    string.buffer = NULL;
  } else {
    string.buffer = CFAllocatorAllocate(allocator, string.maximumSize, 0);
    if (string.buffer) {
      CFStringGetCString(aString, (char *)string.buffer, string.maximumSize, encoding);
    }
  }
  return string;
}

inline const unsigned char *__WebSocketStringGetCString(__WebSocketString string) {
  return string.pointer ? string.pointer : string.buffer;
}

inline CFIndex __WebSocketStringGetMaximumSize(__WebSocketString string) {
  return string.maximumSize;
}

inline void __WebSocketStringDestroy(__WebSocketString string) {
  if (string.buffer)
    CFAllocatorDeallocate(string.allocator, (void *)string.buffer);
}
