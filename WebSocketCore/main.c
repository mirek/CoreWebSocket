//
//  main.c
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#include "WebSocket.h"

void myread(WebSocketRef webSocket, WebSocketClientRef client, CFDataRef data) {
  CFStringRef string = CFStringCreateWithBytes(NULL, CFDataGetBytePtr(data), CFDataGetLength(data), kCFStringEncodingUTF8, 0);
//  printf("callback beg\n");
  if (string) {
    CFShow(string);
    CFRelease(string);
  } else {
//    printf("buu\n");
  }
//  printf("callback end\n");
}


int main (int argc, const char * argv[]) {
  
//  WebSocketRef webSocket = WebSocketCreate(NULL, kWebSocketHostAny, 6001);
//  if (webSocket) {
//    webSocket->callbacks.didClientReadCallback = myread;
//    CFRunLoopRun();
//    WebSocketRelease(webSocket);
//  }
  
  
//  CFDataRef key3 = CFDataCreate(NULL, (const void *)"Tm[K T2u", 8);
//  CFDataRef data = __WebSocketCreateMD5Data(NULL, CFSTR("18x 6]8vM;54 *(5:  {   U1]8  z [  8"), CFSTR("1_ tx7X d  <  nw  334J702) 7]o}` 0"), key3);
//  
//  printf("\n");
//  for (int i = 0; i < CFDataGetLength(data); i++) {
//    printf("%02x", *(uint8_t *)(CFDataGetBytePtr(data) + i));
//  }
//  printf("\n");
    
  
//  1868545188, 12: 09 47 fa 63 00 
//  1733470270, 10: 0a 55 10 d3 00 
  // all 09 47 fa 63 0a 55 10 d3 54 6d 5b 4b 20 54 32 75 
//  66514a2c664e2f344634217e4b7e4d48
  
//  CFRelease(data);
  
  // insert code here...
  CFShow(CFSTR("Hello, World!\n"));
    return 0;
}

