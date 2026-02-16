#include <Arduino.h>
#include "Components/Led.h"
#include "Components/Button.h"
#include "Components/LightSensor.h"
#include "Components/Buzzer.h"
#include "Util/List.hpp"
#include "Util/MemoryWatcher.hpp"
Components::Led led(D7);
Components::Button button(USER_BTN);

Util::MemoryWatcher memWatcher;

Util::List<Component *> comps;

void setup()
{
  Serial.begin(115200);
  led.On();
  // delay(5000); // otherwise serial doesnt do a thing
  led.Off();

  comps.Add(&led);
  comps.Add(&button);
  button.SetEdge(Components::ButtonEdge::Rising);

  button.OnClick.Add(
      [](Components::Button *self)
      {
        led.Toggle();
      });
}

void loop()
{
  for (auto comp : comps)
  {
    comp->Tick();
  }
}