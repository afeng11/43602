/*
 * 802.11 interference stats module header file
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_interfere.h 467328 2014-04-03 01:23:40Z $
*/

#ifndef _wlc_interfere_h_
#define _wlc_interfere_h_
extern itfr_info_t *wlc_itfr_attach(wlc_info_t *wlc);
extern void wlc_itfr_detach(itfr_info_t *itfr);
#endif /* _wlc_interfere_h_ */
