#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Serial test!");
}

int count = 0;

void loop()
{
  Serial.printf("count = %d\n", count++);
  delay(500);
}
