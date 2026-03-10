#include <Arduino.h>
#include "Components/Led.h"
#include "Components/Button.h"
#include "Components/LightSensor.h"
#include "Components/Buzzer.h"
#include "Util/MemoryWatcher.hpp"
#include "Huiswerk/Week1/solution.h"
#include "Util/StateMachine.hpp"
#include "Components/WADA.hpp"
#include <list>

auto *b = new Components::Button(USER_BTN);
auto *wada = new Components::WADA(D3, D4, D5);
auto *led = new Components::Led(LED_BUILTIN);

std::list<Component *> comps;

void panic(const char *reason)
{
  while (true)
  {
    Serial.println(reason);
  }
}

void setup()
{
  Serial.begin(115200);

  try
  {
    comps.push_back(b);
    comps.push_back(wada);
    b->OnClick.Add([](Components::Button *btn)
                      { 
                          wada->Write("cool");
                          led->Toggle();
                      });
  }
  catch (const std::exception &e)
  {
    panic(e.what());
  }
}

void loop()
{
  try
  {
    for (auto comp : comps)
      comp->Tick();


    for(int i = 0; i < 8; i++)
    {
      Components::Button* b = wada->GetButton(i);
      if (b->IsPressed())
        wada->SetLed(i, true);
      if(b->IsReleased())
        wada->SetLed(i, false);
    }
  }
  catch (const std::exception &e)
  {
    panic(e.what());
  }
}
