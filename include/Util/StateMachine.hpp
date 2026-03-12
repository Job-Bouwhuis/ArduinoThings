#pragma once
#include <Arduino.h>
#include <list>
#include <iostream>

namespace States
{
    class StateMachine;

    class State
    {
    public:
        State(StateMachine *sm, const String &n);

        virtual ~State() = default;

        const String &GetName() const { return name; }

        void TransitionTo(const String &targetState);

        void AddAllowedTransition(State *s)
        {
            if (s)
                allowedTransitions.push_back(s);
        }

        const std::list<State *> &GetAllowedTransitions() const
        {
            return allowedTransitions;
        }

        virtual void OnEnter() {}
        virtual void OnTick() {}
        virtual void OnExit() {}

    protected:
        StateMachine *machine;

    private:
        String name;
        std::list<State *> allowedTransitions;
    };

    class StateMachine
    {
    public:
        void RegisterState(State *state)
        {
            if (state)
                states.push_back(state);
        }

        void SetInitialState(const String &name)
        {
            currentState = FindState(name);
            if (currentState)
                currentState->OnEnter();
        }

        void Tick()
        {
            if (currentState)
                currentState->OnTick();
        }

        State *GetCurrentState() const { return currentState; }

        void RequestTransition(const String &targetState)
        {
            if (!currentState)
                return;
            for (State *s : currentState->GetAllowedTransitions())
            {
                if (s->GetName() == targetState)
                {
                    std::cout << "Transition: " << currentState->GetName()
                              << " -> " << s->GetName() << "\n";
                    currentState->OnExit();
                    currentState = s;
                    currentState->OnEnter();
                    return;
                }
            }
        }

    private:
        State *FindState(const String &name)
        {
            for (State *s : states)
                if (s->GetName() == name)
                    return s;
            return nullptr;
        }

        std::list<State *> states;
        State *currentState{nullptr};
    };

    // ---- State method bodies go here, AFTER StateMachine is fully defined ----

    inline State::State(StateMachine *sm, const String &n) : machine(sm), name(n)
    {
        if (machine)
            machine->RegisterState(this);
    }

    inline void State::TransitionTo(const String &targetState)
    {
        if (machine)
            machine->RequestTransition(targetState);
    }
}
