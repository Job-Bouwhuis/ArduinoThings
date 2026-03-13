#include "Util/Includes.h"
#include "Util/CharacterCreator.hpp"
#include "States/MainMenuState.hpp"
#include "States/Game1.hpp"

// catch sticks horizontal (one stick per caracter line)
// dino game 2 rows
// binary converter
// number scroller

Components::Button *userButton = new Components::Button(USER_BTN);
Components::WADA *wada = new Components::WADA(D3, D4, D5);
Components::Led *ledBuiltin = new Components::Led(LED_BUILTIN);
Components::Lcd *lcd = new Components::Lcd(0x27, 16, 2);
Components::Potentiometer *pot = new Components::Potentiometer(A0);
Components::Buzzer *buzzer = new Components::Buzzer(D9);

Components::Button *wadaButton1 = nullptr;
Components::Button *wadaButton2 = nullptr;
Components::Button *wadaButton3 = nullptr;
Components::Button *wadaButton4 = nullptr;
Components::Button *wadaButton5 = nullptr;
Components::Button *wadaButton6 = nullptr;
Components::Button *wadaButton7 = nullptr;
Components::Button *wadaButton8 = nullptr;

Tasks::TaskManager tasks;
States::StateMachine stateMachine;

void setup()
{
  Serial.begin(9600);
  lcd->Init();
  lcd->Clear();
  lcd->Backlight(false);

  wadaButton1 = wada->GetButton(0);
  wadaButton2 = wada->GetButton(1);
  wadaButton3 = wada->GetButton(2);
  wadaButton4 = wada->GetButton(3);
  wadaButton5 = wada->GetButton(4);
  wadaButton6 = wada->GetButton(5);
  wadaButton7 = wada->GetButton(6);
  wadaButton8 = wada->GetButton(7);

  CharacterCreator c;
  c.Create(lcd);

  const uint16_t startup_frequencies[] = {
      NOTE_C4, NOTE_E4, NOTE_G4,
      NOTE_C5, NOTE_B4, NOTE_G4, NOTE_E4,
      NOTE_G4, NOTE_C5, NOTE_E5,
      NOTE_G5, REST, NOTE_G5};

  const uint16_t startup_durations[] = {
      80, 80, 80,
      100, 80, 80, 80,
      100, 100, 100,
      250, 60, 180};

  buzzer->PlayMusicTrack(startup_frequencies, startup_durations, 10, false);

  States::MainMenu *mainmenu = new States::MainMenu(&stateMachine);
  stateMachine.RegisterState(mainmenu);

  Games::Game1 *game1 = new Games::Game1(&stateMachine);
  stateMachine.RegisterState(game1);

  mainmenu->AddAllowedTransition(game1);

  stateMachine.SetInitialState("mainmenu");
}

void loop()
{
  tasks.Tick();
  buzzer->Tick();
  userButton->Tick();
  wada->Tick();
}
