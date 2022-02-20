# on-air

I wanted an "On Air" sign that would show up behind me while i'm in a video call at work.

Since I work from home in the living room, this is a useful indicator to my wife that i'm in a meeting (even if i'm muted and just listening). It's also something fun that shows up behind me while i'm in a call and gets many nice compliments!

# Goals for the project
 * "On Air" status must be triggered automatically
 * Learn some basic woodworking skills
 * Make the finished product look minimal and stylish
 * Learn how to use an RTC (Real-time clock) module
 * Include multiple modes so the project can be interacted with

## Materials required
Electronics:
* NodeMCU v3 ESP8266 microcontroller board
* 8x32 LED matrix
* RTC module
* Cables
* Breadboard
* A long USB cable

Case:
* 5mm darkened perspex / acrylic
* Nuts and bolts
* 7mm MDF board (sourced from leftover IKEA shelving)

Tools:
* Jigsaw
* Jigsaw table
* Drill

## Design, build and test

stuff to mention
* design that inspired me
* put in an API section with swagger and link to it
* exploding clock battery problem
* dropping several features like environment monitoring, voltage monitoring

## Zoom integration
Originally I wanted to use Zoom's webhook API to send messages to a gateway API, and then pass these on to the on-air sign. However, it turned out to be easier to use the fantastic [pyzoomproc](https://github.com/darrenpmeyer/pyzoomproc) utility, which runs on my work laptop and triggers [curl](https://github.com/curl/curl) to hit the appropriate endpoints on my sign.

## Remote control webapp
I based the design for the remote control application on the one I made for my [traffic light](https://github.com/skhg/traffic-light-controller/). It's pure JS/CSS with the only external dependency on [FontAwesome](https://fontawesome.com/) for icons.

**todo insert a screenshot of the webapp**

It's hosted on GitHub pages so all the on-air sign has to do is proxy the initial request to load `index.html`

Updates to the sign state are shared via Websocket to all connected clients. Requests to change state are provided by the REST endpoints mentioned above.

## Next steps and improvements
Some feature requests i've heard already:
* Customisable, scrolling text
* A falling snow animation

## References and helpful tools
https://www.pixilart.com/draw#