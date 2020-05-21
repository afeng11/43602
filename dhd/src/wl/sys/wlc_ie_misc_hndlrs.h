/*
 * Miscellaneous IE hanlers
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_ie_misc_hndlrs.h 467328 2014-04-03 01:23:40Z $
 */
#ifndef _wlc_ie_misc_hndlrs_h
#define _wlc_ie_misc_hndlrs_h

int wlc_register_iem_fns(wlc_info_t *wlc);
#ifdef STA
int wlc_assoc_register_iem_fns(wlc_info_t *wlc);
#else
#define wlc_assoc_register_iem_fns(wlc) BCME_OK
#endif
int wlc_bcn_register_iem_fns(wlc_info_t *wlc);
int wlc_prq_register_iem_fns(wlc_info_t *wlc);
int wlc_auth_register_iem_fns(wlc_info_t *wlc);
int wlc_scan_register_iem_fns(wlc_info_t *wlc);

extern bcm_tlv_t *wlc_find_wme_ie(uint8 *tlvs, uint tlvs_len);
extern void wlc_process_brcm_ie(wlc_info_t *wlc, struct scb *scb, brcm_ie_t *brcm_ie);
extern void wlc_process_extcap_ie(wlc_info_t *wlc, uint8 *tlvs, int len, struct scb *scb);

#endif /* _wlc_ie_misc_hndlrs_h */
