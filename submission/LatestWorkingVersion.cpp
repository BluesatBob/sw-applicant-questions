// C++ code
//
//====CONSTANTS====\\
//pin the LED (output) is linked to
#define PRIMARY_LED_PIN 9
#define SECONDARY_LED_PIN 8

//pin the button (input) is linked to
//must be an interrupt pin (pins 2 or 3)
#define BUTTON_PIN 2

//output for the analog PWN pin
//0 = 0V, 255  = 5V
#define MIN_ANALOG_OUTPUT 0
#define MAX_ANALOG_OUTPUT 255

//max brightness during pulsing of armed phase
//ensures we can distinguish between pusling and time starting
//however still have backup LED for those who can't distinguish
#define MAX_ARMED_OUTPUT 100
//the max and min time (in seconds) the armed phase pulse can be
#define MIN_ARM_TIME 2
#define MAX_ARM_TIME 10

#define MILLISECONDS_PER_SECOND 1000

//how many milliseconds a single pulse (off->on->off) should last (milliseconds)
#define ARMED_PULSE_PERIOD 1000
//how long pulsing for reaction and false start should last (ms)
#define REACTION_PULSE_DURATION 1000
#define FALSE_START_PULSE_DURATION 2000

//milliseconds for a slow reaction pulse 
#define SLOW_PULSE_PERIOD 1000

//milliseconds for a medium reaction pulse
#define MEDIUM_PULSE_PERIOD 500

//milliseconds for a fast reaction pulse
#define FAST_PULSE_PERIOD 250

//milliseconds for a rapid reaction pulse
#define RAPID_PULSE_PERIOD 200


//bounds for reaction time categories
#define FAST_REACTION_MAX 200
#define MEDIUM_REACTION_MAX 351


//====ENUMS====
//all possible states circuit can be in, according to spec
enum state {IDLE, ARMED, REACTION};

//====GLOBAL VARS====
//NOTE: volatile means the variable is used in the interrupt function

//total attempts
volatile int attempts;
volatile int failedAttempts;

//current circuit state
volatile state circuitState;

//flags for pulsing
volatile bool pulseStarted;
//only print user report after reaction
volatile bool reacted;
volatile bool falseStart;


//used to mark the start and end of the period to push the button
volatile unsigned long reactionTimeStart;
volatile unsigned long reactionTimeEnd;
//used for the user report
volatile unsigned long reactionTimeAverage;

void setup()
{
  circuitState = IDLE;
 //we have not started a pulse yet
  pulseStarted = false;
  reacted = false;
  //initially we have no attempts or failed attempts
  failedAttempts = 0;
  attempts = 0;
  //since no initial attempts, our average reaction time is 0
  reactionTimeAverage = 0;
  
  //https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/
  //this essentially tells the arduino that if the button is pushed, stop what you're doing
  //and run the handleButtonInput function
  //RISING keyword is for rising edge, when pin sees 0 -> 5V, i.e button pressed
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInput, RISING);
  
  //set button pin to input, led pin to act as output
  pinMode(BUTTON_PIN, INPUT);
  pinMode(PRIMARY_LED_PIN, OUTPUT);
  pinMode(SECONDARY_LED_PIN, OUTPUT);

  //open serial monitor connection for writing to terminal
  Serial.begin(9600);  
}

void loop()
{
  if (circuitState == ARMED && !pulseStarted) {
  	pulseStarted = true;
    //duration in ms
    int pulseDuration = random(MIN_ARM_TIME * MILLISECONDS_PER_SECOND, MAX_ARM_TIME * MILLISECONDS_PER_SECOND);
    armedPulse(pulseDuration);
  }
  
  if (reacted) {
    //we know that the reaction time will be a couple hundred - a thousand ms, so can be stored in an int
  	int reactionTime = int(reactionTimeEnd - reactionTimeStart);
    generateResultReport(reactionTime);
    
    int pulsePeriod = 0;
    if (reactionTime < FAST_REACTION_MAX) {
      //fast reaction
      pulsePeriod = FAST_PULSE_PERIOD;
    } else if (reactionTime < MEDIUM_REACTION_MAX) {
      //to slow for fast but under medium time, medium reaction
      pulsePeriod = MEDIUM_PULSE_PERIOD;
    }else {
      //SLOW REACTION
      pulsePeriod = SLOW_PULSE_PERIOD;
    }
    
    reactionPulse(pulsePeriod);
    
    reacted = false;
    //back to IDLE, turn off lights, reaction time registered
    analogWrite(PRIMARY_LED_PIN, MIN_ANALOG_OUTPUT); 
    digitalWrite(SECONDARY_LED_PIN, LOW);
    //circuitState = IDLE; happens in handleButtonInput
  }

  if (falseStart) {
    Serial.println("false start");
    falseStart = false;
    falseStartPulse();
    
    //turn off the LED (back to IDLE) after flash
    analogWrite(PRIMARY_LED_PIN, MIN_ANALOG_OUTPUT);
    digitalWrite(SECONDARY_LED_PIN, LOW);    
  }         
  
}

