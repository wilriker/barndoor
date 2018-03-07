Barndoor Control
=======================

# Motivation of this Fork
This fork introduces new variants for curved rod barndoor trackers and possibly other enhancements.

To use either isosceles or curved rod barndoor trackers you have to uncomment the appropriate

    #include "<type>.h"

line in `barndoor.ino` and adjust variables at the top of the header file. The advantage of this
implementation for curved rod is that if you have not exactly met the distance requirements between
middle of the hinge and middle of the rod the code will compensate for this.

# Wiring Schematics
The project has been changed from Arduino Uno + EasyDriver to Arduino Nano v3 + [Pololu MP6500](https://www.pololu.com/product/2968)
with digital current limiting. As recommended on Pololu's description page also a 100µF electrolytic capacitor has been
added to the circuit. On the two images below there is the wiring
![Wiring Schematics](/images/Barndoor_Tracker_schem.png)
and a nicer view
![Wiring Breadboard](/images/Barndoor_Tracker_bb.png)

# Improvements
The wiring and code has been simplified by using digital pins where possible. This reduces checks in code to
    if (digitalRead(pin) == HIGH|LOW) { ... }
instead of the less intuitive approach of reading analogue values and comparing them to being lower than 512 when
there are only digital states involved.

Using the Arduino Nano combined with Pololu MP6500 (or any other driver of the same dimensions) has the advantage
of being able to directly attach the driver to the Arduino by soldering male pin headers to Arduino and female pin
headers to the driver (or vice-versa).

Original contents of README below
---

This project contains an arduino program suitable for controlling an
astrophotography barndoor mount. The circuit being controlled is
described in the blog post:

  http://fstop138.berrange.com/2014/01/building-an-barn-door-mount-part-1-arduino-stepper-motor-control/

The code design is based on the maths calculations documented at

  http://fstop138.berrange.com/2014/01/building-an-barn-door-mount-part-2-calculating-mount-movements/

The code is provided under the terms of the GPL version 3 or later.

The master GIT repository is

    http://gitlab.com/berrange/barndoor

-- End
