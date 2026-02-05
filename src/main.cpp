#include <Arduino.h>
#include "Components/Led.h"
#include "Components/Button.h"
#include "Components/LightSensor.h"
#include "Components/Buzzer.h"
#include "Util/List.hpp"
#include "Util/MemoryWatcher.hpp"
#include "Huiswerk/Week1/solution.h"

// Components::Led led(LED_BUILTIN);
// Components::Button button(USER_BTN);
// Components::LightSensor lightSens(A0);
// Components::Buzzer buzzer(D3);

// Util::MemoryWatcher memWatcher;

// Util::List<Component *, 1, 4> comps;

Huiswerk::Week1::Opdracht3::Solution sol;

void setup()
{
  Serial.begin(115200);

  // led.On();

  // comps.Add(&led);
  // comps.Add(&button);
  // comps.Add(&lightSens);
  // comps.Add(&buzzer);
  // buzzer.SetVolume(128);
  // button.SetEdge(Components::ButtonEdge::Rising);

  // button.OnClick.Add(
  //     [](Components::Button *self)
  //     {
  //       led.Toggle();
  //     });

  // lightSens.AddWatcher(LIGHT_EITHER, 800,
  //                      [](Components::LightSensor *self, int current)
  //                      {
  //                        led.Toggle();
  //                      });
}

void loop()
{
  sol.Tick();

  // for (auto comp : comps)
  // {
  //   comp->Tick();
  // }
}