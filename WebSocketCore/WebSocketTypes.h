//
//  WebSocketTypes.h
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#define kWebSocketHostAny      CFSTR("0.0.0.0")
#define kWebSocketHostLoopBack CFSTR("127.0.0.1")
#define kWebSocketPortAny      0

typedef struct WebSocket WebSocket;
typedef WebSocket *WebSocketRef;

typedef struct WebSocketCallbacks WebSocketCallbacks;

typedef struct WebSocketClient WebSocketClient;
typedef WebSocketClient *WebSocketClientRef;

#pragma mark WebSocket Callbacks

typedef void (*WebSocketDidAddClientCallback)     (WebSocketRef webSocket, WebSocketClientRef client);
typedef void (*WebSocketWillRemoveClientCallback) (WebSocketRef webSocket, WebSocketClientRef client);
typedef void (*WebSocketDidClientReadCallback)    (WebSocketRef webSocket, WebSocketClientRef client, CFStringRef value);

struct WebSocketCallbacks {
  WebSocketDidAddClientCallback     didAddClientCallback;
  WebSocketWillRemoveClientCallback willRemoveClientCallback;
  WebSocketDidClientReadCallback    didClientReadCallback;
};

#pragma mark WebSocket Client

enum WebSocketClientState {
  kWebSocketClientInitialized,
  kWebSocketClientReadStreamOpened,
  kWebSocketClientWriteStreamOpened,
  kWebSocketClientHandShakeError,
  kWebSocketClientHandShakeRead,
  kWebSocketClientHandShakeSent,
  kWebSocketClientReady
};

struct WebSocketClient {
  CFAllocatorRef allocator;
  CFIndex retainCount;
  WebSocketRef webSocket;
  CFSocketNativeHandle handle;
  CFReadStreamRef read;
  CFWriteStreamRef write;
  
  CFMutableArrayRef writeQueue;
  
  CFHTTPMessageRef handShakeRequestHTTPMessage;
  
  // Linked list of clients
  WebSocketClientRef previousClient;
  WebSocketClientRef nextClient;
  
  CFStreamClientContext context;
  
  bool didReadHandShake;
  bool didWriteHandShake;
  
};

struct WebSocket {
  CFAllocatorRef allocator;
  CFIndex retainCount;
  void *userInfo;
  
  struct sockaddr_in addr;
  CFSocketRef socket;
  CFReadStreamRef read;
  CFWriteStreamRef write;
  
  CFIndex clientsUsedLength;
  CFIndex clientsLength;
  WebSocketClientRef *clients;
  
  CFSocketContext context;
  
  WebSocketCallbacks callbacks;
};
