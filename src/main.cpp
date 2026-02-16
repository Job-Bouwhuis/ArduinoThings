#include <Arduino.h>
#include "Components/Led.h"
#include "Components/Button.h"
#include "Components/LightSensor.h"
#include "Components/Buzzer.h"
#include "Util/List.hpp"
#include "Util/MemoryWatcher.hpp"
#include "Huiswerk/Week1/solution.h"
#include "Util/StateMachine.hpp"

Components::Button button(USER_BTN);

Util::MemoryWatcher memWatcher;

Util::List<Component *> comps;

Components::Led carRedLed(D7);
Components::Led carYellowLed(D6);
Components::Led carGreenLed(D5);

Components::Led pedRedLed(D4);
Components::Led pedGreenLed(D3);

StateMachine carSM;
StateMachine pedSM;

static bool carRedFired = false;

class CarGreen : public State
{
public:
  void Tick(float dt) override {}
};

class CarYellow : public TimedState
{
public:
  CarYellow() : TimedState(2.0f) {}

  void Tick(float dt) override { TimedState::Tick(dt); }
};

class CarRed : public State
{
public:
  void Tick(float dt) override {}
};

class PedRed : public TimedState
{
public:
  PedRed() : TimedState(1.0f) {}
  void Tick(float dt) override { TimedState::Tick(dt); }
};

class PedGreen : public TimedState
{
  int blinkCount = 0;
  bool ledOn = true;
  const float blinkInterval = 0.25f;

public:
  PedGreen() : TimedState(10.0f) {}

  void Tick(float dt) override
  {
    timer += dt;
    if (timer >= duration)
    {
      float blinkTime = timer - duration;
      if (blinkTime >= blinkInterval * blinkCount)
      {
        ledOn = !ledOn;
        if (ledOn)
          pedGreenLed.On();
        else
          pedGreenLed.Off();
        blinkCount++;
        if (blinkCount > 12)
        {
          ledOn = false;
          pedGreenLed.Off();
        }
      }
    }
  }

  bool IsBlinkDone() const { return blinkCount > 12; }
};

void setup()
{
  Console.WriteLine("Setup...");
  delay(5000);
  Console.WriteLine("Complete");

  // --- Instantiate states ---
  auto carGreen = std::make_shared<CarGreen>();
  auto carYellow = std::make_shared<CarYellow>();
  auto carRed = std::make_shared<CarRed>();

  auto pedRed = std::make_shared<PedRed>();
  auto pedGreen = std::make_shared<PedGreen>();

  // --- Car LED logic ---
  carGreen->OnStateEnter = []()
  {
    carGreenLed.On();
    carYellowLed.Off();
    carRedLed.Off();
  };
  carGreen->OnStateExit = []()
  { carGreenLed.Off(); };

  carYellow->OnStateEnter = []()
  {
    carYellowLed.On();
    carGreenLed.Off();
    carRedLed.Off();
  };
  carYellow->OnStateExit = []()
  { carYellowLed.Off(); };

  carRed->OnStateEnter = []()
  {
    carRedLed.On();
    carGreenLed.Off();
    carYellowLed.Off();
    carRedFired = true;
  };
  carRed->OnStateExit = []()
  { carRedLed.Off(); };

  // --- Pedestrian LED logic ---
  pedRed->OnStateEnter = []()
  {
    pedRedLed.On();
    pedGreenLed.Off();
  };
  pedRed->OnStateExit = []()
  { pedRedLed.Off(); };

  pedGreen->OnStateEnter = []()
  {
    pedGreenLed.On();
    pedRedLed.Off();
    carRedFired = false;
  };
  pedGreen->OnStateExit = []()
  { pedGreenLed.Off(); };

  carGreen->AddTransition(Transition(carYellow, []()
                                     { return button.IsPressed(); }));

  carYellow->AddTransition(Transition(carRed, [carYellow]()
                                      { return carYellow->IsTimeUp(); }));

  pedRed->AddTransition(Transition(pedGreen, []()
                                   { return carRedFired; }));

  pedGreen->AddTransition(Transition(pedRed, [pedGreen]()
                                     { return pedGreen->IsBlinkDone(); }));

  carRed->AddTransition(Transition(carGreen, [pedRed]()
                                   { return pedRed->IsTimeUp(); }));

  carSM.SetInitialState(carGreen);
  pedSM.SetInitialState(pedRed);
}

void loop()
{
  for (auto comp : comps)
    comp->Tick();

  static unsigned long lastTime = millis();

  unsigned long now = millis();
  unsigned long deltaMs = now - lastTime;
  lastTime = now;

  float dt = deltaMs / 1000.0f;

  carSM.Tick(dt);
  pedSM.Tick(dt);
  Serial.printf("dt: %s\n", std::to_string(dt));
}