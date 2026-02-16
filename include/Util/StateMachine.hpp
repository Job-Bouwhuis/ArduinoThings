#include <memory>
#include "Util/List.hpp"
#include <functional>
#include "Components/Led.h"

class State;
using StatePtr = std::shared_ptr<State>;

class Transition
{
public:
    Transition(StatePtr targetState, std::function<bool()> condition)
    {
        target = targetState;
        conditions.Add(condition);
    }

    StatePtr target;
    Util::List<std::function<bool()>> conditions;
    std::function<void()> onTransition;

    bool CanTrigger() const
    {
        for (auto &cond : conditions)
        {
            if (!cond->operator()())
                return false;
        }
        return true;
    }
};

class State
{
public:
    Util::List<Transition> transitions;
    std::weak_ptr<State> parent;

    std::function<void()> OnStateEnter;
    std::function<void()> OnStateExit;

    virtual ~State() = default;
    virtual void Tick(float dt) = 0;

    void AddTransition(const Transition &t)
    {
        transitions.Add(t);
    }

    virtual void Enter()
    {
        if (OnStateEnter)
            OnStateEnter();
    }

    virtual void Exit()
    {
        if (OnStateExit)
            OnStateExit();
    }
};

class TimedState : public State
{
protected:
    float timer = 0.0f;
    float duration;

public:
    TimedState(float dur) : duration(dur) {}

    void Enter() override
    {
        ResetTimer();
        State::Enter();
    }

    void Tick(float dt) override
    {
        timer += dt;
    }

    bool IsTimeUp() const { return timer >= duration; }
    void ResetTimer() { timer = 0.0f; }
};

class StateMachine
{
    StatePtr currentState;

public:
    void SetInitialState(StatePtr state)
    {
        currentState = state;
        currentState->Enter();
    }

    void Tick(float dt)
    {
        if (!currentState)
            return;

        for (auto &tPtr : currentState->transitions)
        {
            if (!tPtr)
                continue;
            if (!tPtr->target)
                continue;
            if (tPtr->CanTrigger())
            {
                currentState->Exit();
                currentState = tPtr->target;
                currentState->Enter();
                if (tPtr->onTransition)
                    tPtr->onTransition();
                return;
            }
        }

        currentState->Tick(dt);
    }
};
