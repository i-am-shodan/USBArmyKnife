# Example - VNC / Screen sharing

The USB Army Knife agent includes a tiny VNC server, this can only be accessed over the serial connection. The web interface includes a JavaScript client which you can use to view the screen of the victim box.

## Set up
1. Install the serial agent, see other examples for help

## Usage
1. Plug in device
2. Navigate to the web interface and click VNC

## Notes
1. There is an ongoing issue with the webserver that the USB Army Knife uses and the sheer number of files noVNC downloads. If you don't see 'Connected' in the VNC Window you might need to refresh.
2. It takes 10s to deliver a screenshot to the device once you see 'Connected' in your browser