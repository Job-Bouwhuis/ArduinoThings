#pragma once
#include <list>
#include "Task.hpp"

namespace Tasks
{
    class TaskManager
    {
    public:
        void AddTask(Task *t)
        {
            tasks.push_back(t);
        }

        void RemoveTask(Task *t)
        {
            tasks.remove(t);
        }

        int GetTaskCount()
        {
            return tasks.size();
        }

        void Tick()
        {
            for (auto it = tasks.begin(); it != tasks.end();)
            {
                Task *t = *it;
                if (t)
                    t->Tick();

                if (t && t->IsFinished())
                {
                    t->OnRemoved();
                    it = tasks.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

    private:
        std::list<Task *> tasks;
    };
}
