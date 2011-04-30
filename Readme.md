# CoreWebSocket - Core Foundation based WebSocket Library for iOS/OSX

CoreWebSocket is an C language, Core Foundation based library for iOS and Mac OSX. It can be used from Objective-C.

WebSocket enables low latency, bi-directional, full-duplex communication channel over TCP with web browser. It works
with all modern web browsers including Safari, Chrome, Firefox and Opera. It works with iOS Safari as well.

## Installation

To use the library in your git project:

    git submodule add git://github.com/mirek/CoreWebSocket.git CoreWebSocket
    
Link your target with:

* `CFNetwork.framework` - on iOS
* `libcrypto.dylib`, `CoreServices.framework` - on Mac OSX

## Usage

    #include "CoreWebSocket/CoreWebSocket.h"

    // Somewhere in init
    CFAllocatorRef allocator = NULL;
    WebSocketRef webSocket = CoreWebSocketCreate(allocator, CFSTR("localhost"), 60001);

    // Send data to all connected clients
    WebSocketWriteWithString(webSocket, CFSTR("foo, bar"));
    
    // To receive data from connected clients, declare the callback of type:
    // typedef void (*WebSocketDidClientReadCallback) (WebSocketRef webSocket, WebSocketClientRef client, CFStringRef value);
    
    void MyWebSocketDidClientReadCallback(WebSocketRef webSocket, WebSocketClientRef client, CFStringRef value) {
      printf("my read callback, the client sent:\n");
      CFShow(value);
    }
    
    // ...and set the callback:
    WebSocketSetClientReadCallback(webSocket, MyWebSocketDidClientReadCallback);

    // Somewhere in dealloc
    WebSocketRelease(webSocket);

## Web Browser Usage

    <!DOCTYPE html>
    <html>
      <head>
        <meta http-equiv="Content-type" content="text/html; charset=utf-8">
        <script type="text/javascript" charset="utf-8">
  
          // WebSocket
          var ws = null;
  
          // When the page is loaded...
          window.addEventListener('load', function(e) {
    
            // Check if the browser supports WebSockets...
            if ("WebSocket" in window) {
      
              // ...it does, let's connect to localhost default port.
              // Make sure Quartz Composer composition is running with
              // the WebSocket Patch set to port 60001. 
              ws = new WebSocket('ws://localhost:60001');
        
              // Invoked when there was an error with the connection. 
              ws.onerror = function(e) {
                console.log('error', e);
              }
        
              // Invoked when the socket has been opened successfully.
              ws.onopen = function(e) {
                console.log('open', e);
              }
        
              // Callback invoked when incoming messages arrive. Event `data` attribute
              // holds the string passed. WebSocket in current spec supports utf8 text-based
              // communication only. Binary data  is base64 encoded.
              ws.onmessage = function(e) {
                var json = JSON.parse(e.data);
                console.log('message', json);
              }
        
              // Invoked when the socket has been closed
              ws.onclose = function(e) {
                console.log('close', e);
              }
        
            } else {
      
              // ...seems like the web browser doesn't support WebSockets.
              alert('WebSocket not supported by your browser, use Safari, Chrome or Firefox');
        
            }
          }, false);
    
        </script>
      </head>
    </html>
