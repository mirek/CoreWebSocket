Pod::Spec.new do |s|
  s.name         = "CoreWebSocket"
  s.version      = "0.1"
  s.summary      = "CoreWebSocket is an C language, Core Foundation based library for iOS and Mac OSX."

  s.description  = <<-DESC
                   WebSocket enables low latency, bi-directional, full-duplex communication channel over TCP with web browser. It works with all modern web browsers including Safari, Chrome, Firefox and Opera. It works with iOS Safari as well.

                   DESC

  s.homepage     = "https://github.com/mirek/CoreWebSocket"
  s.license      = { :type => 'MIT', :file => 'LICENSE' }
  s.author       = { "Mirek Rusin" => "mirek@me.com" }
  s.ios.deployment_target = '5.0'
  s.osx.deployment_target = '10.7'
  s.source       = { :git => "https://github.com/mirek/CoreWebSocket.git", :tag => 0.1 }
  s.source_files  = 'CoreWebSocket/*.{h,c}'

  s.public_header_files = 'CoreWebSocket/*.h'
  s.framework  = 'CoreServices', 'CFNetwork'

  s.library   = 'crypto'
end
