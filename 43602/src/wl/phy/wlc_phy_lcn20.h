/*
 * LCNPHY module header file
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_phy_lcn20.h $*
 */

#ifndef _wlc_phy_lcn20_h_
#define _wlc_phy_lcn20_h_

#include <typedefs.h>
#include <wlc_phy_int.h>

typedef enum {
	LCN20PHY_TSSI_PRE_PA,
	LCN20PHY_TSSI_POST_PA,
	LCN20PHY_TSSI_EXT,
	LCN20PHY_TSSI_EXT_POST_PAD
} lcn20phy_tssi_mode_t;

typedef struct _lcn20phy_txiqcal_ladder_struct {
	uint8 percent;
	uint8 g_env;
} lcn20phy_txiqcal_ladder_t;

typedef struct _lcn20phy_rx_fam_struct {
	int8 freq;
	int32 angle;
	int32 mag;
} lcn20phy_rx_fam_t;

typedef struct _lcn20phy_rxiqcal_phyregs_struct {
	bool   is_orig;
	uint16 RFOverride0;
	uint16 RFOverrideVal0;
	uint16 rfoverride2;
	uint16 rfoverride2val;
	uint16 rfoverride4;
	uint16 rfoverride4val;
	uint16 rfoverride7;
	uint16 rfoverride7val;
	uint16 rfoverride8;
	uint16 rfoverride8val;
	uint8 bbmult;
	uint16 SAVE_txpwrctrl_on;
	uint16 PapdEnable0;
	uint16 papr_ctrl;
	uint16 RxSdFeConfig1;
	uint16 RxSdFeConfig6;
	uint16 phyreg2dvga2;
	uint16 SAVE_Core1TxControl;
	uint16 sslpnCalibClkEnCtrl;
	uint16 DSSF_control_0;
} lcn20phy_rxiqcal_phyregs_t;

typedef struct _lcn20phy_txiqlocal_phyregs_struct {
	bool   is_orig;
	uint8  bbmult;
	uint16 RxFeCtrl1;
	uint16 TxPwrCtrlCmd;
	uint16 RxSdFeConfig1;
	uint16 sslpnCalibClkEnCtrl;
	uint16 AfeCtrlOvr1Val;
	uint16 AfeCtrlOvr1;
	uint16 ClkEnCtrl;
	uint16 lpfbwlutreg3;
	uint16 RFOverride0;
	uint16 RFOverrideVal0;
	uint16 rfoverride4;
	uint16 rfoverride4val;
	uint16 TxPwrCtrlRfCtrlOvr0;
	uint16 PapdEnable;
} lcn20phy_txiqlocal_phyregs_t;

typedef struct _lcn20phy_rxcal_rxgain_struct {
	int8 lna;
	uint8 tia;
	uint8 far;
	uint8 dvga;
} lcn20phy_rxcal_rxgain_t;

typedef struct _lcn20phy_iq_mismatch_struct {
	int32 angle;
	int32 mag;
	int32 sin_angle;
} lcn20phy_iq_mismatch_t;

#define LCN20PHY_CALBUFFER_MAX_SZ 1024

typedef struct _chan_info_20692_lcn20phy {
	uint8  chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint16 pll_vcocal18;
	uint16 pll_vcocal3;
	uint16 pll_vcocal4;
	uint16 pll_vcocal7;
	uint16 pll_vcocal8;
	uint16 pll_vcocal20;
	uint16 pll_vcocal1;
	uint16 pll_vcocal12;
	uint16 pll_vcocal13;
	uint16 pll_vcocal10;
	uint16 pll_vcocal11;
	uint16 pll_vcocal19;
	uint16 pll_vcocal6;
	uint16 pll_vcocal9;
	uint16 pll_vcocal17;
	uint16 pll_vcocal5;
	uint16 pll_vcocal15;
	uint16 pll_vcocal2;
	uint16 pll_mmd1;
	uint16 pll_mmd2;
	uint16 pll_vco2;
	uint16 pll_vco1;
	uint16 pll_lf4;
	uint16 pll_lf5;
	uint16 pll_lf2;
	uint16 pll_lf3;
	uint16 pll_cp1;
	uint16 pll_cp2;
	uint16 logen_cfg2;
	uint16 lna2g_tune;
	uint16 txmix2g_cfg5;
} chan_info_20692_lcn20phy_t;

typedef struct _lcn20phy_rxiqcal_radioregs_struct {
	bool   is_orig;
	uint16 wl_rx_adc_ovr1;
	uint16 wl_tx_top_2g_ovr1;
	uint16 wl_rx_top_2g_ovr1;
	uint16 wl_rx_bb_ovr1;
	uint16 wl_minipmu_ovr1;
} lcn20phy_rxiqcal_radioregs_t;

struct phy_info_lcn20phy {
	bool	calbuffer_inuse;
	uint8	calbuffer[LCN20PHY_CALBUFFER_MAX_SZ];
	uint16	tssi_idx;	/* Estimated index for target power */
	uint16	cck_tssi_idx;
	uint16	init_ccktxpwrindex;
	uint8	init_txpwrindex;
	uint16	tssi_npt;	/* NPT for TSSI averaging */
	uint16	tssi_tx_cnt; /* Tx frames at that level for NPT calculations */
	uint8	tssi_floor;
	int32	tssi_maxpwr_limit;
	int32	tssi_minpwr_limit;
	int8	tx_power_idx_override; /* Forced tx power index */
	uint8	current_index;
	uint8	txpwr_clamp_dis;
	uint8	txpwr_tssifloor_clamp_dis;
	uint8	txpwr_tssioffset_clamp_dis;
	int32	target_pwr_cck_max;
	int32	target_pwr_ofdm_max;
	int16	cckPwrOffset;
	int8	offset_targetpwr;
	int16	tempsenseCorr;
	int16	idletssi_corr;
	int16	cckPwrIdxCorr;
	uint32	rate_table[WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS];
	phy_idletssi_perband_info_t idletssi0_cache;
	uint8	rssi_vf;		/* RSSI Vmid fine */
	uint8	rssi_vc;		/* RSSI Vmid coarse */
	uint8	rssi_gs;		/* RSSI gain select */
	/* flags */
	bool    ePA;
	uint32	tssical_time;
	bool 	uses_rate_offset_table;
	bool	btc_clamp;
	lcn20phy_rxiqcal_phyregs_t rxiqcal_phyregs;
	lcn20phy_rxiqcal_radioregs_t rxiqcal_radioregs;
	uint8 dccalen_lpbkpath;
	uint8 rxiqcal_lpbkpath;
	int16 cckdigiftype;
	int16 ofdmdigiftype;
	int8 cckscale_fctr_db;
	lcn20phy_txiqlocal_phyregs_t txiqlocal_phyregs;
	/* result of radio rccal */
	uint16 rccal_gmult;
	uint16 rccal_gmult_rc;
	uint8 rccal_dacbuf;
	uint16 rccal_adc_gmult;
};
#endif /* _wlc_phy_lcn20_h_ */
