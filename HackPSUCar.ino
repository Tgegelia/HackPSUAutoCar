/* Include the sonar sensor library */
#include <NewPing.h>

/* Include Servo library. */
#include <Servo.h>

// Define Sonar Params
#define SONAR_NUM      3  // Number of sensors.
#define MAX_DISTANCE 200  // Maximum distance (in cm) to ping.
#define PING_INTERVAL 50  // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo)

void stopPing();
void echoCheck();

// Create two servo objects
Servo ESCServo;
Servo StrServo;

// Define sonar variables
unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
boolean stopped = 0;
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.
int x = 0;

// Define sonar object array
NewPing sonar[SONAR_NUM] = {   // Sensor object array.
  //NewPing(2, 3, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  //NewPing(4, 5, MAX_DISTANCE),
  NewPing(6, 7, MAX_DISTANCE),
  NewPing(12, 11, MAX_DISTANCE),
  NewPing(8,13, MAX_DISTANCE)
};

// Create the initial servo commands
int SpeedCmd = 0; // 0% throttle
int StrCmd = 0;   // 0 deg steering angle

void setup()
{
 // Attach servo objects to output pins
  ESCServo.attach(9);     // ESC Servo attached
  StrServo.attach(10);    // Steering Servo attached
  Serial.begin(9600);     // Open serial data channel
  Serial.setTimeout(100); // Set a short timeout for receiving signals
  for (uint8_t i = 1; i < SONAR_NUM; i++)
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
}

void loop()
{
  if(Serial.available() > 0)                         // Wait for data to be received
   {
    // initialize temporary values (for no serial data)
    int SpeedTmp = 0;                                  // Initial speed = 0
    int StrTmp = 0;                                    // Initial str = 0
    Serial.println("bad");
    stopPing();
    int BluSpd = 0;                                // Throttle = 0%
    int BluStr = 0;                                // Steering angle = 0 deg
    char input[16];                                // Set maxmimum size of serial stream (16 chars)

    // Start of parsing code
    byte sze = Serial.readBytes(input, 15);        // Read the incoming data & store
    input[sze] = 0;                                // Insert EOS with null char
    
    /* Inputs are defined to be inthe form input,value;input,value;etc...
     * input = 1 -> Throttle
     * input = 2 -> Steering
     */
    char* frame = strtok(input, ";");              // Seperate strings by ; symbol
    while (frame!=0){                              // For each ; symbol, process each half
      char* sep = strchr(frame, ',');              // Seperate those substrings by , symbol
      if (sep!=0){                                 // Make sure the string isn't null
        *sep = 0;                                  // Set the , character to null (effectively seperating the two strings)
        int selector = atoi(frame);                // Process first half into int
        ++sep;                                     // Adress the second half of the string
        int value = atoi(sep);                     // Process second half
        
        // Set the appropriate commands for steering and speed
        if (selector == 1){
          BluSpd = value;
        }
        if (selector == 2){
          BluStr = value;
        }
        else{
          // invalid input
        }
      }
      frame = strtok(0, "&");                      // Keep parsing
    }

    // Constrain the speed to limit students. Set to 100% for testing
    SpeedTmp = constrain(BluSpd, -100, 100);
    // Constrain steering to our predefined 'stops'
    StrTmp = constrain(BluStr, -35, 35);
    /* For servo inputs, 90 is dead center
     * Remap speed from so that 0-100 corresponds to 90-180 (0-90 is reverse)
     * Remap steering to be between 145 and 35 (these are the steering stops, determined experimentally)
    */
    StrCmd = map(StrTmp, -35, 35, 145, 35);
    SpeedCmd = map(SpeedTmp, -100,100,0,180);

    // Send the positions to the servo
    ESCServo.write(SpeedCmd);
    if(BluStr == 0){
      StrServo.writeMicroseconds(1500);
    }
    else{
      StrServo.attach(10);
      StrServo.write(StrCmd);
    }
   }
   else{
    /*if(stopped){                                                        // If stopped, then start the measurement
      pingTimer[0] = millis();                                          // First timer is at millis
      for (uint8_t i = 0; i < SONAR_NUM; i++)                           // Run through sensor array
        if (i!=0) pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;      // Update Ping Timers
    }*/
    if(x<SONAR_NUM){
      if (millis() >= pingTimer[x]) {                                   // Is it this sensor's time to ping?
        pingTimer[x] += PING_INTERVAL * SONAR_NUM;                      // Set next time this sensor will be pinged.
        if (x == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
        sonar[currentSensor].timer_stop();                              // Make sure previous timer is canceled before starting a new ping (insurance).
        currentSensor = x;                                              // Sensor being accessed.
        cm[currentSensor] = 0;                                          // Make distance zero in case there's no ping echo for this sensor.
        sonar[currentSensor].ping_timer(echoCheck);                     // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
      }
      x = x+1;
    }
    if (x==SONAR_NUM){
      x=0;
    }
   }
}

void stopPing(){
  for (int i = 0; i < SONAR_NUM; i++) pingTimer[i] = -1;
  stopped = 1;
}

void echoCheck() {
  if (sonar[currentSensor].check_timer()){
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
  }
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  // The following code would be replaced with your code that does something with the ping results.
  for (int t = 0; t < SONAR_NUM; t++) {
    Serial.print(t);
    Serial.print("=");
    Serial.print(cm[t]);
    Serial.print("cm");
  }
  Serial.println();
}
