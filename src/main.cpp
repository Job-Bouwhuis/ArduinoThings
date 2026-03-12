#include <Arduino.h>
#include "Components/Led.h"
#include "Components/WADA.hpp"
#include "Components/lcd.hpp"
#include "Components/Potentiometer.hpp"
#include "Tasks/TaskManager.hpp"
#include "Tasks/PrintTask.hpp"
#include "Util/CharacterCreator.hpp"
#include "Util/StateMachine.hpp"
#include "Components/Buzzer.hpp"

// catch sticks horizontal (one stick per caracter line)
// dino game 2 rows
// binary converter
// number scroller

auto *b = new Components::Button(USER_BTN);
auto *wada = new Components::WADA(D3, D4, D5);
auto *led = new Components::Led(LED_BUILTIN);
auto *lcd = new Components::Lcd(0x27, 16, 2);
auto *pot = new Components::Potentiometer(A0);
auto * buzzer = new Components::Buzzer(D9);

Components::Button *wadaButton1;
Components::Button *wadaButton2;
Components::Button *wadaButton3;
Components::Button *wadaButton4;
Components::Button *wadaButton5;
Components::Button *wadaButton6;
Components::Button *wadaButton7;
Components::Button *wadaButton8;

Tasks::TaskManager tasks;
States::StateMachine stateMachine;

void setup()
{
  Serial.begin(9600);
  lcd->Init();
  lcd->Clear();
  lcd->Backlight(false);

  wadaButton1 = wada->GetButton(0);
  wadaButton1 = wada->GetButton(1);
  wadaButton1 = wada->GetButton(2);
  wadaButton1 = wada->GetButton(3);
  wadaButton1 = wada->GetButton(4);
  wadaButton1 = wada->GetButton(5);
  wadaButton1 = wada->GetButton(6);
  wadaButton1 = wada->GetButton(7);

  CharacterCreator c;
  c.Create(lcd);

  //buzzer->PlayTone(1200, 1000);
}

void loop()
{
  tasks.Tick();

  b->Tick();
  wada->Tick();
}
