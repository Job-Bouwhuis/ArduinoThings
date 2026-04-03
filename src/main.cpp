#include "Util/Includes.h"
#include "Util/CharacterCreator.hpp"
#include "States/MainMenuState.hpp"
#include "States/Game1.hpp"
#include "States/Game2.hpp"
#include "States/Game3.hpp"
#include "States/Game4.hpp"
#include "States/GameOverState.hpp"
#include "States/SessionResult.hpp"

Components::Button *userButton = new Components::Button(USER_BTN);
Components::WADA *wada = new Components::WADA(D3, D4, D5);
Components::Led *ledBuiltin = new Components::Led(LED_BUILTIN);
Components::Lcd *lcd = new Components::Lcd(0x27, 16, 2);
Components::Potentiometer *pot = new Components::Potentiometer(A0);
Components::Buzzer *buzzer = new Components::Buzzer(D9);

Components::Button *wadaButton1 = wada->GetButton(0);
Components::Button *wadaButton2 = wada->GetButton(1);
Components::Button *wadaButton3 = wada->GetButton(2);
Components::Button *wadaButton4 = wada->GetButton(3);
Components::Button *wadaButton5 = wada->GetButton(4);
Components::Button *wadaButton6 = wada->GetButton(5);
Components::Button *wadaButton7 = wada->GetButton(6);
Components::Button *wadaButton8 = wada->GetButton(7);

Tasks::TaskManager tasks;
States::StateMachine stateMachine;
unsigned long sessionStartMs = 0;
bool timeUpHandled = false;

void setup()
{
  Serial.begin(9600);
  lcd->Init();
  lcd->Clear();
  lcd->Backlight(true);

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

  States::ResetSessionWins();
  sessionStartMs = millis();
  timeUpHandled = false;

  States::MainMenu *mainmenu = new States::MainMenu(&stateMachine);
  stateMachine.RegisterState(mainmenu);

  Games::Game1 *game1 = new Games::Game1(&stateMachine);
  stateMachine.RegisterState(game1);
  game1->AddAllowedTransition(game1);

  Games::Game2 *game2 = new Games::Game2(&stateMachine);
  stateMachine.RegisterState(game2);
  game2->AddAllowedTransition(game2);

  Games::Game3 *game3 = new Games::Game3(&stateMachine);
  stateMachine.RegisterState(game3);
  game3->AddAllowedTransition(game3);

  Games::Game4 *game4 = new Games::Game4(&stateMachine);
  stateMachine.RegisterState(game4);
  game4->AddAllowedTransition(game4);

  States::GameOverState *gameOver = new States::GameOverState(&stateMachine);
  stateMachine.RegisterState(gameOver);

  mainmenu->AddAllowedTransition(game1);
  mainmenu->AddAllowedTransition(game2);
  mainmenu->AddAllowedTransition(game3);
  mainmenu->AddAllowedTransition(game4);
  mainmenu->AddAllowedTransition(gameOver);

  game1->AddAllowedTransition(mainmenu);
  game2->AddAllowedTransition(mainmenu);
  game3->AddAllowedTransition(mainmenu);
  game4->AddAllowedTransition(mainmenu);

  game1->AddAllowedTransition(gameOver);
  game2->AddAllowedTransition(gameOver);
  game3->AddAllowedTransition(gameOver);
  game4->AddAllowedTransition(gameOver);

  gameOver->AddAllowedTransition(mainmenu);
  gameOver->AddAllowedTransition(game1);
  gameOver->AddAllowedTransition(game2);
  gameOver->AddAllowedTransition(game3);
  gameOver->AddAllowedTransition(game4);

  stateMachine.SetInitialState("mainmenu");
}

void loop()
{
  buzzer->Tick();
  userButton->Tick();
  wada->Tick();
  tasks.Tick();
  stateMachine.Tick();

  if (timeUpHandled)
    return;

  if (millis() - sessionStartMs > 600000UL)
  {
    timeUpHandled = true;

    String restartState = "mainmenu";
    States::State *currentState = stateMachine.GetCurrentState();
    if (currentState)
      restartState = currentState->GetName();

    if (restartState != "game1" && restartState != "game2" && restartState != "game3" && restartState != "game4")
      restartState = "mainmenu";

    States::PrepareTimeUpResult(restartState);
    stateMachine.RequestTransition("gameover");
  }
}
