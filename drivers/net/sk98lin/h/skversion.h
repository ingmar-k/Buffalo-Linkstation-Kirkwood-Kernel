/******************************************************************************
 *
 * Name:	skversion.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.1.4.1 $
 * Date:	$Date: 2006/08/29 13:32:53 $
 * Purpose:	specific version strings and numbers
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
 *	(C)Copyright 2002-2005 Marvell.
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE 
 *
 ******************************************************************************/

#define BOOT_STRING  "sk98lin: Network Device Driver v10.85.9.3\n" \
                     "(C)Copyright 1999-2010 Marvell(R)."
#define VER_STRING   "10.85.9.3"
#define PATCHLEVEL   "09"
#define DRIVER_FILE_NAME   "sk98lin"
#define DRIVER_REL_DATE    "Aug-13-2010"
#define DRV_NAME   "sk98lin"
#define DRV_VERSION   "10.85.9.3"

#ifdef MV_INCLUDE_SDK_SUPPORT
#define FW_FILE_PATHNAME    "/lib/firmware/txbasesu.bin"
#define FW_FILE_NAME        "txbasesu.bin"
#endif

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
