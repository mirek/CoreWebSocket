//
//  WebSocketClient.c
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#include "WebSocketClient.h"

#pragma mark Write

// Internal function, write provided buffer in a frame [0x00 ... 0xff]
CFIndex __WebSocketClientWriteFrame(WebSocketClientRef client, const UInt8 *buffer, CFIndex length) {
  CFIndex bytes = -1;
  if (client) {
    if (buffer) {
      if (length > 0) {
        CFWriteStreamWrite(client->write, (UInt8[]){ 0x00 }, 1);
        bytes = CFWriteStreamWrite(client->write, buffer, length);
        CFWriteStreamWrite(client->write, (UInt8[]){ 0xff }, 1);
      }
    }
  }
  return bytes;
}

CFIndex WebSocketClientWriteWithData(WebSocketClientRef client, CFDataRef value) {
  return __WebSocketClientWriteFrame(client, CFDataGetBytePtr(value), CFDataGetLength(value));
}

CFIndex WebSocketClientWriteWithString(WebSocketClientRef client, CFStringRef value) {
  CFIndex bytes = -1;
  if (client) {
    if (value) {
      CFDataRef data = CFStringCreateExternalRepresentation(client->allocator, value, kCFStringEncodingUTF8, 0);
      if (data) {
        CFShow(value);
        bytes = WebSocketClientWriteWithData(client, data);
        CFRelease(data);
      }
    }
  }
  return bytes;
}

#pragma mark Read callback

bool __WebSocketClientWriteHandShake(WebSocketClientRef client);

void __WebSocketClientReadCallBack(CFReadStreamRef stream, CFStreamEventType eventType, void *info) {
  WebSocketClientRef client = info;
  switch (eventType) {
    case kCFStreamEventOpenCompleted:
//      printf("read: open completed\n");
      break;
      
    case kCFStreamEventHasBytesAvailable:
//      printf("read: has bytes available %p\n", client);
//      printf("client %p\n", client);
      fflush(stdout);
      
      if (client) {
        if (!client->didReadHandShake) {
//          printf("no handshake yet, will read\n");
          if (WebSocketClientHandShake(client)) {
//            printf("handshaked read\n");
            if (!client->didWriteHandShake) {
              if (CFWriteStreamCanAcceptBytes(client->write)) {
//                printf("no handshake written and client accepts, so writting\n");
                if (__WebSocketClientWriteHandShake(client)) {
//                  printf("all ok\n");
                } else {
//                  printf("not ok\n");
                }
              }
            }
            __WebSocketAppendClient(client->webSocket, client);
          }
          //WebSocketClientRelease(client);
        } else {
//          printf("did already handshake\n");
          
          UInt8 b[4096];
          memset(b, 0, sizeof(b));
          CFIndex by = 0;
          
//          printf("loop >\n");
          if (CFReadStreamHasBytesAvailable(client->read)) {
          
//            printf("loop =\n");
            by = CFReadStreamRead(stream, b, sizeof(b) - 1);
            
            
            
//            for (UInt8 *j = b; j != NULL && j < b + by; j = (UInt8 *)strchr((const char *)j, 0xff)) {
//            }
            
//            for (int i = 0; i < by; i++)
//              printf("%02x ", b[i]);
//            printf("\n");
            
            if (by > 2) {
              
              const char *from = (const char *)b + 1;
              const char *to = strchr(from, 0xff);
              
              while (to) {
                
//                printf("chunk %p - %p\n", from, to);

                if (client->webSocket->callbacks.didClientReadCallback) {
                  CFDataRef data = CFDataCreate(client->allocator, (const void *)from, to - from);
                  if (data) {
                    CFStringRef string = CFStringCreateWithBytes(client->allocator, CFDataGetBytePtr(data), CFDataGetLength(data), kCFStringEncodingUTF8, 0);
                    if (string) {
                      client->webSocket->callbacks.didClientReadCallback(client->webSocket, client, string);
                      CFRelease(string);
                    }
                    CFRelease(data);
                  }
                }
                
                
                from = to + 2;
                to = strchr(from, 0xff);
              }
              
            }
            
            char *end = strchr((const char *)b, 0xff);
            if (end)
              *end = 0x00;
//            printf("found l = %ld\n", (void *)end - (void *)b);
//            printf("read %ld bytes, echo: ?\n", by);
            fflush(stdout);
          } else {
//            printf("liar!\n");
          }
//          printf("loop <\n");
          fflush(stdout);
            
//          for (CFIndex by = 0; (by = CFReadStreamRead(stream, b, sizeof(b))) > 0; ) {
//            if (by > 0)
//              printf("echo: %ld, %s\n", by, b+1);
//            else
//              printf("echo: ERROR %ld\n", by);
//          }
        }
      } else {
        // TODO: Error creating client
      }
      
//      printf("read: done\n");
      break;
      
    case kCFStreamEventErrorOccurred:
//      printf("read: error occured\n");
      
      break;
      
    case kCFStreamEventEndEncountered:
//      printf("read: end\n");
      break;
    
    default:
//      printf("read: buu\n");
      break;
  }
}

