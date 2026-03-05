#include <Arduino.h>
#include "Components/Led.h"
#include "Components/Button.h"
#include "Components/LightSensor.h"
#include "Components/Buzzer.h"
#include "Util/List.hpp"
#include "Util/MemoryWatcher.hpp"
#include "Huiswerk/Week1/solution.h"
#include "Util/StateMachine.hpp"
#include "Components/WADA.hpp"
#include <list>

Components::WADA wada(D3, D4, D5);
Components::Led led(LED_BUILTIN);
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
    auto *b = new Components::Button(USER_BTN);
    comps.push_back(b);

    b->OnClick.Add([](Components::Button *btn)
                   { 
                        wada.Write("cool");
                        led.Toggle(); });
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
  }
  catch (const std::exception &e)
  {
    panic(e.what());
  }
}