// This #include statement was automatically added by the Spark IDE.
#include "SparkTime.h"

int RELAY1 = D0;
int RELAY2 = D1;
int RELAY3 = D2;
int RELAY4 = D3;

int state_a = 0;
int state_b = 0;
int state_c = 0;

char now_as_string[256];
char config_as_string[256];

int requested_a_time  = 0;
int requested_a_state = 0;

int countdown = 0;

void setup()
{
   //Initilize the relay control pins as output
   pinMode(RELAY1, OUTPUT);
   pinMode(RELAY2, OUTPUT);
   pinMode(RELAY3, OUTPUT);
   pinMode(RELAY4, OUTPUT);
   // Initialize all relays to an OFF state
   digitalWrite(RELAY1, LOW);
   digitalWrite(RELAY2, LOW);
   digitalWrite(RELAY3, LOW);
   digitalWrite(RELAY4, LOW);

   //register the Spark function
   Spark.function("relay", relayControl);
   Spark.function("toggleLight", toggle_A_public);
   Spark.function("toggleRelayB", toggle_B_public);
   Spark.function("toggleRelayC", toggle_C_public);

    Spark.variable("light", &state_a, INT);
    Spark.variable("state_b", &state_b, INT);
    Spark.variable("state_c", &state_c, INT);
    Spark.variable("countdown", &countdown, INT);
    Spark.variable("now", &now_as_string, STRING);

    Time.zone(+2);
}

void loop()
{
    int hour   = Time.hour();
    int minute = Time.minute();
    int t = 60*hour + minute;

    String s = print_time(t);
    s.toCharArray(now_as_string, arraySize(now_as_string));

    int now = Time.now();

    if (requested_a_time + 300 < now) {
        // timeout
        requested_a_state = 0;
        countdown = 0;
    }
    else {
        countdown = requested_a_time + 300 - now;
    }

    int scheduled_state = 0;
    if (hour >= 8 && hour < 12) {
        scheduled_state = 1;
    }
    if (hour >= 16 && hour < 20) {
        scheduled_state = 1;
    }

    int calculated_state = (scheduled_state + requested_a_state > 0);

    // set state
    if (state_a != calculated_state) {
        toggle_A_internal();
    }
}

String print_time(int t) {
    int hour   = t / 60;
    int minute = t % 60;

    String s = String("");
    if (hour < 10)
    {
        s.concat("0");
    }
    s.concat(hour);
    s.concat(":");
    if (minute < 10)
    {
        s.concat("0");
    }
    s.concat(minute);
    return s;
}


int toggle_A_public(String not_used) {

    // initialize on first request
    if (requested_a_state == 0) {
        requested_a_state = (state_a) ? 1 : -1;
    }

    // toggle requested_a_state
    requested_a_state = -1 * requested_a_state;

    requested_a_time = Time.now();

    return requested_a_state;
}
int toggle_A_internal() {
    state_a = !state_a;
    Spark.publish("A", (state_a) ? "on" : "off");
    digitalWrite(RELAY4, (state_a) ? HIGH : LOW);
    return (state_a) ? 1 : 0;
}
int toggle_B_public(String not_used) {
    state_b = !state_b;
    Spark.publish("B", (state_b) ? "on" : "off");
    digitalWrite(RELAY3, (state_b) ? HIGH : LOW);
    return (state_b) ? 1 : 0;
}
int toggle_C_public(String not_used) {
    state_c = !state_c;
    Spark.publish("C", (state_c) ? "on" : "off");
    digitalWrite(RELAY2, (state_c) ? HIGH : LOW);
    return (state_c) ? 1 : 0;
}

// command format r1,HIGH
int relayControl(String command)
{
  int relayState = 0;
  // parse the relay number
  int relayNumber = command.charAt(1) - '0';
  // do a sanity check
  if (relayNumber < 1 || relayNumber > 4) return -1;

  // find out the state of the relay
  if (command.substring(3,7) == "HIGH") relayState = 1;
  else if (command.substring(3,6) == "LOW") relayState = 0;
  else return -1;

  // write to the appropriate relay
  digitalWrite(relayNumber-1, relayState);
  return 1;
}