void falseStartPulse() {

  //use unsigned long as user may start button press after very long time
  unsigned long startTime = millis();
  unsigned long currTime = startTime;

  //how bright to make primary LED
  int brightness = 0;
  //elapsed time is curr - start
  while (currTime - startTime < FALSE_START_PULSE_DURATION) {
    //use modulo to find out what part of the period of the pulse we are in (first or second half
  	unsigned long timeInCycle = (currTime - startTime) % (RAPID_PULSE_PERIOD);  
    
    //use the provided map function to create a basic triangle wave for our analog pin output
    if (timeInCycle < RAPID_PULSE_PERIOD / 2) {
      //in first half of cycle - getting brighter
      brightness = map(timeInCycle, 0, RAPID_PULSE_PERIOD/2, MIN_ANALOG_OUTPUT, MAX_ANALOG_OUTPUT);
    } else {
      //in second half of cycle - getting smaller
      brightness = map(timeInCycle, RAPID_PULSE_PERIOD/2, RAPID_PULSE_PERIOD, MAX_ANALOG_OUTPUT, MIN_ANALOG_OUTPUT);
    }
    
   
    analogWrite(PRIMARY_LED_PIN, brightness);
    currTime = millis();
  }
  
  //false start pulse over, return
  return;
}

//duration and period in milliseconds
//map function documentation: https://docs.arduino.cc/language-reference/en/functions/math/map/
//millis function docs: https://docs.arduino.cc/language-reference/en/functions/time/millis/
void armedPulse(int duration) {
  //use unsigned long as user may start button press after very long time
  unsigned long startTime = millis();
  unsigned long currTime = startTime;

  //how bright to make primary LED
  int brightness = 0;
  //elapsed time is curr - start
  while (currTime - startTime < duration) {
    //use modulo to find out what part of the period of the pulse we are in (first or second half
  	unsigned long timeInCycle = (currTime - startTime) % (ARMED_PULSE_PERIOD);  
    
    //use the provided map function to create a basic triangle wave for our analog pin output
    if (timeInCycle < ARMED_PULSE_PERIOD / 2) {
      //in first half of cycle - getting brighter
      brightness = map(timeInCycle, 0, ARMED_PULSE_PERIOD/2, MIN_ANALOG_OUTPUT, MAX_ARMED_OUTPUT);
    } else {
      //in second half of cycle - getting smaller
      brightness = map(timeInCycle, ARMED_PULSE_PERIOD/2, ARMED_PULSE_PERIOD, MAX_ARMED_OUTPUT, MIN_ANALOG_OUTPUT);
    }
    
    if (circuitState == ARMED) {
      //no false starts since we calculated brightness OR this is a reactionary pulse
      analogWrite(PRIMARY_LED_PIN, brightness);
      
    } else {
      //false start occured
      //analogWrite(PRIMARY_LED_PIN, minBrightness);
      return;
      
    }
    
    currTime = millis();
  }

  //duration over, we are now ready to get reaction time
  circuitState = REACTION;
  reactionTimeStart = millis();
  //turn primary and secondary LED all the way on
  //primary is on analog pin, secondary is on digital pin,
  analogWrite(PRIMARY_LED_PIN, MAX_ANALOG_OUTPUT);
  digitalWrite(SECONDARY_LED_PIN, HIGH);
  return;
}

