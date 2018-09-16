/* This example shows how to display a moving gradient pattern on
 * an APA102-based LED strip. */

/* By default, the APA102 library uses pinMode and digitalWrite
 * to write to the LEDs, which works on all Arduino-compatible
 * boards but might be slow.  If you have a board supported by
 * the FastGPIO library and want faster LED updates, then install
 * the FastGPIO library and uncomment the next two lines: */
// #include <FastGPIO.h>
// #define APA102_USE_FAST_GPIO

#include <APA102.h>

// Define which pins to use.
const uint8_t dataPin = 11;
const uint8_t clockPin = 12;
const uint8_t logicInput1 = 3;
const uint8_t logicInput2 = 4;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

enum {
  ledCount = 60,
  brightness = 10,
  stepsBetweenStates = 50, // Time between states is (steps * 10) milliseconds
};

// Create a buffer for holding the colors (3 bytes per color).
rgb_color colors[ledCount];

//-------------------------------------------------------------
typedef struct {
  rgb_color color;
} State_t;

State_t states[] = {
  {rgb_color(10,10,10)},
  {rgb_color(255,0,0)},
  {rgb_color(0,0,255)}
};

typedef struct{
  State_t prevState;
  State_t curState;
  State_t nextState;
  int nextStateNum;
  bool transitioning;
  unsigned int stepNum;
  float floatState[3];
  float stepValues[3];
} Instance_t;

Instance_t instance = {states[0], states[0], states[0], 0, false, 0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
//-------------------------------------------------------------

int ScanInputs(void)
{
  int result = 0;
  result |= digitalRead(logicInput1) << 0;
  result |= digitalRead(logicInput2) << 1;
  return result;
}

void CalculateSteps()
{
  instance.transitioning = true;
  instance.stepNum = 0;
  instance.floatState[0] = 0;
  instance.floatState[1] = 0;
  instance.floatState[2] = 0;

  int diffRed   = instance.nextState.color.red   - instance.prevState.color.red;
  int diffGreen = instance.nextState.color.green - instance.prevState.color.green;
  int diffBlue  = instance.nextState.color.blue  - instance.prevState.color.blue;

  instance.stepValues[0] = (float)diffRed   / (float)stepsBetweenStates;
  instance.stepValues[1] = (float)diffGreen / (float)stepsBetweenStates;
  instance.stepValues[2] = (float)diffBlue  / (float)stepsBetweenStates;
}

void Step()
{
  instance.stepNum++;
  if (instance.stepNum > stepsBetweenStates)
  {
    instance.transitioning = false;
    instance.curState.color.red =   instance.nextState[0];
    instance.curState.color.green = instance.nextState[1];
    instance.curState.color.blue =  instance.nextState[2];
    return;
  }
  
  Serial.print("STep Num: ");
  Serial.println(instance.stepNum);
  
  instance.floatState[0] += instance.stepValues[0];
  instance.floatState[1] += instance.stepValues[1];
  instance.floatState[2] += instance.stepValues[2];

  instance.curState.color.red =   (int)instance.floatState[0];
  instance.curState.color.green = (int)instance.floatState[1];
  instance.curState.color.blue =  (int)instance.floatState[2];
}

void UpdateState(int inputState)
{
  if (instance.nextStateNum == inputState)
  {
    if (instance.transitioning)
    {
      Step();
    }
  } else {
    if (instance.transitioning)
    {
      instance.prevState = instance.curState;
    } else { 
      instance.prevState = instance.nextState;
    }
    instance.nextState = states[inputState];
    instance.nextStateNum = inputState;
    CalculateSteps();
    Step();
  }
}

void UpdateStrip(void)
{
  for(int i = 0; i < ledCount; i++)
  {
    colors[i] = instance.curState.color;
  }
  ledStrip.write(colors, ledCount, brightness);
  Serial.print("Writing ");
  Serial.print(instance.curState.color.red);
  Serial.print(",");
  Serial.print(instance.curState.color.green);
  Serial.print(",");
  Serial.println(instance.curState.color.blue);
}

void setup()
{
  pinMode(logicInput1, INPUT);
  pinMode(logicInput2, INPUT);

  Serial.begin(9600);
}

void loop()
{
  int inputState = ScanInputs();

  UpdateState(inputState);

  Serial.print("Read ");
  Serial.print(inputState);
  Serial.println(" on input");
  
  UpdateStrip();

  Serial.print("Target ");
  Serial.print(instance.nextState.color.red);
  Serial.print(",");
  Serial.print(instance.nextState.color.green);
  Serial.print(",");
  Serial.println(instance.nextState.color.blue);
  Serial.println();
  
  delay(10);
}
