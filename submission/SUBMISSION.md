## Tinkercad Circuit Link

```
https://www.tinkercad.com/things/kpWQ6bq5NlN/editel
```

## Program Explanation
```
replace me with a short explanation of how your program / circuit works and any assumptions or design decisions you made

- assumed that the delay could be any number of milliseconds between 2 and 10.

= Used analog output pin for the LED to make pulsing easier (instead of ON/OFF many times with digital output pin)

CIRCUIT:
- on left side of the breadboard is the input "module", when the button is pushed the input pin (digital pin 7) recieves the signal and acts based on the programs state
- on right side of the breadboard is a simple LED circuit hooked to GND and analog output pin 9.
- despite using an analog pin and having the reaction time trigger be brighter than the pulse,
  I was not consistently able to distinguish between armed pulsing and reaction time starting
  so I added a second resistor (on a digital pin) that only turns on once the reaction time starts

PROGRAM:
- Used a finite state machine (FSM) for the program. Using a FSM made it easy to transition between behaviours.
- interrupts are used to ensure that no matter where in the code my program is, pushing the button leads to an event happening on the side (essentially a second thread watching the button). To get this working properly meant that I could not use delay() in input-dependent functions, since delay paused the CPU and stopped the button input from being registered

```

## THINGS I HAD TO RESEARCH
[17/03]
- had to look into how to wire up a push button to the arduino (https://www.youtube.com/watch?v=GRyDJAJlxAQ)
- had to research the bounds of input to analogWrite function (https://docs.arduino.cc/language-reference/en/functions/analog-io/analogWrite/) 
- specific functions to open connection to serial monitor and turn on/off LED.

[17/03]
- bug came up where during the pulsing the delay meant that sometimes the button press wasn't registered. Fixing this meant coming up with a way to always be checking for button input on the side and not using any delay() calls in the pulse function. This led to me researching Interrupts (https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/). I also swapped out the delay() calls in pulse to instead use the millis() function (https://docs.arduino.cc/language-reference/en/functions/time/millis/). Finally, the map function (https://docs.arduino.cc/language-reference/en/functions/math/map/) was used with the modulo operator to take any general time since the pulse function was called, bound it to somewhere inside the pulse period, and then map it to a specific brightness (used a triangle wave for the pulse for simplicity)
- do not yet know why, but the circuit only behaves if a voltmeter is watching the input voltage (schrodingers current)
 