//duration and period in milliseconds
//map function documentation: https://docs.arduino.cc/language-reference/en/functions/math/map/
//millis function docs: https://docs.arduino.cc/language-reference/en/functions/time/millis/
void reactionPulse(int period) {
  //use unsigned long as user may start button press after very long time
  unsigned long startTime = millis();
  unsigned long currTime = startTime;

  //how bright to make primary LED
  int brightness = 0;
  //elapsed time is curr - start
  while (currTime - startTime < REACTION_PULSE_DURATION) {
    //use modulo to find out what part of the period of the pulse we are in (first or second half
  	unsigned long timeInCycle = (currTime - startTime) % (period);  
    
    //use the provided map function to create a basic triangle wave for our analog pin output
    if (timeInCycle < period / 2) {
      //in first half of cycle - getting brighter
      brightness = map(timeInCycle, 0, period/2, MIN_ANALOG_OUTPUT, MAX_ANALOG_OUTPUT);
    } else {
      //in second half of cycle - getting smaller
      brightness = map(timeInCycle, period/2, period, MAX_ANALOG_OUTPUT, MIN_ANALOG_OUTPUT);
    }
    
    analogWrite(PRIMARY_LED_PIN, brightness);
    currTime = millis();
  }
  
  return;
}


//as per docs for ISR's, must take no input and give no output
void handleButtonInput() {
  if (circuitState == REACTION) {
    //get time taken
  	reactionTimeEnd = millis(); 
    attempts++;
    reacted = true;
    circuitState = IDLE;

  } else if (circuitState == IDLE) {
    circuitState = ARMED;
    pulseStarted = false;
    //loop will handle the pulsing
  } else if (circuitState == ARMED) {
  	//a false start has occured
    circuitState = IDLE;
    failedAttempts++;
    attempts++;
    falseStart = true;
  }
  
  return;
}

//prints the output and feedback section to the serial monitor
//uses global variables for the reaction time
//no format specifiers for arduino, use multiple print statement MIPS style
void generateResultReport(int reactionTime) {
  Serial.print("Attempt: ");
  Serial.println(attempts);

  Serial.print("Reaction: ");
  Serial.print(reactionTime);
  Serial.println(" ms");
  //if the average of n numbers is A = (x1+x2+...+xn)/n
  //then the average of n+1 numbers x1+x2+...+xn+y = A' = (n*A + y)/(n+1)
  //this value accounts for the most recent attempt, i.e this is n+1
  int successfulAttempts = attempts - failedAttempts;
  //new average = (numAttemptsExcludingThisOne * oldAverage + newVal)/numAttemptsIncludingThisOne
  reactionTimeAverage = ((successfulAttempts - 1) * reactionTimeAverage + reactionTime) / successfulAttempts;
  
  
  Serial.print("Average: ");
  Serial.print(reactionTimeAverage);
  Serial.println(" ms");
 
  Serial.print("False Starts: ");
  Serial.println(failedAttempts);
  return;

}

//REFACTORING TESTS

//duration and period in milliseconds
//map function documentation: https://docs.arduino.cc/language-reference/en/functions/math/map/
//millis function docs: https://docs.arduino.cc/language-reference/en/functions/time/millis/
void pulse(int duration, int period, int minBrightness, int maxBrightness) {
  //use unsigned long as user may start button press after very long time
  unsigned long startTime = millis();
  unsigned long currTime = startTime;

  //how bright to make primary LED
  int brightness = 0;
  //elapsed time is curr - start
  while (currTime - startTime < duration) {
    //use modulo to find out what part of the period of the pulse we are in (first or second half
  	unsigned long timeInCycle = (currTime - startTime) % (period);  
    
    //use the provided map function to create a basic triangle wave for our analog pin output
    if (timeInCycle < period / 2) {
      //in first half of cycle - getting brighter
      brightness = map(timeInCycle, 0, period/2, minBrightness, maxBrightness);
    } else {
      //in second half of cycle - getting smaller
      brightness = map(timeInCycle, period/2, period, maxBrightness, minBrightness);
    }
    
    if (circuitState == ARMED || circuitState == REACTION) {
      Serial.println("update brightness");
      //no false starts since we calculated brightness OR this is a reactionary pulse
      analogWrite(PRIMARY_LED_PIN, brightness);
    } else {
      //false start occured
      //analogWrite(PRIMARY_LED_PIN, minBrightness);
      return;
    }
    
    currTime = millis();
  }
  
  if (circuitState == REACTION) {
  	//this function was called as a result of a reaction, so the pulsing that just occured
    //is feedback, so just return
    return;
  }
  //if here, this was an arming pulse
  
  //duration over, we are now ready to get reaction time
  circuitState = REACTION;
  reactionTimeStart = millis();
  //turn primary and secondary LED all the way on
  //primary is on analog pin, secondary is on digital pin,
  analogWrite(PRIMARY_LED_PIN, MAX_ANALOG_OUTPUT);
  digitalWrite(SECONDARY_LED_PIN, HIGH);
  return;
}


