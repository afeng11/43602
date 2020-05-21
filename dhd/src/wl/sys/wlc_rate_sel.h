/*
 * Common OS-independent driver header file for rate selection
 * algorithm of Broadcom 802.11b DCF-only Networking Adapter.
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 *
 * $Id: wlc_rate_sel.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef	_WLC_RATE_SEL_H_
#define	_WLC_RATE_SEL_H_

/* flags returned by wlc_ratesel_gettxrate() */
#define RATESEL_VRATE_PROBE	0x0001	/* vertical rate probe pkt; use small ampdu */

typedef void rssi_ctx_t;
typedef struct rcb rcb_t;

#if defined(D11AC_TXD)
/* D11 rev >= 40 supports 4 txrates, earlier revs support 2 */
#define RATESEL_TXRATES_MAX 4
#else
#define RATESEL_TXRATES_MAX 2
#endif /* defined(D11AC_TXD) */

typedef struct ratesel_txparams {
	uint8 num; /* effective # of rates */
	ratespec_t rspec[RATESEL_TXRATES_MAX];
	uint8 antselid[RATESEL_TXRATES_MAX];
} ratesel_txparams_t;

extern ratesel_info_t *wlc_ratesel_attach(wlc_info_t *wlc);
extern void wlc_ratesel_detach(ratesel_info_t *rsi);

extern void wlc_ratesel_dump_rateset(rcb_t *state, struct bcmstrbuf *b);

/* initialize per-scb state utilized by rate selection */
extern void wlc_ratesel_init(ratesel_info_t *rsi, rcb_t *state, void *scb, void *clronupd,
	wlc_rateset_t *rateset, uint8 bw, int8 sgi_tx, int8 ldpc_tx, int8 vht_ldpc_tx,
	uint8 vht_ratemask, uint8 active_antcfg_num, uint8 antselid_init,
	uint32 max_rate, uint32 min_rate);

extern void wlc_ratesel_rfbr(rcb_t *state);
/* update per-scb state upon received tx status */
extern void wlc_ratesel_upd_txstatus_normalack(rcb_t *state,
	tx_status_t *txs, uint16 sfbl, uint16 lfbl,
	uint8 tx_mcs, uint8 antselid, bool fbr);

#ifdef WL11N
/* change the throughput-based algo parameters upon ACI mitigation state change */
extern void wlc_ratesel_aci_change(ratesel_info_t *rsi, bool aci_state);

/* update per-scb state upon received tx status for ampdu */
extern void wlc_ratesel_upd_txs_blockack(rcb_t *state,
	tx_status_t *txs, uint8 suc_mpdu, uint8 tot_mpdu,
	bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 tx_mcs, uint8 antselid);

#if (defined(WLAMPDU_MAC) || defined(D11AC_TXD))
extern void wlc_ratesel_upd_txs_ampdu(rcb_t *state, uint16 frameid,
	uint8 mrt, uint8 mrt_succ, uint8 fbr, uint8 fbr_succ,
	bool tx_error, uint8 tx_mcs, uint8 antselid);
#endif

/* update rate_sel if a PPDU (ampdu or a reg pkt) is created with probe values */
extern void wlc_ratesel_probe_ready(rcb_t *state, uint16 frameid,
	bool is_ampdu, uint8 ampdu_txretry);

extern void wlc_ratesel_upd_rxstats(ratesel_info_t *rsi, ratespec_t rx_rspec, uint16 rxstatus2);
#endif /* WL11N */

ratespec_t wlc_ratesel_getcurspec(rcb_t *state);

/* select transmit rate given per-scb state */
extern void wlc_ratesel_gettxrate(rcb_t *state,
	uint16 *frameid, ratesel_txparams_t *cur_rate, uint16 *flags);

/* get the fallback rate of the specified mcs rate */
extern ratespec_t wlc_ratesel_getmcsfbr(rcb_t *state, uint16 frameid, uint8 plcp0);

extern bool wlc_ratesel_minrate(rcb_t *state, tx_status_t *txs);

extern int wlc_ratesel_rcb_sz(void);

#ifdef WL11N
/* rssi context function pointers */
typedef void (*disable_rssi_t)(rssi_ctx_t *ctx);
typedef int (*get_rssi_t)(rssi_ctx_t *);
typedef void (*enable_rssi_t)(rssi_ctx_t *ctx);
extern void wlc_ratesel_rssi_attach(ratesel_info_t *rsi, enable_rssi_t en_fn,
	disable_rssi_t dis_fn, get_rssi_t get_fn);
#endif /* WL11N */

#ifdef BCMDBG
extern void wlc_ratesel_dump_rcb(rcb_t *rcb, int32 ac, struct bcmstrbuf *b);
#endif

#define RATESEL_MSG_INFO_VAL	0x01 /* concise rate change msg in addition to WL_RATE */
#define RATESEL_MSG_MORE_VAL	0x02 /* verbose rate change msg */
#define RATESEL_MSG_SP0_VAL	0x04 /* concise spatial/tx_antenna probing msg */
#define RATESEL_MSG_SP1_VAL	0x08 /* verbose spatial/tx_antenna probing msg */
#define RATESEL_MSG_RXA0_VAL	0x10 /* concise rx atenna msg */
#define RATESEL_MSG_RXA1_VAL	0x20 /* verbose rx atenna msg */

#ifdef WL_LPC
extern void wlc_ratesel_lpc_init(rcb_t *state);
extern void wlc_ratesel_get_info(rcb_t *state, uint8 rate_stab_thresh, uint32 *new_rate_kbps,
	bool *rate_stable, rate_lcb_info_t *lcb_info);
extern void wlc_ratesel_clr_cache(rcb_t *state);
#endif /* WL_LPC */
#endif	/* _WLC_RATE_H_ */