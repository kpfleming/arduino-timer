/**
   arduino-timer - library for delaying function calls

   Copyright (c) 2018, Michael Contreras
   Copyright (c) 2020, Kevin P. Fleming
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <Arduino.h>

#undef max
#undef min

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <optional>

#ifndef TIMERSET_DEFAULT_TIMERS
#define TIMERSET_DEFAULT_TIMERS 0x10
#endif

namespace Timers {

using Timepoint = unsigned long;

enum class TimerStatus
    {
     completed,
     repeat,
     reschedule
    };

struct HandlerResult
{
    TimerStatus status;
    Timepoint next;

    // must be constructed with at least a status, but can be constructed
    // with status and next
    HandlerResult() = delete;
    HandlerResult(TimerStatus status) : status(status) {}
    HandlerResult(TimerStatus status, Timepoint next) : status(status), next(next) {}
};

using Handler = std::function<HandlerResult (void)>;

struct Timer
{
    Handler handler;
    Timepoint start; // when timer was added (or repeat execution began)
    Timepoint expires; // when the timer expires
    Timepoint repeat; // default repeat interval

    // ensure that these objects will never be copied or moved
    // (this could only happen by accident)
    Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
};

using TimerHandle = std::optional<std::reference_wrapper<Timer>>;

struct Clock
{
    struct millis
    {
	static
	Timepoint now() noexcept
	{
	    return ::millis();
	}

	static
	void
	delay(Timepoint until) noexcept
	{
	    ::delay(until);
	}
    };

    struct micros
    {
	static
	Timepoint now() noexcept
	{
	    return ::micros();
	}

	static
	void delay(Timepoint until) noexcept
	{
	    unsigned int micros = until % 1000;
	    until -= micros;
	    until /= 1000;

	    ::delayMicroseconds(micros);
	    ::delay(until);
	}
    };

    template <
	Timers::Timepoint (*clock_func)()
	>
    struct custom
    {
	static
	Timepoint now() noexcept
	{
	    return clock_func();
	}
    };
};

template <
    size_t max_timers = TIMERSET_DEFAULT_TIMERS, // max number of timers
    typename clock = Clock::millis // clock for timers
    >
class TimerSet
{
    std::array<Timer, max_timers> timers;

    void
    remove(TimerHandle handle) noexcept
    {
	if (!handle) {
	    return;
	}

	auto& timer = handle.value().get();

	timer.handler = Handler();
	timer.start = 0;
	timer.expires = 0;
	timer.repeat = 0;
    }

    auto
    next_timer_slot() noexcept
    {
	return std::find_if(timers.begin(), timers.end(), [](Timer& t){ return !t.handler; });
    }

    TimerHandle
    add_timer(Timepoint start, Timepoint expires, Handler h, Timepoint repeat = 0) noexcept
    {
	if (auto it = next_timer_slot(); it != timers.end()) {
	    it->handler = h;
	    it->start = start;
	    it->expires = expires;
	    it->repeat = repeat;

	    return TimerHandle(*it);
	}
	else {
	    return TimerHandle();
	}
    }

    // Reschedules handler to be called in delay units of time
    TimerHandle
    reschedule_timer(TimerHandle handle, Timepoint start, Timepoint expires) noexcept
    {
	if (!handle) {
	    return handle;
	}

	auto& timer = handle.value().get();

	if (!timer.handler) {
	    return handle;
	}

	timer.start = start;
	timer.expires = expires;

	return handle;
    }

public:
    // Calls handler in delay units of time
    TimerHandle
    in(Timepoint delay, Handler h) noexcept
    {
	return add_timer(clock::now(), delay, h);
    }

    // Calls handler at time
    TimerHandle
    at(Timepoint when, Handler h) noexcept
    {
	Timepoint now = clock::now();
	return add_timer(now, when - now, h);
    }

    // Calls handler every interval units of time
    TimerHandle
    every(Timepoint interval, Handler h) noexcept
    {
	return add_timer(clock::now(), interval, h, interval);
    }

    // Calls handler immediately and every interval units of time
    TimerHandle
    now_and_every(Timepoint interval, Handler h) noexcept
    {
	Timepoint now = clock::now();
	return add_timer(now, now, h, interval);
    }

    // Cancels timer
    TimerHandle
    cancel(TimerHandle handle) noexcept
    {
	if (!handle) {
	    return handle;
	}

	auto& timer = handle.value().get();

	if (!timer.handler) {
	    return handle;
	}

	remove(timer);

	return handle;
    }

    // Reschedules handler to be called in delay units of time
    TimerHandle
    reschedule_in(TimerHandle handle, Timepoint delay) noexcept
    {
	return reschedule_timer(handle, clock::now(), delay);
    }

    // Reschedules handler to be called at time
    TimerHandle
    reschedule_at(TimerHandle handle, Timepoint when) noexcept
    {
	Timepoint now = clock::now();
	return reschedule_timer(handle, now, when - now);
    }

    // Ticks the timerset forward - call this function in loop()
    // returns Timepoint of next timer expiration */
    Timepoint
    tick() noexcept
    {
	Timepoint next_expiration = std::numeric_limits<Timepoint>::max();

	// execute handlers for any timers which have expired
	for (auto& timer: timers) {
	    if (!timer.handler) {
		continue;
	    }

	    Timepoint now = clock::now();
	    Timepoint elapsed = now - timer.start;

	    if (elapsed >= timer.expires) {
		auto [ status, next ] = timer.handler();

		switch (status) {
		case TimerStatus::completed:
		    remove(timer);
		    break;
		case TimerStatus::repeat:
		    if (timer.repeat > 0) {
			timer.start = now;
			timer.expires = timer.repeat;
		    } else {
			remove(timer);
		    }
		    break;
		case TimerStatus::reschedule:
		    timer.start = now;
		    timer.expires = next;
		    break;
		}
	    }
	}

	// compute lowest remaining time after all handlers have been executed
	// (some timers may have expired during handler execution)
	Timepoint now = clock::now();

	for (auto& timer: timers) {
	    if (!timer.handler) {
		continue;
	    }

	    Timepoint remaining = timer.expires - (now - timer.start);
	    next_expiration = remaining < next_expiration ? remaining : next_expiration;
	}

	return next_expiration == std::numeric_limits<Timepoint>::max() ? 0 : next_expiration;
    }

    // Ticks the timerset forward, then delays until next timer is due
    void
    tick_and_delay() noexcept
    {
	clock::delay(tick());
    }
};


// create TimerSet with default settings
TimerSet<>
create_default() noexcept
{
    return TimerSet<>();
}

}; // end namespace Timers
