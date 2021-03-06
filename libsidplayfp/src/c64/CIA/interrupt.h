/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2018 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "Event.h"
#include "EventScheduler.h"

#include <cstdint>

namespace libsidplayfp
{

class MOS6526;

/**
 * This is the base class for the MOS6526 interrupt sources.
 */
class InterruptSource : protected Event
{
public:
    enum
    {
        INTERRUPT_NONE         = 0,      ///< no interrupt
        INTERRUPT_UNDERFLOW_A  = 1 << 0, ///< underflow Timer A
        INTERRUPT_UNDERFLOW_B  = 1 << 1, ///< underflow Timer B
        INTERRUPT_ALARM        = 1 << 2, ///< alarm clock
        INTERRUPT_SP           = 1 << 3, ///< serial port
        INTERRUPT_FLAG         = 1 << 4, ///< external flag
        INTERRUPT_REQUEST      = 1 << 7  ///< control bit
    };

    virtual ~InterruptSource() = default;

    /**
     * Trigger an interrupt.
     *
     * @param interruptMask Interrupt flag number
     */
    virtual void trigger(uint8_t interruptMask) { idr |= interruptMask; }

    /**
     * Clear interrupt state.
     *
     * @return old interrupt state
     */
    virtual uint8_t clear()
    {
        uint8_t const old = idr;
        idr = 0;
        return old;
    }

    /**
     * Clear pending interrupts, but do not signal to CPU we lost them.
     * It is assumed that all components get reset() calls in synchronous manner.
     */
    virtual void reset()
    {
        icr = 0;
        idr = 0;
        eventScheduler.cancel(*this);
    }

    /**
     * Set interrupt control mask bits.
     *
     * @param interruptMask control mask bits
     */
    void set(uint8_t interruptMask)
    {
        if (interruptMask & 0x80)
        {
            icr |= interruptMask & ~INTERRUPT_REQUEST;
            trigger(INTERRUPT_NONE);
        }
        else
        {
            icr &= ~interruptMask;
        }
    }

protected:
    /**
     * Create a new InterruptSource.
     *
     * @param scheduler event scheduler
     * @param parent the MOS6526 which this Interrupt belongs to
     */
    explicit InterruptSource(EventScheduler& scheduler, MOS6526& parent) :
        Event("CIA Interrupt"),
        parent(parent),
        eventScheduler(scheduler)
    {}

    bool interruptMasked() const { return icr & idr; }

    bool interruptTriggered() const { return idr & INTERRUPT_REQUEST; }

    void triggerInterrupt() { idr |= INTERRUPT_REQUEST; }

    void triggerBug() { idr &= ~INTERRUPT_UNDERFLOW_B; }

    /// Pointer to the MOS6526 which this Interrupt belongs to.
    MOS6526 &parent;

    /// Event scheduler.
    EventScheduler &eventScheduler;

private:
    /// Interrupt control register
    uint8_t icr = 0;

    /// Interrupt data register
    uint8_t idr = 0;
};

}

#endif // INTERRUPT_H
