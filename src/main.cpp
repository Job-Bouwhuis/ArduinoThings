#include <Arduino.h>
#include "Components/Led.h"
#include "Components/WADA.hpp"
#include "Components/lcd.hpp"
#include "Components/Potentiometer.hpp"
#include "Tasks/TaskManager.hpp"
#include "Tasks/PrintTask.hpp"
#include "Util/CharacterCreator.hpp"

auto *b = new Components::Button(USER_BTN);
auto *wada = new Components::WADA(D3, D4, D5);
auto *led = new Components::Led(LED_BUILTIN);
auto *lcd = new Components::Lcd(0x27, 16, 2);
auto *pot = new Components::Potentiometer(A0);

Tasks::TaskManager tasks;

void setup()
{
  Serial.begin(9600);
  lcd->Init();

  CharacterCreator c;
  c.Create(lcd);

  Tasks::PrintTask *printer = new Tasks::PrintTask(lcd);
  tasks.AddTask(printer);

  b->OnClick.Add([](Components::Button *btn)
                 {
                   wada->Write("cool");
                   led->Toggle(); });

  lcd->Write("\n<tree><heartfilled><ghost><heart><sword><flame1><flame2><flame3>");
}

void loop()
{
  tasks.Tick();

  b->Tick();
  wada->Tick();

  for (int i = 0; i < 8; i++)
  {
    Components::Button *but = wada->GetButton(i);
    if (but->IsPressed())
      wada->SetLed(i, true);
    if (but->IsReleased())
      wada->SetLed(i, false);
  }

  Serial.println("yes");
}