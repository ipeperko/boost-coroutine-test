// the idea and parts of the code were taken from the following source: https://www.youtube.com/watch?v=3HBila8pZKk

#include <iostream>
#include <map>
#include <thread>

#include <boost/coroutine2/all.hpp>

using namespace std::chrono_literals;

class RoutineWrapper
{
public:
    using pull_type = boost::coroutines2::coroutine<void>::pull_type;
    using push_type = boost::coroutines2::coroutine<void>::push_type;
    using Routine = std::function<void()>;

    explicit RoutineWrapper(const Routine& func)
        : routine([this, func] (pull_type& pull) { invoke(pull, func); })
    {}

    void run()
    {
        routine();
    }

    void yield()
    {
        (*ret)();
    }

    bool isActive() const
    {
        return static_cast<bool>(routine);
    }

private:

    void invoke(pull_type& pull, const Routine& func)
    {
        ret = std::make_unique<pull_type>(std::move(pull));
        func();
    }

    push_type routine;
    std::unique_ptr<pull_type> ret;
};

class Scheduler
{
public:

    int addRoutine(RoutineWrapper::Routine&& func)
    {
        coroutines.emplace(next_routine_id, func);
        return next_routine_id++;
    }

    void yield()
    {
        if (!is_running)
            return;
        active->second.yield();
    }

    void stop()
    {
        is_running = false;
    }

    void run()
    {
        if (is_running)
            return;

        is_running = true;
        active = coroutines.begin();

        while (is_running && !coroutines.empty())
        {
            active->second.run();

            if (active->second.isActive())
                ++active;
            else
                active = coroutines.erase(active);

            if (active == coroutines.end())
                active = coroutines.begin();
        }

        is_running = false;
    }

private:
    std::map<int, RoutineWrapper> coroutines;
    std::map<int, RoutineWrapper>::iterator  active;
    bool is_running = false;
    int next_routine_id = 0;
};

int main()
{
    Scheduler scheduler;

    auto task = [&](const std::string_view name, int imax)
    {
        int i = 0;
        while (i++ < imax)
        {
            std::cout << name << " ... " << i << "\n";
            std::this_thread::sleep_for(1s);

            scheduler.yield();
        }
    };

    scheduler.addRoutine([&] { task("task1", 5); });
    scheduler.addRoutine([&] { task("task2", 5); });

    scheduler.run();
}
