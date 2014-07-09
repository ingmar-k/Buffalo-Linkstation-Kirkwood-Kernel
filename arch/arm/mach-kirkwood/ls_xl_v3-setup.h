/*
 * arch/arm/mach-kirkwood/ls_xl_v3-nas-setup.h
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 * 
 * Based on the setup files of a number of different Kirkwood files.
 * Edited by Ingmar Klein <ingmar.klein@hs-augsburg.de>
 */

#ifndef __ARCH_KIRKWOOD_LS_XL_V3_SETUP_H
#define __ARCH_KIRKWOOD_LS_XL_V3_SETUP_H

//void lacie_v2_register_flash(void);
//void lacie_v2_register_i2c_devices(void);
void ls_xl_v3_hdd_power_init(void);
void ls_xl_v3_gpio_hdd_power_setup(void);

#endif
