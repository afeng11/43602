/*
 * BSS load IE header file
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id:$
*/


#ifndef _WLC_BSSLOAD_H_
#define _WLC_BSSLOAD_H_

#if defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT)
extern wlc_bssload_info_t *wlc_bssload_attach(wlc_info_t *wlc);
extern void wlc_bssload_detach(wlc_bssload_info_t *mbssload);
#else	/* stubs */
#define wlc_bssload_attach(wlc) NULL
#define wlc_bssload_detach(mbssload) do {} while (0)
#endif /* defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT) */

#if defined(WLBSSLOAD_REPORT)
extern void wlc_bssload_reset_saved_data(wlc_bsscfg_t *cfg);
#endif /* defined(WLBSSLOAD_REPORT) */

#endif /* _WLC_BSSLOAD_H_ */