bool __WebSocketClientWriteHandShake(WebSocketClientRef client) {
  bool success = 0;
  printf("entering handshake %i\n", client->didReadHandShake);

  if (client->didReadHandShake) {
    CFStringRef key1 = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-Websocket-Key1"));
    CFStringRef key2 = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-Websocket-Key2"));
    CFDataRef key3 = CFHTTPMessageCopyBody(client->handShakeRequestHTTPMessage);
    CFStringRef origin = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Origin"));
    CFStringRef host = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Host"));

    CFHTTPMessageRef response = CFHTTPMessageCreateEmpty(NULL, 0);
    CFHTTPMessageAppendBytes(response, (const UInt8 *)"HTTP/1.1 101 Web Socket Protocol Handshake\r\n", 44);
    CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Upgrade"), CFSTR("WebSocket"));
    CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Connection"), CFSTR("Upgrade"));
    CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Sec-Websocket-Origin"), origin);
    
    CFMutableStringRef location = CFStringCreateMutable(client->allocator, 0);
    CFStringAppend(location, CFSTR("ws://"));
    CFStringAppend(location, host);
    CFStringAppend(location, CFSTR("/"));
    CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Sec-Websocket-Location"), location);
    CFRelease(location);
    
    // Set MD5 hash
    {
      CFDataRef data = __WebSocketCreateMD5Data(NULL, key1, key2, key3);
      CFHTTPMessageSetBody(response, data);
      
      unsigned char buffer[4096];
      CFIndex usedBufLen = 0;
      memset(buffer, 0, sizeof(buffer));
      CFStringGetBytes(key1, CFRangeMake(0, CFStringGetLength(key1)), kCFStringEncodingASCII, 0, 0, buffer, sizeof(buffer), &usedBufLen);
      printf("key1: %ul %s \n", __WebSocketGetMagicNumberWithKeyString(key1), buffer);
      
      memset(buffer, 0, sizeof(buffer));
      CFStringGetBytes(key2, CFRangeMake(0, CFStringGetLength(key2)), kCFStringEncodingASCII, 0, 0, buffer, sizeof(buffer), &usedBufLen);
      printf("key2: %ul %s\n", __WebSocketGetMagicNumberWithKeyString(key2), buffer);
      
      printf("key3: ");
      for (CFIndex i = 0; i < CFDataGetLength(key3); i++)
        printf("%02x ", (int)*(CFDataGetBytePtr(key3) + i));
      printf("\n");
      printf("data: ");
      for (CFIndex i = 0; i < CFDataGetLength(data); i++)
        printf("%02x ", (int)*(CFDataGetBytePtr(data) + i));
      printf("\n");
      
      CFRelease(data);
    }
    
    // Get serialized data and send it back
    {
      CFDataRef data = CFHTTPMessageCopySerializedMessage(response);
      
//      CFStringRef string = CFStringCreateWithBytes(NULL, CFDataGetBytePtr(data), CFDataGetLength(data), kCFStringEncodingASCII, 0);
//      if (string) {
//        printf("--->\n");
//        CFShow(string);
//        printf("<---\n");
//        CFRelease(string);
//      } else {
////        printf("string error %i", CFDataGetLength(data));
//      }
      
      // Wait for the stream to be ready
      //        while (!CFWriteStreamCanAcceptBytes(client->write)) {
      //          printf(".");
      //          fflush(stdout);
      //          usleep(1000000);
      //        }
      
      
        CFIndex written = CFWriteStreamWrite(client->write, CFDataGetBytePtr(data), CFDataGetLength(data));
        
        if (written != CFDataGetLength(data)) {
          // TODO: write more
        }
      
      success = 1;
      client->didWriteHandShake = 1;
      
      CFRelease(data);
    }
    
    CFRelease(response);
  }
  
//  printf("leaving handshake with %i\n", success);
  
  return success;
}

