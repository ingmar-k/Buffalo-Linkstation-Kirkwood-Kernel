/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/time.h>
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "cntmr/mvCntmrRegs.h"

/*
 * Timer0: clock_event_device, Tick.
 * Timer1: clocksource, Free running.
 * WatchDog: Not used.
 *
 * Timers are counting down.
 */
#define CLOCKEVENT	0
#define CLOCKSOURCE	1

/*
 * Timers bits
 */
#define BRIDGE_INT_TIMER(x)	(1 << ((x) + 1))
#define TIMER_EN(x)		(1 << ((x) * 2))
#define TIMER_RELOAD_EN(x)	(1 << (((x) * 2) + 1))
#define BRIDGE_INT_TIMER_WD	(1 << 3)
#define TIMER_WD_EN		(1 << 4)
#define TIMER_WD_RELOAD_EN	(1 << 5)

static cycle_t kw_clksrc_read(struct clocksource *cs)
{
	return (0xffffffff - MV_REG_READ(CNTMR_VAL_REG(CLOCKSOURCE)));
}

static struct clocksource kw_clksrc = {
	.name		= "kw_clocksource",
	.shift		= 20,
	.rating		= 300,
	.read		= kw_clksrc_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static int
kw_clkevt_next_event(unsigned long delta, struct clock_event_device *dev)
{
	unsigned long flags;

	if (delta == 0)
		return -ETIME;

	local_irq_save(flags);

	/*
	 * Clear and enable timer interrupt bit
	 */
	MV_REG_WRITE(BRIDGE_INT_CAUSE_REG, ~BRIDGE_INT_TIMER(CLOCKEVENT));
	MV_REG_BIT_SET(BRIDGE_INT_MASK_REG, BRIDGE_INT_TIMER(CLOCKEVENT));

	/*
	 * Setup new timer value
	 */
	MV_REG_WRITE(CNTMR_VAL_REG(CLOCKEVENT), delta);

	/*
	 * Disable auto reload and kickoff the timer
	 */
	MV_REG_BIT_RESET(CNTMR_CTRL_REG, TIMER_RELOAD_EN(CLOCKEVENT));
	MV_REG_BIT_SET(CNTMR_CTRL_REG, TIMER_EN(CLOCKEVENT));

	local_irq_restore(flags);

	return 0;
}

static void
kw_clkevt_mode(enum clock_event_mode mode, struct clock_event_device *dev)
{
	unsigned long flags;

	local_irq_save(flags);

	if (mode == CLOCK_EVT_MODE_PERIODIC) {
		/*
		 * Setup latch cycles in timer and enable reload interrupt.
		 */
		MV_REG_WRITE(CNTMR_RELOAD_REG(CLOCKEVENT), ((mvBoardTclkGet() + HZ/2) / HZ));
		MV_REG_WRITE(CNTMR_VAL_REG(CLOCKEVENT), ((mvBoardTclkGet() + HZ/2) / HZ));
		MV_REG_BIT_SET(BRIDGE_INT_MASK_REG, BRIDGE_INT_TIMER(CLOCKEVENT));
		MV_REG_BIT_SET(CNTMR_CTRL_REG, TIMER_RELOAD_EN(CLOCKEVENT) |
					  TIMER_EN(CLOCKEVENT));
	} else {
		/*
		 * Disable timer and interrupt
		 */
		MV_REG_BIT_RESET(BRIDGE_INT_MASK_REG, BRIDGE_INT_TIMER(CLOCKEVENT));
		MV_REG_WRITE(BRIDGE_INT_CAUSE_REG, ~BRIDGE_INT_TIMER(CLOCKEVENT));
		MV_REG_BIT_RESET(CNTMR_CTRL_REG, TIMER_RELOAD_EN(CLOCKEVENT) |
					  TIMER_EN(CLOCKEVENT));
	}

	local_irq_restore(flags);
}

static struct clock_event_device kw_clkevt = {
	.name		= "kw_tick",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.rating		= 300,
	.set_next_event	= kw_clkevt_next_event,
	.set_mode	= kw_clkevt_mode,
};

extern void mv_leds_hearbeat(void);
static irqreturn_t kw_timer_interrupt(int irq, void *dev_id)
{
	/*
	 * Clear cause bit and do event
	 */
	MV_REG_WRITE(BRIDGE_INT_CAUSE_REG, ~BRIDGE_INT_TIMER(CLOCKEVENT));
	kw_clkevt.event_handler(&kw_clkevt);
	mv_leds_hearbeat();
	return IRQ_HANDLED;
}

static struct irqaction kw_timer_irq = {
	.name		= "kw_tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= kw_timer_interrupt
};

static void mv_init_timer(void)
{
	/*
	 * Setup clocksource free running timer (no interrupt on reload)
	 */
 	MV_REG_WRITE(CNTMR_VAL_REG(CLOCKSOURCE), 0xffffffff);
	MV_REG_WRITE(CNTMR_RELOAD_REG(CLOCKSOURCE), 0xffffffff);
	MV_REG_BIT_RESET(BRIDGE_INT_MASK_REG, BRIDGE_INT_TIMER(CLOCKSOURCE));
	MV_REG_BIT_SET(CNTMR_CTRL_REG, TIMER_RELOAD_EN(CLOCKSOURCE) |
				  TIMER_EN(CLOCKSOURCE));

	kw_clkevt.cpumask = cpumask_of(0);

	/*
	 * Register clocksource
	 */
	kw_clksrc.mult =
		clocksource_hz2mult(mvBoardTclkGet(), kw_clksrc.shift);

	clocksource_register(&kw_clksrc);

	/*
	 * Connect and enable tick handler
	 */
	setup_irq(IRQ_BRIDGE, &kw_timer_irq);

	/*
	 * Register clockevent
	 */
	kw_clkevt.mult =
		div_sc(mvBoardTclkGet(), NSEC_PER_SEC, kw_clkevt.shift);
	kw_clkevt.max_delta_ns =
		clockevent_delta2ns(0xfffffffe, &kw_clkevt);
	kw_clkevt.min_delta_ns =
		clockevent_delta2ns(1, &kw_clkevt);

	/*
	 * Setup clockevent timer (interrupt-driven.)
	 */
	clockevents_register_device(&kw_clkevt);
}

struct sys_timer mv_timer = {
        .init           = mv_init_timer,
};

