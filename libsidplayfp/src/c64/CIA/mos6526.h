/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2018 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2009-2014 VICE Project
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000 Simon White
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MOS6526_H
#define MOS6526_H

#include <memory>

#include <cstdint>

#include "EventScheduler.h"
#include "c64/CIA/interrupt.h"
#include "c64/CIA/SerialPort.h"
#include "c64/CIA/timer.h"
#include "c64/CIA/tod.h"

class EventContext;

namespace libsidplayfp
{

class MOS6526;

/**
 * This is the timer A of this CIA.
 *
 * @author Ken Händel
 *
 */
class TimerA final : public Timer
{
private:
    /**
     * Signal underflows of Timer A to Timer B.
     */
    void underFlow() override;

    void serialPort() override;

public:
    /**
     * Create timer A.
     */
    TimerA(EventScheduler &scheduler, MOS6526 &parent) :
        Timer("CIA Timer A", scheduler, parent) {}
};

/**
 * This is the timer B of this CIA.
 *
 * @author Ken Händel
 *
 */
class TimerB final : public Timer
{
private:
    void underFlow() override;

public:
    /**
     * Create timer B.
     */
    TimerB(EventScheduler &scheduler, MOS6526 &parent) :
        Timer("CIA Timer B", scheduler, parent) {}

    /**
     * Receive an underflow from Timer A.
     */
    void cascade()
    {
        // we pretend that we are CPU doing a write to ctrl register
        syncWithCpu();
        state |= CIAT_STEP;
        wakeUpAfterSyncWithCpu();
    }

    /**
     * Check if start flag is set.
     *
     * @return true if start flag is set, false otherwise
     */
    bool started() const { return (state & CIAT_CR_START) != 0; }
};

/**
 * InterruptSource that acts like new CIA
 */
class InterruptSource8521 final : public InterruptSource
{
public:
    InterruptSource8521(EventScheduler &scheduler, MOS6526 &parent) :
        InterruptSource(scheduler, parent)
    {}

    void trigger(uint8_t interruptMask) override;

    uint8_t clear() override;

    void event() override
    {
        throw "8521 event called unexpectedly";
    }
};

/**
 * InterruptSource that acts like old CIA
 */
class InterruptSource6526 final : public InterruptSource
{
private:
    /// Clock when clear was called last
    event_clock_t last_clear;

    /// Have we already scheduled CIA->CPU interrupt transition?
    bool scheduled;

    /// Timer B bug
    bool tbBug;

private:
    /**
     * Schedules an IRQ asserting state transition for next cycle.
     */
    void schedule()
    {
        if (!scheduled)
        {
            eventScheduler.schedule(*this, 1, EVENT_CLOCK_PHI1);
            scheduled = true;
        }
    }

public:
    explicit InterruptSource6526(EventScheduler &scheduler, MOS6526 &parent) :
        InterruptSource(scheduler, parent),
        last_clear(0),
        scheduled(false),
        tbBug(false)
    {}

    void trigger(uint8_t interruptMask) override;

    uint8_t clear() override;

    /**
     * Signal interrupt to CPU.
     */
    void event() override;

    void reset() override;
};

/**
 * This class is heavily based on the ciacore/ciatimer source code from VICE.
 * The CIA state machine is lifted as-is. Big thanks to VICE project!
 *
 * @author alankila
 */
class MOS6526
{
    friend class InterruptSource6526;
    friend class InterruptSource8521;
    friend class SerialPort;
    friend class TimerA;
    friend class TimerB;
    friend class Tod;

private:
    static const char *credit;

protected:
    /// Event context.
    EventScheduler &eventScheduler;

    /// Ports
    //@{
    uint8_t &pra, &prb, &ddra, &ddrb;
    //@}

    /// These are all CIA registers.
    uint8_t regs[0x10];

    /// Timers A and B.
    //@{
    TimerA timerA;
    TimerB timerB;
    //@}

    /// Interrupt Source
    std::unique_ptr<InterruptSource> interruptSource;

    /// TOD
    Tod tod;

    /// Serial Data Registers
    SerialPort serialPort;

    /// Events
    //@{
    EventCallback<MOS6526> bTickEvent;
    //@}

private:
    /**
     * Trigger an interrupt from TOD.
     */
    void todInterrupt();

    /**
     * Trigger an interrupt from Serial Port.
     */
    void spInterrupt();

    /**
     * This event exists solely to break the ambiguity of what scheduling on
     * top of PHI1 causes, because there is no ordering between events on
     * same phase. Thus it is scheduled in PHI2 to ensure the b.event() is
     * run once before the value changes.
     *
     * - PHI1 a.event() (which calls underFlow())
     * - PHI1 b.event()
     * - PHI2 bTick.event()
     * - PHI1 a.event()
     * - PHI1 b.event()
     */
    void bTick();

    /**
     * Timer A underflow.
     */
    void underflowA();

    /** Timer B underflow. */
    void underflowB();

    /**
     * Handle the serial port.
     */
    void handleSerialPort();

protected:
    /**
     * Create a new CIA.
     *
     * @param context the event context
     */
    explicit MOS6526(EventScheduler &scheduler);

    /**
     * Signal interrupt.
     *
     * @param state
     *            interrupt state
     */
    virtual void interrupt(bool state) = 0;

    virtual void portA() {}
    virtual void portB() {}

    /**
     * Read CIA register.
     *
     * @param addr
     *            register address to read (lowest 4 bits)
     */
    uint8_t read(uint_least8_t addr);

    /**
     * Write CIA register.
     *
     * @param addr
     *            register address to write (lowest 4 bits)
     * @param data
     *            value to write
     */
    void write(uint_least8_t addr, uint8_t data);

public:
    /**
     * Select chip model.
     * 
     * @param newModel true for new model 8521, false for old 6526
     */
    void setModel(bool newModel);

    /**
     * Reset CIA.
     */
    virtual void reset();

    /**
     * Get the credits.
     *
     * @return the credits
     */
    static const char *credits();

    /**
     * Set day-of-time event occurence of rate.
     *
     * @param clock
     */
    void setDayOfTimeRate(unsigned int clock) { tod.setPeriod(clock); }
};

}

#endif // MOS6526_H