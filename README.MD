# ROKU-SLING IR CHANNEL-NUMBER REMOTE for ARDUINO


After cutting the cord and signing up for Sling TV on Roku, I missed being able to get to a channel by entering its number on a remote. I disliked having to scroll to the desired channel, particularly since I had trouble seeing the channel icons on my small TV.

So I built this IR remote which lets me enter the Cox Cable channel numbers, which I know very well, to get to the channels I want to watch.  I put all the channels I might ever want to watch on Sling's "My Channels" list. That list remains unchanged unless I change it. The corresponding Cox channel numbers are entered in the sketch in the "Channel" array in exactly the same order. The firmware finds a match for the channel number entered on the keypad, and automatically scrolls to that channel based on its position in the array, which is the same as its position on the My Channels line.

## Key Entry Options

If already watching a Sling channel, entering a new channel number takes you there.

Entering a channel number followed by the "*" key, from any state, takes you to Roku Home, then Sling, then My Channels, then to the entered channel number. This could be used to come out of Roku screen saver to play any channel.  Because of the time needed for Sling to boot up, this process takes about 18 seconds. Be sure to continue to point the remote toward the Roku until the channel starts playing.

Pressing "*" alone takes the Roku to Home, and toggles the TV's Power button.  This is useful for shutting off all streaming and turning off the TV.

Entering any valid channel number followed by the "#" key sets the "currentChan" variable to point to the entered channel number.  This can be used to get back in sync if the Arduino gets out of step with what's actually playing on the Roku.

Pressing "#" alone toggles the TV's input mode between antenna and HDMI.

## Hardware

The Arduino is a 3.3V 8MHz Pro Mini with it's voltage regulator and power LED removed. Between entries, it is put into Power_Down sleep mode where it draws 400nA of current. There is no ON/OFF switch.  Also needed are an IR LED (borrow one from any old remote, or order a Vishay TSAL6100 or TSAL6200), an 18650 lithium ion battery with holder and separate charger, and any cheap numeric 4x3 matrix keypad.

The current-limiting resistor for the IR LED will depend on your distance from the Roku and TV, and the characteristics of the IR LED you use.  The resistor should be the highest value that still gives good results.  The picture shows a 22-ohm resistor, but I ended up with 10 ohms as shown in the schematic.

The IRremote library used to generate the IR commands requires the use of digital pin 3 to drive the IR LED. This should drive the base of an NPN Darlington transistor which in turn drives the IR LED.  See the schematic.  You could also use a regular NPN transistor, but the base resistor would need to be a lower value.

The IR codes for the TV are those for Sharp TVs.  Other TV brands will require the use of their IR codes.  The IRremote library may be helpful in detecting what those codes are.
