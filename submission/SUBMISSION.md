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

PROGRAM:
- Used a finite state machine (FSM) for the program. Using a FSM made it easy to transition between behaviours.
- loop checks for button pushes and acts depending on the state when the button was pushed
```

## THINGS I HAD TO RESEARCH
[17/03]
- had to look into how to wire up a push button to the arduino (https://www.youtube.com/watch?v=GRyDJAJlxAQ)
- had to research the bounds of input to analogWrite function (https://docs.arduino.cc/language-reference/en/functions/analog-io/analogWrite/) 
- specific functions to open connection to serial monitor and turn on/off LED.
