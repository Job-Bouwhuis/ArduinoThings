#include <Arduino.h>
#include "Components/LedController.h"
#include "Components/Button.h"
#include "Components/LightSensor.h"
#include "Components/Buzzer.h"
#include "Util/List.hpp"
#include "Util/MemoryWatcher.hpp"
Components::LedController led(LED_BUILTIN);
Components::Button button(USER_BTN);
Components::LightSensor lightSens(A0);
Components::Buzzer buzzer(D3);

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
  comps.Add(&lightSens);
  comps.Add(&buzzer);
  button.SetEdge(Components::ButtonEdge::Rising);

  button.OnClick.Add(
      [](Components::Button *self)
      {
        const uint16_t freqs[] = {262, 294, 330, 0}; // C D E rest
        const uint16_t durs[] = {200, 200, 200, 100};

        buzzer.PlayTrack(freqs, durs, 4);
      });

  lightSens.AddWatcher(LIGHT_EITHER, 100,
                       [](Components::LightSensor *self, int current)
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