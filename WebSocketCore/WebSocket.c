//
//  WebSocket.c
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#include "WebSocket.h"

#pragma mark Lifecycle

WebSocketRef WebSocketCreate(CFAllocatorRef allocator, CFStringRef host, UInt16 port, void *userInfo) {
  WebSocketRef webSocket = CFAllocatorAllocate(allocator, sizeof(WebSocket), 0);
  if (webSocket) {
    webSocket->allocator = allocator ? CFRetain(allocator) : NULL;
    webSocket->retainCount = 1;
    webSocket->userInfo = userInfo;
    
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
      
      // Host is set to "0.0.0.0", set it to INADDR_ANY
      webSocket->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
      
      // Set the host based on provided string. TODO: hostname resolution?
      CFIndex hostCStringLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(host), kCFStringEncodingASCII) + 1;
      char *hostCString = CFAllocatorAllocate(webSocket->allocator, hostCStringLength, 0);
      if (hostCString) {
        if (CFStringGetCString(host, hostCString, hostCStringLength, kCFStringEncodingASCII)) {
          inet_aton(hostCString, &webSocket->addr.sin_addr);
        } else {
          // TODO: Couldn't get CString
        }
        CFAllocatorDeallocate(webSocket->allocator, hostCString);
      } else {
        // TODO: Couldn't allocate buffer
      }
    }
    
    webSocket->addr.sin_port = htons(port);
    
    CFDataRef address = CFDataCreate(webSocket->allocator, (const void *)&webSocket->addr, sizeof(webSocket->addr));
    if (address) {
      if (CFSocketSetAddress(webSocket->socket, (CFDataRef)address) != kCFSocketSuccess) {
        webSocket = WebSocketRelease(webSocket);
//        CFRelease(address); // TODO: is it retained by the function?
        goto fin;
      } else {
//        CFRelease(address); // TODO: is it retained bby the function
      }
    }
    
    // Create run loop source and add it to the current run loop
    CFRunLoopSourceRef source = CFSocketCreateRunLoopSource(webSocket->allocator, webSocket->socket, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CFRelease(source);
  }
fin:
  return webSocket;
}

WebSocketRef WebSocketCreateWithUserInfo(CFAllocatorRef allocator, void *userInfo) {
  return WebSocketCreate(allocator, kWebSocketHostLoopBack, kWebSocketPortAny, userInfo);
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

UInt16 WebSocketGetPort(WebSocketRef webSocket) {
  UInt16 port = UINT16_MAX;
  if (webSocket && webSocket->socket) {
    struct sockaddr_in sockname;
    socklen_t sockname_len = sizeof(sockname);
    if (getsockname(CFSocketGetNative(webSocket->socket), (struct sockaddr *)&sockname, &sockname_len) < 0) {
      // Error
    } else {
      port = ntohs(sockname.sin_port);
      // host = inet_ntoa(sockname.sin_addr)
    }
  }
  return port;
}

void WebSocketWriteWithString(WebSocketRef webSocket, CFStringRef value) {
  if (webSocket) {
    for (CFIndex i = 0; i < webSocket->clientsUsedLength; i++) {
      WebSocketWriteWithStringAndClientIndex(webSocket, value, i);
    }
  }
}

CFIndex WebSocketWriteWithStringAndClientIndex(WebSocketRef webSocket, CFStringRef value, CFIndex index) {
  CFIndex bytes = -1;
  if (webSocket) {
    if (value) {
      if (index < webSocket->clientsUsedLength) {
        bytes = WebSocketClientWriteWithString(webSocket->clients[index], value);
      }
    }
  }
  return bytes;
}

#pragma mark Callbacks

void WebSocketSetClientReadCallback(WebSocketRef webSocket, WebSocketDidClientReadCallback callback) {
  if (webSocket) {
    webSocket->callbacks.didClientReadCallback = callback;
  }
}

#pragma mark Internal, client management

CFIndex __WebSocketAppendClient(WebSocketRef webSocket, WebSocketClientRef client) {
  CFIndex count = -1;
  if (webSocket && client) {
    webSocket->clients[webSocket->clientsUsedLength++] = WebSocketClientRetain(client);
    count = webSocket->clientsUsedLength;
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
  printf("adding %p client\n", client);
}
