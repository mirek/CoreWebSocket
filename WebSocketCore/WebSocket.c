//
//  WebSocket.c
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#include "WebSocket.h"

#pragma mark Lifecycle

WebSocketRef WebSocketCreate(CFAllocatorRef allocator, CFStringRef host, UInt16 port) {
  WebSocketRef webSocket = CFAllocatorAllocate(allocator, sizeof(WebSocket), 0);
  if (webSocket) {
    webSocket->allocator = allocator ? CFRetain(allocator) : NULL;
    webSocket->retainCount = 1;
    
    webSocket->clientsLength = 1024;
    webSocket->clientsUsedLength = 0;
    if (NULL == (webSocket->clients = CFAllocatorAllocate(allocator, webSocket->clientsLength, 0))) {
      webSocket = WebSocketRelease(webSocket);
      goto fin;
    }
    
    // Callbacks
    webSocket->callbacks.didAddClientCallback     = NULL;
    webSocket->callbacks.willRemoveClientCallback = NULL;
    webSocket->callbacks.didClientReadCallback    = NULL;
    
    // Setup the context;
    webSocket->context.copyDescription = NULL;
    webSocket->context.retain = NULL;
    webSocket->context.release = NULL;
    webSocket->context.version = 0;
    webSocket->context.info = webSocket;
    
    if (NULL == (webSocket->socket = CFSocketCreate(webSocket->allocator, PF_INET, SOCK_STREAM, IPPROTO_TCP, kCFSocketAcceptCallBack, __WebSocketAcceptCallBack, &webSocket->context))) {
      webSocket = WebSocketRelease(webSocket);
      goto fin;
    }
    
    // Re-use local addresses, if they're still in TIME_WAIT
    int yes = 1;
    setsockopt(CFSocketGetNative(webSocket->socket), SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes));
    
    /* Set the port and address we want to listen on */
    memset(&webSocket->addr, 0, sizeof(webSocket->addr));
    webSocket->addr.sin_len = sizeof(webSocket->addr);
    webSocket->addr.sin_family = AF_INET;
    
    if (CFEqual(kWebSocketHostAny, host)) {
      
      // Host is set to "*", set it to INADDR_ANY
      webSocket->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
      
      // Set the host based on provided string. TODO: hostname resolution?
      __WebSocketString hostASCII = __WebSocketStringMake(NULL, host, kCFStringEncodingASCII);
      inet_aton((const char *)__WebSocketStringGetCString(hostASCII), &webSocket->addr.sin_addr);
      __WebSocketStringDestroy(hostASCII);
    }
    
    webSocket->addr.sin_port = htons(port);
    
    CFDataRef address = CFDataCreate(webSocket->allocator, (const void *)&webSocket->addr, sizeof(webSocket->addr));
    if (CFSocketSetAddress(webSocket->socket, (CFDataRef)address) != kCFSocketSuccess) {
      webSocket = WebSocketRelease(webSocket);
      goto fin;
    }
    //CFRelease(address);
    
    // Create run loop source and add it to the current run loop
    CFRunLoopSourceRef source = CFSocketCreateRunLoopSource(webSocket->allocator, webSocket->socket, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CFRelease(source);
  }
fin:
  return webSocket;
}

WebSocketRef WebSocketRetain(WebSocketRef webSocket) {
  webSocket->retainCount++;
  return webSocket;
}

WebSocketRef WebSocketRelease(WebSocketRef webSocket) {
  if (webSocket) {
    if (--webSocket->retainCount == 0) {
      CFAllocatorRef allocator = webSocket->allocator;
      
      if (webSocket->clients) {
        while (--webSocket->clientsUsedLength >= 0)
          WebSocketClientRelease(webSocket->clients[webSocket->clientsUsedLength]);
        CFAllocatorDeallocate(allocator, webSocket->clients);
        webSocket->clients = NULL;
      }
      
      if (webSocket->socket) {
        CFSocketInvalidate(webSocket->socket);
        CFRelease(webSocket->socket);
        webSocket->socket = NULL;
      }

      CFAllocatorDeallocate(allocator, webSocket);
      webSocket = NULL;
      
      if (allocator)
        CFRelease(allocator);
    }
  }
  return webSocket;
}

#pragma mark Internal, client management

CFIndex __WebSocketAppendClient(WebSocketRef webSocket, WebSocketClientRef client) {
  CFIndex count = -1;
  if (webSocket && client) {
    webSocket->clients[count = ++webSocket->clientsUsedLength] = WebSocketClientRetain(client);
    if (webSocket->callbacks.didAddClientCallback)
      webSocket->callbacks.didAddClientCallback(webSocket, client);
  }
  return count;
}

CFIndex __WebSocketRemoveClient(WebSocketRef webSocket, WebSocketClientRef client) {
  CFIndex count = -1;
  if (webSocket && client) {
    for (CFIndex i = 0; i < webSocket->clientsUsedLength; i++) {
      if (webSocket->clients[i] == client) {
        if (webSocket->callbacks.willRemoveClientCallback)
          webSocket->callbacks.willRemoveClientCallback(webSocket, client);
        webSocket->clients[i] = webSocket->clients[count = --webSocket->clientsUsedLength];
        WebSocketClientRelease(client);
      }
    }
  }
  return count;
}

#pragma mark Callbacks

void __WebSocketAcceptCallBack(CFSocketRef socket, CFSocketCallBackType type, CFDataRef address, const void *sock, void *info) {
  WebSocketRef webSocket = (WebSocketRef)info;
  WebSocketClientRef client = WebSocketClientCreate(webSocket, *(CFSocketNativeHandle *)sock);
}
