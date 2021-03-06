/*
 * Power statistics
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


#ifndef _wlc_pwrstats_h_
#define _wlc_pwrstats_h_

/* debugging... */
#ifdef EVENT_LOG_COMPILE
#define WL_PWRSTATS_INFO(str, p1, p2, p3, p4) \
	EVENT_LOG(EVENT_LOG_TAG_PWRSTATS_INFO, str, p1, p2, p3, p4)
#else /* EVENT_LOG_COMPILE */
#define WL_PWRSTATS_INFO(str, p1, p2, p3, p4)
#endif /* EVENT_LOG_COMPILE */

typedef struct wlc_pwrstats_info wlc_pwrstats_info_t;

extern void wlc_pwrstats_detach(wlc_pwrstats_info_t *ps);
extern wlc_pwrstats_info_t *wlc_pwrstats_attach(wlc_info_t *wlc);
extern void wlc_pwrstats_wake_reason_upd(wlc_info_t *wlc, bool stay_awake);
extern void wlc_pwrstats_bcn_process(wlc_bsscfg_t *bsscfg, wlc_pwrstats_info_t *ps, uint16 seq);
extern uint8 *wlc_pwrstats_fill_pmalert(wlc_info_t *wlc, uint8 *data);
extern void wlc_pwrstats_copy_event_wake_dur(void *buf, wlc_pwrstats_info_t *ps);
extern void wlc_pwrstats_frts_start(wlc_pwrstats_info_t *ps);
extern void wlc_pwrstats_frts_end(wlc_pwrstats_info_t *ps);
#endif /* _wlc_pwrstats_h_ */
