/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-buzzer
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26XX Buzzer
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "buzzer.h"
#include "ti-lib.h"
#include "lpm.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
static uint8_t buzzer_on;
static lpm_power_domain_lock_t lock;
/*---------------------------------------------------------------------------*/
void
buzzer_init()
{
  buzzer_on = 0;

  /* Drive the I/O ID with GPT0 / Timer A */
  ti_lib_ioc_port_configure_set(BOARD_IOID_BUZZER, IOC_PORT_MCU_PORT_EVENT0,
                                IOC_STD_OUTPUT);

  /* GPT0 / Timer A: PWM, Interrupt Enable */
  HWREG(GPT0_BASE + GPT_O_TAMR) = (TIMER_CFG_A_PWM & 0xFF) | GPT_TAMR_TAPWMIE;
}
/*---------------------------------------------------------------------------*/
uint8_t
buzzer_state()
{
  return buzzer_on;
}
/*---------------------------------------------------------------------------*/
void
buzzer_start(int freq)
{
  uint32_t load;

  buzzer_on = 1;

  lpm_pd_lock_obtain(&lock, PRCM_DOMAIN_PERIPH);

  /* Stop the timer */
  ti_lib_timer_disable(GPT0_BASE, TIMER_A);

  if(freq > 0) {
    load = (GET_MCU_CLOCK / freq);

    ti_lib_timer_load_set(GPT0_BASE, TIMER_A, load);
    ti_lib_timer_match_set(GPT0_BASE, TIMER_A, load / 2);

    /* Start */
    ti_lib_timer_enable(GPT0_BASE, TIMER_A);
  }

  /* Run in sleep mode */
  ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_peripheral_deep_sleep_enable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
void
buzzer_stop()
{
  buzzer_on = 0;

  lpm_pd_lock_release(&lock);

  /* Stop the timer */
  ti_lib_timer_disable(GPT0_BASE, TIMER_A);

  /*
   * Stop running in sleep mode.
   * ToDo: Currently GPT0 is in use by clock_delay_usec (GPT0/TB) and by this
   * module here (GPT0/TA). clock_delay_usec will never need GPT0/TB in sleep
   * mode and we control TA here. Thus, with the current setup, it's OK to
   * control whether GPT0 runs in sleep mode in this module here. However, if
   * some other module at some point starts using GPT0, we should change this
   * to happen through an umbrella module
   */
  ti_lib_prcm_peripheral_sleep_disable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_peripheral_deep_sleep_disable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
/** @} */
