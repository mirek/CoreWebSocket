//
//  WebSocketClient.h
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#ifndef __CORE_WEB_SOCKET_CLIENT__
#define __CORE_WEB_SOCKET_CLIENT__ 1

#include "WebSocket.h"
#include "cuEnc64.h"

#pragma mark Lifecycle

WebSocketClientRef WebSocketClientCreate  (WebSocketRef webSocket, CFSocketNativeHandle handle);
WebSocketClientRef WebSocketClientRetain  (WebSocketClientRef client);
WebSocketClientRef WebSocketClientRelease (WebSocketClientRef client);

#pragma mark Write

CFIndex WebSocketClientWriteWithData   (WebSocketClientRef client, CFDataRef value);
CFIndex WebSocketClientWriteWithString (WebSocketClientRef client, CFStringRef value);

#pragma mark Handshake (internal)

uint32_t  __WebSocketGetMagicNumberWithKeyValueString         (CFStringRef string);
bool      __WebSocketDataAppendMagickNumberWithKeyValueString (CFMutableDataRef data, CFStringRef string);
CFDataRef __WebSocketCreateMD5Data                            (CFAllocatorRef allocator, CFDataRef value) CF_RETURNS_RETAINED;
CFDataRef __WebSocketCreateSHA1DataWithData                   (CFAllocatorRef allocator, CFDataRef value) CF_RETURNS_RETAINED;
CFDataRef __WebSocketCreateSHA1DataWithString                 (CFAllocatorRef allocator, CFStringRef value, CFStringEncoding encoding) CF_RETURNS_RETAINED;
bool      __WebSocketClientReadHandShake                      (WebSocketClientRef client);
bool      __WebSocketClientWriteWithHTTPMessage               (WebSocketClientRef client, CFHTTPMessageRef message);

#endif
