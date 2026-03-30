# ArduinoThings Codebase Report

## 1. High-level architecture

- `src/main.cpp` initializes all hardware components, state machine and tasks, then enters Arduino `loop()`.
- States are implemented in `include/States/*.hpp`:
  - `MainMenu` displays options and animates WADA.
  - `Game1` is binary conversion challenge with WADA button input.
  - `Game2` is a Katakana demo (visual display, no win/lose logic yet).
  - `Game3` is lane-avoidance obstacle game.
  - `Game4` is placeholder stub.
  - `Replay` handles returning to previously played state.
- `TaskManager` schedules and ticks ongoing coroutines/tasks.
- `StateMachine` manages transitions between states and resets WADA button callback events on transition.

## 2. Pin assignment table

| Component | Class / Object | Arduino pin | Notes |
|---------|----------------|------------|-------|
| User button | `Components::Button *userButton` | `USER_BTN` | external button, custom define from board config or platformio environment (typically `D2` on some boards). |
| WADA module | `Components::WADA *wada` | `D3` (STB), `D4` (CLK), `D5` (DIO) | TM1638 display + 8 buttons + 8 7-seg + 8 LEDs. |
| Built-in LED | `Components::Led *ledBuiltin` | `LED_BUILTIN` | board builtin LED. |
| LCD (I2C) | `Components::Lcd *lcd` | `0x27` I2C addr | 16x2 display (SDA/SCL on board). |
| Potentiometer | `Components::Potentiometer *pot` | `A0` | analog input volume/unused. |
| Buzzer | `Components::Buzzer *buzzer` | `D9` | PWM beep/audio output. |

> **Note:** `USER_BTN` is referenced in code but not defined here; check platformio board variant or `Util/Includes.h` for concrete assignment.

## 3. Main program flow

1. `setup()`:
   - Initialize `Serial` and `lcd`, load character generator.
   - Play startup melody on `buzzer`.
   - Instantiate states and register with `stateMachine`.
   - Configure allowed transitions.
   - Set initial state to `mainmenu`.
2. `loop()`:
   - `buzzer->Tick()` updates sound voice state.
   - `userButton->Tick()` polls user button (unused in state machine by default).
   - `wada->Tick()` polls TM1638 button states.
   - `tasks.Tick()` runs active tasks (coroutines animations, game loops, etc.).
   - `stateMachine.Tick()` runs current state tick.

## 4. State machine transition diagram (vertical)

```mermaid
flowchart TB
    A[MainMenu] -->|BTN1| B[Game1]
    A -->|BTN2| C[Game2 (T.B.D.)]
    A -->|BTN3| D[Game3]
    A -->|BTN4| E[Game4 (T.B.D.)]
    B --> F[Replay]
    C --> F
    D --> F
    E --> F
    F --> A
    F --> B
    F --> C
    F --> D
    F --> E
```

## 5. Game-specific behavior summaries

### 5.1 Game1 — binary puzzle
- On enter: `Reset()` sets round, lives, score, target random number and `SetupButtons()`.
- WADA buttons toggle bits on 8-bit answer; button 1 submits with <=1 second remaining.
- `RoundTask` has timer, 4 rounds (1..4), correct/incorrect feedback, score update,
- On complete: show win/lose, go to `replay` state.

### 5.2 Game2 — 

### 5.3 Game3 — obstacle lane runner
- OnEnter resets game and sets up WADA button1 to swap rows.
- `Tick()` handles game over, timed steps (`STEP_DELAY` = 500ms), `StepGame()`, and `Render()`.
- Obstacles spawn on two lanes and move; collision sets game over.
- Score increment for obstacles passing, displayed via WADA number.

### 5.4 Game4 — placeholder
- network class has state name `game3` (bug likely), empty lifecycle methods.

## 6. Task management and coroutines
- `Tasks::Task` has `Tick()`, `CoroBegin`, `CoroWait`, `CoroEnd` to simulate cooperative multitasking.
- `TaskManager::Tick()` propagates and cleans finished tasks.

## 7. Component class interactions
- `WADA` contains 8 internal `Button`s with `UpdateState` and uses bitmask from TM1638.
- `Button` uses internal `Debouncer` to avoid bounce and supports edge selection.
- `Buzzer` handles multiple tones with mixing logic and scheduled timers.

## 8. Mermaid game flow detail (vertical, state-by-state)

### 8.1 MainMenu subflow
```mermaid
flowchart TB
    N[MainMenu Start] --> N1[Show animated LCD text]
    N1 --> N2[WADA LED animation]
    N2 --> N3[Press WADA 1..4 handlers]
    N3 --> |1| Game1Start
    N3 --> |2| Game2Start
    N3 --> |3| Game3Start
    N3 --> |4| Game4Start
```

### 8.2 Game1 internal flow
```mermaid
flowchart TB
    G1[Game1 OnEnter] --> Reset
    Reset --> SetupButtons
    SetupButtons --> ExplainTask
    ExplainTask --> RoundTask
    RoundTask --> TimeLoop
    TimeLoop -->|Correct| IncrementScore
    TimeLoop -->|Incorrect| DecrementLives
    TimeLoop -->|Round>=5 or lives==0| Finish
    Finish --> Replay
```

### 8.3 Game3 internal flow
```mermaid
flowchart TB
    G3[Game3 OnEnter] --> ResetGame
    ResetGame --> SetupButtons
    SetupButtons --> Render
    Render --> TickLoop
    TickLoop --> StepGame
    StepGame --> UpdateObstacles
    StepGame --> CheckCollision
    CheckCollision -->|Collision| GameOver
    CheckCollision -->|No Collision| Continue
    Continue --> Render
    GameOver --> WaitGameOverDelay
    WaitGameOverDelay --> Replay
```
