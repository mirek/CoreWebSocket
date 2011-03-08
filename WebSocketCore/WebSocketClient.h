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

#pragma mark Lifecycle

WebSocketClientRef WebSocketClientCreate  (WebSocketRef webSocket, CFSocketNativeHandle handle);
WebSocketClientRef WebSocketClientRetain  (WebSocketClientRef client);
WebSocketClientRef WebSocketClientRelease (WebSocketClientRef client);

#pragma Handshake

uint32_t  __WebSocketGetMagicNumberWithKeyString (CFStringRef string);
void      __WebSocketDataAppendKey               (CFMutableDataRef data, CFStringRef string);
CFDataRef __WebSocketCreateMD5Data               (CFAllocatorRef allocator, CFStringRef key1, CFStringRef key2, CFDataRef key3);
bool        WebSocketClientHandShake             (WebSocketClientRef client);

#endif
