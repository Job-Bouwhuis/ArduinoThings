#pragma once
#include <Arduino.h>
#include "States/States.hpp"
#include "Components/lcd.hpp"
#include "Components/Button.hpp"
#include "Components/WADA.hpp"
#include "Tasks/Task.hpp"
#include "Tasks/TaskManager.hpp"
#include "Util/eight.hpp"
#include "Components/Led.h"
#include "Components/Potentiometer.hpp"
#include "Components/Buzzer.hpp"
#include "Util/pitches.h"

extern Components::Button *userButton;
extern Components::WADA *wada;
extern Components::Led *ledBuiltin;
extern Components::Lcd *lcd;
extern Components::Potentiometer *pot;
extern Components::Buzzer *buzzer;

extern Components::Button *wadaButton1;
extern Components::Button *wadaButton2;
extern Components::Button *wadaButton3;
extern Components::Button *wadaButton4;
extern Components::Button *wadaButton5;
extern Components::Button *wadaButton6;
extern Components::Button *wadaButton7;
extern Components::Button *wadaButton8;

extern Tasks::TaskManager tasks;
extern States::StateMachine stateMachine;