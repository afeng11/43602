/*
 * MAC Offload Module Interface
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: $
 */
#ifndef _WLC_MACOL_H_
#define _WLC_MACOL_H_

#include <bcm_ol_msg.h>

/* security */
#define RC4_STATE_NBYTES		256
#define WLC_AES_EXTENDED_PACKET		(1 << 5)
#define WLC_AES_OCB_IV_MAX		((1 << 28) - 3)
typedef struct tx_info tx_info_t;
typedef struct sec_info sec_info_t;
typedef struct macol_info macol_info_t;

struct sec_info {
	uint8		idx;		/* key index in wsec_keys array */
	uint8		id;		/* key ID [0-3] */
	uint8		algo;		/* CRYPTO_ALGO_AES_CCM, CRYPTO_ALGO_WEP128, etc */
	uint8 		algo_hw;	/* cache for hw register */
	int8		iv_len;		/* IV length */
	int8 		icv_len;
	iv_t		txiv;		/* Tx IV */
	uint8		data[DOT11_MAX_KEY_SIZE];	/* key data */
	iv_t		rxiv[NUMRXIVS];		/* Rx IV (one per TID) */
	tkip_info_t	tkip_tx;
	tkip_info_t	tkip_rx;	/* tkip receive state */
};

struct tx_info {
	uint		TX;
	sec_info_t	key;
	sec_info_t	defaultkeys[DEFAULT_KEYS];
	uint8		qos;
	uint8		hwmic;
	uint16		rate;
	uint16      	seqnum;
	uint16		PhyTxControlWord_0;
	uint16		PhyTxControlWord_1;
	uint16		PhyTxControlWord_2;
	struct ether_addr	BSSID;
};

#define MAX_FRAME_PENDING	30

struct macol_info {
	wlc_hw_info_t 	*hw;
	wlc_pub_t       *pub;
	osl_t		*osh;		/* pointer to os handle */
	uint		unit;		/* device instance number */

	tx_info_t	txinfo;
	sec_info_t	secinfo;
	uint32		counter;

	uint16		frameid[MAX_FRAME_PENDING];
	uint16		frame_pend;

	uint8		txchain;
	uint8		rxchain;

	uint32		bcn_count;
	bool		bcn_filter_enable;

	uint8		wakeinterval;

	bool		rssi_low_indicated;
	uint32		rxbcn;
	uint32		mode;

	uint		nmulticast;		/* # enabled multicast addresses */

	uint32		tbtt_thresh;

	bool		bss_htcapable;
	ht_cap_ie_t	bss_htcap;
	ht_add_ie_t	bss_htinfo;

	/* rxdrops */
	uint32		rxdrops;

	wlc_bsscfg_t	*bsscfg;
	struct ether_addr	*multicast; 	/* ptr to list of multicast addresses */
	struct ether_addr	cur_etheraddr;
};

extern void BCMATTACHFN(wlc_macol_attach)(wlc_hw_info_t *wlc_hw, int *err);
extern void BCMATTACHFN(wlc_macol_detach)(wlc_hw_info_t *wlc_hw);
extern uint16 wlc_macol_d11hdrs(macol_info_t *macol, void *p, ratespec_t rspec, uint16 fifo);
extern void wlc_macol_frameid_add(macol_info_t *macol, uint16 frameid);
extern int wlc_macol_chain_set(macol_info_t *macol, uint8 txchain, uint8 rxchain);
extern void wlc_macol_intr_enable(wlc_hw_info_t *wlc_hw, uint bit);
extern void wlc_macol_intr_disable(wlc_hw_info_t *wlc_hw, uint bit);
extern void macol_print_txstatus(tx_status_t* txs);
extern void wlc_macol_set_shmem_coremask(wlc_hw_info_t *wlc_hw);
extern int wlc_macol_pso_shmem_upd(wlc_hw_info_t *wlc_hw);
extern uint16 wlc_recv_mgmt_rx_channel_get(wlc_hw_info_t *wlc_hw, wlc_d11rxhdr_t *wrxh);
#endif /* _WLC_MACOL_H_ */
