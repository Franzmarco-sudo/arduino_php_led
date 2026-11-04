/*
  PHP -> USB Serial -> Arduino LED Controller

  Commands:
    1 = LED ON
    0 = LED OFF

  Built-in LED: Arduino Uno pin 13.
*/

const int LED_PIN = 13;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  delay(500);

  Serial.println("READY");
}

void loop()
{
  if (Serial.available() > 0)
  {
    char command = Serial.read();

    if (command == '1')
    {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    }
    else if (command == '0')
    {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    }
  }
}
