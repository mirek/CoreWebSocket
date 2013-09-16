//
// The MIT License
//
// Copyright (c) 2011 - 2013, Mirek Rusin <mirek [at] me [dot] com>
// http://github.com/mirek/CoreWebSocket
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#ifndef __CORE_WEB_SOCKET_WEB_SOCKET_CLIENT_H__
#define __CORE_WEB_SOCKET_WEB_SOCKET_CLIENT_H__ 1

#include <CoreFoundation/CFString.h>
#include "WebSocket.h"
#include "WebSocketFrame.h"
#include "cuEnc64.h"

#pragma mark Lifecycle

WebSocketClientRef WebSocketClientCreate  (WebSocketRef webSocket, CFSocketNativeHandle handle);
void               WebSocketClientRetain  (WebSocketClientRef self);
void               WebSocketClientRelease (WebSocketClientRef self);

#pragma mark Write

CFIndex WebSocketClientWriteWithData   (WebSocketClientRef self, CFDataRef value);
CFIndex WebSocketClientWriteWithString (WebSocketClientRef self, CFStringRef value);
CFIndex WebSocketClientWriteWithFormat (WebSocketClientRef self, CFStringRef fmt, ...) CF_FORMAT_FUNCTION(2,0);

#pragma mark Handshake (internal)

uint32_t  __WebSocketGetMagicNumberWithKeyValueString         (CFStringRef string);
Boolean   __WebSocketDataAppendMagickNumberWithKeyValueString (CFMutableDataRef data, CFStringRef string);
CFDataRef __WebSocketCreateMD5Data                            (CFAllocatorRef allocator, CFDataRef value) CF_RETURNS_RETAINED;
CFDataRef __WebSocketCreateSHA1DataWithData                   (CFAllocatorRef allocator, CFDataRef value) CF_RETURNS_RETAINED;
CFDataRef __WebSocketCreateSHA1DataWithString                 (CFAllocatorRef allocator, CFStringRef value, CFStringEncoding encoding) CF_RETURNS_RETAINED;
Boolean   __WebSocketClientReadHandShake                      (WebSocketClientRef client);
Boolean   __WebSocketClientWriteWithHTTPMessage               (WebSocketClientRef client, CFHTTPMessageRef message);

#endif
