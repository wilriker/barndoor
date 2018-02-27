Barndoor Control
=======================

This fork introduces new variants for curved rod barndoor trackers and possibly other enhancements.

To use either isosceles or curved rod barndoor trackers you have to uncomment the appropriate
	#include "<type>.h"
line in `barndoor.ino` and adjust variables at the top of the header file. The advantage of this
implementation for curved rod is that if you have not exactly met the distance requirements between
middle of the hinge and middle of the rod the code will compensate for this.

---
Original contents of README below

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