void __WebSocketClientWriteCallBack(CFWriteStreamRef stream, CFStreamEventType eventType, void *info) {
  WebSocketClientRef client = info;
  switch (eventType) {
    
    case kCFStreamEventCanAcceptBytes:
      if (!client->didWriteHandShake) {
        __WebSocketClientWriteHandShake(client);
//        if (__WebSocketClientWriteHandShake(client))
//          printf("whs+\n");
//        else
//          printf("whs-\n");
      } else {
//        printf("w+\n");
      }
      break;
      
  }
}

#pragma mark Lifecycle

WebSocketClientRef WebSocketClientCreate(WebSocketRef webSocket, CFSocketNativeHandle handle) {
  WebSocketClientRef client = NULL;
  if (webSocket) {
    client = CFAllocatorAllocate(webSocket->allocator, sizeof(WebSocketClient), 0);
    if (client) {
      client->allocator = webSocket->allocator ? CFRetain(webSocket->allocator) : NULL;
      client->retainCount = 1;
      
      client->webSocket = WebSocketRetain(webSocket);
      client->handle = handle;
      
      client->read = NULL;
      client->write = NULL;
      
      client->context.version = 0;
      client->context.info = client;
      client->context.copyDescription = NULL;
      client->context.retain = NULL;
      client->context.release = NULL;
      
      client->handShakeRequestHTTPMessage = NULL;
      client->didReadHandShake = 0;
      client->didWriteHandShake = 0;
      
      CFStreamCreatePairWithSocket(client->allocator, handle, &client->read, &client->write);
      if (!client->read || !client->write) {
        close(handle);
        fprintf(stderr, "CFStreamCreatePairWithSocket() failed, %p, %p\n", read, write);
      } else {
//        printf("ok\n");
      }

      CFReadStreamSetClient(client->read, kCFStreamEventOpenCompleted | kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, __WebSocketClientReadCallBack, &client->context);
      CFWriteStreamSetClient(client->write, kCFStreamEventOpenCompleted | kCFStreamEventCanAcceptBytes | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, __WebSocketClientWriteCallBack, &client->context);
      

      CFReadStreamScheduleWithRunLoop(client->read, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
      CFWriteStreamScheduleWithRunLoop(client->write, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
      
      if (!CFReadStreamOpen(client->read)) {
        printf("couldn't open read stream\n");
      } else {
//        printf("opened read stream\n");
      }

      if (!CFWriteStreamOpen(client->write)) {
        printf("couldn't open write stream\n");
      } else {
        //        printf("opened write stream\n");
      }
    }
  }
  return client;
}

WebSocketClientRef WebSocketClientRetain(WebSocketClientRef client) {
  if (client)
    client->retainCount++;
  return client;
}

WebSocketClientRef WebSocketClientRelease(WebSocketClientRef client) {
  if (client) {
    printf("release\n");
    if (--client->retainCount == 0) {
      CFAllocatorRef allocator = client->allocator;
      
      if (client->read) {
        if (CFReadStreamGetStatus(client->read) != kCFStreamStatusClosed)
          CFReadStreamClose(client->read);
        CFRelease(client->read);
        client->read = NULL;
      }
      
      if (client->write) {
        if (CFWriteStreamGetStatus(client->write) != kCFStreamStatusClosed)
          CFWriteStreamClose(client->write);
        CFRelease(client->write);
        client->write = NULL;
      }
      
      CFAllocatorDeallocate(allocator, client);
      client = NULL;
      
      if (allocator)
        CFRelease(allocator);
    }
  }
  return client;
}

#pragma Handshake

// Return magic number for the key needed to generate handshake hash
uint32_t __WebSocketGetMagicNumberWithKeyString(CFStringRef string) {
  UInt8 buffer[__WebSocketMaxHeaderKeyLength];
  CFIndex usedBufferLength = 0;
  char numberBuffer[__WebSocketMaxHeaderKeyLength];
  memset(numberBuffer, 0, sizeof(numberBuffer));
  CFIndex usedNumberBufferLength = 0;
  CFStringGetBytes(string, CFRangeMake(0, CFStringGetLength(string)), kCFStringEncodingASCII, 0, 0, buffer, sizeof(buffer), &usedBufferLength);
  UInt32 number = 0;
  UInt32 spaces = 0;
  for (int i = 0; i < usedBufferLength; i++) {
    if (buffer[i] >= '0' && buffer[i] <= '9')
      numberBuffer[usedNumberBufferLength++] = buffer[i];
    if (buffer[i] == ' ')
      spaces++;
  }
  number = (UInt32)strtoul(numberBuffer, NULL, 10);
  return number / spaces;
}

// Appends big-endian uint32 magic number with key string to the mutable data
void __WebSocketDataAppendKey(CFMutableDataRef data, CFStringRef string) {
  uint32_t swapped = CFSwapInt32HostToBig(__WebSocketGetMagicNumberWithKeyString(string));
  CFDataAppendBytes(data, (const void *)&swapped, sizeof(swapped));
}

// Generates md5 data from two header keys and data
CFDataRef __WebSocketCreateMD5Data(CFAllocatorRef allocator, CFStringRef key1, CFStringRef key2, CFDataRef key3) {
#if (TARGET_OS_IPHONE)
  CC_MD5_CTX mdctx;
  CC_MD5_Init(&mdctx);
  unsigned char buffer[CC_MD5_DIGEST_LENGTH];
  CFMutableDataRef data = CFDataCreateMutable(allocator, 0);
  __WebSocketDataAppendKey(data, key1);
  __WebSocketDataAppendKey(data, key2);
  CFDataAppendBytes(data, CFDataGetBytePtr(key3), CFDataGetLength(key3));
  CC_MD5_Update(&mdctx, CFDataGetBytePtr(data), CFDataGetLength(data));
  CC_MD5_Final(buffer, &mdctx);
  CFRelease(data);
  return CFDataCreate(allocator, buffer, CC_MD5_DIGEST_LENGTH);
#else
  EVP_MD_CTX mdctx;
  unsigned char buffer[EVP_MAX_MD_SIZE];
  unsigned int length = 0;
  CFMutableDataRef data = CFDataCreateMutable(allocator, 0);
  EVP_DigestInit(&mdctx, EVP_md5());
  __WebSocketDataAppendKey(data, key1);
  __WebSocketDataAppendKey(data, key2);
  CFDataAppendBytes(data, CFDataGetBytePtr(key3), CFDataGetLength(key3));
  EVP_DigestUpdate(&mdctx, CFDataGetBytePtr(data), CFDataGetLength(data));
  EVP_DigestFinal(&mdctx, buffer, &length);
  CFRelease(data);
  return CFDataCreate(allocator, buffer, length);
#endif
}

bool WebSocketClientHandShake(WebSocketClientRef client) {
  bool success = 0;
  if (client) {
    
    UInt8 buffer[4096];
    CFIndex bytes = 0;
    
    client->handShakeRequestHTTPMessage = CFHTTPMessageCreateEmpty(NULL, 1);
    printf("created empty msg\n");
    while (CFReadStreamHasBytesAvailable(client->read)) {
      if ((bytes = CFReadStreamRead(client->read, buffer, sizeof(buffer))) > 0) {
        printf("adding bytes %ld from %p\n", bytes, client->read);
        CFHTTPMessageAppendBytes(client->handShakeRequestHTTPMessage, buffer, bytes);
      } else {
        if (bytes < 0) {
          CFErrorRef error = CFReadStreamCopyError(client->read);
          CFShow(error);
          CFRelease(error);
        }
//        printf("bytes is %ld, breaking from %p\n", bytes, client->read);
        break;
      }
    }
    
    success = 1;
    client->didReadHandShake = 1;
  }
  return success;
}

