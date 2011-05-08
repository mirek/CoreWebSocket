//
//  WebSocket.h
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#ifndef __CORE_WEB_SOCKET_WEB_SOCKET__
#define __CORE_WEB_SOCKET_WEB_SOCKET__ 1

#include "CoreWebSocket/CoreWebSocket.h"

#define __WebSocketMaxHeaderKeyLength 4096

#pragma mark Lifecycle

WebSocketRef WebSocketCreate  (CFAllocatorRef allocator, CFStringRef host, UInt16 port, void *userInfo);

// Create WebSocketRef using any host and any available port.
WebSocketRef WebSocketCreateWithUserInfo(CFAllocatorRef allocator, void *userInfo);

WebSocketRef WebSocketRetain  (WebSocketRef webSocket);
WebSocketRef WebSocketRelease (WebSocketRef webSocket);

UInt16 WebSocketGetPort(WebSocketRef webSocket);

void    WebSocketWriteWithString               (WebSocketRef webSocket, CFStringRef value);
CFIndex WebSocketWriteWithStringAndClientIndex (WebSocketRef webSocket, CFStringRef value, CFIndex index);

#pragma mark Callbacks

void WebSocketSetClientReadCallback(WebSocketRef webSocket, WebSocketDidClientReadCallback callback);

#pragma mark Internal, client management

CFIndex __WebSocketAppendClient (WebSocketRef webSocket, WebSocketClientRef client);
CFIndex __WebSocketRemoveClient (WebSocketRef webSocket, WebSocketClientRef client);

#pragma mark Internal, socket callback

void __WebSocketAcceptCallBack(CFSocketRef socket, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);

#endif

