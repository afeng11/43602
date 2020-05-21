/**
 * @file
 * @brief
 * Common (OS-independent) portion of Broadcom 802.11bang Networking Device Driver
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc.c 491883 2014-07-17 23:06:47Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <pcie_core.h>
#include <nicpci.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#ifdef PKTC
#include <proto/ethernet.h>
#endif
#if defined(PKTC) || defined(PKTC_TX_DONGLE)
#include <proto/802.3.h>
#endif
#include <proto/802.11e.h>
#include <proto/bcmip.h>
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#if defined(WLTEST)
#include <bcmnvram.h>
#endif
#include <wlioctl.h>
#include <epivers.h>
#if defined(BCMSUP_PSK) || defined(STA)
#include <proto/eapol.h>
#endif
#include <bcmwpa.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_cca.h>
#include <wlc_interfere.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc_mbss.h>
#include <wlc_vndr_ie_list.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <phy_utils_api.h>
#include <phy_ana_api.h>
#include <phy_radio_api.h>
#include <phy_antdiv_api.h>
#include <phy_rssi_api.h>
#include <wlc_iocv_high.h>
#include <wlc_iocv_high.h>
#include <phy_high_api.h>
#include <wlc_bmac_iocv.h>
#include <wlc_antsel.h>
#include <wlc_led.h>
#include <wlc_frmutil.h>
#include <wlc_stf.h>
#include <wlc_rsdb.h>
#if defined(NDIS) || (defined(WLC_HOSTOID) && !defined(HOSTOIDS_DISABLED))
#include <wlc_ndis_iovar.h>
#endif
#ifdef WLNAR
#include <wlc_nar.h>
#endif
#if defined(SCB_BS_DATA)
#include <wlc_bs_data.h>
#endif /* SCB_BS_DATA */
#ifdef WLAMSDU
#include <wlc_amsdu.h>
#endif
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#include <wlc_ampdu_rx.h>
#include <wlc_ampdu_cmn.h>
#endif
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif
#ifdef WLDLS
#include <wlc_dls.h>
#endif
#if defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT)
#include <wlc_bssload.h>
#endif
#ifdef L2_FILTER
#include <wlc_l2_filter.h>
#endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#include <wlc_tbtt.h>
#endif
#include <wlc_p2p.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#include <wlc_scb_ratesel.h>
#ifdef WL_LPC
#include <wlc_scb_powersel.h>
#endif
#include <wlc_event.h>
#ifdef WOWL
#include <wlc_wowl.h>
#endif
#ifdef WOWLPF
#include <wlc_wowlpf.h>
#endif
#include <wlc_seq_cmds.h>
#ifdef WLOTA_EN
#include <wlc_ota_test.h>
#endif /* WLOTA_EN */
#ifdef WLDIAG
#include <wlc_diag.h>
#endif
#include <wl_export.h>
#include "d11ucode.h"
#if defined(BCMSUP_PSK)
#include <wlc_sup.h>
#endif
#include <wlc_pmkid.h>
#if defined(BCMAUTH_PSK)
#include <wlc_auth.h>
#endif
#ifdef WET
#include <wlc_wet.h>
#endif
#ifdef WET_TUNNEL
#include <wlc_wet_tunnel.h>
#endif
#ifdef WMF
#include <wlc_wmf.h>
#endif
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#if defined(BCMNVRAMW) || defined(WLTEST)
#include <bcmotp.h>
#endif
#ifdef BCMCCMP
#include <bcmcrypto/aes.h>
#endif
#include <wlc_rm.h>
#include "wlc_cac.h"
#include <wlc_ap.h>
#ifdef AP
#include <wlc_apcs.h>
#endif
#include <wlc_scan.h>
#ifdef WL11K
#include <wlc_rrm.h>
#endif /* WL11K */
#ifdef WLWNM
#include <wlc_wnm.h>
#endif
#ifdef WLC_HIGH_ONLY
#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <bcm_xdr.h>
#include <wlc_rpc.h>
#include <wlc_rpctx.h>
#endif /* WLC_HIGH_ONLY */
#if defined(RWL_DONGLE) || defined(UART_REFLECTOR)
#include <rwl_shared.h>
#include <rwl_uart.h>
#endif /* RWL_DONGLE || UART_REFLECTOR */
#include <wlc_extlog.h>
#include <wlc_alloc.h>
#include <wlc_assoc.h>
#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR)
#include <wlc_rwl.h>
#endif
#ifdef WLC_HOSTOID
#include <wlc_hostoid.h>
#endif /* WLC_HOSTOID */
#ifdef WLPFN
#include <wl_pfn.h>
#endif
#ifdef STA
#include <wlc_wpa.h>
#endif /* STA */
#if defined(PROP_TXSTATUS)
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif
#include <wlc_lq.h>
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_dfs.h>
#include <wlc_11d.h>
#include <wlc_cntry.h>
#include <bcm_mpool_pub.h>
#include <wlc_utils.h>
#include <wlc_hrt.h>
#include <wlc_prot.h>
#include <wlc_prot_g.h>
#define _inc_wlc_prot_n_preamble_	/* include static INLINE uint8 wlc_prot_n_preamble() */
#include <wlc_prot_n.h>
#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
#include <wlc_prot_obss.h>
#endif
#include <wlc_11u.h>
#include <wlc_probresp.h>
#ifdef WLTOEHW
#include <wlc_tso.h>
#endif /* WLTOEHW */
#include <wlc_vht.h>
#include <wlc_txbf.h>
#include <wlc_pcb.h>
#include <wlc_txc.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif
#ifdef WL_BCN_COALESCING
#include <wlc_bcn_clsg.h>
#endif /* WL_BCN_COALESCING */
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif
#ifdef MFP
#include <wlc_mfp.h>
#endif
#include <wlc_macfltr.h>
#include <wlc_addrmatch.h>
#include <wlc_bmon.h>
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif
#include <wlc_btcx.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_akm_ie.h>
#include <wlc_ht.h>
#include <wlc_obss.h>
#ifdef ANQPO
#include <wl_anqpo.h>
#endif
#include <wlc_hs20.h>
#ifdef STA
#include <wlc_pm.h>
#endif /* STA */
#ifdef WLFBT
#include <wlc_fbt.h>
#endif
#if defined(WL_OKC) || defined(WLRCC)
#include <wlc_okc.h>
#endif
#ifdef WLOLPC
#include <wlc_olpc_engine.h>
#endif /* OPEN LOOP POWER CAL */
#ifdef WL_STAPRIO
#include <wlc_staprio.h>
#endif /* WL_STAPRIO */
#include <wlc_monitor.h>
#include <wlc_stamon.h>
#include <wlc_ie_misc_hndlrs.h>
#ifdef WDS
#include <wlc_wds.h>
#endif
#ifdef	WLAIBSS
#include <wlc_aibss.h>
#endif /* WLAIBSS */
#ifdef WLIPFO
#include <wlc_ipfo.h>
#endif
#ifdef WLDURATION
#include <wlc_duration.h>
#endif
#if defined(WL_STATS)
#include <wlc_stats.h>
#endif /* WL_STATS */
#include <wlc_objregistry.h>
#if defined(SAVERESTORE) && defined(WLTEST)
#include <saverestore.h>
#endif  
#include <wlc_tx.h>
#ifdef WL_PROXDETECT
#include <wlc_pdsvc.h>
#endif /* PROXIMITY DETECTION */
#ifdef BCMLTECOEX
#include <wlc_ltecx.h>
#endif /* BCMLTECOEX */
#ifdef WLPROBRESP_MAC_FILTER
#include <wlc_probresp_mac_filter.h>
#endif
#ifdef BCMSPLITRX
#include <wlc_ampdu_rx.h>
#endif
#ifdef WL_BWTE
#include <wlc_bwte.h>
#endif
#ifdef WL_LTR
#include <wlc_ltr.h>
#endif /* Latency Tolerance Reporting */

#ifdef WL_TBOW
#include <wlc_tbow.h>
#endif
#include <wlc_rx.h>
#include <wlc_dbg.h>

/*
 * buffer length needed for wlc_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL.
 */
#define SSID_FMT_BUF_LEN	((4 * DOT11_MAX_SSID_LEN) + 1)

#define	TIMER_INTERVAL_WATCHDOG	1000	/* watchdog timer, in unit of ms */
#define	TIMER_INTERVAL_RADIOCHK	2000	/* radio monitor timer, in unit of ms */

#if defined(DNGL_WD_KEEP_ALIVE) && !defined(WLC_HIGH_ONLY)
#error "DNGL_WD_KEEP_ALIVE needs WLC_HIGH_ONLY"
#endif
#ifdef DNGL_WD_KEEP_ALIVE
/* dongle watchdog timer in unit of wlc watchdog */
#define TIMER_INTERVAL_DNGL_WATCHDOG (2 * TIMER_INTERVAL_WATCHDOG)
#endif
#ifndef AP_KEEP_ALIVE_INTERVAL
#define AP_KEEP_ALIVE_INTERVAL  10 /* seconds */
#endif /* AP_KEEP_ALIVE_INTERVAL */

#ifndef WLC_MPC_MAX_DELAYCNT
#define	WLC_MPC_MAX_DELAYCNT	10	/* Max MPC timeout, in unit of watchdog */
#endif
#define	WLC_MPC_MIN_DELAYCNT	0	/* Min MPC timeout, in unit of watchdog */
#define	WLC_MPC_THRESHOLD	3	/* MPC count threshold level */

#define	BEACON_INTERVAL_DEFAULT	100	/* beacon interval, in unit of 1024TU */
#define	DTIM_INTERVAL_DEFAULT	3	/* DTIM interval, in unit of beacon interval */

/* Scale down delays to accommodate QT slow speed */
#define	BEACON_INTERVAL_DEF_QT	20	/* beacon interval, in unit of 1024TU */
#define	DTIM_INTERVAL_DEF_QT	1	/* DTIM interval, in unit of beacon interval */

/* QT PHY */
#define	PRETBTT_PHY_US_QT		10		/* phy pretbtt time in us on QT(no radio) */
/* real PHYs */
#define	PRETBTT_APHY_US			120		/* a phy pretbtt time in us */
#define	PRETBTT_BPHY_US			250		/* b/g phy pretbtt time in us */
#define	PRETBTT_LPPHY_US		100		/* lpphy pretbtt time in us */
#define	PRETBTT_LCNPHY_US		250		/* lcn phy pretbtt time in us */
#define	PRETBTT_LCN40PHY_US		750		/* lcn40 phy pretbtt time in us */
#define	PRETBTT_NPHY_US			512		/* n phy REV3 pretbtt time in us */
#define	PRETBTT_HTPHY_US		512		/* ht phy pretbtt time in us */
#define	PRETBTT_ACPHY_US		512		/* acphy pretbtt time in us */
#define	PRETBTT_ACPHY_4335_US	128		/* 4335 dongly tiny pre-btt */
#define PRETBTT_ACPHY_43162_US  512             /* 43162 pre-btt */
#define	PRETBTT_SSLPNPHY_US		250		/* sslpnphy pretbtt time in us */
/* chip specific */
#define PRETBTT_LCNPHY_4336_US		800		/* lcn phy 4336 pretbtt time in us */
/* ??? */
#define PRETBTT_ACPHY_AP_US		2		/* ac phy pretbtt time in us - for AP */

#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif
/*
 * driver maintains internal 'tick'(wlc->pub->now) which increments in 1s OS timer(soft
 * watchdog) it is not a wall clock and won't increment when driver is in "down" state
 * this low resolution driver tick can be used for maintenance tasks such as phy
 * calibration and scb update
 */
#define	SW_TIMER_IBSS_GMODE_RATEPROBE	60	/* periodic IBSS gmode rate probing */
/* watchdog trigger mode: OSL timer or TBTT */
#define WLC_WATCHDOG_TBTT(wlc) \
	(wlc->cfg->associated && wlc->cfg->pm->PM != PM_OFF && wlc->pub->align_wd_tbtt)
/* debug/trace */
uint wl_msg_level =
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	WL_ERROR_VAL;
#else
	0;
#endif /* BCMDBG */

uint wl_msg_level2 =
#if defined(BCMDBG)
	0;
#else
	0;
#endif /* BCMDBG */

#ifdef RWL_DONGLE
int g_rwl_dongle_flag = 0;
#endif

/* Find basic rate for a given rate */
#define PHY_TXC_FRAMETYPE(r)	(RSPEC_ISVHT(r) ?			\
				 FT_VHT :				\
				 (RSPEC_ISHT(r) ?			\
				  FT_HT :				\
				  (IS_CCK(r) ? FT_CCK : FT_OFDM)))

#define RFDISABLE_DEFAULT	10000000 /* rfdisable delay timer 500 ms, runs of ALP clock */
#define	SW_TIMER_LINK_REPORT	10

/* Get the bw, in required BW_XXMHZ encoding, from chanspec */
#define WLC_GET_BW_FROM_CHSPEC(cs) \
	((CHSPEC_IS8080(cs) || CHSPEC_IS160(cs)) ? BW_160MHZ : \
	CHSPEC_IS80(cs) ? BW_80MHZ : \
	CHSPEC_IS40(cs) ? BW_40MHZ : BW_20MHZ)

#ifdef BCMDBG
uint32 wl_apsta_dbg = WL_APSTA_UPDN_VAL;
/* pointer to most recently allocated wl/wlc */
static wlc_info_t *wlc_info_dbg = (wlc_info_t *)(NULL);
#if defined(WLC_LOW) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
wlc_info_t *wlc_info_time_dbg = (wlc_info_t *)(NULL);
#endif /* WLC_LOW && !BCMDBG_EXCLUDE_HW_TIMESTAMP */
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(WLMSG_PRPKT)
struct wlc_id_name_entry {
	int id;
	const char *name;
};
typedef struct wlc_id_name_entry wlc_id_name_table_t[];
#endif

#ifdef WL_SAR_SIMPLE_CONTROL
extern void wlc_phy_dynamic_sarctrl_set(wlc_phy_t *pi, bool isctrlon);
#endif /* WL_SAR_SIMPLE_CONTROL */

/* IOVar table */
/* Parameter IDs, for use only internally to wlc -- in the wlc_iovars
 * table and by the wlc_doiovar() function.  No ordering is imposed:
 * the table is keyed by name, and the function uses a switch.
 */
enum {
	IOV_RTSTHRESH = 1,
	IOV_D11_AUTH,
	IOV_STA_RETRY_TIME,
	IOV_ASSOC_RETRY_MAX,
	IOV_ASSOC_CACHE_ASSIST,
	IOV_RM_REQ,
	IOV_RM_REP,
	IOV_VLAN_MODE,
	IOV_WME,
	IOV_WME_BSS_DISABLE,
	IOV_WME_NOACK,
	IOV_WME_APSD,
	IOV_WME_APSD_TRIGGER,	/* APSD Trigger Frame interval in ms */
	IOV_WME_AUTO_TRIGGER,	/* enable/disable APSD AUTO Trigger frame */
	IOV_WME_TRIGGER_AC,	/* APSD trigger frame AC */
	IOV_SEND_NULLDATA,	/* Used to tx a null frame to the given mac addr */
	IOV_WME_QOSINFO,
	IOV_WME_DP,
	IOV_WME_COUNTERS,
	IOV_WME_CLEAR_COUNTERS,
	IOV_WME_PREC_QUEUING,
#ifdef MCAST_REGEN
	IOV_MCAST_REGEN_BSS_ENABLE,
#endif
	IOV_WLFEATUREFLAG,
	IOV_DBGSEL,
	IOV_STA_INFO,
	IOV_CAP,
	IOV_MALLOCED,
	IOV_VNDR_IE,
	IOV_WSEC,
	IOV_WSEC_RESTRICT,
	IOV_WET,
#ifdef MAC_SPOOF
	IOV_MAC_SPOOF,
#endif /* MAC_SPOOF */
	IOV_EAP_RESTRICT,
	IOV_IBSS_JOIN_ONLY,
	IOV_FRAGTHRESH,
	IOV_5G_RATE,
	IOV_5G_MRATE,
	IOV_2G_RATE,
	IOV_2G_MRATE,
	IOV_QTXPOWER,
	IOV_WPA_MSGS,
	IOV_WPA_AUTH,
	IOV_EIRP,
	IOV_CUR_ETHERADDR,
	IOV_PERM_ETHERADDR,
	IOV_RAND,
	IOV_MPC,
	IOV_WATCHDOG_DISABLE,
	IOV_MPC_DUR,
	IOV_JOIN_PREF,
	IOV_BCMERRORSTR,
	IOV_BCMERROR,
	IOV_FREQTRACK,
	IOV_COUNTERS,
	IOV_ASSOC_INFO,
	IOV_APSTA_DBG,
	IOV_DIAG,
	IOV_ASSOC_REQ_IES,
	IOV_ASSOC_RESP_IES,
	IOV_PMKID_INFO,
	IOV_SBGPIOTIMERVAL,
	IOV_SBGPIOTIMERMASK,
	IOV_SBGPIOOUT,
	IOV_RFDISABLEDLY,
	IOV_ANTSEL_TYPE,
	IOV_COUNTRY_LIST_EXTENDED,
	IOV_RESET_D11CNTS,
	IOV_SCANABORT,
	IOV_IBSS_ALLOWED,
	IOV_MCAST_LIST,
	IOV_PKTENG,
	IOV_PKTENG_MAXLEN,  /* max packet size limit for packet engine */
	IOV_BOARDFLAGS,
	IOV_BOARDFLAGS2,
	IOV_ANTGAIN,
	IOV_NVRAM_GET,
	IOV_NVRAM_DUMP,
	IOV_CISWRITE,
	IOV_CISDUMP,
	IOV_LIFETIME,		/* Packet lifetime */
	IOV_BCN_TIMEOUT,	/* Beacon timeout */
	IOV_TXMAXPKTS,		/* max txpkts that can be pending in txfifo */
	IOV_NOBCNSSID,		/* No SSID in Beacons */
	IOV_NOBCPRBRESP,	/* No response to Broadcast Probes */
	IOV_CHANSPEC,		/* return the current chanspec */
	IOV_CHANSPECS,		/* return the available chanspecs (all or based on options) */
	IOV_CURR_MCSSET,	/* return current MCS set */
	IOV_DUMP,		/* Dump iovar */
	IOV_TXFIFO_SZ,		/* size of tx fifo */
	IOV_VER,
	IOV_ANTENNAS,		/* num of antennas to be used */
	IOV_SDCIS,
	IOV_SDIO_DRIVE,		/* SDIO drive strength in mA */
	IOV_BCN_THRESH,
	IOV_SCANRESULTS_MINRSSI,
	IOV_TOE,
	IOV_ARPOE,
#ifdef PLC
	IOV_PLC,
	IOV_PLC_PATH,
#endif /* PLC */
#ifdef NLO
	IOV_NLO,			/* NLO configuration */
#endif /* NLO */
#if defined(MACOSX)
	IOV_SENDUP_MGMT,	/* Send up management packet per packet filter setting */
#endif
	IOV_ISCAN,
	IOV_ISCANRESULTS,
#if defined(WME_PER_AC_TUNING) && defined(WME_PER_AC_TX_PARAMS)
	IOV_WME_TX_PARAMS,
#endif
	IOV_IBSS_COALESCE_ALLOWED,
	IOV_PSPOLL_PRD,		/* PS poll interval in milliseconds */
	IOV_BCN_LI_BCN,		/* Beacon listen interval in # of beacons */
	IOV_BCN_LI_DTIM,	/* Beacon listen interval in # of DTIMs */
	IOV_ASSOC_LISTEN,	/* Request this listen interval frm AP */
	IOV_MSGLEVEL2,
	IOV_ASSOC_RECREATE,
	IOV_PRESERVE_ASSOC,
	IOV_ASSOC_PRESERVED,
	IOV_ASSOC_VERIFY_TIMEOUT,
	IOV_RECREATE_BI_TIMEOUT,
	IOV_INFRA_CONFIGURATION, /* Reads the configured infra mode */
	IOV_DOWN_OVERRIDE,
	IOV_ALLMULTI,
	IOV_LEGACY_PROBE,
	IOV_WOWL_PKT, 		/* Generate a wakup packet */
	IOV_ASSOCROAM,
	IOV_ASSOCROAM_START,
	IOV_ROAM_OFF,		/* Disable/Enable roaming */
	IOV_FULLROAM_PERIOD,
	IOV_ROAM_PERIOD,
	IOV_ROAM_NFSCAN,
	IOV_TXMIN_ROAMTHRESH,	/* Roam threshold for too many pkts at min rate */
	IOV_AP_ENV_DETECT,	/* Auto-detect the environment for optimal roaming */
	IOV_TXFAIL_ROAMTHRESH,	/* Roam threshold for tx failures at min rate */
	IOV_MOTION_RSSI_DELTA,	/* enable/disable motion detection and set threshold */
	IOV_SCAN_PIGGYBACK,	/* Build roam cache from periodic upper-layer scans */
	IOV_ROAM_RSSI_CANCEL_HYSTERESIS,
	IOV_BANDUNIT,		/* Get the bandunit from dongle */
#if defined(BCMDBG)
	IOV_RATE_HISTO,		/* Rate Histogram */
#endif
#if defined(WL_PM2_RCV_DUR_LIMIT)
	IOV_PM2_RCV_DUR,	/* PM=2 burst receive duration limit, in ms */
#endif /* WL_PM2_RCV_DUR_LIMIT */
#if defined(BCMDBG) && defined(MBSS)
	IOV_SRCHMEM,	/* ucode search engine memory */
#endif	/* BCMDBG && MBSS */
	IOV_PM2_SLEEP_RET,	/* PM=2 inactivity return to PS mode time, in ms */
	IOV_PM2_SLEEP_RET_EXT,	/* Extended PM=2 inactivity return to sleep */
#if defined(DELTASTATS)
	IOV_DELTA_STATS_INTERVAL,
	IOV_DELTA_STATS,
#endif
	IOV_BMAC_REBOOT,
	IOV_BMAC_DNGL_SUSPEND_ENABLE, /* enable USB suspend, linux only */
	IOV_AGGDBG,
	IOV_RPC_AGG,
	IOV_RPC_MSGLEVEL,
	IOV_RPC_DNGL_TXQ_WM,
	IOV_RPC_DNGL_AGG_LIMIT,
	IOV_RPC_HOST_AGG_LIMIT,
	IOV_RPCHIST_RPCQ_CLEAR,
	IOV_RPCHIST_TXQ_CLEAR,
#ifdef RWL_DONGLE
	IOV_RWLDONGLE_DATA,
	IOV_DONGLE_FLAG,
#endif /* RWL_DONGLE */
#ifdef WIFI_ACT_FRAME
	IOV_ACTION_FRAME,	/* Wifi protocol action frame */
	IOV_AF,			/* Wifi protocol action frame send */
#endif /* #ifdef WIFI_ACT_FRAME */
	IOV_NAV_RESET_WAR_DISABLE,	/* WAR to reset NAV on 0 dur ack reception */
	IOV_RFAWARE_LIFETIME,
	IOV_ASSERTLOG,
	IOV_ASSERT_TYPE,
#ifdef ADV_PS_POLL
	IOV_ADV_PS_POLL,
#endif
#ifdef PHYCAL_CACHING
	IOV_PHYCAL_CACHE,
	IOV_CACHE_CHANSWEEP,
#endif /* PHYCAL_CACHING */
	IOV_PM2_RADIO_SHUTOFF_DLY,	/* PM2 Radio Shutoff delay after NULL PM=1 */
	IOV_SEND_FRAME,		/* send a frame */
	IOV_MANF_INFO,
	IOV_SMF_STATS,  /* selected management frames (smf) stats */
	IOV_SMF_STATS_ENABLE, /* SMFS enable */
	IOV_EXEMPT_M4,
	IOV_ESCAN,
	IOV_POOL,
	IOV_CURR_RATESET,
	IOV_RATESET,
	IOV_BRCM_DCS_REQ, /* BRCM DCS (RFAware feature) */
	IOV_PM2_REFRESH_BADIV,	/* Refresh PM2 timeout with bad iv frames */
	IOV_WAKEUP_SCAN,
	IOV_PM_DUR,	/* Retrieve accumulated PM duration and reset accumulator */
	IOV_AUTHOPS,
	IOV_NOLINKUP,
#ifdef DNGL_WD_KEEP_ALIVE
	IOV_DNGL_WD_KEEP_ALIVE,
	IOV_DNGL_WD_FORCE_TRIGGER,
#endif
	IOV_SAFE_MODE_ENABLE, /* safe_mode for EXT_STA */
	IOV_IS_WPS_ENROLLEE,
	IOV_WAKE_EVENT,	        /* Start wake event timer */
	IOV_TLV_SIGNAL,
	IOV_TSF,
	IOV_TSF_ADJUST,
	IOV_WLIF_BSSCFG_IDX,
	IOV_SSID,
	IOV_JOIN,
	IOV_RPT_HITXRATE,	/* report highest tx rate from tx rate history */
	IOV_RXOV,
	IOV_RPMT,		/* Rapid PM transition */
#ifdef BPRESET
	IOV_BPRESET_ENAB,
#ifdef BCMDBG
	IOV_BACKPLANE_RESET,
#endif /* BCMDBG */
#endif /* BPRESET */
#ifdef PKTQ_LOG
	IOV_PKTQ_STATS,
#endif
#ifndef WLP2P_UCODE_MACP
	IOV_WME_AC_STA,
	IOV_WME_AC_AP,
#endif
	IOV_EDCF_ACP,		/* ACP used by the h/w */
	IOV_EDCF_ACP_AD,	/* ACP advertised in bcn/prbrsp */
	IOV_EXCESS_PM_PERIOD,
	IOV_EXCESS_PM_PERCENT,
	IOV_MEMPOOL,		/* Memory pool manager. */
	IOV_OVERLAY,
	IOV_CLMVER,
	IOV_CLM_DATA_VER,
	IOV_FABID,
	IOV_FORCE_VSDB,
	IOV_FORCE_VSDB_CHANS,
#if defined(SAVERESTORE) && defined(WLTEST)
	IOV_SR_ENABLE,
#endif
	IOV_BRCM_IE, /* Advertise brcm ie (default is to advertise) */
	IOV_IE,
#ifdef MACOSX
	IOV_HEALTH_STATUS,	/* Health status of the card */
#endif
	IOV_SAR_ENABLE,
#ifdef WL_SARLIMIT
	IOV_SAR_LIMIT,
#endif
	IOV_RESET_CNTS,
#ifdef WLNDOE
	IOV_NDOE,		/* Neighbor Advertisement Offload for ipv6 */
#endif
#ifdef  STA
	IOV_ROAM_CONF_AGGRESSIVE,
	IOV_ROAM_MULTI_AP_ENV,
	IOV_ROAM_CI_DELTA,
	IOV_SPLIT_ROAMSCAN,
#endif
	IOV_CHANSPECS_DEFSET, /* Chanspecs with current driver settings */
	IOV_ANTDIV_BCNLOSS,
#if defined(PKTC) || defined(PKTC_DONGLE)
	IOV_PKTC,		/* Packet chaining */
	IOV_PKTCBND,		/* Max packets to chain */
#endif
#if defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND)
	IOV_TXDELAY_PARAMS,
#endif /* defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND) */
	IOV_ROAM_CHN_CACHE_LIMIT,
	IOV_ROAM_CHANNELS_IN_CACHE,
	IOV_ROAM_CHANNELS_IN_HOTLIST,
	IOV_ROAMSCAN_PARMS,
	IOV_PASSIVE_ON_RESTRICTED_MODE,
#ifdef EVENT_LOG_COMPILE
	IOV_EVENT_LOG_SET_INIT,
	IOV_EVENT_LOG_SET_EXPAND,
	IOV_EVENT_LOG_SET_SHRINK,
	IOV_EVENT_LOG_TAG_CONTROL,
#endif	 /* EVENT_LOG_COMPILE */
	IOV_PER_CHAN_INFO,
	IOV_ATF,
	IOV_NAS_NOTIFY_EVENT,
#ifdef PROP_TXSTATUS
	IOV_COMPSTAT,
#endif
#ifdef STA
	IOV_PS_RESEND_MODE,
	IOV_SLEEP_BETWEEN_PS_RESEND,
	IOV_PM2_BCN_SLEEP_RET,
	IOV_PM2_MD_SLEEP_EXT,
	IOV_EARLY_BCN_THRESH,
#endif
#ifdef SR_DEBUG
	IOV_DUMP_PMU,
	IOV_PMU_KEEP_ON,
	IOV_POWER_ISLAND,
#endif /* SR_DEBUG */
	IOV_BSS_PEER_INFO,
#if defined(WLTEST) || defined(WLPKTENG)
	IOV_LONGPKT,		/* Enable long pkts for pktengine */
#endif
#ifdef ATE_BUILD
	IOV_GPAIO,
#endif
#ifdef WLRSDB
#if defined(BCMDBG)
	IOV_IOC,		/* IOCtl as IOVAR ioc\0<ioctl_cmd_id><params> */
#endif
#endif /* WLRSDB */
#ifdef STA
	IOV_PM_BCMC_WAIT,
#ifdef LPAS
	IOV_LPAS,
#endif /* LPAS */
	IOV_PFN_ROAM_ALERT_THRESH,
	IOV_CONST_AWAKE_THRESH,
#endif /* STA */
	IOV_PM_BCNRX,
#ifdef OPPORTUNISTIC_ROAM
	IOV_OPPORTUNISTIC_ROAM_OFF, /* Disable/Enable Opportunistic roam */
#endif
	IOV_TCPACK_FAST_TX,	/* Faster AMPDU release and transmission in ucode for TCPACK */
	IOV_ROAM_PROF,
	IOV_WD_ON_BCN,	/* Defer watchdog on beacon */
	IOV_LAST		/* In case of a need to check max ID number */
};

static const bcm_iovar_t wlc_iovars[] = {
#ifdef STA
	{"escan", IOV_ESCAN,
	(0), IOVT_BUFFER, WL_ESCAN_PARAMS_FIXED_SIZE
	},
#endif
	{"rtsthresh", IOV_RTSTHRESH,
	(IOVF_WHL|IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT16, 0
	},
	{"auth", IOV_D11_AUTH,
	IOVF_OPEN_ALLOW, IOVT_INT32, 0
	},
	{"nas_notify_event", IOV_NAS_NOTIFY_EVENT,
	IOVF_OPEN_ALLOW, IOVT_BUFFER, sizeof(scb_val_t)
	},
#ifdef STA
	{"sta_retry_time", IOV_STA_RETRY_TIME,
	(IOVF_WHL|IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"assoc_retry_max", IOV_ASSOC_RETRY_MAX,
	(IOVF_WHL), IOVT_UINT8, 0
	},
#ifdef WLSCANCACHE
	{"assoc_cache_assist", IOV_ASSOC_CACHE_ASSIST,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#endif
	{"join_pref", IOV_JOIN_PREF,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, 0
	},
	{"freqtrack", IOV_FREQTRACK,
	(IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_INT8, 0
	},
	{"send_nulldata", IOV_SEND_NULLDATA,
	IOVF_SET_UP | IOVF_BSSCFG_STA_ONLY, IOVT_BUFFER, ETHER_ADDR_LEN},
#endif	/* STA */
	{"vlan_mode", IOV_VLAN_MODE,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_INT32, 0
	},
#ifdef WME
	{"wme", IOV_WME,
	(IOVF_SET_DOWN | IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_INT32, 0
	},
	{"wme_bss_disable", IOV_WME_BSS_DISABLE,
	(IOVF_OPEN_ALLOW), IOVT_INT32, 0
	},
	{"wme_noack", IOV_WME_NOACK,
	(IOVF_OPEN_ALLOW), IOVT_BOOL,	0
	},
	{"wme_apsd", IOV_WME_APSD,
	(IOVF_SET_DOWN|IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
#ifdef STA
	{"wme_apsd_trigger", IOV_WME_APSD_TRIGGER,
	(IOVF_OPEN_ALLOW|IOVF_BSSCFG_STA_ONLY), IOVT_UINT32, 0
	},
	{"wme_trigger_ac", IOV_WME_TRIGGER_AC,
	(IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
	{"wme_auto_trigger", IOV_WME_AUTO_TRIGGER,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"wme_qosinfo", IOV_WME_QOSINFO,
	(IOVF_BSS_SET_DOWN |IOVF_OPEN_ALLOW|IOVF_BSSCFG_STA_ONLY), IOVT_UINT8, 0
	},
#endif /* STA */
	{"wme_dp", IOV_WME_DP,
	(IOVF_SET_DOWN|IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT8, 0
	},
	{"wme_prec_queuing", IOV_WME_PREC_QUEUING,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_BOOL, 0},
#if defined(WLCNT)
	{"wme_counters", IOV_WME_COUNTERS,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(wl_wme_cnt_t)
	},
	{"wme_clear_counters", IOV_WME_CLEAR_COUNTERS,
	(IOVF_OPEN_ALLOW), IOVT_VOID, 0
	},
#endif
#endif /* WME */
#ifdef MCAST_REGEN
	{"mcast_regen_bss_enable", IOV_MCAST_REGEN_BSS_ENABLE,
	(0), IOVT_BOOL, 0
	},
#endif
	{"wlfeatureflag", IOV_WLFEATUREFLAG,
	(IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_INT32, 0
	},
#ifndef WLP2P_UCODE_MACP
	{"wme_ac_sta", IOV_WME_AC_STA, IOVF_OPEN_ALLOW, IOVT_BUFFER, sizeof(edcf_acparam_t)},
	{"wme_ac_ap", IOV_WME_AC_AP, 0, IOVT_BUFFER, sizeof(edcf_acparam_t)},
#endif
	{"edcf_acp", IOV_EDCF_ACP, IOVF_OPEN_ALLOW, IOVT_BUFFER, sizeof(edcf_acparam_t)},
	{"edcf_acp_ad", IOV_EDCF_ACP_AD, IOVF_OPEN_ALLOW, IOVT_BUFFER, sizeof(edcf_acparam_t)},
	{"sta_info", IOV_STA_INFO,
	(IOVF_SET_UP|IOVF_OPEN_ALLOW), IOVT_BUFFER, WL_OLD_STAINFO_SIZE
	},
	{"cap", IOV_CAP,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, WLC_IOCTL_SMLEN
	},
	{"wpa_msgs", IOV_WPA_MSGS,
	(0), IOVT_BOOL, 0
	},
	{"wpa_auth", IOV_WPA_AUTH,
	(IOVF_OPEN_ALLOW), IOVT_INT32, 0
	},
	{"malloced", IOV_MALLOCED,
	(IOVF_WHL), IOVT_UINT32, 0
	},
	{"vndr_ie", IOV_VNDR_IE,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, (sizeof(int))
	},
	{"wsec", IOV_WSEC,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"wsec_restrict", IOV_WSEC_RESTRICT,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
#ifdef WET
	{"wet", IOV_WET,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#endif
#ifdef MAC_SPOOF
	{"mac_spoof", IOV_MAC_SPOOF,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#endif /* MAC_SPOOF */
#if defined(BCMDBG) || defined(WLTEST)
	{"eirp", IOV_EIRP,
	(IOVF_MFG), IOVT_UINT32, 0
	},
#endif /* defined(BCMDBG) || defined(WLTEST) */
	{"eap_restrict", IOV_EAP_RESTRICT,
	(0), IOVT_BOOL, 0
	},
	{"fragthresh", IOV_FRAGTHRESH,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT16, 0
	},
	{"5g_rate", IOV_5G_RATE,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"5g_mrate", IOV_5G_MRATE,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"2g_rate", IOV_2G_RATE,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"2g_mrate", IOV_2G_MRATE,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"qtxpower", IOV_QTXPOWER,
	(IOVF_WHL|IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT32, 0
	},      /* constructed in wlu.c with txpwr or txpwr1 */
#if defined(WLP2P)
	{"cur_etheraddr", IOV_CUR_ETHERADDR,
	(0), IOVT_BUFFER, ETHER_ADDR_LEN
	},
#else
	{"cur_etheraddr", IOV_CUR_ETHERADDR,
	(IOVF_SET_DOWN), IOVT_BUFFER, ETHER_ADDR_LEN
	},
#endif 
	{"perm_etheraddr", IOV_PERM_ETHERADDR,
	(0), IOVT_BUFFER, ETHER_ADDR_LEN
	},
#ifdef BCMDBG
	{"rand", IOV_RAND,
	(IOVF_GET_UP), IOVT_UINT16, 0
	},
#endif
	{"mpc", IOV_MPC,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"wd_disable", IOV_WATCHDOG_DISABLE,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#ifdef STA
	{"mpc_dur", IOV_MPC_DUR,
	(IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"apsta_dbg", IOV_APSTA_DBG,
	(0), IOVT_UINT32, sizeof(uint32)
	},
	{"IBSS_join_only", IOV_IBSS_JOIN_ONLY,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#endif /* STA */
	{"bcmerrorstr", IOV_BCMERRORSTR,
	(0), IOVT_BUFFER, BCME_STRLEN
	},
	{"bcmerror", IOV_BCMERROR,
	(0), IOVT_INT8, 0
	},
	{"counters", IOV_COUNTERS,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(wl_cnt_t)
	},
	{"assoc_info", IOV_ASSOC_INFO,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, (sizeof(wl_assoc_info_t))
	},
	{"assoc_req_ies", IOV_ASSOC_REQ_IES,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, 0
	},
	{"assoc_resp_ies", IOV_ASSOC_RESP_IES,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, 0
	},
#if defined(MACOSX)
	{"txfifo_sz", IOV_TXFIFO_SZ,
	(IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_BUFFER, sizeof(wl_txfifo_sz_t)
	},
#endif 
	{"antsel_type", IOV_ANTSEL_TYPE,
	0, IOVT_UINT8, 0
	},
	{"reset_d11cnts", IOV_RESET_D11CNTS,
	0, IOVT_VOID, 0
	},
	{"scanabort", IOV_SCANABORT,
	0, IOVT_VOID, 0
	},
	{"ibss_allowed", IOV_IBSS_ALLOWED,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"country_list_extended", IOV_COUNTRY_LIST_EXTENDED,
	(0), IOVT_BOOL, 0
	},
	{"mcast_list", IOV_MCAST_LIST,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(uint32)
	},
	{"chanspec", IOV_CHANSPEC,
	(IOVF_OPEN_ALLOW), IOVT_UINT16, 0
	},
	{"chanspecs", IOV_CHANSPECS,
	(0), IOVT_BUFFER, (sizeof(uint32))
	},
	/* Chanspecs with current driver settings */
	{"chanspecs_defset", IOV_CHANSPECS_DEFSET,
	(0), IOVT_BUFFER, (sizeof(uint32)*(WL_NUMCHANSPECS+1))
	},
#ifdef WLATF
	{"atf", IOV_ATF, IOVF_SET_DOWN|IOVF_RSDB_SET, IOVT_BOOL, 0 },
#endif
#if defined(WLTEST) || defined(WLPKTENG)
	{"pkteng", IOV_PKTENG,
	IOVF_SET_UP | IOVF_MFG, IOVT_BUFFER, sizeof(wl_pkteng_t)
	},
	{"pkteng_maxlen", IOV_PKTENG_MAXLEN,
	IOVF_SET_UP | IOVF_MFG, IOVT_UINT32, 0
	},
#endif 
#if defined(WLTEST)
	{"boardflags", IOV_BOARDFLAGS,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT32, 0
	},
	{"boardflags2", IOV_BOARDFLAGS2,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT32, 0
	},
	{"antgain", IOV_ANTGAIN,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"nvram_get", IOV_NVRAM_GET,
	(IOVF_MFG), IOVT_BUFFER, 0
	},
	{"nvram_dump", IOV_NVRAM_DUMP,
	(IOVF_MFG), IOVT_BUFFER, 0
	},
	{"ciswrite", IOV_CISWRITE,
	(IOVF_MFG), IOVT_BUFFER, sizeof(cis_rw_t)
	},
	{"cisdump", IOV_CISDUMP,
	(IOVF_MFG), IOVT_BUFFER, sizeof(cis_rw_t)
	},
#endif 
	{"lifetime", IOV_LIFETIME,
	(0), IOVT_BUFFER, sizeof(wl_lifetime_t)
	},
#ifdef STA
	{"bcn_timeout", IOV_BCN_TIMEOUT,
	(IOVF_NTRL), IOVT_UINT32, 0
	},
#endif /* STA */
	{"dump", IOV_DUMP,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, 0
	},
	{"ver", IOV_VER,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, 0
	},
	{"antennas", IOV_ANTENNAS,
	(0), IOVT_UINT8, 0
	},
#ifdef TOE
	{"toe", IOV_TOE,
	(0), IOVT_BOOL, 0
	},
#endif /* TOE */
#ifdef ARPOE
	{"arpoe", IOV_ARPOE,
	(0), IOVT_BOOL, 0
	},
#endif /* ARPOE */
#ifdef PLC
	{"plc", IOV_PLC,
	(0), IOVT_BUFFER, sizeof(wl_plc_params_t)
	},
#ifdef PLC_WET
	{"plc_path", IOV_PLC_PATH,
	(0), IOVT_BOOL, 0
	},
#endif
#endif /* PLC */
#ifdef STA
	{"scanresults_minrssi", IOV_SCANRESULTS_MINRSSI,
	0, IOVT_INT32, 0
	},
	{"iscan", IOV_ISCAN,
	(0), IOVT_BUFFER, WL_ISCAN_PARAMS_FIXED_SIZE
	},
	{"iscanresults", IOV_ISCANRESULTS,
	(0), IOVT_BUFFER, WL_ISCAN_RESULTS_FIXED_SIZE
	},
#endif /* STA */
#ifdef NLO
	{"nlo", IOV_NLO,
	(IOVF_BSSCFG_STA_ONLY), IOVT_BOOL, 0
	},
#endif /* NLO */
#if defined(MACOSX)
	{"sendup_mgmt", IOV_SENDUP_MGMT,
	(0), IOVT_BOOL, 0
	},
#endif
#if defined(WME_PER_AC_TUNING) && defined(WME_PER_AC_TX_PARAMS)
	{"wme_tx_params", IOV_WME_TX_PARAMS,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, WL_WME_TX_PARAMS_IO_BYTES
	},
#endif
	{"ibss_coalesce_allowed", IOV_IBSS_COALESCE_ALLOWED,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
#ifdef STA
	{"pspoll_prd", IOV_PSPOLL_PRD, IOVF_BSSCFG_STA_ONLY, IOVT_UINT8, 0},
	{"bcn_li_bcn", IOV_BCN_LI_BCN, 0, IOVT_UINT8, 0},
	{"bcn_li_dtim", IOV_BCN_LI_DTIM, 0, IOVT_UINT8, 0},
	{"assoc_listen", IOV_ASSOC_LISTEN, 0, IOVT_UINT16, 0},
#endif
	{"msglevel", IOV_MSGLEVEL2, 0, IOVT_BUFFER, sizeof(struct wl_msglevel2)},
#ifdef WL_ASSOC_RECREATE
	{"assoc_recreate", IOV_ASSOC_RECREATE, IOVF_OPEN_ALLOW | IOVF_RSDB_SET, IOVT_BOOL, 0},
	{"preserve_assoc", IOV_PRESERVE_ASSOC, IOVF_OPEN_ALLOW, IOVT_BOOL, 0},
	{"assoc_preserved", IOV_ASSOC_PRESERVED, IOVF_OPEN_ALLOW, IOVT_BOOL, 0},
	{"assoc_verify_timeout", IOV_ASSOC_VERIFY_TIMEOUT, IOVF_OPEN_ALLOW, IOVT_UINT32, 0},
	{"recreate_bi_timeout", IOV_RECREATE_BI_TIMEOUT, IOVF_OPEN_ALLOW, IOVT_UINT32, 0},
#endif
#ifdef STA
	{"infra_configuration", IOV_INFRA_CONFIGURATION, IOVF_OPEN_ALLOW, IOVT_UINT32, 0},
#endif /* STA */
	{"down_override", IOV_DOWN_OVERRIDE,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"allmulti", IOV_ALLMULTI,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"legacy_probe", IOV_LEGACY_PROBE,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
#ifdef AP
	{"wowl_pkt", IOV_WOWL_PKT,
	(0), IOVT_BUFFER, 0
	},
#endif /* AP */
	/* roam related params */
#ifdef STA
	{"roam_off", IOV_ROAM_OFF,
	(IOVF_OPEN_ALLOW), IOVT_BOOL,   0
	},
	{"assocroam", IOV_ASSOCROAM,
	(0), IOVT_BOOL, 0
	},
	{"assocroam_start", IOV_ASSOCROAM_START,
	(0), IOVT_BOOL, 0
	},
	{"fullroamperiod", IOV_FULLROAM_PERIOD,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"roamperiod", IOV_ROAM_PERIOD,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"roam_nfscan", IOV_ROAM_NFSCAN,
	(0), IOVT_UINT8, 0
	},
	{"txfail_roamthresh", IOV_TXFAIL_ROAMTHRESH,
	0, IOVT_UINT8, 0
	},
	{"txmin_roamthresh", IOV_TXMIN_ROAMTHRESH,
	0, IOVT_UINT8, 0
	},
	{"roam_env_detection", IOV_AP_ENV_DETECT,
	0, IOVT_BOOL, 0
	},
	{"roam_motion_detection", IOV_MOTION_RSSI_DELTA,
	0, IOVT_UINT8, 0
	},
	{"roam_scan_piggyback", IOV_SCAN_PIGGYBACK,
	0, IOVT_BOOL, 0
	},
	{"roam_rssi_cancel_hysteresis", IOV_ROAM_RSSI_CANCEL_HYSTERESIS,
	0, IOVT_UINT8, 0
	},
#endif /* STA */
	{"bandunit", IOV_BANDUNIT,
	(0), IOVT_UINT32, 0
	},
#if defined(BCMDBG)
	{"rate_histo", IOV_RATE_HISTO,
	(IOVF_GET_UP), IOVT_BUFFER, (WLC_MAXRATE + 1 + WLC_MAXMCS + 1) * sizeof(ratespec_t)
	},
#endif
#ifdef STA
#if defined(WL_PM2_RCV_DUR_LIMIT)
	{"pm2_rcv_dur", IOV_PM2_RCV_DUR,
	(0), IOVT_UINT16, 0
	},
#endif /* WL_PM2_RCV_DUR_LIMIT */
	{"pm2_sleep_ret", IOV_PM2_SLEEP_RET,
	(0), IOVT_INT16, 0
	},
	{"pm2_sleep_ret_ext", IOV_PM2_SLEEP_RET_EXT,
	(0), IOVT_BUFFER, sizeof(wl_pm2_sleep_ret_ext_t)
	},
	{"nolinkup", IOV_NOLINKUP,
	(0), IOVT_BOOL, 0
	},
#endif /* STA */

#if defined(DELTASTATS)
	{"delta_stats_interval", IOV_DELTA_STATS_INTERVAL,
	(0), IOVT_INT32, 0
	},
	{"delta_stats", IOV_DELTA_STATS,
	(0), IOVT_BUFFER, sizeof(wl_delta_stats_t)
	},
#endif
#if defined(BCMDBG) && defined(MBSS)
	{"srchmem", IOV_SRCHMEM,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_BUFFER, DOT11_MAX_SSID_LEN + 2 * sizeof(uint32)
	},
#endif	/* BCMDBG && MBSS */
	{"bmac_reboot", IOV_BMAC_REBOOT,
	(IOVF_SET_DOWN), IOVT_VOID, 0
	},
	{"bmac_dngl_suspend_enable", IOV_BMAC_DNGL_SUSPEND_ENABLE,
	(0), IOVT_UINT32, 0
	},
	{"rpc_agg", IOV_RPC_AGG,
	(0), IOVT_UINT32, 0
	},
	{"rpc_msglevel", IOV_RPC_MSGLEVEL,
	(0), IOVT_UINT32, 0
	},
	{"rpc_dngl_txqwm", IOV_RPC_DNGL_TXQ_WM,
	(0), IOVT_UINT32, 0
	},
	{"rpc_dngl_agglimit", IOV_RPC_DNGL_AGG_LIMIT,
	(0), IOVT_UINT32, 0
	},
	{"rpc_host_agglimit", IOV_RPC_HOST_AGG_LIMIT,
	(0), IOVT_UINT32, 0
	},
#ifdef RWL_DONGLE
	{"remote", IOV_RWLDONGLE_DATA,
	(0), IOVT_BUFFER, 1040
	},
	{"dongleset", IOV_DONGLE_FLAG,
	0, IOVT_UINT32, 0
	},
#endif /* RWL_DONGLE */
#ifdef WIFI_ACT_FRAME
	{"wifiaction", IOV_ACTION_FRAME,
	(0), IOVT_BUFFER, WL_WIFI_ACTION_FRAME_SIZE
	},
	{"actframe", IOV_AF,
	(0), IOVT_BUFFER, OFFSETOF(wl_af_params_t, action_frame) +
	OFFSETOF(wl_action_frame_t, data)
	},
#endif /* WIFI_ACT_FRAME */
#if defined(STA) && defined(ADV_PS_POLL)
	{"adv_ps_poll", IOV_ADV_PS_POLL, IOVF_BSSCFG_STA_ONLY, IOVT_BOOL, 0},
#endif
	{"nav_reset_war_disable", IOV_NAV_RESET_WAR_DISABLE,
	0, IOVT_BOOL, 0
	},
	{"rfaware_lifetime", IOV_RFAWARE_LIFETIME,
	(IOVF_RSDB_SET|IOVF_RSDB_SET), IOVT_UINT16, 0
	},
	{"assert_type", IOV_ASSERT_TYPE,
	(0), IOVT_UINT32, 0,
	},
#ifdef PHYCAL_CACHING
	{"phycal_caching", IOV_PHYCAL_CACHE,
	0, IOVT_BOOL, 0
	},
	{"cachedcal_scan", IOV_CACHE_CHANSWEEP,
	0, IOVT_UINT16, 0
	},
#endif /* PHYCAL_CACHING */
	{"pm2_radio_shutoff_dly", IOV_PM2_RADIO_SHUTOFF_DLY,
	(0), IOVT_UINT16, 0
	},
	{"send_frame", IOV_SEND_FRAME,
	(IOVF_SET_UP), IOVT_BUFFER, 0
	},
#ifdef SMF_STATS
	{"smfstats", IOV_SMF_STATS,
	(0), IOVT_INT32, 0,
	},
	{"smfstats_enable", IOV_SMF_STATS_ENABLE,
	(0), IOVT_INT32, 0
	},
#endif /* SMF_STATS */
#ifdef STA
	{"wake_event", IOV_WAKE_EVENT,
	(0), IOVT_UINT16, 0
	},
	{"pm2_refresh_badiv", IOV_PM2_REFRESH_BADIV,
	(IOVF_BSSCFG_STA_ONLY), IOVT_INT16, 0
	},
#endif /* STA */
#if defined(WLTEST)
	{"manfinfo", IOV_MANF_INFO,
	(0), IOVT_BUFFER, 0
	},
#endif 
	{"pool", IOV_POOL,
	(0), IOVT_UINT8, 0
	},
	{"cur_rateset", IOV_CURR_RATESET,
	(0), IOVT_BUFFER, sizeof(wl_rateset_args_t)
	},
	{"rateset", IOV_RATESET,
	(IOVF_SET_DOWN), IOVT_BUFFER, sizeof(wl_rateset_args_t)
	},
	{"dcs_req", IOV_BRCM_DCS_REQ,
	(IOVF_SET_UP), IOVT_UINT16, 0
	},
	{"pm_dur", IOV_PM_DUR,
	(0), IOVT_UINT32, 0
	},
	{"auth_ops", IOV_AUTHOPS,
	(IOVF_SET_UP), IOVT_BUFFER, 0
	},
#ifdef PROP_TXSTATUS
	{"tlv", IOV_TLV_SIGNAL,
	0, IOVT_UINT8, 0
	},
#endif /* PROP_TXSTATUS */
#ifdef DNGL_WD_KEEP_ALIVE
	{"dngl_wd", IOV_DNGL_WD_KEEP_ALIVE,
	(IOVF_SET_UP|IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"dngl_wdtrigger", IOV_DNGL_WD_FORCE_TRIGGER,
	(0), IOVT_UINT32, 0
	},
#endif
	{"is_WPS_enrollee", IOV_IS_WPS_ENROLLEE,
	(0), IOVT_BOOL, 0
	},
#ifdef BCMDBG
	{"tsf", IOV_TSF,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_UINT32, 0
	},
	{"tsf_adj", IOV_TSF_ADJUST,
	(IOVF_SET_UP), IOVT_UINT32, 0
	},
#endif /* BCMDBG */
	{"bsscfg_idx", IOV_WLIF_BSSCFG_IDX,
	(0), IOVT_UINT8, 0
	},
	{"ssid", IOV_SSID,
	(0), IOVT_INT32, 0
	},
#ifdef STA
	{"join", IOV_JOIN, IOVF_BSSCFG_STA_ONLY, IOVT_BUFFER, WL_EXTJOIN_PARAMS_FIXED_SIZE},
#endif
	{"rpt_hitxrate", IOV_RPT_HITXRATE,
	(0), IOVT_INT32, 0
	},
#ifdef WLRXOV
	{"rxov", IOV_RXOV,
	(0), IOVT_UINT32, 0
	},
#endif
#ifdef BCMDBG
#ifdef STA
	{"rpmt", IOV_RPMT, IOVF_BSSCFG_STA_ONLY, IOVT_BUFFER, 8},
#endif
#endif /* BCMDBG */
#ifdef BPRESET
	{"bpreset_enab", IOV_BPRESET_ENAB,
	(0), IOVT_UINT32, 0},
#ifdef BCMDBG
	{"bpreset", IOV_BACKPLANE_RESET,
	(0), IOVT_UINT32, 0},
#endif /* BCMDBG */
#endif /* BPRESET */
	{"ucdload_status", IOV_BMAC_UCDLOAD_STATUS,
	(0), IOVT_INT32, 0
	},
	{"ucdload_chunk_len", IOV_BMAC_UC_CHUNK_LEN,
	(0), IOVT_INT32, 0
	},
#ifdef PKTQ_LOG
	{"pktq_stats", IOV_PKTQ_STATS, (0), IOVT_BUFFER,
	MAX(sizeof(wl_iov_mac_params_t), sizeof(wl_iov_pktq_log_t))},
#endif
#ifdef STA
	{"excess_pm_period", IOV_EXCESS_PM_PERIOD,
	(IOVF_WHL), IOVT_INT16, 0
	},
	{"excess_pm_percent", IOV_EXCESS_PM_PERCENT,
	(IOVF_WHL), IOVT_INT16, 0
	},
#endif
	{"mempool", IOV_MEMPOOL,
	0, IOVT_BUFFER, sizeof(wl_mempool_stats_t)
	},
#ifdef BCMOVLHW
	{"overlay", IOV_OVERLAY,
	(0), IOVT_BUFFER, 0
	},
#endif
	{"clmver", IOV_CLMVER,
	(0), IOVT_BUFFER, MAX_CLMVER_STRLEN
	},
	{"clm_data_ver", IOV_CLM_DATA_VER,
	(0), IOVT_BUFFER, 32
	},
	{"fabid", IOV_FABID,
	(0), IOVT_UINT16, 0
	},
	{"force_vsdb", IOV_FORCE_VSDB,
	(IOVF_RSDB_SET), IOVT_UINT8, 0,
	},
	{"force_vsdb_chans", IOV_FORCE_VSDB_CHANS,
	IOVF_SET_UP | IOVF_GET_UP | IOVF_RSDB_SET, IOVT_BUFFER,  2 * sizeof(uint16)
	},
#ifdef SAVERESTORE
#if defined(WLTEST)
	{"sr_enable", IOV_SR_ENABLE,
	(0), IOVT_BOOL, 0,
	},
#endif 
#endif /* SAVERESTORE */
	{"brcm_ie", IOV_BRCM_IE,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"ie", IOV_IE,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, (sizeof(int))
	},
#ifdef MACOSX
	{"health_status", IOV_HEALTH_STATUS,
	(IOVF_RSDB_SET), IOVT_UINT32, 0
	},
#endif
	{"sar_enable", IOV_SAR_ENABLE,
	0, IOVT_BOOL, 0
	},
#ifdef WL_SARLIMIT
	{"sar_limit", IOV_SAR_LIMIT,
	IOVF_RSDB_SET, IOVT_BUFFER, (sizeof(sar_limit_t))
	},
#endif /* WL_SARLIMIT */
#ifdef WLCNT
	{"reset_cnts", IOV_RESET_CNTS,
	0, IOVT_UINT32, 0
	},
#endif /* WLCNT */
#ifdef WLNDOE
	{"ndoe", IOV_NDOE,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#endif /* WLNDOE */
#ifdef  STA
	{"roam_conf_aggressive", IOV_ROAM_CONF_AGGRESSIVE,
	(0),  IOVT_INT32, 0
	},
	{"roam_multi_ap_env", IOV_ROAM_MULTI_AP_ENV,
	(0),  IOVT_BOOL, 0
	},
	{"roam_ci_delta", IOV_ROAM_CI_DELTA,
	(0), IOVT_UINT8, 0
	},
	{"split_roamscan", IOV_SPLIT_ROAMSCAN,
	(0),  IOVT_BOOL, 0
	},
#endif /* STA */
	{"antdiv_bcnloss", IOV_ANTDIV_BCNLOSS,
	(0), IOVT_UINT16, 0
	},
#if defined(PKTC) || defined(PKTC_DONGLE)
	{"pktc", IOV_PKTC,
	(0), IOVT_BOOL, 0
	},
	{"pktcbnd", IOV_PKTCBND,
	(0), IOVT_INT32, 0
	},
#endif
#if defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND)
	{"txdelay_params", IOV_TXDELAY_PARAMS,
	(0), IOVT_BUFFER, sizeof(txdelay_params_t)
	},
#endif /* defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND) */
	{"roam_chn_cache_limit", IOV_ROAM_CHN_CACHE_LIMIT,
	(0), IOVT_UINT8, 0
	},
	{"roam_channels_in_cache", IOV_ROAM_CHANNELS_IN_CACHE,
	(0), IOVT_BUFFER, (sizeof(uint32)*(WL_NUMCHANSPECS+1))
	},
	{"roam_channels_in_hotlist", IOV_ROAM_CHANNELS_IN_HOTLIST,
	(0), IOVT_BUFFER, (sizeof(uint32)*(WL_NUMCHANSPECS+1))
	},
	{"roamscan_parms", IOV_ROAMSCAN_PARMS,
	(0), IOVT_BUFFER, 0
	},
	{"passive_on_restricted_mode", IOV_PASSIVE_ON_RESTRICTED_MODE,
	(IOVF_RSDB_SET), IOVT_UINT32, 0
	},
#ifdef EVENT_LOG_COMPILE
	{"event_log_set_init", IOV_EVENT_LOG_SET_INIT,
	(IOVF_RSDB_SET), IOVT_BUFFER, sizeof(wl_el_set_params_t)
	},
	{"event_log_set_expand", IOV_EVENT_LOG_SET_EXPAND,
	(IOVF_RSDB_SET), IOVT_BUFFER, sizeof(wl_el_set_params_t)
	},
	{"event_log_set_shrink", IOV_EVENT_LOG_SET_SHRINK,
	(IOVF_RSDB_SET), IOVT_BUFFER, sizeof(wl_el_set_params_t)
	},
	{"event_log_tag_control", IOV_EVENT_LOG_TAG_CONTROL,
	(IOVF_RSDB_SET), IOVT_BUFFER, sizeof(wl_el_tag_params_t)
	},
#endif /* EVENT_LOG_COMPILE */
	/* it is required for regulatory testing */
	{"per_chan_info", IOV_PER_CHAN_INFO,
	(0), IOVT_UINT16, 0
	},
#ifdef PROP_TXSTATUS
	{"compstat", IOV_COMPSTAT,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
#endif
#ifdef STA
	{"ps_resend_mode", IOV_PS_RESEND_MODE,
	(0),  IOVT_UINT8, 0
	},
	/* Olympic alias for "ps_resend_mode" */
	{"sleep_between_ps_resend", IOV_SLEEP_BETWEEN_PS_RESEND,
	(IOVF_RSDB_SET),  IOVT_UINT8, 0
	},
	{"pm2_bcn_sleep_ret", IOV_PM2_BCN_SLEEP_RET,
	(0), IOVT_INT16, 0
	},
	{"pm2_md_sleep_ext", IOV_PM2_MD_SLEEP_EXT,
	(IOVF_WHL), IOVT_INT8, 0
	},
	{"early_bcn_thresh", IOV_EARLY_BCN_THRESH,
	(0), IOVT_UINT32, 0
	},
#endif /* STA */
	{"bss_peer_info", IOV_BSS_PEER_INFO,
	(0), IOVT_BUFFER,
	(BSS_PEER_LIST_INFO_FIXED_LEN + sizeof(bss_peer_info_t))
	},
#if defined(WLTEST) || defined(WLPKTENG)
	{"longpkt", IOV_LONGPKT, (IOVF_SET_UP | IOVF_GET_UP), IOVT_INT16, 0},
#endif
#ifdef SR_DEBUG
	{"sr_dump_pmu", IOV_DUMP_PMU,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(pmu_reg_t)
	},
	{"sr_pmu_keep_on", IOV_PMU_KEEP_ON,
	IOVF_SET_UP | IOVF_GET_UP, IOVT_UINT16, sizeof(int)
	},
	{"sr_power_island", IOV_POWER_ISLAND,
	IOVF_SET_UP | IOVF_GET_UP, IOVT_UINT16, sizeof(int)
	},
#endif /* SR_DEBUG */
#ifdef ATE_BUILD
	{"gpaio", IOV_GPAIO,
	IOVF_SET_UP | IOVF_MFG, IOVT_UINT32, 0
	},
#endif
#ifdef WLRSDB
#if defined(BCMDBG)
	{"ioc", IOV_IOC, 0, IOVT_BUFFER, sizeof(int),
	},
#endif
#endif /* WLRSDB */
#ifdef STA
	{"pm_bcmc_wait", IOV_PM_BCMC_WAIT,
	(0), IOVT_UINT16, 0
	},
#ifdef LPAS
	{"lpas", IOV_LPAS,
	0, IOVT_UINT32, 0
	},
#endif /* LPAS */
	{"pfn_roam_alert_thresh", IOV_PFN_ROAM_ALERT_THRESH,
	(0), IOVT_BUFFER, 8
	},
	{"const_awake_thresh", IOV_CONST_AWAKE_THRESH,
	(0), IOVT_UINT32, 0
	},
#endif /* STA */
#ifdef WLPM_BCNRX
	{"pm_bcnrx", IOV_PM_BCNRX,
	(IOVF_SET_UP), IOVT_BOOL, 0
	},
#endif
#ifdef OPPORTUNISTIC_ROAM
	{"oppr_roam_off", IOV_OPPORTUNISTIC_ROAM_OFF,
	(0), IOVT_BOOL,   0
	},
#endif
	{"tcpack_fast_tx", IOV_TCPACK_FAST_TX, (0), IOVT_BOOL, 0},
	{"roam_prof", IOV_ROAM_PROF,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_BUFFER, 0
	},
	{"wd_on_bcn", IOV_WD_ON_BCN,
	(0), IOVT_BOOL, 0
	},
	{NULL, 0, 0, 0, 0}
};

const uint8 prio2fifo[NUMPRIO] = {
	TX_AC_BE_FIFO,	/* 0	BE	AC_BE	Best Effort */
	TX_AC_BK_FIFO,	/* 1	BK	AC_BK	Background */
	TX_AC_BK_FIFO,	/* 2	--	AC_BK	Background */
	TX_AC_BE_FIFO,	/* 3	EE	AC_BE	Best Effort */
	TX_AC_VI_FIFO,	/* 4	CL	AC_VI	Video */
	TX_AC_VI_FIFO,	/* 5	VI	AC_VI	Video */
	TX_AC_VO_FIFO,	/* 6	VO	AC_VO	Voice */
	TX_AC_VO_FIFO	/* 7	NC	AC_VO	Voice */
};

/** Mapping between Fifo to Priority */
const uint8 fifo2prio[NFIFO] = { PRIO_8021D_BK, PRIO_8021D_BE, PRIO_8021D_VI, PRIO_8021D_VO,
	PRIO_8021D_BE, PRIO_8021D_BE };

/* precedences numbers for wlc queues. These are twice as may levels as
 * 802.1D priorities.
 * Odd numbers are used for HI priority traffic at same precedence levels
 * These constants are used ONLY by wlc_prio2prec_map.  Do not use them elsewhere.
 */
#define	_WLC_PREC_NONE		0	/* None = - */
#define	_WLC_PREC_BK		2	/* BK - Background */
#define	_WLC_PREC_BE		4	/* BE - Best-effort */
#define	_WLC_PREC_EE		6	/* EE - Excellent-effort */
#define	_WLC_PREC_CL		8	/* CL - Controlled Load */
#define	_WLC_PREC_VI		10	/* Vi - Video */
#define	_WLC_PREC_VO		12	/* Vo - Voice */
#define	_WLC_PREC_NC		14	/* NC - Network Control */

/** 802.1D Priority to precedence queue mapping */
const uint8 wlc_prio2prec_map[] = {
	_WLC_PREC_BE,		/* 0 BE - Best-effort */
	_WLC_PREC_BK,		/* 1 BK - Background */
	_WLC_PREC_NONE,		/* 2 None = - */
	_WLC_PREC_EE,		/* 3 EE - Excellent-effort */
	_WLC_PREC_CL,		/* 4 CL - Controlled Load */
	_WLC_PREC_VI,		/* 5 Vi - Video */
	_WLC_PREC_VO,		/* 6 Vo - Voice */
	_WLC_PREC_NC,		/* 7 NC - Network Control */
};

/** TX FIFO number to WME/802.1E Access Category */
const uint8 wme_fifo2ac[] = { AC_BK, AC_BE, AC_VI, AC_VO, AC_BE, AC_BE };

/** WME/802.1E Access Category to TX FIFO number */
static const uint8 wme_ac2fifo[] = { 1, 0, 2, 3 };

/* Shared memory location index for various AC params */
#define wme_shmemacindex(ac)	wme_ac2fifo[ac]

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_INFORM)
const char *const aci_names[] = { "AC_BE", "AC_BK", "AC_VI", "AC_VO"};
#endif

#if defined(WLTEST)
#define MANF_INFO_LEN			8
#define MANF_INFO_ROW_WIDTH		64
struct wlc_otp_manf_info {
	const char *name;	/* name for the segment */
	uint16	bit_pos_start;	/* start position for the segment */
	uint16	bit_pos_end;	/* end position for the segment */
	uint16	len;		/* length of the segment */
};

/** MFG OTP info twiki: Mwgroup/OtpProgramming#ATE_test_flow */
static const struct wlc_otp_manf_info wlc_manf_info[] = {
	/* row0: wafer sort data */
	{"OTP_LOT_NUM", 0, 16, 17},
	{"WAFER_NUM", 17, 21, 5},
	{"WAFER_X", 22, 30, 9},
	{"WAFER_Y", 31, 39, 9},
	{"PROG_REL_DATE", 40, 55, 16},
	{"PROG_REV_CRTL_0", 56, 60, 5},
	{"MEM_REP_0", 61, 61, 1},
	{"PROBED_BIN1", 62, 62, 1},
	{"LOCK_BIT_0", 63, 63, 1},

	/* row1: final(packaging) test data */
	{"FT1_PROG_REL", 0, 15, 16},
	{"FT2_PROG_REL", 16, 31, 16},
	{"FT_PROG_RESCRN", 32, 47, 16},
	{"PROG_REV_CTRL_1", 48, 52, 5},
	{"MEM_REP_1", 53, 53, 1},
	{"ANALOG_TRIM", 54, 54, 1},
	{"SCREEN_BIT", 55, 59, 5},
	{"QA_SAMP_TEST", 60, 61, 2},
	{"FT_BIN1", 62, 62, 1},
	{"LOCK_BIT_1", 63, 63, 1},

	{NULL, 0, 0, 0},
	};

#endif 

/* conversion between auth values set externally and 802.11 auth type values */
#define DOT11AUTH2WLAUTH(bsscfg) (bsscfg->openshared ? WL_AUTH_OPEN_SHARED :\
	(bsscfg->auth == DOT11_OPEN_SYSTEM ? WL_AUTH_OPEN_SYSTEM : WL_AUTH_SHARED_KEY))
#define WLAUTH2DOT11AUTH(val) (val == WL_AUTH_OPEN_SYSTEM ? DOT11_OPEN_SYSTEM : DOT11_SHARED_KEY)

/* local prototypes */
static uint8 wlc_template_plcp_offset(wlc_info_t *wlc, ratespec_t rspec);

static void wlc_monitor_down(wlc_info_t *wlc);
static void wlc_bss_default_init(wlc_info_t *wlc);
static void wlc_edcf_acp_set_sw(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *edcf_acp);
static void wlc_edcf_acp_set_hw(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *edcf_acp);
static void wlc_ucode_mac_upd(wlc_info_t *wlc);
static int  wlc_xmtfifo_sz_get(wlc_info_t *wlc, uint fifo, uint *blocks);
#if defined(MACOSX)
static int  wlc_xmtfifo_sz_set(wlc_info_t *wlc, uint fifo, uint16 blocks);
#endif
static void wlc_xmtfifo_sz_upd_high(wlc_info_t *wlc, uint fifo, uint16 blocks);

static const txmod_fns_t BCMATTACHDATA(txq_txmod_fns) = {
	wlc_txq_enq,
	wlc_txq_txpktcnt,
	NULL,
	NULL
};

#if defined(WLC_LOW) && defined(WLC_HIGH) && (defined(BCMDBG) || \
	defined(MCHAN_MINIDUMP))
static int wlc_dump_chanswitch(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif

static void wlc_tx_prec_map_init(wlc_info_t *wlc);
static char *wlc_cap(wlc_info_t *wlc, char *buf, uint bufsize);
static void wlc_cap_bcmstrbuf(wlc_info_t *wlc, struct bcmstrbuf *b);

static void wlc_watchdog_timer(void *arg);
static int wlc_set_rateset(wlc_info_t *wlc, wlc_rateset_t *rs_arg);
static int wlc_wlrspec2ratespec(uint32 wlrspec, ratespec_t *pratespec);
static int wlc_ratespec2wlrspec(ratespec_t ratespec, uint32 *pwlrspec);

static int wlc_iovar_rangecheck(wlc_info_t *wlc, uint32 val, const bcm_iovar_t *vi);
static void wlc_mfbr_retrylimit_upd(wlc_info_t *wlc);

static int wlc_module_do_ioctl(wlc_info_t *wlc, int cmd, void *arg, int len, wlc_if_t *wlcif);

#if defined(BCMDBG)
static int wlc_dump_mempool(void *arg, struct bcmstrbuf *b);
#endif


#if defined(BCMDBG)
static int wlc_dump_bcntpls(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif


#ifdef BCMDBG
static void wlc_tsf_set(wlc_info_t *wlc, uint32 tsf_l, uint32 tsf_h);
#endif

#ifdef WLCNT
static void wlc_ctrupd_cache(uint16 cur_stat, uint16 *macstat_snapshot, uint32 *macstat);
#endif

#if defined(BCMDBG) || defined(WLTEST)
static int wlc_dump_list(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST) || \
	defined(TDLS_TESTBED) || defined(BCMDBG_AMPDU) || defined(MCHAN_MINIDUMP) || \
	defined(BCM_DNGDMP) || defined(BCMDBG_PHYDUMP)
static char* wlc_dump_next_name(char **buf);
static int wlc_dump_registered_name(wlc_info_t *wlc, char *name, struct bcmstrbuf *b);
int wlc_iovar_dump(wlc_info_t *wlc, const char *params, int p_len, char *out_buf, int out_len);
#ifdef WLTINYDUMP
static int wlc_tinydump(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif /* WLTINYDUMP */
#endif 

#if defined(BCMDBG)
static int wlc_dump_default(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_all(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_bssinfo_dump(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_wlc(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_ratestuff(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_mac(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_pio(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_dma(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_wme(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_stats(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_bss_info(const char *name, wlc_bss_info_t *bi, struct bcmstrbuf *b);
#endif 

#if defined(BCMDBG) || defined(BCMDBG_ERR)
static void wlc_print_event(wlc_info_t* wlc, wlc_event_t *e);
#endif

static void _wlc_event_if(wlc_info_t *wlc, wlc_event_t *e,
	wlc_bsscfg_t *src_bsscfg, wlc_if_t *src_wlfif, wlc_if_t *dst_wlcif);

/* send and receive */
static void wlc_pdu_push_txparams(wlc_info_t *wlc, void *p,
	uint32 flags, wlc_key_t *key, ratespec_t rate_override, uint fifo);

static uint16 wlc_compute_airtime(wlc_info_t *wlc, ratespec_t rspec, uint length);
static void wlc_compute_cck_plcp(ratespec_t rate, uint length, uint8 *plcp);
static void wlc_compute_ofdm_plcp(ratespec_t rate, uint length, uint8 *plcp);
static void wlc_compute_mimo_plcp(ratespec_t rate, uint length, uint8 *plcp);
static uint32 wlc_vht_siga1_get_partial_aid(void);
static vht_group_id_t wlc_compute_vht_groupid(uint16 fcs);
static void wlc_compute_vht_plcp(ratespec_t rate, uint length, uint16 fc, uint8 *plcp);

static int wlc_validate_mac(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *addr);

static uint wlc_calc_ack_time(wlc_info_t *wlc, ratespec_t rate, uint8 preamble_type);

/* interrupt, up/down, band */
static void wlc_setband(wlc_info_t *wlc, uint bandunit);
static chanspec_t wlc_init_chanspec(wlc_info_t *wlc);
static void wlc_bandinit_ordered(wlc_info_t *wlc, chanspec_t chanspec);
static void wlc_bsinit(wlc_info_t *wlc);

static void wlc_radio_hwdisable_upd(wlc_info_t* wlc);
static bool wlc_radio_monitor_start(wlc_info_t *wlc);
static void wlc_radio_timer(void *arg);
static bool wlc_radio_enable(wlc_info_t *wlc);
static bool wlc_mpccap(wlc_info_t* wlc);


static uint8 wlc_get_antennas(wlc_info_t *wlc);

static void wlc_nav_reset_war(wlc_info_t *wlc, bool enable);

static uint8 wlc_bss_scb_getcnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	bool (*scb_test_cb)(struct scb *scb));

#if ((defined(BCMDBG) || defined(WLMSG_PRPKT)) && defined(STA)) || defined(BCMDBG)
static const char* wlc_lookup_name(const wlc_id_name_table_t tbl, int id);
#endif

static void wlc_sta_tbtt(wlc_info_t *wlc);
static void wlc_ap_tbtt(wlc_info_t *wlc);

static void _wlc_bss_update_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void wlc_bss_update_dtim_period(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

/* ** STA-only routines ** */
#ifdef STA
static void _wlc_pspoll_timer(wlc_bsscfg_t *cfg);

static void wlc_sendpspoll_complete(wlc_info_t *wlc, void *pkt, uint txstatus);

static void wlc_radio_shutoff_dly_timer_upd(wlc_info_t *wlc);

static void wlc_pm2_radio_shutoff_dly_timer(void *arg);

static void wlc_bss_pm_pending_upd(wlc_bsscfg_t *cfg, uint txstatus);

static bool wlc_sendpmnotif(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct ether_addr *da, ratespec_t rate_override, int prio, bool track);

static bool _wlc_sendapsdtrigger(wlc_info_t *wlc, struct scb *scb);

static void wlc_iscan_timeout(void *arg);

static void wlc_wake_event_timer(void *arg);

/* PM2 tick timer functions.
 * Some are inline because they are used in the time-critical tx path
 */

static void wlc_restart_sta(wlc_info_t *wlc);
static void _wlc_set_wake_ctrl(wlc_info_t *wlc);
static void wlc_rateprobe_complete(wlc_info_t *wlc, void *pkt, uint txs);
static void wlc_rateprobe_scan(wlc_bsscfg_t *cfg);
static void wlc_tkip_countermeasures(wlc_info_t *wlc, void *pkt, uint txs);
static void wlc_pm_notif_complete(wlc_info_t *wlc, void *pkt, uint txs);

static void *wlc_frame_get_ps_ctl(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
                                  const struct ether_addr *bssid,
                                  const struct ether_addr *da);

static void wlc_freqtrack(wlc_info_t *wlc);
static void wlc_freqtrack_verify(wlc_info_t *wlc);

static uint8 wlc_bss_wdsscb_getcnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

#ifdef PHYCAL_CACHING
static int wlc_cache_cals(wlc_info_t *wlc);
static int wlc_cachedcal_sweep(wlc_info_t *wlc);
static int wlc_cachedcal_tune(wlc_info_t *wlc, uint16 chanspec);
#endif /* PHYCAL_CACHING */

#endif	/* STA */

static void wlc_process_eventq(void *arg);


static void wlc_wme_retries_write(wlc_info_t *wlc);

#if defined(DELTASTATS)
static void wlc_delta_stats_update(wlc_info_t *wlc);
static int wlc_get_delta_stats(wlc_info_t *wlc, wl_delta_stats_t *stats);
#endif

#ifdef WIFI_ACT_FRAME
static int wlc_tx_action_frame_now(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt);
#ifdef STA
static int wlc_send_action_frame_off_channel(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	uint32 channel, int32 dwell_time, struct ether_addr *bssid,
	wl_action_frame_t *action_frame);
#endif
static void *wlc_prepare_action_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	const struct ether_addr *bssid, void *action_frame);
static void wlc_actionframetx_complete(wlc_info_t *wlc, void *pkt, uint txstatus);
#endif /* WIFI_ACT_FRAME */

static bool wlc_attach_stf_ant_init(wlc_info_t *wlc);
static uint wlc_attach_module(wlc_info_t *wlc);

static void wlc_detach_module(wlc_info_t *wlc);
static void wlc_timers_deinit(wlc_info_t *wlc);

static void wlc_down_led_upd(wlc_info_t *wlc);
static uint wlc_down_del_timer(wlc_info_t *wlc);

static void wlc_ofdm_rateset_war(wlc_info_t *wlc);
static int _wlc_ioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

#if defined(BCM_DCS) && defined(STA)
static int wlc_send_action_brcm_dcs(wlc_info_t *wlc, wl_bcmdcs_data_t *data, struct scb *scb);
#endif

static void wlc_read_rt_dirmap(wlc_info_t *wlc);

static uint16 wlc_acphy_txctl0_calc(wlc_info_t *wlc, ratespec_t rspec, uint8 preamble);
static uint16 wlc_acphy_txctl1_calc(wlc_info_t *wlc, ratespec_t rspec,
	int8 lpc_offset, uint8 txpwr);
static uint16 wlc_acphy_txctl2_calc(wlc_info_t *wlc, ratespec_t rspec);

static void wlc_config_ucode_probe_resp(wlc_info_t *wlc);

#ifndef WLC_DISABLE_ACI
static void wlc_weakest_link_rssi_chan_stats_upd(wlc_info_t *wlc);
#endif

#if defined(WLTEST) || defined(WLPKTENG)
static void *wlc_tx_testframe_get(wlc_info_t *wlc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len);
#endif
#ifdef WLPM_BCNRX
static void wlc_pm_bcnrx_init(wlc_info_t *wlc);
#endif

static void wlc_fill_mgmt_hdr(struct dot11_management_header *hdr, uint16 fc,
	const struct ether_addr *da, const struct ether_addr *sa,
	const struct ether_addr *bssid);

static uint32 wlc_get_chan_info(wlc_info_t *wlc, uint16 old_chanspec);

static int wlc_srvsdb_force_set(wlc_info_t *wlc, uint8 val);

static int8 wlc_antgain_calc(int8 ag);
static void wlc_edcf_acp_ad_get_all(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp);
static int wlc_edcf_acp_ad_set_one(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp);
static void wlc_edcf_acp_get_all(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp);
static int
wlc_edcf_acp_set_one(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp, bool suspend);

static int wlc_scan_results(wlc_info_t *wlc, wl_scan_results_t *iob, int *len, uint results_state);
static int wlc_ioctl_filter(wlc_info_t *wlc, int cmd, void *arg, int len);

#if defined(WLTEST)
static int wlc_manf_info_get(wlc_info_t *wlc, char *buf, int len, struct wlc_if *wlcif);
#endif 

#if defined(WLTEST)
static int wlc_nvram_dump(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif 

#ifdef WME
static uint16 wlc_convert_acbitmap_to_precbitmap(ac_bitmap_t acbitmap);
#endif /* WME */

static void wlc_copy_peer_info(wlc_bsscfg_t *bsscfg, struct scb *scb, bss_peer_info_t *peer_info);

#ifdef STA
static void wlc_nulldp_tpl_upd(wlc_bsscfg_t *cfg);
#endif /* STA */

static uint32 _wlc_get_accum_pmdur(wlc_info_t *wlc);
#define FLOW_RING_CREATE	1
#define FLOW_RING_DELETE	2
#define FLOW_RING_FLUSH		3
#define FLOW_RING_OPEN 		4
#define FLOW_RING_CLOSED	5
#define FLOW_RING_FLUSED	6
#define FLOW_RING_TIM_SET	7
#define FLOW_RING_TIM_RESET	8


/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

#ifdef PKTQ_LOG
static int
wlc_pktq_stats(wl_iov_mac_full_params_t* full_params, uint8 params_version, struct bcmstrbuf* b,
               wl_iov_pktq_log_t* iov_pktq, wlc_info_t* wlc, wlc_bsscfg_t *cfg);
#endif /* PKTQ_LOG */


#ifdef BCMDBG_CTRACE
static int
wlc_pkt_ctrace_dump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	PKT_CTRACE_DUMP(wlc->osh, b);

	return 0;
}
#endif /* BCMDBG_CTRACE */

/**
 * returns channel info (20 MHZ channels only)
 * This used to pass in a ptr to a uint16 which was very similar to the new chanspec except
 * it did not have b/w or sideband info. Given that this routine has already been exposed via an
 * iovar then ignore the new extra info in the chanspec passed in and recreate a chanspec from
 * just the channel number this routine is used to figure out the number of scan channels, so
 * only report the 20MHZ channels.
 */
static uint32
wlc_get_chan_info(wlc_info_t *wlc, uint16 old_chanspec)
{
	uint32 result;
	uint32 dfs_chan_info;
	uint channel = CHSPEC_CHANNEL(old_chanspec);
	chanspec_t chspec = CH20MHZ_CHSPEC(channel);

	result = 0;
	if (channel && channel < MAXCHANNEL) {
		if ((channel <= CH_MAX_2G_CHANNEL) && isset(chanvec_all_2G.vec, channel))
			result |= WL_CHAN_VALID_HW;
		else if ((channel > CH_MAX_2G_CHANNEL) && isset(chanvec_all_5G.vec, channel))
			result |= WL_CHAN_VALID_HW | WL_CHAN_BAND_5G;
	}

	if (result & WL_CHAN_VALID_HW) {
		if (wlc_valid_chanspec_db(wlc->cmi, chspec))
			result |= WL_CHAN_VALID_SW;

		if ((AP_ENAB(wlc->pub)) && (wlc->dfs != NULL)) {
			dfs_chan_info = wlc_dfs_get_chan_info(wlc->dfs, channel);
			result |= dfs_chan_info;
		}

		if (result & WL_CHAN_VALID_SW) {
			if (wlc_radar_chanspec(wlc->cmi, chspec) == TRUE)
				result |= WL_CHAN_RADAR;
			if (wlc_restricted_chanspec(wlc->cmi, chspec))
				result |= WL_CHAN_RESTRICTED;
			if (wlc_quiet_chanspec(wlc->cmi, chspec))
				result |= WL_CHAN_PASSIVE;
		}
	}

	return (result);
}

#if defined(BCMDBG)
static int
wlc_dump_mempool(void *arg, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = (wlc_info_t *) arg;

	return (bcm_mpm_dump(wlc->mem_pool_mgr, b));
}
#endif   

#if defined(WLTEST)
static int
wlc_nvram_dump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	char *nvram_vars;
	const char *q = NULL;
	int err;

	/* per-device vars first, if any */
	if (wlc->pub->vars) {
		q = wlc->pub->vars;
		/* loop to copy vars which contain null separated strings */
		while (*q != '\0') {
			bcm_bprintf(b, "%s\n", q);
			q += strlen(q) + 1;
		}
	}

	/* followed by global nvram vars second, if any */
	if ((nvram_vars = MALLOC(wlc->osh, MAXSZ_NVRAM_VARS)) == NULL) {
		err = BCME_NOMEM;
		goto exit;
	}
	if ((err = nvram_getall(nvram_vars, MAXSZ_NVRAM_VARS)) != BCME_OK)
		goto exit;
	if (nvram_vars[0]) {
		q = nvram_vars;
		/* loop to copy vars which contain null separated strings */
		while (((q - nvram_vars) < MAXSZ_NVRAM_VARS) && *q != '\0') {
			bcm_bprintf(b, "%s\n", q);
			q += strlen(q) + 1;
		}
	}

	/* check empty nvram */
	if (q == NULL)
		err = BCME_NOTFOUND;
exit:
	if (nvram_vars)
		MFREE(wlc->osh, nvram_vars, MAXSZ_NVRAM_VARS);

	return err;
}
#endif	

#if defined(BCMDBG)
static void
wlc_dump_pm(wlc_info_t *wlc, struct bcmstrbuf *b)
{
#ifdef STA
	int idx;
	wlc_bsscfg_t *cfg;
	bcm_bprintf(b, "STAY_AWAKE() %d "
	            "SCAN_IN_PROGRESS() %d WLC_RM_IN_PROGRESS() %d AS_IN_PROGRESS() %d "
	            "wlc->wake %d wlc->PMpending %d wlc->PSpoll %d wlc->apsd_sta_usp %d\n",
	            STAY_AWAKE(wlc),
	            SCAN_IN_PROGRESS(wlc->scan), WLC_RM_IN_PROGRESS(wlc), AS_IN_PROGRESS(wlc),
	            wlc->wake, wlc->PMpending, wlc->PSpoll, wlc->apsd_sta_usp);
	bcm_bprintf(b, "wlc->PMawakebcn %d wlc->txpend16165war %d "
	            "wlc->check_for_unaligned_tbtt %d\n",
	            wlc->PMawakebcn, wlc->txpend16165war,
	            wlc->check_for_unaligned_tbtt);
	bcm_bprintf(b, "wlc->gptimer_stay_awake_req %d wlc->pm2_radio_shutoff_pending %d"
	            " wlc->user_wake_req %d BTA_ACTIVE() %d\n",
	            wlc->gptimer_stay_awake_req, wlc->pm2_radio_shutoff_pending, wlc->user_wake_req,
	            BTA_ACTIVE(wlc));
#ifdef WL11K
	bcm_bprintf(b, "wlc_rrm_inprog %d\n", wlc_rrm_inprog(wlc));
#endif
	FOREACH_BSS(wlc, idx, cfg) {
		wlc_pm_st_t *pm = cfg->pm;
		if (BSSCFG_STA(cfg) && (cfg->associated || BSS_TDLS_ENAB(wlc, cfg))) {
			bcm_bprintf(b, "bsscfg %d BSS %d PS_ALLOWED() %d WLC_PORTOPEN() %d "
		            "dtim_programmed %d PMpending %d priorPMstate %d PMawakebcn %d "
		            "WME_PM_blocked %d PM %d PMenabled %d PSpoll %d apsd_sta_usp %d "
		            "check_for_unaligned_tbtt %d PMblocked 0x%x\n",
		            WLC_BSSCFG_IDX(cfg), cfg->BSS, PS_ALLOWED(cfg), WLC_PORTOPEN(cfg),
		            cfg->dtim_programmed, pm->PMpending, pm->priorPMstate, pm->PMawakebcn,
		            pm->WME_PM_blocked, pm->PM, pm->PMenabled, pm->PSpoll, pm->PMpending,
		            pm->check_for_unaligned_tbtt, pm->PMblocked);
		}
	}
#endif /* STA */
}

#endif 

#if defined(BCMDBG)
static int
wlc_bssinfo_dump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int idx;
	wlc_bsscfg_t *cfg;

	bcm_bprintf(b, "\n");
	wlc_dump_bss_info("default_bss", wlc->default_bss, b);
	bcm_bprintf(b, "\n");
	FOREACH_BSS(wlc, idx, cfg) {
		bcm_bprintf(b, "bsscfg %d (0x%p):\n", idx, cfg);
		bcm_bprintf(b, "\n");
		wlc_dump_bss_info("target_bss", cfg->target_bss, b);
		bcm_bprintf(b, "\n");
		wlc_dump_bss_info("current_bss", cfg->current_bss, b);
	}
	return 0;
}

/** This function calls all dumps */
static int
wlc_dump_all(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int bcmerror = 0;
	dumpcb_t *d;

	for (d = wlc->dumpcb_head; d != NULL; d = d->next) {
		/* don't parse "all", else it will recurse */
		if ((strncmp(d->name, "all", sizeof("all") - 1) == 0))
			continue;
		/* don't print "list" option in "all" */
		if ((strncmp(d->name, "list", sizeof("list") - 1) == 0))
			continue;
		/* don't print "default" option in "all" */
		if ((strncmp(d->name, "default", sizeof("default") - 1) == 0))
			continue;
		/* phytbl & phytbl2 are too big to be included in all */
		if ((strncmp(d->name, "phytbl", sizeof("phytbl") - 1) == 0))
			continue;
		/* phytbl & phytbl2 are too big to be included in all */
		if ((strncmp(d->name, "phytbl2", sizeof("phytbl2") - 1) == 0))
			continue;

		bcm_bprintf(b, "\n%s:------\n", d->name);

		/* Continue if there's no BCME_BUFTOOSHORT error */
		bcmerror = d->dump_fn(d->dump_fn_arg, b);
		if (bcmerror == BCME_BUFTOOSHORT)
			return bcmerror;
	}

	return 0;
}

#if defined(WLC_HIGH_ONLY) && defined(BCMDBG)
static int
wlc_dump_rpcpktlog(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint32 buf[RPC_PKTLOG_DUMP_SIZE];
	int ret;
	int i;

	ASSERT(wlc->rpc);

	ret = bcm_rpc_pktlog_get(wlc->rpc, buf, RPC_PKTLOG_DUMP_SIZE, TRUE);
	if (ret < 0)
		return ret;

	bcm_bprintf(b, "Transmit log %d:\n", ret);
	for (i = 0; i < ret; i++) {
		bcm_bprintf(b, "[%d] trans 0x%x len %d ID %s\n", i,
			buf[i*RPC_PKTLOG_RD_LEN+1], buf[i*RPC_PKTLOG_RD_LEN+2],
			WLC_RPC_ID_LOOKUP(rpc_name_tbl, buf[i*RPC_PKTLOG_RD_LEN]));
	}

	ret = bcm_rpc_pktlog_get(wlc->rpc, buf, RPC_PKTLOG_DUMP_SIZE, FALSE);

	if (ret <= 0)
		return ret;

	bcm_bprintf(b, "Recv log %d:\n", ret);
	for (i = 0; i < ret; i++) {
		bcm_bprintf(b, "[%d] trans 0x%x len %d ID %s\n", i,
			buf[i*RPC_PKTLOG_RD_LEN+1], buf[i*RPC_PKTLOG_RD_LEN+2],
			WLC_RPC_ID_LOOKUP(rpc_name_tbl, buf[i*RPC_PKTLOG_RD_LEN]));
	}

	return 0;
}

static int
wlc_dump_rpc(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	return bcm_rpc_dump(wlc->rpc, b);
}
#endif /* WLC_HIGH_ONLY && BCMDBG */

/** Format a general info dump */
static int
wlc_dump_default(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	char *name_list;
	int name_list_len;
	struct bcmstrbuf names;
	char *name;
	char *p;
	int err = 0;

	/* create the name list for a default dump */
	name_list_len = 128;
	name_list = (char*)MALLOC(wlc->osh, name_list_len);
	if (!name_list)
		return BCME_NOMEM;

	bcm_binit(&names, name_list, name_list_len);

	bcm_bprintf(&names, "wlc phystate bsscfg bssinfo ratestuff stats ");

	if (wlc->clk)
		bcm_bprintf(&names, "pio dma ");

	if (EDCF_ENAB(wlc->pub) && wlc->pub->up)
		bcm_bprintf(&names, "wme ");

	if (AMPDU_ENAB(wlc->pub))
		bcm_bprintf(&names, "ampdu ");
#ifdef WET
	if (wlc->wet)
		bcm_bprintf(&names, "wet ");
#endif /* WET */

	if (TOE_ENAB(wlc->pub))
		bcm_bprintf(&names, "toe ");

#ifdef WLLED
	if (wlc->ledh)
		bcm_bprintf(&names, "led ");
#endif

#ifdef WLAMSDU
	/* only dump amsdu if we were handed a large dump buffer */
	if (b->size > 8000 &&
	    (AMSDU_TX_ENAB(wlc->pub) || wlc->_amsdu_rx))
		bcm_bprintf(&names, "amsdu ");
#endif


	if (CAC_ENAB(wlc->pub))
		bcm_bprintf(&names, "cac ");

#ifdef TRAFFIC_MGMT
	if (TRAFFIC_MGMT_ENAB(wlc->pub)) {
		bcm_bprintf(&names, "trfmgmt_stats ");
		bcm_bprintf(&names, "trfmgmt_shaping ");
	}
#endif

	/* dump the list */
	p = name_list;
	while ((name = wlc_dump_next_name(&p)) != NULL) {
		bcm_bprintf(b, "\n%s:------\n", name);
		err = wlc_dump_registered_name(wlc, name, b);
		if (err)
			break;
	}

	MFREE(wlc->osh, name_list, name_list_len);

	return err;
}
#endif 

#if defined(BCMDBG) || defined(WLTEST)
/** This function lists all dump option */
static int
wlc_dump_list(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	dumpcb_t *ptr;

	bcm_bprintf(b, "\nRegistered dumps:\n");

	for (ptr = wlc->dumpcb_head; ptr != NULL; ptr = ptr->next) {
		bcm_bprintf(b, "%s\n", ptr->name);
	}

	return 0;
}
#endif 

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST) || \
	defined(TDLS_TESTBED) || defined(BCMDBG_AMPDU) || defined(MCHAN_MINIDUMP) || \
	defined(BCM_DNGDMP) || defined(BCMDBG_PHYDUMP)
int
wlc_iovar_dump(wlc_info_t *wlc, const char *params, int p_len, char *out_buf, int out_len)
{
	struct bcmstrbuf b;
	char *name_list;
	int name_list_len;
	char *name1;
	char *name;
	const char *p;
	const char *endp;
	int err = 0;
	char *name_list_ptr;

	bcm_binit(&b, out_buf, out_len);
	p = params;
	endp = p + p_len;

	/* find the dump name list length to make a copy */
	while (p != endp && *p != '\0')
		p++;

	/* return an err if the name list was not null terminated */
	if (p == endp)
		return BCME_BADARG;

	/* copy the dump name list to a new buffer since the output buffer
	 * may be the same memory as the dump name list
	 */
	name_list_len = (int) ((const uint8*)p - (const uint8*)params + 1);
	name_list = (char*)MALLOC(wlc->osh, name_list_len);
	if (!name_list)
	      return BCME_NOMEM;
	bcopy(params, name_list, name_list_len);

	name_list_ptr = name_list;

	/* get the first two dump names */
	name1 = wlc_dump_next_name(&name_list_ptr);
	name = wlc_dump_next_name(&name_list_ptr);

	/* if the dump list was empty, return the default dump */
	if (name1 == NULL) {
		WL_ERROR(("doing default dump\n"));
#if defined(WLTEST) && defined(WLTINYDUMP)
		err = wlc_tinydump(wlc, &b);
#elif defined(BCMDBG)
		err = wlc_dump_default(wlc, &b);
#endif /* defined(WLTEST) && defined(WLTINYDUMP) */
		goto exit;
	}

	/* dump the first section
	 * only print the separator if there are more than one dump
	 */
	if (name != NULL)
		bcm_bprintf(&b, "\n%s:------\n", name1);
	err = wlc_dump_registered_name(wlc, name1, &b);
	if (err)
		goto exit;

	/* dump the rest */
	while (name != NULL) {
		bcm_bprintf(&b, "\n%s:------\n", name);
		err = wlc_dump_registered_name(wlc, name, &b);
		if (err)
			break;

		name = wlc_dump_next_name(&name_list_ptr);
	}

exit:
	MFREE(wlc->osh, name_list, name_list_len);

	/* make sure the output is at least a null terminated empty string */
	if (b.origbuf == b.buf && b.size > 0)
		b.buf[0] = '\0';

	return err;
}

static char*
wlc_dump_next_name(char **buf)
{
	char *p;
	char *name;

	p = *buf;

	if (p == NULL)
		return NULL;

	/* skip leading space */
	while (bcm_isspace(*p) && *p != '\0')
		p++;

	/* remember the name start position */
	if (*p != '\0')
		name = p;
	else
		name = NULL;

	/* find the end of the name
	 * name is terminated by space or null
	 */
	while (!bcm_isspace(*p) && *p != '\0')
		p++;

	/* replace the delimiter (or '\0' character) with '\0'
	 * and set the buffer pointer to the character past the delimiter
	 * (or to NULL if the end of the string was reached)
	 */
	if (*p != '\0') {
		*p++ = '\0';
		*buf = p;
	} else {
		*buf = NULL;
	}

	/* return the pointer to the name */
	return name;
}

/** Dump all matching the given name */
static int
wlc_dump_registered_name(wlc_info_t *wlc, char *name, struct bcmstrbuf *b)
{
	dumpcb_t *dumpcb;
	int err = 0;
	int rv = BCME_UNSUPPORTED; /* If nothing found, return this. */

	/* find the given dump name */
	for (dumpcb = wlc->dumpcb_head; dumpcb != NULL; dumpcb = dumpcb->next) {
		if (!strcmp(name, dumpcb->name)) {
			if (rv == BCME_UNSUPPORTED) { /* Found one */
				rv = BCME_OK;
			}
			err = dumpcb->dump_fn(dumpcb->dump_fn_arg, b);
			if (b->size == 0) { /* check for output buffer overflow */
				rv = BCME_BUFTOOSHORT;
				break;
			}
			if (err != 0) { /* Record last non successful error code */
				rv = err;
			}
		}
	}

	if (rv == BCME_UNSUPPORTED)
		rv = wlc_bmac_dump(wlc->hw, name, b);

	return rv;
}
#endif 

bool
wlc_rminprog(wlc_info_t *wlc)
{
	return WLC_RM_IN_PROGRESS(wlc);
}

#ifdef STA
uint32
wlc_get_mpc_dur(wlc_info_t *wlc)
{
	/* hw_up=1 when out of MPC & hw_up=0 when in MPC.  wlc->mpc_dur is updated when coming
	 * out of MPC and wlc->mpc_laston_ts is updated when going into MPC. In MPC mode, the sum of
	 * last mpc_dur and time since we have been in MPC currently gives the total mpc_dur.
	 */
	if ((wlc->mpc == 0) || wlc->pub->hw_up)
		return wlc->mpc_dur;
	return wlc->mpc_dur + (OSL_SYSUPTIME() - wlc->mpc_laston_ts);
}

/** conditions under which the chip is kept awake */
bool
wlc_stay_awake(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	bool ret = FALSE;

	/* stay awake when one of these global conditions meets */
	if (wlc->wake ||
	    wlc->PMawakebcn ||
	    BTA_ACTIVE(wlc) ||
	    SCAN_IN_PROGRESS(wlc->scan) ||
	    WLC_RM_IN_PROGRESS(wlc) ||
#ifdef WL11K
	    wlc_rrm_inprog(wlc) ||
#endif /* WL11K */
	    AS_IN_PROGRESS_WLC(wlc) ||
	    wlc->PMpending ||
	    wlc->PSpoll ||
	    wlc->check_for_unaligned_tbtt ||
	    wlc->apsd_sta_usp ||
	    wlc->txpend16165war ||
	    wlc->pm2_radio_shutoff_pending ||
	    wlc->gptimer_stay_awake_req ||
	    wlc->monitor != 0 ||
	    wlc->user_wake_req != 0) {
		ret = TRUE;
		goto done;
	}

	/* stay awake as soon as we bring up a non-P2P AP bsscfg */
	FOREACH_UP_AP(wlc, idx, cfg) {
#ifdef WLP2P
		if (!BSS_P2P_ENAB(wlc, cfg))
#endif
		{
			ret = TRUE;
			goto done;
		}
	}

	/* stay awake as soon as we bring up a non-primary STA bsscfg */
	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg != wlc->cfg &&
#ifdef WLMCNX
		    !MCNX_ENAB(wlc->pub) &&
#endif
		    TRUE)
		{
			ret = TRUE;
			goto done;
		}
	}

#ifdef WLTDLS
	FOREACH_BSS(wlc, idx, cfg) {
		if (BSS_TDLS_ENAB(wlc, cfg)) {
			if (wlc_tdls_stay_awake(wlc, cfg)) {
				ret = TRUE;
				goto done;
			}
		}
	}
#endif

	/* stay awake as long as an infra STA is associating */
	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->BSS) {
			if (cfg->assoc != NULL &&
			    cfg->assoc->state != AS_IDLE)
			{
				ret = TRUE;
				goto done;
			}
		}
	}

done:
#ifdef WL_PWRSTATS
	if (PWRSTATS_ENAB(wlc->pub))
		wlc_pwrstats_wake_reason_upd(wlc, ret);
#endif /* WL_PWRSTATS */
	return ret;
}

/** conditions under which the PM bit should be set in outgoing frames. */
bool
wlc_ps_allowed(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;

#ifdef PSTA
	if (PSTA_ENAB(wlc->pub) && !wlc_wlancoex_on(wlc))
		return FALSE;
#endif

#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, cfg))
		return pm->PMenabled;
#endif

#ifdef WL_PM_MUTE_TX
	if (wlc->pm_mute_tx->state != DISABLED_ST) {
		return pm->PMenabled;
	}
#endif

	/* when ps request comes form the mchan module
	 * during switch, we should always set ps
	 */
	if (mboolisset(pm->PMenabledModuleId, WLC_BSSCFG_PS_REQ_MCHAN)) {
		ASSERT(pm->PMenabled);
		return TRUE;
	}

	/* disallow PS when one of the following bsscfg specific conditions meets */
	if (!cfg->BSS ||
	    !cfg->associated ||
	    !pm->PMenabled ||
	    pm->PM_override ||
	    !cfg->dtim_programmed ||
#ifdef WLTDLS
	    !WLC_TDLS_PM_ALLOWED(wlc, cfg) ||
#endif
	    !WLC_PORTOPEN(cfg))
		return FALSE;
	/* disallow PS when it is a non-primary STA bsscfg */
	if (cfg != wlc->cfg &&
#ifdef WLMCNX
	    !MCNX_ENAB(wlc->pub) &&
#endif
	    TRUE)
		return FALSE;

	return TRUE;
}

bool
wlc_associnprog(wlc_info_t *wlc)
{
	return AS_IN_PROGRESS(wlc);
}
#endif /* STA */

#if defined(WLTINYDUMP)
/* APSTA: TODO: Add APSTA fields for tinydump */
/** Tiny dump that is available when full debug dump functionality is not compiled in */
static int
wlc_tinydump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	char perm[32], cur[32];
	char ssidbuf[SSID_FMT_BUF_LEN];
	int i;
	wlc_bsscfg_t *bsscfg;

	wl_dump_ver(wlc->wl, b);

	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "resets %d\n", WLCNTVAL(wlc->pub->_cnt->reset));

	bcm_bprintf(b, "perm_etheraddr %s cur_etheraddr %s\n",
		bcm_ether_ntoa(&wlc->perm_etheraddr, perm),
		bcm_ether_ntoa(&wlc->pub->cur_etheraddr, cur));

	bcm_bprintf(b, "board 0x%x, board rev %s", wlc->pub->sih->boardtype,
	            bcm_brev_str(wlc->pub->boardrev, cur));
	if (wlc->pub->boardrev == BOARDREV_PROMOTED)
		bcm_bprintf(b, " (actually 0x%02x)", BOARDREV_PROMOTABLE);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "rate_override: A %d, B %d\n",
		wlc->bandstate[BAND_5G_INDEX]->rspec_override,
		wlc->bandstate[BAND_2G_INDEX]->rspec_override);

	bcm_bprintf(b, "ant_rx_ovr %d txant %d\n", wlc->stf->ant_rx_ovr, wlc->stf->txant);

	FOREACH_BSS(wlc, i, bsscfg) {
		char ifname[32];
		wlc_key_info_t bss_key_info;

		(void)wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE,
			&bss_key_info);
		bcm_bprintf(b, "\n");
		wlc_format_ssid(ssidbuf, bsscfg->SSID, bsscfg->SSID_len);
		strncpy(ifname, wl_ifname(wlc->wl, bsscfg->wlcif->wlif), sizeof(ifname));
		ifname[sizeof(ifname) - 1] = '\0';
		bcm_bprintf(b, "BSS Config %d: \"%s\"\n", i, ssidbuf);

		bcm_bprintf(b, "enable %d up %d wlif 0x%p \"%s\"\n",
		            bsscfg->enable,
		            bsscfg->up, bsscfg->wlcif->wlif, ifname);
		bcm_bprintf(b, "wsec 0x%x auth %d wsec_index %d wep_algo %d\n",
		            bsscfg->wsec,
		            bsscfg->auth, bss_key_info.key_id, bss_key_info.algo);

		bcm_bprintf(b, "current_bss->BSSID %s\n",
		            bcm_ether_ntoa(&bsscfg->current_bss->BSSID, (char*)perm));

		wlc_format_ssid(ssidbuf, bsscfg->current_bss->SSID,
		                bsscfg->current_bss->SSID_len);
		bcm_bprintf(b, "current_bss->SSID \"%s\"\n", ssidbuf);

#ifdef STA
		/* STA ONLY */
		if (!BSSCFG_STA(bsscfg))
			continue;

		bcm_bprintf(b, "bsscfg %d assoc_state %d\n", WLC_BSSCFG_IDX(bsscfg),
		            bsscfg->assoc->state);
#endif /* STA */
	}
	bcm_bprintf(b, "\n");

#ifdef STA
	bcm_bprintf(b, "AS_IN_PROGRESS() %d stas_associated %d\n", AS_IN_PROGRESS(wlc),
	            wlc->stas_associated);
#endif /* STA */

	bcm_bprintf(b, "aps_associated %d\n", wlc->aps_associated);
	FOREACH_UP_AP(wlc, i, bsscfg)
	        bcm_bprintf(b, "BSSID %s\n", bcm_ether_ntoa(&bsscfg->BSSID, (char*)perm));

	return 0;
}
#endif /* WLTINYDUMP */

#ifdef BCMDBG
static void
wlc_dump_perf_stats(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	wlc_perf_stats_t *cnt = &wlc->perf_stats;
	wlc_isr_stats_t *ints = &cnt->isr_stats;
	uint32 i;

	const char * int_names[32] =
	{
		"MACSSPNDD     ",
		"BCNTPL        ",
		"TBTT          ",
		"BCNSUCCESS    ",
		"BCNCANCLD     ",
		"ATIMWINEND    ",
		"PMQ           ",
		"NSPECGEN_0    ",
		"NSPECGEN_1    ",
		"MACTXERR      ",
		"NSPECGEN_3    ",
		"PHYTXERR      ",
		"PME           ",
		"GP0           ",
		"GP1           ",
		"DMAINT        ",
		"TXSTOP        ",
		"CCA           ",
		"BG_NOISE      ",
		"DTIM_TBTT     ",
		"PRQ           ",
		"PWRUP         ",
		"BT_RFACT_STUCK",
		"BT_PRED_REQ   ",
		"INT_24        ",
		"P2P           ",
		"INT_26        ",
		"INT_27        ",
		"RFDISABLE     ",
		"TFS           ",
		"PHYCHANGED    ",
		"TO            "
	};

#ifdef WLP2P
	const char * p2p_int_names[32] =
	{
		"PRE_TBTT",
		"CTW_END ",
		"ABS     ",
		"PRS     "
	};
#endif
	/* Print perf stats */

	bcm_bprintf(b, "\nGeneral Performance Stats:-\n");

	bcm_bprintf(b,
		"\nisr        : %d"
		"\ndpc        : %d"
		"\ntimer dpc  : %d"
		"\nbcn isr    : %d"
		"\nbeacons    : %d"
		"\nprobe req  : %d"
		"\nprobe resp : %d\n",
			cnt->n_isr, cnt->n_dpc, cnt->n_timer_dpc,
			cnt->n_bcn_isr, cnt->n_beacons, cnt->n_probe_req, cnt->n_probe_resp);

	bcm_bprintf(b, "\nInterrupt       num  percent");

	for (i = 0; i < 32; i++) {
		if (ints->n_counts[i]) {
			bcm_bprintf(b, "\n%s	%d	%d", int_names[i], ints->n_counts[i],
				((ints->n_counts[i])*100)/(cnt->n_isr));
		}
	}
	bcm_bprintf(b, "\n");

#ifdef WLP2P
	if (P2P_ENAB(wlc->pub) && ints->n_counts[25]) {
		bcm_bprintf(b, "\nP2P Interrupt   num  percent");

		for (i = 0; i < M_P2P_I_BLK_SZ; i++) {
			bcm_bprintf(b, "\n%s	%d	%d", p2p_int_names[i], ints->n_p2p[i],
				((ints->n_p2p[i])*100)/(ints->n_counts[25]));
		}
		bcm_bprintf(b, "\n");
	}
#endif
}

#endif /* BCMDBG */

void
BCMINITFN(wlc_reset)(wlc_info_t *wlc)
{
	WL_TRACE(("wl%d: wlc_reset\n", wlc->pub->unit));

	wlc->check_for_unaligned_tbtt = FALSE;

#ifdef STA
	/* Turn off PM mode for any stations that are up so we can utilize
	   unaligned tbtt to recover tsf related values.
	   With PM enabled before wlc_reset() is called, it seems that the
	   driver does not come out of PM mode properly afterwards and is
	   unable to receive beacons when wlc_phy_chanspec_set() doesn't get called.
	*/
	if (wlc->pub->associated) {
		int i;
		wlc_bsscfg_t *bsscfg;

		FOREACH_AS_STA(wlc, i, bsscfg) {
			if (bsscfg->up) {
				wlc_set_pmstate(bsscfg, FALSE);
				wlc_set_uatbtt(bsscfg, TRUE);
				WL_PS(("%s: turn off pm mode\n", __FUNCTION__));
				wlc->reset_triggered_pmoff = TRUE;
			}
		}
#if defined(WLOFFLD) && defined(UCODE_SEQ)
		if (WLOFFLD_CAP(wlc)) {
			wlc_ol_update_seq_iv(wlc, TRUE, NULL);
		}
#endif /* WLOFFLD  && UCODE_SEQ */
	}
#endif /* STA */

	/* slurp up hw mac counters before core reset */
	if (WLC_UPDATE_STATS(wlc)) {
		wlc_statsupd(wlc);
		/* reset our snapshot of macstat counters */
		bzero((char*)wlc->core->macstat_snapshot, sizeof(macstat_t));
	}
#ifdef WLAMSDU_TX
	/* clear amsdu agg so pkts don't come out during reset/before init */
	wlc_amsdu_agg_flush(wlc->ami);
#endif

	wlc_bmac_reset(wlc->hw);

#ifdef WLC_LOW
#ifdef STA
	if (D11REV_GE(wlc->pub->corerev, 41)) {
		/* preserve original PM state upon a reset to save power.
		 * this is because, wlc_bmac_reset() would have unconditionally
		 * turned on the wlc_bmac_core_phypll_ctl().
		 */
		if (wlc->cfg->pm->PM != PM_OFF) {
			wlc_bmac_core_phypll_ctl(wlc->hw, FALSE);
		}
	}
#endif /* STA */
#endif /* WLC_LOW */

#ifdef WLOFFLD
	if (WLOFFLD_ENAB(wlc->pub) && wlc->pub->up)
		wlc_ol_down(wlc->ol);
#endif

	wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_WLC_DOWN, NULL, NULL, NULL, NULL);

#ifdef WLAMPDU
	wlc_ampdu_tx_reset(wlc->ampdu_tx);
#endif /* WLAMPDU */

#ifdef PSTA
	if (PSTA_ENAB(wlc->pub))
		wlc_psta_disable_all(wlc->psta);
#endif /* PSTA */

	wlc->txretried = 0;

#ifdef WLC_HIGH_ONLY
	/* Need to set a flag(to be cleared asynchronously by BMAC driver with high call)
	 *  in order to prevent wlc_rpctx_txreclaim() from screwing wlc_rpctx_getnexttxp(),
	 *  which could be invoked by already QUEUED high call(s) from BMAC driver before
	 *  wlc_bmac_reset() finishes.
	 * It's not needed before in monolithic driver model because d11core interrupts would
	 *  have been cleared instantly in wlc_bmac_reset() and no txstatus interrupt
	 *  will come to driver to fetch those flushed dma pkt pointers.
	 */
	wlc->reset_bmac_pending = TRUE;

	wlc_rpctx_txreclaim(wlc->rpctx);

	wlc_stf_phy_txant_upd(wlc);
	phy_antdiv_set_rx((phy_info_t *)WLC_PI(wlc), wlc->stf->ant_rx_ovr);
#endif

#ifdef NEW_TXQ
	/* Clear out all packets in the low_txq
	 *
	 * The wlc_bmac_reset() call flushes all pkts on the DMA ring. Or in the case of a split
	 * driver (WLC_HIGH_ONLY), wlc_rpctx_txreclaim() would have flushed all pending packets
	 * queued for wlc_rpctx_tx().
	 * This call will flush any remaining packets in the low_txq software queue and
	 * reset the low_txq state.
	 */
	wlc_low_txq_flush(wlc->txqi, wlc->active_queue->low_txq);
#endif /* NEW_TXQ */
}

void
wlc_fatal_error(wlc_info_t *wlc)
{
	uint32 gptime;
#ifdef STA
	bool PMenabled = 0;
#endif
	WLCNTINCR(wlc->pub->_cnt->reinit);
	WL_ERROR(("wl%d: fatal error, reinitializing\n", wlc->pub->unit));

	/* cache gptime out count */
	gptime = wlc_hrt_gptimer_get(wlc);
#ifdef STA
	if (wlc->cfg) {
		PMenabled = wlc->cfg->pm->PMenabled;
		/* put the PM into known state and insync */
		wlc_set_pmenabled(wlc->cfg, PMenabled);
	}
#endif
	wl_init(wlc->wl);
	/* restore gptime out count after init (gptimer is reset due to wlc_reset */
	wlc_hrt_gptimer_set(wlc, gptime);
#ifdef STA
	if (wlc->cfg) {
		/* restore last known PM state */
		wlc_set_pmenabled(wlc->cfg, PMenabled);
	}
#endif
}

/* Return the channel the driver should initialize during wlc_init.
 * the channel may have to be changed from the currently configured channel
 * if other configurations are in conflict (bandlocked, 11n mode disabled,
 * invalid channel for current country, etc.)
 */
static chanspec_t
BCMINITFN(wlc_init_chanspec)(wlc_info_t *wlc)
{
	chanspec_t chanspec;

	/* start with the default_bss channel since it store the last
	 * channel set by ioctl or iovar, or default to 2G
	 */
	chanspec = wlc->default_bss->chanspec;


	/* Sanitize user setting for 80+80 MHz against current settings */
	if (CHSPEC_IS8080_UNCOND(chanspec) &&
		!VALID_8080CHANSPEC(wlc, chanspec)) {
		/* select the 80 0MHz primary channel in case 80 is allowed */
		chanspec = wf_chspec_primary80_chspec(chanspec);
	}

	/* Sanitize user setting for 160 MHz against current settings */
	if (CHSPEC_IS160_UNCOND(chanspec) &&
		!VALID_160CHANSPEC(wlc, chanspec)) {
		/* select the 80MHz primary channel in case 80 is allowed */
		chanspec = wf_chspec_primary80_chspec(chanspec);
	}

	/* Sanitize user setting for 80MHz against current settings */
	if (CHSPEC_IS80_UNCOND(chanspec) &&
		!VALID_80CHANSPEC(wlc, chanspec)) {
		/* select the 40MHz primary channel in case 40 is allowed */
		chanspec = wf_chspec_primary40_chspec(chanspec);
	}

	/* Sanitize user setting for 40MHz against current settings */
	if (CHSPEC_IS40_UNCOND(chanspec) &&
	    (!N_ENAB(wlc->pub) || !VALID_40CHANSPEC_IN_BAND(wlc, CHSPEC_WLCBANDUNIT(chanspec))))
		chanspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan(chanspec));

	/* make sure the channel is on the supported band if we are band-restricted */
	if (wlc->bandlocked || NBANDS(wlc) == 1) {
		/* driver is configured for the current band only,
		 * pick another channel if the chanspec is not on the current band.
		 */
		if (CHSPEC_WLCBANDUNIT(chanspec) != wlc->band->bandunit)
			chanspec = wlc_default_chanspec(wlc->cmi, TRUE);
	}

	/* validate channel */
	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		wlcband_t *save_band = wlc->band;

		wlc_pi_band_update(wlc, CHSPEC_WLCBANDUNIT(chanspec));
		chanspec = wlc_default_chanspec(wlc->cmi, TRUE);
		wlc_pi_band_update(wlc, save_band->bandunit);
	}

	return chanspec;
}

/** (re)initializes MAC + PHY during 'wl up' or after a serious fault condition */
void BCMINITFN(wlc_init)(wlc_info_t *wlc)
{
	d11regs_t *regs;
	chanspec_t chanspec;
	int i;
	wlc_bsscfg_t *bsscfg;
	bool mute = FALSE;
	bool sar = FALSE;
	int ac;
	uint16 temp;
	int min_txpower = 0;
	wlcband_t * band;
	wlc_phy_t * pi;
#ifdef STA
	int idx;
	wlc_bsscfg_t *cfg;
#endif

	WL_TRACE(("wl%d: wlc_init\n", wlc->pub->unit));

	regs = wlc->regs;

	/* This will happen if a big-hammer was executed. In that case, we want to go back
	 * to the channel that we were on and not new channel
	 */
	if (wlc->pub->associated)
		chanspec = wlc->home_chanspec;
	else
		chanspec = wlc_init_chanspec(wlc);

	band = wlc->band;

	/* Choose the right bandunit in case it is not yet synchronized to the chanspe */
	if (NBANDS(wlc) > 1 && band->bandunit != CHSPEC_WLCBANDUNIT(chanspec))
		band = wlc->bandstate[OTHERBANDUNIT(wlc)];

	/* Update bss rates to the band specific default rate set */
	wlc_rateset_default(&wlc->default_bss->rateset, &band->defrateset, band->phytype,
		band->bandtype, FALSE, RATE_MASK_FULL, wlc_get_mcsallow(wlc, NULL),
		CHSPEC_WLC_BW(chanspec), wlc->stf->op_rxstreams);

	/* Mute the phy when 11h is enabled and we are operating on radar
	 * sensitive channel.
	 */
	if (WL11H_AP_ENAB(wlc) && wlc_radar_chanspec(wlc->cmi, chanspec))
		mute = TRUE;
#ifdef WL_SARLIMIT
	/* initialize SAR limit per SROM */
	wlc_channel_sar_init(wlc->cmi);
#endif /* WL_SARLIMIT */
	wlc_bmac_init(wlc->hw, chanspec, mute, 0);
#ifdef WLC_LOW
#if !defined(BCMDONGLEHOST)
#if !defined(SAVERESTORE) || (defined(SAVERESTORE) && defined(SAVERESTORE_DISABLED))
	if ((CHIPID(wlc->pub->sih->chip) == BCM4345_CHIP_ID) ||
		(CHIPID(wlc->pub->sih->chip) == BCM4335_CHIP_ID)) {
		si_force_islanding(wlc->hw->sih, FALSE);
		si_update_masks(wlc->hw->sih);

		/* Calculating fast power up delay again as min_res_mask is changed now */
		wlc->hw->fastpwrup_dly = si_clkctl_fast_pwrup_delay(wlc->hw->sih);
		W_REG(wlc->osh, &wlc->regs->u.d11regs.scc_fastpwrup_dly, wlc->hw->fastpwrup_dly);
		si_pcie_set_ctrlreg(wlc->pub->sih, PCIE_DISSPROMLD, PCIE_DISSPROMLD);
	}
#endif
#endif /* !defined(BCMDONGLEHOST) */
#endif /* WLC_LOW */

	/* read rate table direct address maps from shm */
	wlc_read_rt_dirmap(wlc);

#ifdef STA
	/* update beacon listen interval */
	wlc_bcn_li_upd(wlc);

	/* maximum time to wait for beacon after wakeup from doze (at pretbtt).
	 * read the initial ucode config only...
	 * SHM init value is in 8us unit.
	 */
	if (wlc->bcn_wait_prd == 0) {
		wlc->bcn_wait_prd = wlc_bmac_read_shm(wlc->hw, M_NOSLPZNATDTIM) << 3 >> 10;
		ASSERT(wlc->bcn_wait_prd > 0);
	}
#endif

	/* the world is new again, so is our reported rate */
	wlc_reprate_init(wlc);

	/* Call any registered UP handlers */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (wlc->modulecb[i].up_fn == NULL)
			continue;
		WL_INFORM(("wl%d: %s: calling UP callback for \"%s\"\n",
		           wlc->pub->unit, __FUNCTION__, wlc->modulecb[i].name));
		if (wlc->modulecb[i].up_fn(wlc->modulecb_data[i].hdl) != BCME_OK) {
			WL_ERROR(("wl%d: %s UP handler error\n",
			          wlc->pub->unit, wlc->modulecb[i].name));
			/* continue??? */
		}
	}

	/* write ethernet address to core */
	FOREACH_BSS(wlc, i, bsscfg) {
		wlc_set_mac(bsscfg);
		wlc_set_bssid(bsscfg);
	}

	wlc_bandinit_ordered(wlc, chanspec);

	pi = WLC_PI(wlc);

	/* init probe response timeout */
	wlc_write_shm(wlc, M_PRS_MAXTIME, wlc->prb_resp_timeout);

	/* init max burst txop (framebursting) */
	wlc_write_shm(wlc, M_MBURST_TXOP,
	              (WLC_HT_GET_RIFS(wlc->hti) ? (EDCF_AC_VO_TXOP_AP << 5) : MAXFRAMEBURST_TXOP));
	/* in case rifs was set when not up, need to run war here */
	wlc_phy_tkip_rifs_war(pi, WLC_HT_GET_RIFS(wlc->hti));

#ifdef WL11N
	wlc_set_pwrthrottle_config(wlc);
#endif /* WL11N */
	if (wlc->pub->sih->boardvendor == VENDOR_APPLE) {
		if (wlc->pub->sih->chip == BCM4331_CHIP_ID) {
			min_txpower = BCM94331X19_MINTXPOWER;
			sar = TRUE;
		} else if (wlc->pub->sih->chip == BCM4360_CHIP_ID ||
			wlc->pub->sih->chip == BCM43460_CHIP_ID ||
			wlc->pub->sih->chip == BCM43526_CHIP_ID ||
		        wlc->pub->sih->chip == BCM4352_CHIP_ID) {
		         min_txpower = BCM94360_MINTXPOWER;
		} else  {
			min_txpower = ARPT_MODULES_MINTXPOWER;
		}
	} else if ((wlc->pub->sih->boardvendor == VENDOR_BROADCOM) &&
	           ((wlc->pub->sih->boardtype == BCM94331PCIEBT4_SSID) ||
	            (wlc->pub->sih->boardtype == BCM94331CS_SSID))) {
		min_txpower = BCM94331X19_MINTXPOWER;
		sar = TRUE;
	}
	if (min_txpower)
		wlc_iovar_setint(wlc, "min_txpower", min_txpower);
	wlc_iovar_setint(wlc, "sar_enable", sar);
#ifdef WLOFFLD
	/* initialize curpwr_cache array */
	wlc_ol_curpwr_upd(wlc->ol, 0, 0);
#endif
#if defined(BCMDBG) || defined(WLTEST)
	if ((wlc->pub->sih->chip == BCM4331_CHIP_ID &&
	    wlc->pub->sih->boardtype == BCM94331CS_SSID &&
	    wlc->pub->sih->boardrev == 0x1202)) {
		wlc_iovar_setint(wlc, "txchain", 5);
		wlc_iovar_setint(wlc, "rxchain", 5);
	}
#endif


#ifdef WLAMPDU
	/* Update some shared memory locations related to max AMPDU size allowed to received */
	wlc_ampdu_shm_upd(wlc->ampdu_rx);
#endif

#ifdef WL11N
	/* Update txcore-mask and spatial-mapping in shared memory */
	if (N_ENAB(wlc->pub)) {
		wlc->stf->shmem_base = wlc->pub->m_coremask_blk;
		wlc_stf_txcore_shmem_write(wlc, TRUE);
	}
#endif

	/* band-specific inits */
	wlc_bsinit(wlc);

	/* Enable EDCF mode (while the MAC is suspended) */
	if (EDCF_ENAB(wlc->pub)) {
		OR_REG(wlc->osh, &regs->u.d11regs.ifs_ctl, IFS_USEEDCF);
		FOREACH_BSS(wlc, i, bsscfg) {
			if (BSS_WME_ENAB(wlc, bsscfg))
				wlc_edcf_acp_apply(wlc, bsscfg, FALSE);
		}
	}

	/* Init precedence maps for empty FIFOs */
	wlc_tx_prec_map_init(wlc);

	/* read the ucode version if we have not yet done so */
	if (wlc->ucode_rev == 0) {
		wlc->ucode_rev = wlc_read_shm(wlc, M_BOM_REV_MAJOR) << NBITS(uint16);
		wlc->ucode_rev |= wlc_read_shm(wlc, M_BOM_REV_MINOR);
	}

#ifdef WLAMPDU_MAC
	/* Enable ucode AMPDU aggregation */
	wlc_sidechannel_init(wlc->ampdu_tx);
#endif

	/* ..now really unleash hell (allow the MAC out of suspend) */
	wlc_enable_mac(wlc);

#ifdef STA
	_wlc_set_wake_ctrl(wlc);
#endif

	/* Propagate rfaware_lifetime setting to ucode */
	wlc_rfaware_lifetime_set(wlc, wlc->rfaware_lifetime);

	/* clear tx flow control */
	wlc_txflowcontrol_reset(wlc);

	/* clear tx data fifo suspends */
	wlc->tx_suspended = FALSE;

	/* enable the RF Disable Delay timer */
	if (D11REV_GE(wlc->pub->corerev, 10))
		W_REG(wlc->osh, &wlc->regs->rfdisabledly, RFDISABLE_DEFAULT);

	/*
	 * Initialize WME parameters; if they haven't been set by some other
	 * mechanism (IOVar, etc) then read them from the hardware.
	 */
	for (ac = 0; ac < AC_COUNT; ac++) {
		temp = wlc_read_shm(wlc, M_AC_TXLMT_ADDR(ac));
		if (WLC_WME_RETRY_SHORT_GET(wlc, ac) == 0)
			WLC_WME_RETRY_SHORT_SET(wlc, ac, WME_RETRY_SHORT_GET(temp, ac));

		if (WLC_WME_RETRY_SFB_GET(wlc, ac) == 0)
			WLC_WME_RETRY_SFB_SET(wlc, ac, WME_RETRY_SFB_GET(temp, ac));

		if (WLC_WME_RETRY_LONG_GET(wlc, ac) == 0)
			WLC_WME_RETRY_LONG_SET(wlc, ac, WME_RETRY_LONG_GET(temp, ac));

		if (WLC_WME_RETRY_LFB_GET(wlc, ac) == 0)
			WLC_WME_RETRY_LFB_SET(wlc, ac, WME_RETRY_LFB_GET(temp, ac));
	}

	wlc_ht_update_coex_support(wlc, wlc->pub->_coex);

#ifdef CCA_STATS
	cca_stats_upd(wlc, 0);
#endif

	wlc_lq_chanim_update(wlc, wlc->chanspec, CHANIM_CHANSPEC);



	wlc->block_datafifo &= DATA_BLOCK_QUIET;

	if (wlc->pub->up) {
		/* Restart the AP if we are up and reiniting due to a big-hammer.
		 * If we are not yet up (and being called within wlc_up) do not
		 * call wlc_restart_ap() since it calls wlc_set_chanspec()
		 * which may attempt to switch to a band that has not yet been initialized.
		 * Clear aps_associated to force the first AP up to re-init core
		 * information, in particular the TSF (since we reset it above).
		 */
		if (AP_ENAB(wlc->pub)) {
			wlc->aps_associated = 0;
			wlc_restart_ap(wlc->ap);
		}
		/*
		 * If single AP, phy cal for AP was done at wlc_restart_ap();
		 * otherwise do phy cal on the current chanspec for everything else,
		 * e.g. multiple APs, single/multiple STAs
		 */
		if ((wlc->stas_associated > 0) || (wlc->aps_associated > 1)) {
			if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band) ||
			    WLCISACPHY(wlc->band)) {
				wlc_phy_cal_perical(pi, PHY_PERICAL_UP_BSS);
			}
		}
#ifdef STA
		wlc_restart_sta(wlc);
#endif
#ifdef WLOFFLD
		if (WLOFFLD_ENAB(wlc->pub))
			wlc_ol_restart(wlc->ol);
#endif
	}


#ifdef STA
	/* Cleanup assoc in progress in case of reinit */
	FOREACH_STA(wlc, idx, cfg) {
		wlc_assoc_abort(cfg);
	}

	/* Possibly resync scan state machine as channel and mac status would have changed */
	wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
#endif

#if defined(WLOFFLD) && defined(UCODE_SEQ)
	if (WLOFFLD_CAP(wlc)) {
		wlc_ol_update_seq_iv(wlc, FALSE, NULL);
	}
#endif /* WLOFFLD  && UCODE_SEQ */

#ifdef WLPM_BCNRX
	if (PM_BCNRX_ENAB(wlc->pub))
		wlc_pm_bcnrx_init(wlc);
#endif /* WLPM_BCNRX */
} /* wlc_init() */

#if defined(STA)
static void
wlc_restart_sta(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	bool update_tsf_cfprep = TRUE;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));
#ifdef WLMCNX
	/* handled by mcnx */
	if (MCNX_ENAB(wlc->pub))
		return;
#endif /* WLMCNX */

	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->up) {
			/* update tsf_cfprep if first up STA  */
			if (update_tsf_cfprep) {
				uint32 bi;
				/* get beacon period from primary bsscfg and convert to uS */
				bi = wlc->cfg->current_bss->beacon_period << 10;
				/* update the tsf_cfprep register */
				/* since init path would reset to default value */
				W_REG(wlc->osh, &wlc->regs->tsf_cfprep, (bi << CFPREP_CBI_SHIFT));
				update_tsf_cfprep = FALSE;
			}

			/* restore state upon receiving next beacon */
			wlc_assoc_change_state(cfg, AS_WAIT_RCV_BCN);

			/* Update maccontrol PM related bits */
			wlc_set_ps_ctrl(cfg);

			/* keep the chip awake until a beacon is received */
			if (cfg->pm->PMenabled) {
				wlc_set_pmawakebcn(cfg, TRUE);
				wlc_dtimnoslp_set(cfg);
			}

			wlc_pretbtt_set(cfg);
		}
	}
}
#endif /* STA */

void
wlc_mac_bcn_promisc(wlc_info_t *wlc)
{
	if ((AP_ACTIVE(wlc) && (N_ENAB(wlc->pub) || wlc->band->gmode)) ||
	    wlc->bcnmisc_ibss || wlc->bcnmisc_scan || wlc->bcnmisc_monitor)
		wlc_mctrl(wlc, MCTL_BCNS_PROMISC, MCTL_BCNS_PROMISC);
	else
		wlc_mctrl(wlc, MCTL_BCNS_PROMISC, 0);
}

/** set or clear maccontrol bits MCTL_PROMISC, MCTL_KEEPCONTROL and MCTL_KEEPBADFCS */
void
wlc_mac_promisc(wlc_info_t *wlc)
{
	uint32 promisc_bits = 0;
	uint32 mask = MCTL_PROMISC | MCTL_KEEPCONTROL | MCTL_KEEPBADFCS;
#ifdef DWDS
	int idx;
	wlc_bsscfg_t *cfg;
#endif

	/* promiscuous mode just sets MCTL_PROMISC
	 * Note: APs get all BSS traffic without the need to set the MCTL_PROMISC bit
	 * since all BSS data traffic is directed at the AP
	 */
	if (PROMISC_ENAB(wlc->pub) && !AP_ENAB(wlc->pub) &&
	    !wlc->wet && !PSTA_ENAB(wlc->pub))
		promisc_bits |= MCTL_PROMISC;

	promisc_bits |= wlc_monitor_get_mctl_promisc_bits(wlc->mon_info);
#ifdef DWDS
	/* disable promisc mode if any of the bsscfg is dwds */
	FOREACH_BSS(wlc, idx, cfg) {
		if (cfg->_dwds) {
			promisc_bits &= ~MCTL_PROMISC;
			break;
		}
	}
#endif

	wlc_mctrl(wlc, mask, promisc_bits);

	/* For promiscous mode, also enable probe requests */
	wlc_enable_probe_req(wlc, PROBE_REQ_PRMS_MASK,
		(promisc_bits & MCTL_PROMISC & mask) ? PROBE_REQ_PRMS_MASK:0);

#ifdef WL_BCN_COALESCING
	wlc_bcn_clsg_disable(wlc->bc, BCN_CLSG_PRMS_MASK,
		(promisc_bits & MCTL_PROMISC & mask) ? BCN_CLSG_PRMS_MASK:0);
#endif /* WL_BCN_COALESCING */

}

#ifdef BCMASSERT_SUPPORT
/** check if hps and wake states of sw and hw are in sync */
bool
wlc_ps_check(wlc_info_t *wlc)
{
#ifdef STA
	bool hps, wake;
	bool wake_ok;
	volatile uint32 tmp;
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_STA(wlc, idx, cfg) {
		bool hw_hps = FALSE;

		if (!cfg->BSS)
			continue;


		if (cfg == wlc->cfg) {
			tmp = R_REG(wlc->osh, &wlc->regs->maccontrol);

			/* If deviceremoved is detected, then don't take any action
			 * as this can be called in any context. Assume that caller
			 * will take care of the condition. This is just to avoid assert
			 */
			if (tmp == 0xffffffff) {
				WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
				WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
				return DEVICEREMOVED(wlc);
			}

			hw_hps = ((tmp & MCTL_HPS) != 0);
		}
#ifdef WLMCNX
		else if (MCNX_ENAB(wlc->pub)) {
			/* when HPS is forced i.e. during roaming ignore the check */
			if (TRUE &&
#ifdef WLP2P
			    BSS_P2P_ENAB(wlc, cfg) &&
#else
			    FALSE &&
#endif
			    wlc_mcnx_hps_forced(wlc->mcnx, cfg))
				hw_hps = PS_ALLOWED(cfg);
			else {
				tmp = wlc_mcnx_hps_get(wlc->mcnx, cfg);
				hw_hps = (tmp != 0);
			}
		}
#endif /* WLMCNX */

		hps = PS_ALLOWED(cfg);
		if (hps != hw_hps) {
			WL_ERROR(("wl%d.%d: hps not sync, sw %d, hw %d\n",
			          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), hps, hw_hps));
			WL_PS(("PM %d PMenabled %d PM_override %d "
			       "dtim_programmed %d PORTOPEN() %d\n",
			       cfg->pm->PM, cfg->pm->PMenabled, cfg->pm->PM_override,
			       cfg->dtim_programmed, WLC_PORTOPEN(cfg)));

			ASSERT(hps == hw_hps);
			return FALSE;
		}
	}

	tmp = R_REG(wlc->osh, &wlc->regs->maccontrol);

	/* If deviceremoved is detected, then don't take any action
	 * as this can be called in any context. Assume that caller
	 * will take care of the condition. This is just to avoid assert
	 */
	if (tmp == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		return DEVICEREMOVED(wlc);
	}

#ifdef WLC_LOW

	/* For a monolithic build the wake check can be exact since it looks at wake
	 * override bits. The MCTL_WAKE bit should match the 'wake' value.
	 */
	wake = STAY_AWAKE(wlc) || (wlc_hw_get_wake_override(wlc->hw) != 0);
	wake_ok = (wake == ((tmp & MCTL_WAKE) != 0));
#else
	/* For a split build we will not have access to any wake overrides from the low
	 * level. The check can only make sure the MCTL_WAKE bit is on if the high
	 * level 'wake' value is true. If the high level 'wake' is false, the MCTL_WAKE
	 * may be either true or false due to the low level override.
	 */
	wake = STAY_AWAKE(wlc);
	wake_ok = (wake && ((tmp & MCTL_WAKE) != 0)) || !wake;
#endif
	if (!wake_ok) {
		WL_ERROR(("wl%d: wake not sync, sw %d maccontrol 0x%x\n",
		          wlc->pub->unit, wake, tmp));
		WL_PS(("wake %d PMawakebcn %d BTA_ACTIVE() %d "
		       "SCAN_IN_PROGRESS() %d WLC_RM_IN_PROGRESS() %d AS_IN_PROGRESS() %d "
		       "PMpending %d PSpoll %d check_for_unaligned_tbtt %d apsd_sta_usp %d "
		       "txpend16165war %d pm2_radio_shutoff_pending %d monitor %d\n",
		       wlc->wake, wlc->PMawakebcn, BTA_ACTIVE(wlc),
		       SCAN_IN_PROGRESS(wlc->scan), WLC_RM_IN_PROGRESS(wlc),
		       AS_IN_PROGRESS_WLC(wlc),
		       wlc->PMpending, wlc->PSpoll, wlc->check_for_unaligned_tbtt,
		       wlc->apsd_sta_usp,
		       wlc->txpend16165war, wlc->pm2_radio_shutoff_pending, wlc->monitor));

		FOREACH_BSS(wlc, idx, cfg) {
			if (!cfg->BSS ||
			    !cfg->associated)
				continue;
			WL_PS(("bsscfg %d PMenabled %d "
			       "PMpending %d PSpoll %d apsd_sta_usp %d "
			       "check_for_unaligned_tbtt %d\n",
			       WLC_BSSCFG_IDX(cfg), cfg->pm->PMenabled,
			       cfg->pm->PMpending, cfg->pm->PSpoll, cfg->pm->apsd_sta_usp,
			       cfg->pm->check_for_unaligned_tbtt));
#ifdef WLP2P
			WL_PS(("P2P %d\n", BSS_P2P_ENAB(wlc, cfg)));
#endif
		}
#ifdef WAR4360_UCODE
		WL_ERROR(("wl%d: %s: Wake State Out-of-Sync, Big Hammer\n",
			wlc->pub->unit, __FUNCTION__));
		wlc->hw->need_reinit = 1;
#ifdef WLOFFLD
		if (WLOFFLD_ENAB(wlc->pub))
			wlc_ol_down(wlc->ol);
#endif
		return TRUE; /* return TRUE to avoid caller ASSERT */
#endif /* WAR4360_UCODE */

		ASSERT(wake_ok);
		return FALSE;
	}
#endif /* STA */
	return TRUE;
}
#endif 

/** push sw wake state through hardware */
static void
__wlc_set_wake_ctrl(wlc_info_t *wlc, uint32 mc)
{
	uint32 new_mc;
	bool wake;
	bool awake_before;

	wake = STAY_AWAKE(wlc);

	WL_TRACE(("wl%d: __wlc_set_wake_ctrl: wake %d\n", wlc->pub->unit, wake));
	WL_RTDC2(wlc, wake ? "__wlc_set_wake_ctrl: wake=%02u PS---" :
	         "__wlc_set_wake_ctrl: wake=%02u PS+++", (wake ? 1 : 0), 0);

	new_mc = wake ? MCTL_WAKE : 0;

#if defined(BCMDBG) || defined(WLMSG_PS)
	if ((mc & MCTL_WAKE) && !wake)
		WL_PS(("wl%d: PS mode: clear WAKE (sleep if permitted) at tick %u\n",
		       wlc->pub->unit, R_REG(wlc->osh, &wlc->regs->tsf_timerlow)));
	if (!(mc & MCTL_WAKE) && wake)
		WL_PS(("wl%d: PS mode: set WAKE (stay awake) at tick %u\n",
		       wlc->pub->unit, R_REG(wlc->osh, &wlc->regs->tsf_timerlow)));
#endif	/* BCMDBG || WLMSG_PS */

	wlc_mctrl(wlc, MCTL_WAKE, new_mc);

	awake_before = (mc & MCTL_WAKE);

	if (wake && !awake_before)
		wlc_bmac_wait_for_wake(wlc->hw);

#ifdef WL_LTR
	if (LTR_ENAB(wlc->pub)) {
		/*
		 * Set latency tolerance based on wake and hps
		 * If stay awake, change to LTR active
		 * Else if hps bit is set, LTR sleep else LTR active
		 */
		if (wake) {
			wlc_ltr_hwset(wlc->hw, wlc->regs, LTR_ACTIVE);
		}
		else {
			wlc_ltr_hwset(wlc->hw, wlc->regs, (mc & MCTL_HPS)?LTR_SLEEP:LTR_ACTIVE);
		}
	}
#endif
}

static void
_wlc_set_wake_ctrl(wlc_info_t *wlc)
{
	volatile uint32 mc;

	mc = R_REG(wlc->osh, &wlc->regs->maccontrol);

	__wlc_set_wake_ctrl(wlc, mc);
}

/** push sw hps and wake state through hardware */
void
wlc_set_ps_ctrl(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	bool hps;
#if defined(BCMDBG) || defined(WLMSG_PS)
	volatile uint32 mc;
#endif  /* BCMDBG || WLMSG_PS */

	if (!wlc->clk)
		return;

	hps = PS_ALLOWED(cfg);

#if defined(BCMDBG) || defined(WLMSG_PS)
	mc = R_REG(wlc->osh, &wlc->regs->maccontrol);
#endif  /* BCMDBG || WLMSG_PS */

	if (cfg == wlc->cfg) {
		uint32 new_mc;

		WL_TRACE(("wl%d.%d: wlc_set_ps_ctrl: hps %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), hps));

		new_mc = hps ? MCTL_HPS : 0;
#if defined(BCMDBG) || defined(WLMSG_PS)
		if ((mc & MCTL_HPS) && !hps)
			WL_PS(("wl%d.%d: PM-MODE: clear HPS (no sleep and no PM)\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		if (!(mc & MCTL_HPS) && hps)
			WL_PS(("wl%d.%d: PM-MODE: set HPS (permit sleep and enable PM)\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
#endif	/* BCMDBG || WLMSG_PS */

		if (wlc->ebd->thresh && wlc->ebd->assoc_start_time) {
			/* if awake for EARLY_DETECT_PERIOD after assoc, early_detect is done */
			if ((OSL_SYSUPTIME() - wlc->ebd->assoc_start_time) > EARLY_DETECT_PERIOD)
				wlc->ebd->detect_done = TRUE;
			wlc->ebd->assoc_start_time = 0;
		}
		wlc_mctrl(wlc, MCTL_HPS, new_mc);
	}
#ifdef WLMCNX
	else if (MCNX_ENAB(wlc->pub)) {
#if defined(BCMDBG) || defined(WLMSG_PS)
		if (!hps)
			WL_PS(("wl%d.%d: PM-MODE: clear HPS (P2P no sleep and no PM)\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		if (hps)
			WL_PS(("wl%d.%d: PM-MODE: set HPS (P2P permit sleep and enable PM)\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
#endif	/* BCMDBG || WLMSG_PS */
		wlc_mcnx_hps_set(wlc->mcnx, cfg, hps);
	}
#endif /* WLMCNX */

	_wlc_set_wake_ctrl(wlc);

	WL_PS(("wl%d.%d: wlc_set_ps_ctrl: mc 0x%x\n",
	        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), R_REG(wlc->osh, &wlc->regs->maccontrol)));

}

void
wlc_set_wake_ctrl(wlc_info_t *wlc)
{
	if (!wlc->pub->up)
		return;

	_wlc_set_wake_ctrl(wlc);
}

/**
 * Validate the MAC address for this bsscfg.
 * If valid, the MAC address will be set in the bsscfg's cur_etheraddr.
 */
static int
wlc_validate_mac(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *addr)
{
	int err = 0;

	/* don't let a broadcast address be set */
	if (ETHER_ISMULTI(addr))
		return BCME_BADADDR;

	if (cfg == wlc->cfg) {
		bcopy(addr, &wlc->pub->cur_etheraddr, ETHER_ADDR_LEN);
		wlc_bmac_set_hw_etheraddr(wlc->hw, addr);

		if (MBSS_ENAB(wlc->pub)) {
			int ii;
			wlc_bsscfg_t *bsscfg;
			/* regardless of a clash, every time the user sets
			 * the primary config's cur_etheraddr, we will clear all
			 * all of the secondary config ethernet addresses.  If we
			 * don't do this, we'll have to prevent the user from
			 * configuring a MAC for the primary that collides(ucidx)
			 * with secondary configs.  this is way easier and is
			 * documented this way in the IOCTL/IOVAR manual.
			 */
			FOREACH_BSS(wlc, ii, bsscfg) {
				if (BSSCFG_AP(bsscfg))
					bcopy(&ether_null, &bsscfg->cur_etheraddr, ETHER_ADDR_LEN);
			}

			/* also clear the base address for MBSS */
			bcopy(&ether_null, &wlc->mbss->vether_base, ETHER_ADDR_LEN);
		}
	}
#ifdef MBSS
	else if (MBSS_ENAB(wlc->pub) && BSSCFG_AP(cfg)) {
		struct ether_addr temp;
		int ucidx;
		int ii;
		wlc_bsscfg_t *bsscfg;

		/* Has the primary config's address been set? */
		if (ETHER_ISNULLADDR(&wlc->cfg->cur_etheraddr))
			return BCME_BADADDR;

		if (ETHER_ISNULLADDR(&wlc->mbss->vether_base)) {
			/* setting first VIF addr, start by checking
			 * for collision with primary config
			 */
			if (EADDR_TO_UC_IDX(*addr, WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)) ==
			    EADDR_TO_UC_IDX(wlc->cfg->cur_etheraddr,
			                    WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)))
				return BCME_BADADDR;

			/* Apply mask and save the base */
			bcopy(addr, &temp, ETHER_ADDR_LEN);
			temp.octet[5] &= ~(WLC_MBSS_UCIDX_MASK(wlc->pub->corerev));
			bcopy(&temp, &wlc->mbss->vether_base, ETHER_ADDR_LEN);
		}
		else {
			/* verify that the upper bits of the address
			 * match our base
			 */
			bcopy(addr, &temp, ETHER_ADDR_LEN);
			temp.octet[5] &= ~(WLC_MBSS_UCIDX_MASK(wlc->pub->corerev));
			if (bcmp(&temp, &wlc->mbss->vether_base, ETHER_ADDR_LEN))
				return BCME_BADADDR;

			/* verify that there isn't a
			 * collision with any other configs.
			 */
			ucidx = EADDR_TO_UC_IDX(*addr, WLC_MBSS_UCIDX_MASK(wlc->pub->corerev));

			FOREACH_BSS(wlc, ii, bsscfg) {
				if ((bsscfg == cfg) ||
				    (ETHER_ISNULLADDR(&bsscfg->cur_etheraddr)))
					continue;
				if (ucidx == EADDR_TO_UC_IDX(bsscfg->cur_etheraddr,
					WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)))
					return BCME_BADADDR;
			}

			/* make sure the index is in bound */
			if (MBSS_ENAB16(wlc->pub) &&
			    ((uint32)AP_BSS_UP_COUNT(wlc) >= WLC_MAX_AP_BSS(wlc->pub->corerev)))
				return BCME_BADADDR;
		}
	}
#endif /* MBSS */

	if (!err) {
		/* Accept the user's MAC address */
		bcopy(addr, &cfg->cur_etheraddr, ETHER_ADDR_LEN);
		/* Takes effect immediately. Note that it's still recommended that
		 * the BSS be down before doing this to avoid having frames in transit
		 * with this as TA, but that's a corner case.
		 */
	/* VSDB TODO: changes for rcmta -> AMT ? or is it taken care under wlc_set_mac() */
		if (wlc->clk &&
		    (R_REG(wlc->osh, &wlc->regs->maccontrol) & MCTL_EN_MAC)) {
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_mac(cfg);
			wlc_enable_mac(wlc);
		}
	}

	return err;
}

void
wlc_clear_mac(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	WL_TRACE(("wl%d: %s: config %d\n", wlc->pub->unit, __FUNCTION__,
	          WLC_BSSCFG_IDX(cfg)));

	WL_APSTA_UPDN(("wl%d: wlc_set_mac for config %d\n", wlc->pub->unit,
	          WLC_BSSCFG_IDX(cfg)));

	if (cfg == wlc->cfg) {
		/* clear the MAC addr from the match registers */
		wlc_clear_addrmatch(wlc, WLC_ADDRMATCH_IDX_MAC);
	}
#ifdef WLMCNX
	else if (MCNX_ENAB(wlc->pub)) {
		wlc_mcnx_ra_unset(wlc->mcnx, cfg);
	}
#endif /* WLMCNX */
}

/** Write this BSS config's MAC address to core. Updates RXE match engine. */
int
wlc_set_mac(wlc_bsscfg_t *cfg)
{
	int err = 0;
	wlc_info_t *wlc = cfg->wlc;
	const struct ether_addr *addr;

	addr = &cfg->cur_etheraddr;

	WL_APSTA_UPDN(("wl%d: wlc_set_mac for config %d\n", wlc->pub->unit,
	          WLC_BSSCFG_IDX(cfg)));

	if (cfg == wlc->cfg) {
		/* enter the MAC addr into the match registers */
		uint16 attr = AMT_ATTR_VALID | AMT_ATTR_A1;
		wlc_set_addrmatch(wlc, WLC_ADDRMATCH_IDX_MAC, addr, attr);
	}
#ifdef PSTA
	else if (BSSCFG_PSTA(cfg)) {
		;
	}
#endif /* PSTA */
#ifdef MBSS
	else if (MBSS_ENAB(wlc->pub) && BSSCFG_AP(cfg)) {
		wlc_write_mbss_basemac(wlc, &wlc->mbss->vether_base);
	}
#endif /* MBSS */
#ifdef WLMCNX
	else if (MCNX_ENAB(wlc->pub)) {
		wlc_mcnx_ra_set(wlc->mcnx, cfg);
	}
#endif /* WLMCNX */


#ifdef WLAMPDU
	wlc_ampdu_macaddr_upd(wlc);
#endif

	return err;
}

void
wlc_clear_bssid(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	if (cfg == wlc->cfg)
		/* clear the BSSID from the match registers */
		wlc_clear_addrmatch(wlc, WLC_ADDRMATCH_IDX_BSSID);
#ifdef WLMCNX
	else if (MCNX_ENAB(wlc->pub))
		wlc_mcnx_bssid_unset(wlc->mcnx, cfg);
#endif

	wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_BSSID_UPDATE, cfg,
		NULL, NULL, NULL);
}

void
wlc_set_bssid(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	/* if primary config, we need to update BSSID in RXE match registers */
	if (cfg == wlc->cfg) {
		uint16 attr = 0;
		/* enter the MAC addr into the match registers */
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			/* Null address gets zero attributes */
			if (!ETHER_ISNULLADDR(&cfg->BSSID)) {
				/* STA and AP configs need different match attributes
				 * for the BSSID postion, A1/A2/A3
				 */
				attr |= AMT_ATTR_VALID;
				if (BSSCFG_STA(cfg) && cfg->BSS) {
					/* For infra STA, BSSID can be in A2 (management)
					   or A3 (data)
					 */
					attr |= (AMT_ATTR_A2 | AMT_ATTR_A3);
				} else {
					/* For IBSS STA, BSSID is in A3 */
					/* For AP, BSSID might be in A3 for a probe request */
					attr |= AMT_ATTR_A3;
				}
			}
		}
		wlc_set_addrmatch(wlc, WLC_ADDRMATCH_IDX_BSSID, &cfg->BSSID, attr);
	}
#ifdef WLMCNX
	else if (MCNX_ENAB(wlc->pub))
		wlc_mcnx_bssid_set(wlc->mcnx, cfg);
#endif

#ifdef STA
	if (BSSCFG_STA(cfg) && cfg->BSS)
	{
		/*
		Set the BSSID, TA and RA in the NULL DP template for ucode generated
		Null data frames only for Mac Revs 34 or greater
		*/
		if (D11REV_GE(wlc->pub->corerev, 34)) {
			wlc_nulldp_tpl_upd(cfg);
		}
	}
#endif

	wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_BSSID_UPDATE, cfg,
		NULL, NULL, NULL);


#if defined(WET) && defined(PLC_WET)
	/* Update the BSSID for WET */
	if (PLC_ENAB(wlc->pub) && wlc->wet && BSSCFG_STA(cfg))
		wlc_wet_bssid_upd(wlc->weth, cfg);
#endif /* WET && PLC_WET */
}

/**
 * Suspend the the MAC and update the slot timing for standard 11b/g (20us slots) or shortslot 11g
 * (9us slots).
 */
void
wlc_switch_shortslot(wlc_info_t *wlc, bool shortslot)
{
	int idx;
	wlc_bsscfg_t *cfg;

	ASSERT(wlc->band->gmode);

	/* use the override if it is set */
	if (wlc->shortslot_override != WLC_SHORTSLOT_AUTO)
		shortslot = (wlc->shortslot_override == WLC_SHORTSLOT_ON);

	if (wlc->shortslot == shortslot)
		return;

	wlc->shortslot = shortslot;

	/* update the capability based on current shortslot mode */
	FOREACH_BSS(wlc, idx, cfg) {
		if (!cfg->associated)
			continue;
		cfg->current_bss->capability &= ~DOT11_CAP_SHORTSLOT;
		if (wlc->shortslot)
			cfg->current_bss->capability |= DOT11_CAP_SHORTSLOT;
	}

	wlc_bmac_set_shortslot(wlc->hw, shortslot);
}

/**
 * Update aCWmin based on supported rates. 11g sec 9.2.12, 19.8.4, supported rates are all in the
 * set 1, 2, 5.5, and 11 use B PHY aCWmin == 31, The PSM needs to be suspended for this call.
 */
void
wlc_cwmin_gphy_update(wlc_info_t *wlc, wlc_rateset_t *rs, bool associated)
{
	uint16 cwmin;
	uint i;
	uint r;

	if (associated) {
		/* use the BSS supported rates to determine aCWmin */
		cwmin = BPHY_CWMIN;
		for (i = 0; i < rs->count; i++) {
			r = rs->rates[i] & RATE_MASK;
			/*
			 * Checking for IS_OFDM(rate) is not sufficient.
			 * If the rateset includes PBCC rates 22 or 33Mbps,
			 * we should also use APHY_CWMIN,
			 * but the PBCC rates are not OFDM.
			 */
			if (!IS_CCK(r)) {
				cwmin = APHY_CWMIN;
				break;
			}
		}
	} else {
		/* unassociated aCWmin A phy value (11g sec 9.2.12, 19.8.4) */
		cwmin = APHY_CWMIN;
	}

	WL_INFORM(("wl%d: wlc_cwmin_gphy_update(): setting aCWmin = %d\n", wlc->pub->unit, cwmin));

	wlc_set_cwmin(wlc, cwmin);
}

/** propagate home chanspec to all bsscfgs in case bsscfg->current_bss->chanspec is referenced */
void
wlc_set_home_chanspec(wlc_info_t *wlc, chanspec_t chanspec)
{
	if (wlc->home_chanspec != chanspec) {
		int idx;
		wlc_bsscfg_t *cfg;

		WL_INFORM(("wl%d: change shared chanspec wlc->home_chanspec from "
		           "0x%04x to 0x%04x\n", wlc->pub->unit, wlc->home_chanspec, chanspec));

		wlc->home_chanspec = chanspec;

		FOREACH_AS_BSS(wlc, idx, cfg) {
			if (BSS_EXCRX_ENAB(wlc, cfg))
				continue;
#ifdef WLMCHAN
			if (!MCHAN_ENAB(wlc->pub) ||
			    _wlc_mchan_same_chan(wlc->mchan, cfg, chanspec))
#endif
			{
				cfg->target_bss->chanspec = chanspec;
				wlc_bsscfg_set_current_bss_chan(cfg, chanspec);
			}
		}

		if ((WLCISNPHY(wlc->band) && NREV_GE(wlc->band->phyrev, 3)) ||
		    WLCISHTPHY(wlc->band)) {
			wlc_phy_t *pi = WLC_PI(wlc);
			wlc_phy_interference_set(pi, TRUE);

			wlc_phy_acimode_noisemode_reset(pi,
				chanspec, TRUE, TRUE, FALSE);
		}
	}
}

/** full phy cal when start or join a network */
void
wlc_full_phy_cal(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 reason)
{
	int idx;
	wlc_bsscfg_t *cfg;
	chanspec_t chanspec;

	chanspec = bsscfg->associated ?
		bsscfg->current_bss->chanspec : bsscfg->target_bss->chanspec;

	/* check if channel in bsscfg is a channel to share */
	FOREACH_BSS(wlc, idx, cfg) {
		if (!cfg->associated || cfg == bsscfg)
			continue;
		if (cfg->current_bss->chanspec == chanspec)
			/* skip full phy cal if already did by other interface */
			return;
	}

	wlc_phy_cal_perical(WLC_PI(wlc), reason);
}

#ifdef WL11N
#if defined(WLC_HIGH) && defined(WLC_LOW)
/** update bandwidth */
static void
wlc_txbw_update(wlc_info_t *wlc)
{
	wlc_bmac_txbw_update(wlc->hw);

}
#endif
#endif /* WL11N */

#if !(defined(WLMCHAN) && defined(WLC_HIGH_ONLY)) && defined(WL11N)
/**
 * Speed up band switch time by avoiding full stf initialization when possible, performing save or
 * restore of stf settings instead.
 */
static void
wlc_srvsdb_stf_ss_algo_chan_get(wlc_info_t *wlc, chanspec_t chanspec)
{

	uint8 i_ppr = 0;
	wlc_srvsdb_info_t *srvsdb = wlc->srvsdb_info;

	/* In VSDB, if ss algo chan was saved before, just restore it now */
	if ((srvsdb->iovar_force_vsdb) ||
#ifdef WLMCHAN
		(MCHAN_ENAB(wlc->pub) && MCHAN_ACTIVE(wlc->pub)) ||
#endif /* WLMCHAN */
		FALSE) {
		i_ppr = (chanspec == srvsdb->vsdb_chans[0]) ? 0 : 1;

		if (srvsdb->ss_algo_save_valid[i_ppr] == 0) {
			/* Save */
			if (wlc->stf->ss_algosel_auto) {
				wlc_stf_ss_algo_channel_get(wlc, &wlc->stf->ss_algo_channel,
					chanspec);
			}
			srvsdb->ss_algo_save_valid[i_ppr] = 1;
			srvsdb->ss_algosaved[i_ppr] = wlc->stf->ss_algo_channel;
		} else {
			/* Restore */
			wlc->stf->ss_algo_channel = srvsdb->ss_algosaved[i_ppr];
		}
	} else {
		/* MCHAN not active, call full initialisation */
		if (wlc->stf->ss_algosel_auto) {
			wlc_stf_ss_algo_channel_get(wlc, &wlc->stf->ss_algo_channel, chanspec);
		}
	}
}
#endif /* !(WLMCHAN && WLC_HIGH_ONLY) && WL11N */

static void
wlc_set_phy_chanspec(wlc_info_t *wlc, chanspec_t chanspec)
{
	uint8 constraint;
	wlc_phy_t *pi;
	BCM_REFERENCE(pi);

	/* Save our copy of the chanspec */
	wlc->chanspec = chanspec;

	WL_CHANLOG(wlc, __FUNCTION__, TS_ENTER, 0);
	/* Set the chanspec and power limits for this locale after computing
	 * any 11h local tx power constraints.
	 */
	constraint = wlc_tpc_get_local_constraint_qdbm(wlc->tpc);
	wlc_channel_set_chanspec(wlc->cmi, chanspec, constraint);

#ifdef WL11AC
	wlc_stf_chanspec_upd(wlc);
#endif /* WL11AC */

#ifdef WL11N
#if !(defined(WLMCHAN) && defined(WLC_HIGH_ONLY))
	if (SRHWVSDB_ENAB(wlc->pub)) {
		wlc_srvsdb_stf_ss_algo_chan_get(wlc, chanspec);
	}
	else if (wlc->stf->ss_algosel_auto) {
		/* following code brings ~3ms delay for split driver */
		wlc_stf_ss_algo_channel_get(wlc, &wlc->stf->ss_algo_channel, chanspec);
	}
	wlc_stf_ss_update(wlc, wlc->band);
#if defined(WLC_HIGH) && defined(WLC_LOW)
	wlc_txbw_update(wlc);
#endif
#endif /* !(defined(WLMCHAN) && defined(WLC_HIGH_ONLY)) */
#endif /* WL11N */

#ifdef WL_BEAMFORMING
	wlc_txbf_impbf_upd(wlc->txbf);
#endif
	WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
}

void
wlc_set_chanspec(wlc_info_t *wlc, chanspec_t chanspec)
{
	uint bandunit;
	uint32 tsf_l;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char chanbuf1[CHANSPEC_STR_LEN];
#endif
	DBGONLY(char chanbuf2[CHANSPEC_STR_LEN]; )
#ifdef STA
	bool switchband = FALSE;
#endif
	chanspec_t old_chanspec = wlc->chanspec;
#ifdef CCA_STATS
	int idx;
	wlc_bsscfg_t *cfg;
	bool skip_cca = FALSE;
#endif /* CCA_STATS */
#if defined(WLC_LOW) && defined(WLC_HIGH) && (defined(BCMDBG) || \
	defined(MCHAN_MINIDUMP))
	chanspec_t orig;
	uint32 now;
#endif

	WL_CHANLOG(wlc, "wlc_set_chanspec from => to", wlc->chanspec, chanspec);
	WL_CHANLOG(wlc, __FUNCTION__, TS_ENTER, 0);
#ifdef WL_BCN_COALESCING
	/* Release any accumulated beacons */
	OR_REG(wlc->osh, &wlc->regs->maccommand, MCMD_BCNREL);
#endif /* WL_BCN_COALESCING */

	tsf_l = 0;
	if (WL_MCHAN_ON()) {
		WL_MCHAN(("wl%d: %s - changing chanspec from %s to %s\n",
			wlc->pub->unit, __FUNCTION__,
			wf_chspec_ntoa_ex(wlc->chanspec, chanbuf1),
			wf_chspec_ntoa_ex(chanspec, chanbuf2)));
		tsf_l = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	}
#if defined(WLC_LOW) && defined(WLC_HIGH) && (defined(BCMDBG) || \
	defined(MCHAN_MINIDUMP))
	tsf_l = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	orig = wlc->chanspec;
#endif

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec) &&
		!(wlc->scan->state & SCAN_STATE_PROHIBIT)) {
		WL_ERROR(("wl%d: %s: Bad chanspec %s\n",
			wlc->pub->unit, __FUNCTION__, wf_chspec_ntoa_ex(chanspec, chanbuf1)));
		goto set_chanspec_done;
	}

#ifdef CCA_STATS
	/* to speed up roaming process, especially in dongle case, do not read
	 * counters from shared memory for cca during roaming scan
	 */
	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->BSS && cfg->assoc->type == AS_ROAM) {
			skip_cca = TRUE;
			break;
		}
	}

	/* About to leave this channel, so calculate cca stats from this channel */
	if (!skip_cca)
		cca_stats_upd(wlc, 1);
#endif /* CCA_STATS */

	wlc_lq_chanim_update(wlc, wlc->chanspec, CHANIM_CHANSPEC);


	if (old_chanspec == chanspec) {
		WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
		return;
	}


	WL_CHANLOG(wlc, "Before wlc_setband", 0, 0);
	/* Switch bands if necessary */
	if (NBANDS(wlc) > 1) {
		bandunit = CHSPEC_WLCBANDUNIT(chanspec);
		if (wlc->band->bandunit != bandunit || wlc->bandinit_pending) {
#ifdef STA
			switchband = TRUE;
#endif
			if (wlc->bandlocked) {
				WL_ERROR(("wl%d: %s: chspec %s band is locked!\n",
					wlc->pub->unit, __FUNCTION__,
					wf_chspec_ntoa_ex(chanspec, chanbuf1)));
				goto set_chanspec_done;
			}
			/* BMAC_NOTE: should the setband call come after the wlc_bmac_chanspec() ?
			 * if the setband updates (wlc_bsinit) use low level calls to inspect and
			 * set state, the state inspected may be from the wrong band, or the
			 * following wlc_bmac_set_chanspec() may undo the work.
			 */
			wlc_setband(wlc, bandunit);
		}
	}

	ASSERT((VHT_ENAB_BAND(wlc->pub, (CHSPEC_IS2G(chanspec) ? WLC_BAND_2G : WLC_BAND_5G)) ||
		CHSPEC_IS20(chanspec) || CHSPEC_IS40(chanspec)));
	ASSERT(N_ENAB(wlc->pub) || CHSPEC_IS20(chanspec));

	/* sync up phy/radio chanspec */
	wlc_set_phy_chanspec(wlc, chanspec);

#ifdef WL11N
	/* update state that depends on channel bandwidth */
	if (CHSPEC_WLC_BW(old_chanspec) != CHSPEC_WLC_BW(chanspec)) {
		/* init antenna selection */
		if (WLANTSEL_ENAB(wlc))
			wlc_antsel_init(wlc->asi);

		/* Fix the hardware rateset based on bw.
		 * Mainly add MCS32 for 40Mhz, remove MCS 32 for 20Mhz
		 */
		wlc_rateset_bw_mcs_filter(&wlc->band->hw_rateset, CHSPEC_WLC_BW(chanspec));
	}
#endif /* WL11N */

	/* update some mac configuration since chanspec changed */
	wlc_ucode_mac_upd(wlc);

#ifdef STA
	if (NBANDS(wlc) > 1) {
		if (switchband && wlc->pub->up) {
			if ((wlc->freqtrack || (wlc->freqtrack_override == FREQTRACK_ON)) &&
			    WLCISGPHY(wlc->band))
				wlc_phy_freqtrack_start(WLC_PI(wlc));
		}
	}
#endif

#ifdef CCA_STATS
	if (!skip_cca)
		/* update cca time */
		cca_stats_tsf_upd(wlc);
#endif

	/* invalidate txcache as transmit b/w may have changed */
	if (N_ENAB(wlc->pub) &&
	    WLC_TXC_ENAB(wlc))
		wlc_txc_inv_all(wlc->txc);

	/* update ED/CRS settings */
	wlc_bmac_ifsctl_edcrs_set(wlc->hw, WLCISHTPHY(wlc->band));

set_chanspec_done:
	if (WL_MCHAN_ON()) {
		uint32 now_l = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
		/* Use ABS here in case of wrap around */
		uint32 delta_time = (uint32)ABS((int32)(now_l - tsf_l));
		BCM_REFERENCE(delta_time);

		WL_MCHAN(("wl%d: %s - completed in %d uS at 0x%x\n",
		          wlc->pub->unit, __FUNCTION__, delta_time, now_l));
	}
#if defined(WLC_LOW) && defined(WLC_HIGH) && (defined(BCMDBG) || \
	defined(MCHAN_MINIDUMP))
	now = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	chanswitch_history(wlc, orig, chanspec, tsf_l, now, CHANSWITCH_SET_CHANSPEC);
#endif
#ifdef BCMLTECOEX
	/* Update LTECX states on Channel Switch */
	if (BCMLTECOEX_ENAB(wlc->pub)) {
		wlc_ltecx_update_all_states(wlc->ltecx);
	}
#endif /* BCMLTECOEX */
#ifdef WLOLPC
	if (OLPC_ENAB(wlc)) {
	/* we migrated to a new channel; check for phy cal need (and do it) */
	/* also, terminate cal if needed */
		wlc_olpc_eng_hdl_chan_update(wlc->olpc_info);
	}
#endif /* OPEN LOOP POWER CAL */
	WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
}

int
wlc_get_last_txpwr(wlc_info_t *wlc, wlc_txchain_pwr_t *last_pwr)
{
	uint i;
	uint8 est_Pout[WLC_NUM_TXCHAIN_MAX];
	uint8 est_Pout_act[WLC_NUM_TXCHAIN_MAX];
	uint8 est_Pout_cck;

	if (!wlc->pub->up)
		return BCME_NOTUP;

	bzero(est_Pout, WLC_NUM_TXCHAIN_MAX);

	wlc_phy_get_est_pout(WLC_PI(wlc), est_Pout, est_Pout_act, &est_Pout_cck);

	for (i = 0; i < WLC_NUM_TXCHAIN_MAX; i++) {
		last_pwr->chain[i] = est_Pout[i];
	}

	return BCME_OK;
}

#ifdef STA
/** validate and sanitize chanspecs passed with assoc params */
int
wlc_assoc_chanspec_sanitize(wlc_info_t *wlc, chanspec_list_t *list, int len)
{
	uint32 chanspec_num;
	chanspec_t *chanspec_list;
	int i;

	if ((uint)len < sizeof(list->num))
		return BCME_BUFTOOSHORT;

	chanspec_num = load32_ua((uint8 *)&list->num);

	if ((uint)len < sizeof(list->num) + chanspec_num * sizeof(list->list[0]))
		return BCME_BUFTOOSHORT;

	chanspec_list = list->list;

	for (i = 0; i < (int)chanspec_num; i++) {
		chanspec_t chanspec = load16_ua((uint8 *)&chanspec_list[i]);
		if (wf_chspec_malformed(chanspec))
			return BCME_BADCHAN;
		/* get the control channel from the chanspec */
		chanspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan(chanspec));
		htol16_ua_store(chanspec, (uint8 *)&chanspec_list[i]);
		if (!wlc_valid_chanspec_db(wlc->cmi, chanspec))
			return BCME_BADCHAN;
	}

	return BCME_OK;
}

#ifdef PHYCAL_CACHING
static int
wlc_cache_cals(wlc_info_t *wlc)
{
	chanspec_t chanspec = wlc_default_chanspec(wlc->cmi, TRUE);
	uint i;
	char abbrev[WLC_CNTRY_BUF_SZ];
	wl_uint32_list_t *list;

	if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) ||
	    mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE))
		return BCME_RADIOOFF;

	list = (wl_uint32_list_t *)MALLOC(wlc->osh, (WL_NUMCHANSPECS+1) * sizeof(uint32));

	if (!list) {
		WL_ERROR(("wl%d: %s: out of mem, %d bytes malloced\n", wlc->pub->unit,
		          __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	bzero(list, (WL_NUMCHANSPECS+1) * sizeof(uint32));
	bzero(abbrev, WLC_CNTRY_BUF_SZ);
	list->count = 0;
	wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_20, TRUE, abbrev);
	wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_20, FALSE, abbrev);

	/* 20MHz only for now
	 * wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_40, TRUE, abbrev);
	 * wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_40, FALSE, abbrev);
	 */

	/* need this to be set */
	wlc_iovar_setint(wlc, "phy_percal", PHY_PERICAL_SPHASE);

	/* we'll assert in PHY code if we are in MPC */
	wlc->mpc = FALSE;
	wlc_radio_mpc_upd(wlc);

	wl_up(wlc->wl);

	wlc_bmac_set_phycal_cache_flag(wlc->hw, TRUE);

	wlc_suspend_mac_and_wait(wlc);

	for (i = 0; i < list->count; i++) {
		wlc_phy_t *pi = WLC_PI(wlc);
		chanspec = (chanspec_t) list->element[i];
		WL_INFORM(("wl%d: %s: setting chanspec to 0x%04x and running a periodic cal\n",
		           wlc->pub->unit, __FUNCTION__, chanspec));
		wlc_phy_create_chanctx(pi, chanspec);
		wlc_set_chanspec(wlc, chanspec);
		wlc_phy_cal_perical(pi, PHY_PERICAL_JOIN_BSS);
	}

	wlc_enable_mac(wlc);

	wlc_iovar_setint(wlc, "phy_percal", PHY_PERICAL_MANUAL);

	if (list)
		MFREE(wlc->osh, list, (WL_NUMCHANSPECS+1) * sizeof(uint32));

	return BCME_OK;
}

static int
wlc_cachedcal_sweep(wlc_info_t *wlc)
{
	chanspec_t chanspec = wlc_default_chanspec(wlc->cmi, TRUE);
	uint i;
	char abbrev[WLC_CNTRY_BUF_SZ];
	wl_uint32_list_t *list = (wl_uint32_list_t *)
	        MALLOC(wlc->osh, (WL_NUMCHANSPECS+1) * sizeof(uint32));

	if (!list) {
		WL_ERROR(("wl%d: %s: out of mem, %d bytes malloced\n", wlc->pub->unit,
		          __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	bzero(list, (WL_NUMCHANSPECS+1) * sizeof(uint32));
	bzero(abbrev, WLC_CNTRY_BUF_SZ);
	list->count = 0;

	wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_20, TRUE, abbrev);
	wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_20, FALSE, abbrev);

	/* 20MHz only for now
	 * wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_40, TRUE, abbrev);
	 * wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_40, FALSE, abbrev);
	 */

	/* we'll assert in PHY code if we are in MPC */
	wlc->mpc = FALSE;
	wlc_radio_mpc_upd(wlc);

	wlc_suspend_mac_and_wait(wlc);

	for (i = 0; i < list->count; i++) {
		wlc_phy_t *pi = WLC_PI(wlc);
		chanspec = (chanspec_t) list->element[i];
		WL_INFORM(("wl%d: %s: setting chanspec to 0x%04x and running a periodic cal\n",
		           wlc->pub->unit, __FUNCTION__, chanspec));
		wlc_phy_create_chanctx(pi, chanspec);
		wlc_set_chanspec(wlc, chanspec);
		wlc_phy_cal_perical(pi, PHY_PERICAL_JOIN_BSS);
	}

	wlc_enable_mac(wlc);

	if (list)
		MFREE(wlc->osh, list, (WL_NUMCHANSPECS+1) * sizeof(uint32));

	return BCME_OK;
}

static int
wlc_cachedcal_tune(wlc_info_t *wlc, uint16 channel)
{
	uint16 chanspec = (channel | WL_CHANSPEC_BW_20);
	channel <= CH_MAX_2G_CHANNEL ? (chanspec |= WL_CHANSPEC_BAND_2G) :
	        (chanspec |= WL_CHANSPEC_BAND_5G);

	if (wf_chspec_malformed(chanspec)) {
		WL_ERROR(("wl%d: %s: Malformed chanspec 0x%x\n", wlc->pub->unit, __FUNCTION__,
		          chanspec));
		return BCME_BADCHAN;
	}

	if (!VALID_CHANNEL20_IN_BAND(wlc, CHANNEL_BANDUNIT(wlc, channel), channel)) {
		WL_ERROR(("wl%d: %s: Bad channel %d\n", wlc->pub->unit, __FUNCTION__, channel));
		return BCME_BADCHAN;
	}

	/* we'll assert in PHY code if we are in MPC */
	wlc->mpc = FALSE;
	wlc_radio_mpc_upd(wlc);

	wlc_suspend_mac_and_wait(wlc);
	WL_INFORM(("wl%d: %s: setting chanspec to 0x%04x and running a periodic cal\n",
	          wlc->pub->unit, __FUNCTION__, chanspec));
	wlc_set_chanspec(wlc, chanspec);
	wlc_phy_cal_perical(WLC_PI(wlc), PHY_PERICAL_JOIN_BSS);
	wlc_enable_mac(wlc);

	return BCME_OK;
}
#endif /* PHYCAL_CACHING */
#endif	/* STA */

/** "extended" scan request */
int
wlc_scan_request_ex(
	wlc_info_t *wlc,
	int bss_type,
	const struct ether_addr* bssid,
	int nssid,
	wlc_ssid_t *ssids,
	int scan_type,
	int nprobes,
	int active_time,
	int passive_time,
	int home_time,
	const chanspec_t* chanspec_list,
	int chanspec_num,
	chanspec_t chanspec_start,
	bool save_prb,
	scancb_fn_t fn, void* arg,
	int macreq,
	uint scan_flags,
	wlc_bsscfg_t *cfg,
	actcb_fn_t act_cb, void *act_cb_arg)
{
	int err = 0;
	bool cb = TRUE;

	ASSERT((ssids != NULL) && (nssid != 0));
	ASSERT(ssids->SSID_len <= DOT11_MAX_SSID_LEN);

	/* if radio is disabled due to disassociation, turn it on */
	wlc->mpc_scan = TRUE;
#ifdef STA
	wlc_radio_mpc_upd(wlc);
#endif /* STA */
	if (!wlc->pub->up) {
		WL_ERROR(("wl%d: %s, can not scan while driver is down\n",
			wlc->pub->unit, __FUNCTION__));
		err = BCME_NOTUP;
	}
	else if ((err = wlc_mac_request_entry(wlc, NULL, macreq)) == BCME_OK) {
		uint8 usage;

		switch (macreq) {
		case WLC_ACTION_SCAN:
		case WLC_ACTION_ISCAN:
		case WLC_ACTION_PNOSCAN:
			usage = SCAN_ENGINE_USAGE_NORM;
			break;
		case WLC_ACTION_ESCAN:
			usage = SCAN_ENGINE_USAGE_ESCAN;
			break;
		case WLC_ACTION_ACTFRAME:
			usage = SCAN_ENGINE_USAGE_AF;
			break;

		default:
			WL_NONE(("wl%d: using scan engine for low priority excursion\n",
			         wlc->pub->unit));
			usage = SCAN_ENGINE_USAGE_EXCRX;
			break;
		}

		err = wlc_scan(wlc->scan, bss_type, bssid, nssid, ssids,
		               scan_type, nprobes, active_time, passive_time, home_time,
		               chanspec_list, chanspec_num, chanspec_start, save_prb,
		               fn, arg, 0, FALSE, FALSE, SCANCACHE_ENAB(wlc->scan),
		               scan_flags, cfg, usage, act_cb, act_cb_arg);
		/* wlc_scan() invokes 'fn' even in error cases */
		cb = FALSE;
	}
	else {
		WL_INFORM(("wl%d: scan is blocked by others\n", wlc->pub->unit));
	}

	/* make it consistent with wlc_scan() w.r.t. invoking callback */
	if (cb && fn != NULL) {
		WL_ERROR(("wl%d: %s, can not scan due to error %d\n",
		          wlc->pub->unit, __FUNCTION__, err));
		(fn)(arg, WLC_E_STATUS_ERROR, cfg);
	}

	wlc->mpc_scan = FALSE;
#ifdef STA
	wlc_radio_mpc_upd(wlc);
#endif /* STA */

	return err;
}

void
wlc_custom_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;

	/* This scan is blocked by other existing scan of equal or higher priority */
	/* Do this at the beginning of this function to avoid sending unnecessary events */
	if (status == WLC_E_STATUS_ERROR)
		return;

#ifdef STA
	if (ESCAN_IN_PROGRESS(wlc->scan)) {
		wl_escan_result_t escan_result;

		escan_result.sync_id = wlc->escan_sync_id;
		escan_result.version = WL_BSS_INFO_VERSION;
		escan_result.bss_count = 0;
		escan_result.buflen = WL_ESCAN_RESULTS_FIXED_SIZE;

		ASSERT(status != WLC_E_STATUS_PARTIAL);
		wlc_bss_mac_event(wlc, cfg, WLC_E_ESCAN_RESULT, NULL, status,
		                  0, 0, &escan_result, escan_result.buflen);
	}
	else if (wlc->pub->scanresults_on_evtcplt) {
		/* copy all bss to scan_result and return to per-port layer */
		wl_scan_results_t *scan_result;
		uint32 i, scan_result_len = 0, len_left = 0;
		wl_bss_info_t *wl_bi;

		/* find total length */
		scan_result_len = OFFSETOF(wl_scan_results_t, bss_info);
		for (i = 0; i < wlc->scan_results->count; i++) {
			scan_result_len += sizeof(wl_bss_info_t);
			if (wlc->scan_results->ptrs[i]->bcn_prb_len > DOT11_BCN_PRB_LEN) {
				scan_result_len +=
					(ROUNDUP(wlc->scan_results->ptrs[i]->bcn_prb_len -
					DOT11_BCN_PRB_LEN, 4));
			}
		}
		if ((scan_result = (wl_scan_results_t*)MALLOC(wlc->osh, scan_result_len)) == NULL) {
			WL_ERROR(("wl%d: %s: Can't allocate scan_result\n", wlc->pub->unit,
				__FUNCTION__));
			return;
		}

		wl_bi = scan_result->bss_info;
		len_left = scan_result_len;

		for (i = 0; i < wlc->scan_results->count; i++) {
			wlc_bss_info_t *bi = wlc->scan_results->ptrs[i];
			/* convert to wl_bss_info */
			if (wlc_bss2wl_bss(wlc, bi, wl_bi, len_left, TRUE) != BCME_OK) {
				/* we should have allocated enough memory above */
				/* if fires, buf length needs to sync with wlc_bss2wl_bss */
				ASSERT(0);
				break;
			}
			/* update next scan_result->bss_info */
			len_left -= wl_bi->length;
			wl_bi = (wl_bss_info_t *)((char *)wl_bi + wl_bi->length);
		}
		scan_result->count = i;

		wlc_bss_mac_event(wlc, cfg, WLC_E_SCAN_COMPLETE, NULL, status, 0, 0,
			scan_result, scan_result_len);

		MFREE(wlc->osh, scan_result, scan_result_len);
	}
	else {
		wlc_bss_mac_event(wlc, cfg, WLC_E_SCAN_COMPLETE, NULL, status, 0, 0, 0, 0);
	}
	if (ISCAN_IN_PROGRESS(wlc))
		wl_del_timer(wlc->wl, wlc->iscan_timer);
#endif /* STA */

	if (status == WLC_E_STATUS_PARTIAL)
		status = WL_SCAN_RESULTS_PARTIAL;
	else if (status != WLC_E_STATUS_SUCCESS) {
		if (ISCAN_IN_PROGRESS(wlc))
			wlc->custom_iscan_results_state = WL_SCAN_RESULTS_ABORTED;
		else
			wlc->custom_scan_results_state = WL_SCAN_RESULTS_ABORTED;
		return;
	}

	/* release old BSS information */
	wlc_bss_list_free(wlc, wlc->custom_scan_results);

	/* copy scan results to custom_scan_results for reporting via ioctl */
	wlc_bss_list_xfer(wlc->scan_results, wlc->custom_scan_results);

	if (ISCAN_IN_PROGRESS(wlc)) {
		wlc->custom_iscan_results_state = status;
	} else if (wlc->custom_scan_results_state == WL_SCAN_RESULTS_PENDING)
		wlc->custom_scan_results_state = status;
}

int
wlc_custom_scan(wlc_info_t *wlc, char *arg, int arg_len,
	chanspec_t chanspec_start, int macreq, wlc_bsscfg_t *cfg)
{
	wl_scan_params_t params;
	uint nssid = 0;
	wlc_ssid_t *ssids = NULL;
	int bss_type = DOT11_BSSTYPE_ANY;
	const struct ether_addr* bssid = &ether_bcast;
	int scan_type = -1;
	int nprobes = -1;
	int active_time = -1;
	int passive_time = -1;
	int home_time = -1;
	chanspec_t* chanspec_list = NULL;
	uint chanspec_num = 0;
	int bcmerror = BCME_OK;
	uint i;
	uint scan_flags = 0;

	/* default to single fixed-part ssid */
	ssids = &params.ssid;

	if (arg_len >= WL_SCAN_PARAMS_FIXED_SIZE) {
		/* full wl_scan_params_t provided */
		bcopy(arg, &params, WL_SCAN_PARAMS_FIXED_SIZE);
		bssid = &params.bssid;
		bss_type = (int)params.bss_type;
		nprobes = (int)params.nprobes;
		active_time = (int)params.active_time;
		passive_time = (int)params.passive_time;
		home_time = (int)params.home_time;

		if (params.scan_type == (uint8)-1)
			scan_type = -1;
		else if (params.scan_type == 0)
			scan_type = DOT11_SCANTYPE_ACTIVE;
		else {
			if (params.scan_type & WL_SCANFLAGS_PASSIVE)
				scan_type = DOT11_SCANTYPE_PASSIVE;
			scan_flags = params.scan_type;
		}

		chanspec_num = (uint)(params.channel_num & WL_SCAN_PARAMS_COUNT_MASK);
		nssid = (uint)((params.channel_num >> WL_SCAN_PARAMS_NSSID_SHIFT) &
		               WL_SCAN_PARAMS_COUNT_MASK);
		if ((int)(WL_SCAN_PARAMS_FIXED_SIZE + chanspec_num * sizeof(uint16)) > arg_len) {
			bcmerror = BCME_BUFTOOSHORT; /* arg buffer too short */
			goto done;
		}

		/* Optional remainder (chanspecs/ssids) will be used in place but
		 * may be unaligned.  Fixed portion copied out above, so use that
		 * space to copy remainder to max required alignment boundary.
		 */
		STATIC_ASSERT(OFFSETOF(wl_scan_params_t, channel_list) >= sizeof(uint32));
		arg += OFFSETOF(wl_scan_params_t, channel_list);
		arg_len -= OFFSETOF(wl_scan_params_t, channel_list);
		chanspec_list = (chanspec_t*)((uintptr)arg & ~(sizeof(uint32) - 1));
		bcopy(arg, (char*)chanspec_list, arg_len);

		if (chanspec_num > 0) {
			ASSERT(ISALIGNED(chanspec_list, sizeof(uint16)));
			/* if chanspec is valid leave it alone, if not assume its really
			 * just a 20Mhz channel number and convert it to a chanspec
			 */
			for (i = 0; i < chanspec_num; i++) {
				uint16 ch = chanspec_list[i] & WL_CHANSPEC_CHAN_MASK;

				/* Magic value of -1 to abort an existing scan, else just return */
				if (chanspec_list[i] == (uint16) -1) {
#ifdef STA
					wlc_bsscfg_t *bc = wlc->as_info->assoc_req[0];
					wlc_assoc_t *as = bc != NULL ? bc->assoc : NULL;

					if (as != NULL && (as->type == AS_ASSOCIATION ||
						as->type == AS_RECREATE ||
						as->type == AS_ROAM)) {
						wlc_assoc_abort(bc);
					} else
#endif
					{
						wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
					}

					bcmerror = WL_SCAN_RESULTS_ABORTED;
					goto done;
				}

				if (wf_chspec_malformed(chanspec_list[i]))
					chanspec_list[i] = CH20MHZ_CHSPEC(ch);
			}
		}

		/* locate appended ssids */
		if (nssid) {
			uint offset = chanspec_num * sizeof(uint16);
			offset = ROUNDUP(offset, sizeof(uint32));
			if ((uint)arg_len < (offset + nssid * sizeof(wlc_ssid_t))) {
				bcmerror = BCME_BUFTOOSHORT;
				goto done;
			}
			ssids = (wlc_ssid_t *)((char*)chanspec_list + offset);
			ASSERT(ISALIGNED(ssids, sizeof(uint32)));
		}
	} else if (arg_len >= (int)sizeof(uint32)) {
		/* just an SSID provided */
		bcopy(arg, &params.ssid, MIN((int)sizeof(wlc_ssid_t), arg_len));
		if ((uint)arg_len < params.ssid.SSID_len) {
			bcmerror = BCME_BUFTOOSHORT;
			goto done;
		}
	} else {
		bcmerror = BCME_BUFTOOSHORT;
		goto done;
	}

	if (nssid == 0) {
		nssid = 1;
		ssids = &params.ssid;
	}

	for (i = 0; i < nssid; i++) {
		if (ssids[i].SSID_len > DOT11_MAX_SSID_LEN) {
			bcmerror = BCME_BADSSIDLEN;
			goto done;
		}
	}

	WL_ASSOC(("SCAN: WLC_SCAN custom scan\n"));
	bcmerror = wlc_scan_request_ex(wlc, bss_type, bssid, nssid, ssids,
	                               scan_type, nprobes, active_time, passive_time,
	                               home_time, chanspec_list, chanspec_num,
	                               chanspec_start, TRUE, wlc_custom_scan_complete,
	                               wlc, macreq, scan_flags, cfg, NULL, NULL);
	if (!bcmerror)
		wlc_bss_list_free(wlc, wlc->custom_scan_results);
done:
	if (bcmerror) {
		if (macreq == WLC_ACTION_ISCAN)
			wlc->custom_iscan_results_state = WL_SCAN_RESULTS_ABORTED;
		else if (bcmerror != BCME_NOTREADY)
			wlc->custom_scan_results_state = WL_SCAN_RESULTS_ABORTED;
	} else {
		/* invalidate results for other scan method's query */
		if (macreq == WLC_ACTION_ISCAN) {
			wlc->custom_scan_results_state = WL_SCAN_RESULTS_ABORTED;
			wlc->custom_iscan_results_state = WL_SCAN_RESULTS_PENDING;
		} else if (macreq == WLC_ACTION_SCAN) {
			wlc->custom_iscan_results_state = WL_SCAN_RESULTS_ABORTED;
			wlc->custom_scan_results_state = WL_SCAN_RESULTS_PENDING;
		}
	}
	return bcmerror;
}

static int
wlc_scan_results(wlc_info_t *wlc, wl_scan_results_t *iob, int *len, uint results_state)
{
	wl_bss_info_t *wl_bi;
	uint32 totallen = 0;
	uint32 datalen;
	wlc_bss_info_t **scan_results = 0;
	uint scan_results_num = 0;
	int bcmerror = BCME_OK;
	uint i = 0, start_idx = 0;
	int buflen = *len;

	iob->version = WL_BSS_INFO_VERSION;
	datalen = WL_SCAN_RESULTS_FIXED_SIZE;

	if (results_state == WL_SCAN_RESULTS_PENDING) {
		bcmerror = BCME_NOTREADY;
		goto done;
	}

	if (results_state == WL_SCAN_RESULTS_ABORTED) {
		bcmerror = BCME_NOTFOUND;
		goto done;
	}

	scan_results_num = wlc->custom_scan_results->count;

	scan_results = wlc->custom_scan_results->ptrs;

	/* If part of the results were returned before, start from there */
	if (ISCAN_RESULTS_OK(wlc) && wlc->iscan_result_last) {
		start_idx = wlc->iscan_result_last;
		wlc->iscan_result_last = 0;
	}

	/* calc buffer length: fixed + IEs */
	totallen = WL_SCAN_RESULTS_FIXED_SIZE +
	    (scan_results_num - start_idx) * sizeof(wl_bss_info_t);
	for (i = start_idx; i < scan_results_num; i ++) {
		ASSERT(scan_results[i] != NULL);
		if (scan_results[i]->bcn_prb_len > DOT11_BCN_PRB_LEN)
			totallen += ROUNDUP(scan_results[i]->bcn_prb_len -
			                    DOT11_BCN_PRB_LEN, 4);
	}

	/* convert each wlc_bss_info_t, writing into buffer */
	wl_bi = iob->bss_info;
	for (i = start_idx; i < scan_results_num; i++) {
		if (wlc_bss2wl_bss(wlc, scan_results[i], wl_bi, buflen, TRUE) == BCME_BUFTOOSHORT) {
#ifdef STA
			if (ISCAN_RESULTS_OK(wlc)) {
				 /* Remember the num of results returned */
				if (i != 0)
					wlc->iscan_result_last = i;
			}
#endif /* STA */
			break;
		}
		datalen += wl_bi->length;
		buflen -= wl_bi->length;
		wl_bi = (wl_bss_info_t *)((char *)wl_bi + wl_bi->length);

	}

done:
	/* buffer too short */
	if (iob->buflen < totallen) {
		/* return bytes needed if buffer is too short */
		iob->buflen = totallen;
		bcmerror = BCME_BUFTOOSHORT;
	} else
		iob->buflen = datalen;
	*len = datalen;
	iob->count = (i - start_idx);

	return bcmerror;
}

/** common AP/STA funnel function to perform operations when scb disassocs */
void
wlc_scb_disassoc_cleanup(wlc_info_t *wlc, struct scb *scb)
{
	wlc_bsscfg_t *bsscfg;
#if defined(BCMDBG) || defined(WLMSG_WSEC) || defined(WLMSG_BTA)
	char eabuf[ETHER_ADDR_STR_LEN];

	bcm_ether_ntoa(&scb->ea, eabuf);
#endif

	/* reset pairwise key on disassociate */
	wlc_keymgmt_reset(wlc->keymgmt, NULL, scb);

	bsscfg = SCB_BSSCFG(scb);
	/* delete *all* group keys on disassociate in STA mode with wpa */
	if (BSSCFG_STA(bsscfg) && (bsscfg->WPA_auth != WPA_AUTH_DISABLED) &&
		(bsscfg->WPA_auth != WPA_AUTH_NONE)) {
		wlc_keymgmt_reset(wlc->keymgmt, bsscfg, NULL);
	}

	scb->flags &= ~SCB_PENDING_PSPOLL;

#ifdef WLAMPDU
	/* cleanup ampdu at end of association */
	if (SCB_AMPDU(scb)) {
		WL_AMPDU(("wl%d: scb ampdu cleanup for %s\n", wlc->pub->unit, eabuf));
		scb_ampdu_cleanup(wlc, scb);
	}
#endif


	if (BSSCFG_AP(SCB_BSSCFG(scb))) {
	   /* If the STA has been configured to be monitored before,
		* association then continue sniffing that STA frames.
		*/
		if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &scb->ea))
			wlc_stamon_sta_sniff_enab(wlc->stamon_info, &scb->ea, TRUE);

		if (scb->wpaie) {
			MFREE(wlc->osh, scb->wpaie, scb->wpaie_len);
			scb->wpaie = NULL;
			scb->wpaie_len = 0;
		}
	}

	/* Reset PS state if needed */
	if (SCB_PS(scb))
		wlc_apps_scb_ps_off(wlc, scb, TRUE);

	scb->flags2 &= ~(SCB2_MFP|SCB2_SHA256|SCB2_CCX_MFP);

#ifdef PLC_WET
	if (PLC_ENAB(wlc->pub))
		wl_plc_node_delete(wlc->wl, scb->ea.octet, TRUE);
#endif /* PLC_WET */
#ifdef WLWNM
	if (WLWNM_ENAB(wlc->pub))
		wlc_wnm_scb_cleanup(wlc, scb);
#endif

	/* cleanup the a4-data frames flag */
	if (!SCB_LEGACY_WDS(scb))
		SCB_A4_DATA_DISABLE(scb);

#if defined(WL_PWRSTATS)
	/* Update connect time for primary infra STA only */
	if (PWRSTATS_ENAB(wlc->pub) && (bsscfg == wlc->cfg) &&
		BSSCFG_STA(bsscfg) && bsscfg->BSS)
		wlc_connect_time_upd(wlc);
#endif /* WL_PWRSTATS */
}

#ifdef STA
/**
 * callback function for the APSD Trigger Frame Timer. This is also used by WME_AUTO_TRIGGER routine
 * to send out QoS NULL DATA frame as trigger to AP
 */
void
wlc_apsd_trigger_timeout(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	wlc_info_t *wlc = cfg->wlc;
	struct scb *scb;

	WL_INFORM(("wl%d: %s, entering\n", wlc->pub->unit, __FUNCTION__));

	ASSERT(cfg->pm->PMenabled);
	ASSERT(cfg->associated);

	/*
	 * The Trigger Frame Timer has expired, so send a QoS NULL
	 * data packet of the highest WMM APSD-enabled AC priority.
	 * The Trigger Frame Timer will be reloaded elsewhere.
	 */
	scb = wlc_scbfindband(wlc, cfg, &cfg->BSSID,
	                      CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));
	if (scb == NULL) {
		WL_ERROR(("wl%d: %s : failed to find scb\n", wlc->pub->unit, __FUNCTION__));
		/* Link is down so clear the timer */
		wlc_apsd_trigger_upd(cfg, FALSE);
		return;
	}

	/*
	 * Don't send excessive null frames. Bail out if a QoS Data/Null frame
	 * was send out recently.
	 */
	if (scb->flags & SCB_SENT_APSD_TRIG) {
		scb->flags &= ~SCB_SENT_APSD_TRIG;
		WL_PS(("wl%d.%d: %s: trigger frame was recently send out\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return;
	}

	if (!_wlc_sendapsdtrigger(wlc, scb))
		WL_ERROR(("wl%d: failed to send apsd trigger frame\n", wlc->pub->unit));

	return;
}

/** Configure the WMM U-APSD trigger frame timer */
int
wlc_apsd_trigger_upd(wlc_bsscfg_t *cfg, bool allow)
{
	wlc_info_t *wlc = cfg->wlc;
	struct scb *scb;
	wlc_pm_st_t *pm = cfg->pm;
	int callbacks = 0;

	WL_TRACE(("wl%d: %s : state %d, timeout %d\n",
		wlc->pub->unit, __FUNCTION__, allow, pm->apsd_trigger_timeout));

	if (pm->apsd_trigger_timer == NULL) {
		WL_ERROR(("wl%d.%d: %s: Trying to update NULL apsd trigger timer.\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return 0;
	}

	if (!wl_del_timer(wlc->wl, pm->apsd_trigger_timer))
		callbacks ++;

	if (!allow || !cfg->associated || !pm->PMenabled ||
	    pm->apsd_trigger_timeout == 0)
		return callbacks;

	/*
	 * - check for WMM
	 * - check for APSD trigger- & delivery- enabled ACs
	 * - set timeout
	 * - enable timer as necessary
	 */
	scb = wlc_scbfindband(wlc, cfg, &cfg->BSSID,
	                      CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));
	if (scb != NULL && SCB_WME(scb) && scb->apsd.ac_trig) {
		/* set timer for half the requested period so that we send frame
		 * only if there was no tx activity (in the APSD ac for the last
		 * 1/2 of the trigger timeout period.
		 */
		wl_add_timer(wlc->wl, pm->apsd_trigger_timer, pm->apsd_trigger_timeout / 2, TRUE);
	} else {
		WL_PS(("wl%d.%d: %s: apsd trigger timer not set\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
	}

	return 0;
}

void
wlc_bss_clear_bssid(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	bzero(&cfg->BSSID, ETHER_ADDR_LEN);
	bzero(&cfg->current_bss->BSSID, ETHER_ADDR_LEN);
	wlc_clear_bssid(cfg);

	wlc->stas_connected = wlc_stas_connected(wlc);

	wlc_enable_btc_ps_protection(wlc, cfg, FALSE);

	if (cfg->pm->PMpending)
		wlc_update_pmstate(cfg, TX_STATUS_BE);

	/* Clear the quiet flag and transfer the packets */
	wlc_bsscfg_tx_start(cfg);
}

uint32
wlc_watchdog_backup_bi(wlc_info_t * wlc)
{
	uint32 bi;
	bi = 2*wlc->cfg->current_bss->dtim_period * wlc->cfg->current_bss->beacon_period;
	if (wlc->bcn_li_dtim)
		bi *= wlc->bcn_li_dtim;
	else if (wlc->bcn_li_bcn)
		/* recalculate bi based on bcn_li_bcn */
		bi = 2*wlc->bcn_li_bcn*wlc->cfg->current_bss->beacon_period;

	if (bi < 2*TIMER_INTERVAL_WATCHDOG)
		bi = 2*TIMER_INTERVAL_WATCHDOG;
	return bi;
}

/* Change to run the watchdog either from a periodic timer or from tbtt handler.
 * Call watchdog from tbtt handler if tbtt is TRUE, watchdog timer otherwise.
 */
void
wlc_watchdog_upd(wlc_bsscfg_t *cfg, bool tbtt)
{
	wlc_info_t *wlc = cfg->wlc;

	if (cfg != wlc->cfg)
		return;

	/* make sure changing watchdog driver is allowed */
	if (!wlc->pub->up)
		return;
	if (!wlc->pub->align_wd_tbtt) {
#ifdef LPAS
		/* NIC builds do not have align_wd_tbtt set to TRUE and may not
		 * expect wdog timer to be restarted everytime wlc_watchdog_upd is invoked.
		 */
		if (wlc->lpas == 1) {
			wl_add_timer(wlc->wl, wlc->wdtimer, TIMER_INTERVAL_WATCHDOG, TRUE);
			wlc->WDarmed = TRUE;
			WL_INFORM(("wl%d: wlc_watchdog_upd: align_wd disallowed\n",
				WLCWLUNIT(wlc)));
		}
#endif /* LPAS */
		return;
	}

	if (!tbtt && wlc->WDarmed) {
		wl_del_timer(wlc->wl, wlc->wdtimer);
		wlc->WDarmed = FALSE;
	}

	/* stop watchdog timer and use tbtt interrupt to drive watchdog */
	if (tbtt && wlc->WDarmed) {
		wl_del_timer(wlc->wl, wlc->wdtimer);
		wlc->WDarmed = FALSE;
		wlc->WDlast = OSL_SYSUPTIME();
		WL_INFORM(("wl%d: wlc_watchdog_upd: tbtt handler is chosen\n", WLCWLUNIT(wlc)));
	}
	/* arm watchdog timer and drive the watchdog there */
	else if (!tbtt && !wlc->WDarmed) {
		wl_add_timer(wlc->wl, wlc->wdtimer, TIMER_INTERVAL_WATCHDOG, TRUE);
		wlc->WDarmed = TRUE;
		WL_INFORM(("wl%d: wlc_watchdog_upd: watchdog timer is armed\n", WLCWLUNIT(wlc)));
	}
	if (tbtt && !wlc->WDarmed) {
		wl_add_timer(wlc->wl, wlc->wdtimer, wlc_watchdog_backup_bi(wlc), TRUE);
		wlc->WDarmed = TRUE;
	}
}
#endif /* STA */

/** return true if the rateset contains an OFDM rate */
bool
wlc_rateset_isofdm(uint count, uint8 *rates)
{
	int i;

	for (i = count - 1; i >= 0; i--)
		if (RATE_ISOFDM(rates[i]))
			return (TRUE);
	return (FALSE);
}

ratespec_t
wlc_lowest_basic_rspec(wlc_info_t *wlc, wlc_rateset_t *rs)
{
#if defined(NEW_TXQ) && defined(USING_MUX)
#define _LOWEST_BASIC_RATE(rate) (LEGACY_RSPEC(rate))
#else
#define _LOWEST_BASIC_RATE(rate) ((rate) & RATE_MASK)
#endif /* defined(NEW_TXQ) && defined (USING_MUX) */

	ratespec_t lowest_basic_rspec;
	uint i;

	/* Use the lowest basic rate */
	lowest_basic_rspec = _LOWEST_BASIC_RATE(rs->rates[0]);
	for (i = 0; i < rs->count; i++) {
		if (rs->rates[i] & WLC_RATE_FLAG) {

			lowest_basic_rspec = _LOWEST_BASIC_RATE(rs->rates[i]);

			break;
		}
	}

#ifdef WL11N
	if (IS_OFDM(lowest_basic_rspec)) {
		if (WLCISNPHY(wlc->band) &&
		    wlc->stf->ss_opmode == PHY_TXC1_MODE_CDD) {
			lowest_basic_rspec |= (1 << RSPEC_TXEXP_SHIFT);
		}
		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			uint ntx = wlc_stf_txchain_get(wlc, lowest_basic_rspec);
			uint nss = wlc_ratespec_nss(lowest_basic_rspec);
			lowest_basic_rspec |= ((ntx - nss) << RSPEC_TXEXP_SHIFT);
		}
	}
#endif

	return (lowest_basic_rspec);
}

/* This function changes the phytxctl for beacon based on current beacon ratespec AND txant
 * setting as per this table:
 *  ratespec     CCK		ant = wlc->stf->txant
 *  		OFDM		ant = 3
 */
void
wlc_beacon_phytxctl_txant_upd(wlc_info_t *wlc, ratespec_t bcn_rspec)
{
	uint16 phytxant = wlc->stf->phytxant;
	uint16 mask = PHY_TXC_ANT_MASK;
	uint shm_bcn_phy_ctlwd0;


	if (D11REV_GE(wlc->pub->corerev, 40))
		shm_bcn_phy_ctlwd0 = D11AC_BCN_TXPCTL0;
	else
		shm_bcn_phy_ctlwd0 = D11_M_BCN_PCTLWD;

	/* for non-siso rates or default setting, use the available chains */
	if (WLC_PHY_11N_CAP(wlc->band) || WLC_PHY_VHT_CAP(wlc->band)) {
		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			mask = PHY_TXC_HTANT_MASK;
			if (bcn_rspec == 0)
				bcn_rspec = wlc->bcn_rspec;
		}
		phytxant = wlc_stf_phytxchain_sel(wlc, bcn_rspec);
	}
	WL_NONE(("wl%d: wlc_beacon_phytxctl_txant_upd: beacon txant 0x%04x mask 0x%04x\n",
		wlc->pub->unit, phytxant, mask));
	wlc_update_shm(wlc, shm_bcn_phy_ctlwd0, phytxant, mask);
}

/**
 * This function doesn't change beacon body(plcp, mac hdr). It only updates the
 *   phyctl0 and phyctl1 with the exception of tx antenna,
 *   which is handled in wlc_stf_phy_txant_upd() and wlc_beacon_phytxctl_txant_upd()
 */
void
wlc_beacon_phytxctl(wlc_info_t *wlc, ratespec_t bcn_rspec, chanspec_t chanspec)
{
	uint16 phyctl;
	int rate;
	wlcband_t *band;

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		txpwr204080_t txpwrs;
		bcn_rspec &= ~RSPEC_BW_MASK;
		bcn_rspec |= RSPEC_BW_20MHZ;

		phyctl = wlc_acphy_txctl0_calc(wlc, bcn_rspec, WLC_LONG_PREAMBLE);
		wlc_write_shm(wlc, D11AC_BCN_TXPCTL0, phyctl);
		if (wlc_stf_get_204080_pwrs(wlc, bcn_rspec, &txpwrs) != BCME_OK) {
			ASSERT(!"beacon phyctl1 ppr returns error!");
		}
		phyctl = wlc_acphy_txctl1_calc(wlc, bcn_rspec, 0,
			txpwrs.pbw[BW20_IDX][TXBF_OFF_IDX]);
		wlc_write_shm(wlc, D11AC_BCN_TXPCTL1, phyctl);
		phyctl = wlc_acphy_txctl2_calc(wlc, bcn_rspec);
		wlc_write_shm(wlc, D11AC_BCN_TXPCTL2, phyctl);
		return;
	}

	phyctl = wlc_read_shm(wlc, D11_M_BCN_PCTLWD);

	phyctl &= ~PHY_TXC_FT_MASK;
	phyctl |= PHY_TXC_FRAMETYPE(bcn_rspec);
	/* ??? If ever beacons use short headers or phy pwr override, add the proper bits here. */

	rate = RSPEC2RATE(bcn_rspec);
	band = wlc->bandstate[CHSPEC_BANDUNIT(chanspec)];
	if ((WLCISNPHY(band) || WLCISHTPHY(band)) && IS_CCK(bcn_rspec))
		phyctl |= ((rate * 5) << 10);

	wlc_write_shm(wlc, D11_M_BCN_PCTLWD, phyctl);

	if (WLC_PHY_11N_CAP(band) || WLCISLPPHY(band)) {
		uint16 phyctl1;

		if (!WLCISLPPHY(band) && !WLCISLCNPHY(band)) {
			bcn_rspec &= ~RSPEC_BW_MASK;
			bcn_rspec |= RSPEC_BW_20MHZ;
		}

		phyctl1 = wlc_phytxctl1_calc(wlc, bcn_rspec, chanspec);
		wlc_write_shm(wlc, D11_M_BCN_PCT1WD, phyctl1);
	}
}

#ifdef BCMASSERT_SUPPORT
/** Validate the beacon phytxctl given current band */
static bool
wlc_valid_beacon_phytxctl(wlc_info_t *wlc)
{
	uint16 phyctl;
	uint shm_bcn_phy_ctlwd0;

	if (D11REV_GE(wlc->pub->corerev, 40))
		shm_bcn_phy_ctlwd0 = D11AC_BCN_TXPCTL0;
	else
		shm_bcn_phy_ctlwd0 = D11_M_BCN_PCTLWD;

	phyctl = wlc_read_shm(wlc, shm_bcn_phy_ctlwd0);

	return ((phyctl & PHY_TXC_FT_MASK) == PHY_TXC_FRAMETYPE(wlc->bcn_rspec));
}

/**
 * pass cfg pointer for future "per BSS bcn" validation - it is NULL
 * when the function is called from STA association context.
 */
void
wlc_validate_bcn_phytxctl(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int idx;
	wlc_bsscfg_t *bc;

	FOREACH_BSS(wlc, idx, bc) {
		if (HWBCN_ENAB(bc)) {
			ASSERT(wlc_valid_beacon_phytxctl(wlc));
			break;
		}
	}
}
#endif /* BCMASSERT_SUPPORT */

static void
wlc_BSSinit_rateset_filter(wlc_bsscfg_t *cfg)
{
	uint i, val;
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	/* Japan Channel 14 restrictions */
	if ((wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC) == 14) && (wlc_japan(wlc) == TRUE)) {
		/* Japan does not allow OFDM or MIMO on channel 14 */
		if (wlc->band->gmode) {
			/* filter-out OFDM rates and MCS set */
			wlc_rateset_filter(&current_bss->rateset, &current_bss->rateset,
			                   FALSE, WLC_RATES_CCK, RATE_MASK_FULL, 0);
		}
	}


	/* filter-out unsupported rates */
	wlc_rate_hwrs_filter_sort_validate(&current_bss->rateset,
		&wlc->band->hw_rateset, FALSE, wlc->stf->txstreams);

	/* Keep only CCK if gmode == GMODE_LEGACY_B */
	if (BAND_2G(wlc->band->bandtype) && (wlc->band->gmode == GMODE_LEGACY_B))
		wlc_rateset_filter(&current_bss->rateset, &current_bss->rateset,
		                   FALSE, WLC_RATES_CCK, RATE_MASK_FULL, 0);

	/* if empty or no basic rates - use driver default rateset */
	/* ??? should fixup all WLC_RATE_FLAG reference to remove this requirement */
	val = 0;
	for (i = 0; i < current_bss->rateset.count; i++)
		if (current_bss->rateset.rates[i] & WLC_RATE_FLAG)
			val = 1;

	if (val == 0) {
		wlc_default_rateset(wlc, &current_bss->rateset);
		/* Keep only CCK if gmode == GMODE_LEGACY_B */
		if (BAND_2G(wlc->band->bandtype) && (wlc->band->gmode == GMODE_LEGACY_B))
			wlc_rateset_filter(&current_bss->rateset, &current_bss->rateset,
			                   FALSE, WLC_RATES_CCK, RATE_MASK_FULL, 0);
	}

	/* OFDM rates only */
	if (
#if defined(WLP2P)
	    BSS_P2P_ENAB(wlc, cfg) ||
#endif
#if defined(AP)
	    (cfg->rateset == WLC_BSSCFG_RATESET_OFDM) ||
#endif
	    FALSE) {
		wlc_rateset_filter(&current_bss->rateset, &current_bss->rateset, FALSE,
		                   WLC_RATES_OFDM, RATE_MASK_FULL, wlc_get_mcsallow(wlc, cfg));
	}

	wlc_rate_lookup_init(wlc, &current_bss->rateset);
}

/* Minimum pretbtt time required for PHY specific power up.  This time does not
 *   include the preceeding time necessary to power up the non phy related
 *   clocks, etc.  This time is SYNTHPU_* and is separately communicated to the
 *   ucode via shared memory.  Ucode adds SYNTHPU and PRETBTT to determine
 *   when to wake for a TBTT.  The pretbtt interrupt is given by ucode to the
 *   driver at PRETBTT time befoe TBTT, not when it first wakes up.
 */
static uint
wlc_pretbtt_min(wlc_info_t *wlc)
{
	wlcband_t *band = wlc->band;
	uint pretbtt;


	if (ISSIM_ENAB(wlc->pub->sih))
		pretbtt = PRETBTT_PHY_US_QT;
	else if (WLCISAPHY(band))
		pretbtt = PRETBTT_APHY_US;
	else if (WLCISLPPHY(band)) {
		pretbtt =  LPREV_GE(band->phyrev, 2) ? PRETBTT_LPPHY_US : PRETBTT_BPHY_US;
	} else if (WLCISSSLPNPHY(band))
		pretbtt = PRETBTT_SSLPNPHY_US;
	else if (WLCISNPHY(band)) {
		pretbtt = NREV_GE(band->phyrev, 3) ? PRETBTT_NPHY_US : PRETBTT_BPHY_US;
	} else if (WLCISHTPHY(band)) {
		pretbtt = PRETBTT_HTPHY_US;
	} else if (WLCISACPHY(band)) {
		pretbtt = PRETBTT_ACPHY_US;
	} else if (WLCISLCNPHY(band)) {
		if (CHIPID(wlc->pub->sih->chip) == BCM4336_CHIP_ID)
			pretbtt = PRETBTT_LCNPHY_4336_US;
		else
			pretbtt = PRETBTT_LCNPHY_US;
	} else if (WLCISLCN40PHY(band)) {
		pretbtt = PRETBTT_LCN40PHY_US;
	} else
		pretbtt = PRETBTT_BPHY_US;

	return pretbtt;
}

/**
 * Update PRETBTT: use the maximum value of all needs. Look at the given bsscfg's states as well as
 * other bsscfgs' states to decide what to do.
 */
uint16
wlc_pretbtt_calc(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	uint mbss_pretbtt = 0;
	uint bss_pretbtt;
	uint pretbtt;

	/* MBSS pretbtt needs */
	if (MBSS_ENAB(wlc->pub) && BSSCFG_AP(cfg))
		mbss_pretbtt = MBSS_PRE_TBTT_DEFAULT_us;

	/* Normal BSS (i.e. non-MBSS) phy specific pretbtt value.  See wlc_pretbtt_min() functions
	 *   comment for more details on exactly for a definition of this value and how it is
	 *   used by the ucode.
	 */
	bss_pretbtt = wlc_pretbtt_min(wlc);

	if (WLCISACPHY(wlc->band)) {
		if (!MBSS_ENAB(wlc->pub) && BSSCFG_AP(cfg)) {
			bss_pretbtt = PRETBTT_ACPHY_AP_US;
		} else if (CHIPID(wlc->pub->sih->chip) == BCM4335_CHIP_ID) {
			/* WAR for 43162 dtim 3 power issue */
			if (wlc->pub->sih->bustype == PCI_BUS)
				bss_pretbtt = PRETBTT_ACPHY_43162_US;
			else
				bss_pretbtt = PRETBTT_ACPHY_4335_US;
		} else {
			bss_pretbtt = PRETBTT_ACPHY_US;
		}
	}

	/* THIS IS THE MINIMUM PRETBTT REQUIREMENT */
	pretbtt = MAX(mbss_pretbtt, bss_pretbtt);
	if (pretbtt == 0)
		pretbtt = 2;

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		pretbtt += wlc_mchan_get_pretbtt_time(wlc->mchan);
	}
#endif

#ifdef WLTDLS
	if (TDLS_ENAB(wlc->pub)) {
		WL_INFORM(("wl%d.%d:%s(): config pretbtt = %d\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pretbtt));
		pretbtt += wlc_tdls_get_pretbtt_time(wlc->tdls, cfg);
	}
#endif

	/* per BSS pretbtt, pick a maximum value amongst all registered clients */
	pretbtt = wlc_bss_pretbtt_query(wlc, cfg, pretbtt);

	WL_INFORM(("wl%d.%d: %s: pretbtt = %d\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pretbtt));

	return (uint16)pretbtt;
}

/** Set pre Beacon Transmission Time. Mac is assumed to be suspended at this point */
void
wlc_pretbtt_set(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	uint16 phyctl = wlc_pretbtt_calc(cfg);

#ifdef WLMCNX
	/* STA uses per BSS PreTBTT setting when p2p ucode is used */
	/* When WOWL is active, pretbtt needs to be set. */
	if (MCNX_ENAB(wlc->pub) && !BSSCFG_AP(cfg) && !WOWL_ACTIVE(wlc->pub))
		return;
#endif

	W_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_cfppretbtt, phyctl);	/* IHR */
	wlc_write_shm(wlc, M_PRETBTT, phyctl);		/* SHM */
}

void
wlc_dtimnoslp_set(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	uint16 noslp = wlc_pretbtt_calc(cfg);

	noslp += wlc->bcn_wait_prd << 10;

	wlc_write_shm(wlc, M_NOSLPZNATDTIM, noslp >> 3);
}

void
wlc_macctrl_init(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint32 mask = (MCTL_INFRA | MCTL_DISCARD_PMQ | MCTL_CLOSED_NETWORK);
	uint32 val = 0;
	int idx;
	wlc_bsscfg_t *bc = NULL;

	/* infrastructure vs ad-hoc, STA vs AP, and PMQ use */
	/* Set AP if this BSS is AP, or an AP BSS is up */
	if ((cfg && BSSCFG_AP(cfg)) || AP_ACTIVE(wlc)) {
		wlc_validate_bcn_phytxctl(wlc, cfg); /* Can handle NULL cfg */
		wlc_ap_ctrl(wlc, TRUE, cfg, -1);     /* Can handle NULL cfg */
	}
	/* As PMQ entries are currently used only by AP for detecting PM Mode transitions
	 * for STA, instruct ucode not to fill up entries in PMQ
	 * CAUTION: For STA, this bit will need to be cleared if we ever support IBSS using
	 * PMQ. Also, validate the phytxctl for the beacon
	 */
	else
		val |= MCTL_DISCARD_PMQ;

	/* Set infrastructure mode if this BSS is INFRA, or other INFRA BSS are up */
	if ((cfg && cfg->BSS) || AP_ACTIVE(wlc) ||
	    (wlc->stas_associated > 0 && wlc->ibss_bsscfgs == 0))
		val |= MCTL_INFRA;

	/* if this is the bsscfg for ucode beacon/probe responses,
	 * indicate whether this is a closed network
	 */
	if (cfg && HWPRB_ENAB(cfg) && cfg->closednet_nobcprbresp)
		bc = cfg;
	else {
		FOREACH_BSS(wlc, idx, bc) {
			if (bc != cfg && bc->associated &&
			    HWPRB_ENAB(bc) && bc->closednet_nobcprbresp)
				break;
			bc = NULL;
		}
	}
	if (bc != NULL && !MBSS_ENAB(wlc->pub)) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif
		WL_INFORM(("wl%d: BSS %s is configured as a Closed Network, "
		           "enable MCTL bit in PSM\n", wlc->pub->unit,
		           bcm_ether_ntoa(&cfg->BSSID, eabuf)));
		val |= MCTL_CLOSED_NETWORK;
	}

	wlc_mctrl(wlc, mask, val);
}

/* adjust h/w TSF counters as well SHM TSF offsets if applicable.
 * 'tgt' is the target h/w TSF time
 * 'ref' is the current h/w TSF time
 * 'nxttbtt' is the next TBTT time
 * 'bcnint' is the TBTT interval in us units
 * 'comp' indicates if 'tgt' needs to be compensated
 */
void
wlc_tsf_adj(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 tgt_h, uint32 tgt_l,
	uint32 ref_h, uint32 ref_l, uint32 nxttbtt, uint32 bcnint, bool comp)
{
	int32 off_h, off_l;
	uint cfpp = 0;
	osl_t *osh = wlc->osh;
	d11regs_t *regs = wlc->regs;

	/* program the ATIM window */
	if (!cfg->BSS) {
		if (cfg->current_bss->atim_window != 0) {
			/* note that CFP is present */
			cfpp = CFPREP_CFPP;
		}
	}

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		/* make sure the ucode is awake and takes the adjustment... */
		wlc_mcnx_mac_suspend(wlc->mcnx);
	}
#endif

	/* add the time has passed by so far from the ref time */
	if (comp) {
		uint32 now_h, now_l;

		wlc_read_tsf(wlc, &now_l, &now_h);
		off_h = now_h;
		off_l = now_l;
		wlc_uint64_sub((uint32 *)&off_h, (uint32 *)&off_l, ref_h, ref_l);
		wlc_uint64_add(&tgt_h, &tgt_l, off_h, off_l);
	}

	WL_ASSOC(("wl%d.%d: %s: tgt 0x%x%08x, ref 0x%x%08x, tbtt 0x%x, interval 0x%x, "
	          "compensate %d\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
	          tgt_h, tgt_l, ref_h, ref_l, nxttbtt, bcnint, comp));

	/* Hold TBTTs */
	wlc_mctrl(wlc, MCTL_TBTT_HOLD, MCTL_TBTT_HOLD);

	/* low first, then high so the 64bit write is atomic */
	W_REG(osh, &regs->tsf_timerlow, tgt_l);
	W_REG(osh, &regs->tsf_timerhigh, tgt_h);

	/* write the beacon interval (to TSF) */
	W_REG(osh, &regs->tsf_cfprep, (bcnint << CFPREP_CBI_SHIFT) | cfpp);

	/* write the tbtt time */
	W_REG(osh, &regs->tsf_cfpstart, nxttbtt);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		/* adjust other connections' TSF offset */
		off_h = tgt_h;
		off_l = tgt_l;
		wlc_uint64_sub((uint32 *)&off_h, (uint32 *)&off_l, ref_h, ref_l);
		wlc_mcnx_tbtt_adj_all(wlc->mcnx, off_h, off_l);
	}
#endif

	/* Release hold on TBTTs */
	wlc_mctrl(wlc, MCTL_TBTT_HOLD, 0);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		/* restore ucode running state */
		wlc_mcnx_mac_resume(wlc->mcnx);
	}
#endif
}

void
wlc_BSSinit(wlc_info_t *wlc, wlc_bss_info_t *bi, wlc_bsscfg_t *cfg, int type)
{
	uint bcnint, bcnint_us;
	const bool restricted_ap = BSSCFG_AP(cfg) && (wlc->stas_associated > 0);
	osl_t *osh = wlc->osh;
	struct scb *scb;
#ifdef STA
	wlc_roam_t *roam = cfg->roam;
#endif
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlc_bss_info_t *default_bss = wlc->default_bss;


	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

	WL_APSTA_UPDN(("wl%d: wlc_BSSinit(), %s, %s, stas/aps/associated %d/%d/%d\n",
		wlc->pub->unit, (BSSCFG_AP(cfg) ? "AP" : "STA"),
		(type == WLC_BSS_START ? "START" : "JOIN"),
		wlc->stas_associated, wlc->aps_associated, wlc->pub->associated));

	/* update our infrastructure mode */
	wlc_bsscfg_set_infra_mode(wlc, cfg, bi->infra == 1);

	/* clear out bookkeeping */
#ifdef STA
	/* Only clear when STA (re)joins a BSS */
	if (BSSCFG_STA(cfg)) {
		roam->time_since_bcn = 0;
#ifdef WL_BCN_COALESCING
		roam->old_bcn = FALSE;
#endif /* WL_BCN_COALESCING */
		roam->bcns_lost = FALSE;
		roam->tsf_l = roam->tsf_h = 0;
		roam->original_reason = WLC_E_REASON_INITIAL_ASSOC;
	}
#endif /* STA */

	if (wlc->aps_associated == 0 || !cfg->BSS) {
		wlc_tpc_reset_all(wlc->tpc);
	}

#ifdef STA
	/* Reset the frequency tracking b/w to default values */
	if (BSSCFG_STA(cfg))
		wlc_freqtrack_reset(wlc);
#endif /* STA */

	/* update current BSS information */
	if (bi != current_bss) {
		if (current_bss->bcn_prb)
			MFREE(osh, current_bss->bcn_prb, current_bss->bcn_prb_len);
		current_bss->bcn_prb = NULL;
		current_bss->bcn_prb_len = 0;
		bcopy((char*)bi, (char*)current_bss, sizeof(wlc_bss_info_t));
		if (bi->bcn_prb) {
			/* need to copy bcn_prb, too */
			current_bss->bcn_prb =
			        (struct dot11_bcn_prb *)MALLOC(osh, bi->bcn_prb_len);
			if (current_bss->bcn_prb) {
				bcopy((char*)bi->bcn_prb, (char*)current_bss->bcn_prb,
					bi->bcn_prb_len);
				current_bss->bcn_prb_len = bi->bcn_prb_len;
			} else {
				WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(osh)));
			}
		}
	}

#ifdef STA
	/* apply the capability and additional IE params stored in wlc_join_BSS() */
	if (N_ENAB(wlc->pub) && type == WLC_BSS_JOIN) {

	}
#endif

	/* Validate chanspec in case the locale has changed */
	if (BSSCFG_AP(cfg) &&
	    !wlc_valid_chanspec_db(wlc->cmi, current_bss->chanspec)) {
		ASSERT(!cfg->associated);
		wlc_bsscfg_set_current_bss_chan(cfg, wlc_default_chanspec(wlc->cmi, FALSE));
	}

#ifdef WLMCHAN
	/* AP does not have a MCHAN context at this point so
	 * create the MCHAN context here
	 */
	if (MCHAN_ENAB(wlc->pub) && BSSCFG_AP(cfg)) {
		if (wlc_mchan_create_bss_chan_context(wlc, cfg, current_bss->chanspec)) {
			WL_ERROR(("wl%d : %s Failed to create bss_chan_context\n",
			          wlc->pub->unit, __FUNCTION__));
		}
	}
#endif /* WLMCHAN */

	/* no need to change the wlc->home_chanspec and the radio chanspec
	 * if we will schedule our channel access through excursions,
	 * we just need to somehow install the PHY cache here.
	 */
	if (BSS_EXCRX_ENAB(wlc, cfg)) {
		if (WLC_BAND_PI_RADIO_CHANSPEC != current_bss->chanspec) {
			wlc_set_chanspec(wlc, current_bss->chanspec);
			wlc_set_chanspec(wlc, WLC_BAND_PI_RADIO_CHANSPEC);
		}
	} else {
		/*
		 * Set the radio channel to current_bss->chanspec.
		 * Must do this before the rate calculations below.
		 */
		if (WLC_BAND_PI_RADIO_CHANSPEC != current_bss->chanspec) {
			if (WLC_SAME_CTLCHAN(WLC_BAND_PI_RADIO_CHANSPEC, current_bss->chanspec)) {
				wlc_update_bandwidth(wlc, cfg, current_bss->chanspec);
			} else {
				wlc_set_home_chanspec(wlc, current_bss->chanspec);
				wlc_set_chanspec(wlc, current_bss->chanspec);
			}
		} else {
			if (wlc->home_chanspec != WLC_BAND_PI_RADIO_CHANSPEC) {
				wlc->home_chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
			}
		}
#if defined(WLCHANIM)
		wlc_lq_chanim_acc_reset(wlc);
		wlc_lq_chanim_update(wlc, current_bss->chanspec, CHANIM_CHANSPEC);
#endif /* WLCHANIM */
	}

	wlc_BSSinit_rateset_filter(cfg);

	/* Validate beacon interval, we should never create a 0 beacon period BSS/IBSS */
	if (type == WLC_BSS_START) {
		ASSERT(current_bss->beacon_period);
	}
	/* don't adopt a illegal beacon_period IBSS */
	if (current_bss->beacon_period == 0) {
		current_bss->beacon_period = ISSIM_ENAB(wlc->pub->sih) ?
			BEACON_INTERVAL_DEF_QT : BEACON_INTERVAL_DEFAULT;
	}

	/* initialize BSSID information */
	cfg->BSSID = current_bss->BSSID;
	cfg->prev_BSSID = current_bss->BSSID;

	/* clear all old non-permanent scbs for IBSS only if assoc recreate is not enabled */
	/* and WLC_BSSCFG_PRESERVE cfg flag is not set */
	if (!cfg->BSS &&
	    !(ASSOC_RECREATE_ENAB(wlc->pub) && (cfg->flags & WLC_BSSCFG_PRESERVE)))
		wlc_scb_bsscfg_scbclear(wlc, cfg, FALSE);

	/* write the BSSID (to RXE) */
	if (!BSSCFG_PSTA(cfg))
		wlc_set_bssid(cfg);

	/* Update shared memory with MAC */
	wlc_set_mac(cfg);

	/* write to SHM to adjust the ucode ACK/CTS rate tables... */
	wlc_set_ratetable(wlc);

	/* the world is new again, so is our reported rate */
	if (!wlc->pub->associated)
		wlc_bsscfg_reprate_init(cfg);

	if (wlc->band->gmode && !restricted_ap) {

		/* update shortslot state now before founding or joining a BSS */
		if (wlc->shortslot_override == WLC_SHORTSLOT_AUTO) {
			if (type == WLC_BSS_JOIN)
				/* STA JOIN: adopt the 11g shortslot mode of the network */
				wlc->shortslot =
				        (current_bss->capability & DOT11_CAP_SHORTSLOT) != 0;
			else if (BSSCFG_AP(cfg))	/* AP START: in short slot mode */
				wlc->shortslot = TRUE;
			else	/* IBSS START: depending on user setting in advanced tab. */
				wlc->shortslot =
				        (default_bss->capability & DOT11_CAP_SHORTSLOT) != 0;
		} else {
			/* Switch to long or short slot mode based on the override */
			wlc->shortslot = (wlc->shortslot_override == WLC_SHORTSLOT_ON);
		}
		wlc_bmac_set_shortslot(wlc->hw, wlc->shortslot);
	}

	/* Legacy B update */
	if (BAND_2G(wlc->band->bandtype) &&
	    (wlc->band->gmode == GMODE_LEGACY_B) && BSSCFG_AP(cfg)) {
		ASSERT(wlc->shortslot_override == WLC_SHORTSLOT_OFF);
		wlc->shortslot = (wlc->shortslot_override == WLC_SHORTSLOT_ON);
		wlc_bmac_set_shortslot(wlc->hw, wlc->shortslot);

		/* Update aCWmin based on basic rates. */
		wlc_cwmin_gphy_update(wlc, &current_bss->rateset, TRUE);
	}

	wlc_prot_g_init(wlc->prot_g, cfg);
#ifdef WL11N
	/* set default n-protection */
	if (N_ENAB(wlc->pub))
		wlc_prot_n_init(wlc->prot_n, cfg);
#endif /* WL11N */

	if (BSSCFG_AP(cfg) || !cfg->BSS) {
		/* update our capability */
		current_bss->capability &= ~DOT11_CAP_SHORT;
		if (BSSCFG_AP(cfg)) {
			if (cfg->PLCPHdr_override == WLC_PLCP_SHORT)
				current_bss->capability |= DOT11_CAP_SHORT;
		} else {
			/* use IBSSGmode setting for bcn and prb cap */
			if (default_bss->capability & DOT11_CAP_SHORT)
				current_bss->capability |= DOT11_CAP_SHORT;
		}

		/* update the capability based on current shortslot mode */
		current_bss->capability &= ~DOT11_CAP_SHORTSLOT;
		if (wlc->shortslot && wlc->band->gmode)
			current_bss->capability |= DOT11_CAP_SHORTSLOT;

		if (BSSCFG_AP(cfg)) {
			wlc_bss_update_dtim_period(wlc, cfg);
		}
		_wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, FALSE);
	}

#ifdef STA
	if (BSSCFG_STA(cfg)) {
		if (!cfg->BSS) {
			/* set CFP duration */
			W_REG(osh, &wlc->regs->tsf_cfpmaxdur32, (uint32)current_bss->atim_window);
		}

		/* set initial RSSI value */
		wlc_lq_rssi_snr_noise_reset_ma(wlc, cfg, WLC_RSSI_EXCELLENT,
			WLC_SNR_EXCELLENT, WLC_RSSI_NO_SIGNAL);
		wlc_phy_BSSinit(WLC_PI(wlc), FALSE, WLC_RSSI_NO_SIGNAL);
		wlc_lq_rssi_init(wlc, WLC_RSSI_EXCELLENT);
	}
#endif /* STA */

	/* First AP starting a BSS, or STA starting an IBSS */
	if (type == WLC_BSS_START && wlc->aps_associated == 0) {
#ifdef WLMCHAN
		/* If already have stations associated, need to move STA to per bss
		 * tbtt and reset tsf for AP to make AP tbtt land in the middle of
		 * STA's bcn period
		 */
		if (MCHAN_ENAB(wlc->pub) &&
		    BSSCFG_AP(cfg) && STA_ACTIVE(wlc) &&
		    wlc_mchan_ap_tbtt_setup(wlc, cfg)) {
			; /* empty */
		}
		else
#endif
		/* AP or IBSS with h/w beacon enabled uses h/w TSF */
		if (BSSCFG_AP(cfg) || IBSS_HW_ENAB(cfg)) {
			uint32 old_l, old_h;

			/* Program the next TBTT and adjust TSF timer.
			 * If we are not starting the BSS, then the TSF timer adjust is done
			 * at beacon reception time.
			 */
			/* Using upstream beacon period will make APSTA 11H easier */
			bcnint = current_bss->beacon_period;
			bcnint_us = ((uint32)bcnint << 10);

			WL_APSTA_TSF(("wl%d: wlc_BSSinit(): starting TSF, bcnint %d\n",
			              wlc->pub->unit, bcnint));

			wlc_read_tsf(wlc, &old_l, &old_h);
			wlc_tsf_adj(wlc, cfg, 0, 0, old_h, old_l, bcnint_us, bcnint_us, FALSE);
		}
	}

	wlc_macctrl_init(wlc, cfg);

#ifdef WLMCNX
	/* update AP's or primary IBSS's channel */
	if (MCNX_ENAB(wlc->pub) &&
	    (BSSCFG_AP(cfg) ||
	     (BSSCFG_IBSS(cfg) && cfg == wlc->cfg))) {
		chanspec_t chanspec = current_bss->chanspec;
		uint16 go_chan;
		ASSERT(chanspec != INVCHANSPEC);
		if (D11REV_LT(wlc->pub->corerev, 40)) {
			go_chan = CHSPEC_CHANNEL(chanspec);
			if (CHSPEC_IS5G(chanspec))
				go_chan |= D11_CURCHANNEL_5G;
			if (CHSPEC_IS40(chanspec))
				go_chan |= D11_CURCHANNEL_40;
		} else
			/* store chanspec */
			go_chan = chanspec;
		wlc_mcnx_write_shm(wlc->mcnx, M_P2P_GO_CHANNEL, go_chan);
	}
#endif /* WLMCNX */

#ifdef STA
	/* Entering IBSS: make sure bcn promisc is on */
	if (!cfg->BSS) {
		wlc_ibss_enable(cfg);
	}
#endif /* STA */

	/* update bcast/mcast rateset */
	if (!(cfg->flags & WLC_BSSCFG_NOBCMC)) {
		scb = WLC_BCMCSCB_GET(wlc, cfg);
		ASSERT(scb != NULL);
		wlc_rateset_filter(&current_bss->rateset, &scb->rateset, FALSE,
		                   WLC_RATES_CCK_OFDM, RATE_MASK, wlc_get_mcsallow(wlc, cfg));
	}

	wlc_pretbtt_set(cfg);

#ifdef WLMCNX
	/* extend beacon timeout to cover possible long pretbtt */
	if (MCNX_ENAB(wlc->pub)) {
		if (BSSCFG_STA(cfg) && cfg->BSS)
			wlc_dtimnoslp_set(cfg);
	}
#endif

#if defined(STA) && defined(WLCAC)
	if (BSSCFG_STA(cfg) && cfg->BSS && CAC_ENAB(wlc->pub)) {
		wlc_cac_on_join_bss(wlc->cac, cfg, &current_bss->BSSID,
		                    (cfg->assoc->type == AS_ROAM));
	}
#endif /* STA && WLCAC */
}

/** ucode, hwmac update. Channel dependent updates for ucode and hw */
static void
wlc_ucode_mac_upd(wlc_info_t *wlc)
{
#ifdef STA
	/* enable or disable any active IBSSs depending on whether or not
	 * we are on the home channel
	 */
	if (wlc->home_chanspec == WLC_BAND_PI_RADIO_CHANSPEC) {
		wlc_ibss_enable_all(wlc);
		if (wlc->pub->associated) {
			/* BMAC_NOTE: This is something that should be fixed in ucode inits.
			 * I think that the ucode inits set up the bcn templates and shm values
			 * with a bogus beacon. This should not be done in the inits. If ucode needs
			 * to set up a beacon for testing, the test routines should write it down,
			 * not expect the inits to populate a bogus beacon.
			 */
			if (WLC_PHY_11N_CAP(wlc->band)) {
				wlc_write_shm(wlc, M_BCN_TXTSF_OFFSET, wlc->band->bcntsfoff);
			}
		}
	} else {
		/* disable an active IBSS if we are not on the home channel */
		wlc_ibss_disable_all(wlc);
	}
#endif /* STA */

	/* update the various promisc bits */
	wlc_mac_bcn_promisc(wlc);
	wlc_mac_promisc(wlc);
}

static void
wlc_bandinit_ordered(wlc_info_t *wlc, chanspec_t chanspec)
{
	wlc_rateset_t default_rateset;
	uint parkband;
	uint i, band_order[2];

	WL_TRACE(("wl%d: wlc_bandinit_ordered\n", wlc->pub->unit));
	/*
	 * We might have been bandlocked during down and the chip power-cycled (hibernate).
	 * figure out the right band to park on
	 */
	if (wlc->bandlocked || NBANDS(wlc) == 1) {
		ASSERT(CHSPEC_WLCBANDUNIT(chanspec) == wlc->band->bandunit);

		parkband = wlc->band->bandunit;	/* updated in wlc_bandlock() */
		band_order[0] = band_order[1] = parkband;
	} else {
		/* park on the band of the specified chanspec */
		parkband = CHSPEC_WLCBANDUNIT(chanspec);

		/* order so that parkband initialize last */
		band_order[0] = parkband ^ 1;
		band_order[1] = parkband;
	}

	/* make each band operational, software state init */
	for (i = 0; i < NBANDS(wlc); i++) {
		uint j = band_order[i];

		wlc_pi_band_update(wlc, j);

		ASSERT(wlc->bandstate[j]->hwrs_scb != NULL);

		wlc_default_rateset(wlc, &default_rateset);

		/* fill in hw_rate */
		wlc_rateset_filter(&default_rateset, &wlc->band->hw_rateset, FALSE,
		                   WLC_RATES_CCK_OFDM, RATE_MASK, wlc_get_mcsallow(wlc, NULL));
#ifdef WL11N
		/* patch up MCS32 */
		wlc_rateset_bw_mcs_filter(&wlc->band->hw_rateset, CHSPEC_WLC_BW(chanspec));
#endif
		/* init basic rate lookup */
		wlc_rate_lookup_init(wlc, &default_rateset);
	}

	wlc_set_home_chanspec(wlc, chanspec);

	wlc_lq_chanim_acc_reset(wlc);
	wlc_lq_chanim_update(wlc, wlc->chanspec, CHANIM_CHANSPEC);

	/* sync up phy/radio chanspec */
	wlc_set_phy_chanspec(wlc, chanspec);
}

/** band-specific init */
static void
WLBANDINITFN(wlc_bsinit)(wlc_info_t *wlc)
{
	WL_TRACE(("wl%d: wlc_bsinit: bandunit %d\n", wlc->pub->unit, wlc->band->bandunit));

	/* write ucode ACK/CTS rate table */
	wlc_set_ratetable(wlc);

	/* update some band specific mac configuration */
	wlc_ucode_mac_upd(wlc);

#ifdef WL11N
	/* init antenna selection */
	if (WLANTSEL_ENAB(wlc))
		wlc_antsel_init(wlc->asi);
#endif

#ifdef STA
#ifdef WLC_LOW
	/* If we just switched to the B/G band and we had enabled wideband frequency
	 * tracking OR if wideband frequency tracking is always on, then ensure that
	 * the PHY registers have the right settings
	 */
	if (wlc->band->bandunit == wlc_hw_get_bandunit(wlc->hw)) {
		if ((wlc->freqtrack || (wlc->freqtrack_override == FREQTRACK_ON)) &&
		    WLCISGPHY(wlc->band)) {
			wlc_phy_freqtrack_start(WLC_PI(wlc));
		}
	}
#endif /* WLC_LOW */
#endif /* STA */
}

/** switch to and initialize new band */
static void
WLBANDINITFN(wlc_setband)(wlc_info_t *wlc, uint bandunit)
{
#ifdef STA
	int idx;
	wlc_bsscfg_t *cfg;
#endif

	ASSERT(NBANDS(wlc) > 1);
	ASSERT(!wlc->bandlocked);
	ASSERT(bandunit != wlc->band->bandunit || wlc->bandinit_pending);

	WL_DUAL(("DUALBAND: wlc_setband: bandunit %d\n", bandunit));

	wlc_pi_band_update(wlc, bandunit);

	if (!wlc->pub->up)
		return;

	WL_CHANLOG(wlc, __FUNCTION__, TS_ENTER, 0);
#ifdef STA
	/* wait for at least one beacon before entering sleeping state */
	FOREACH_AS_STA(wlc, idx, cfg)
	        wlc_set_pmawakebcn(cfg, TRUE);
#endif

	/* band-specific initializations */
	wlc_bsinit(wlc);
	WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
}

#ifdef WME
/** Initialize a WME Parameter Info Element with default STA parameters from WMM Spec, Table 12 */
void
wlc_wme_initparams_sta(wlc_info_t *wlc, wme_param_ie_t *pe)
{
	static const wme_param_ie_t stadef = {
		WME_OUI,
		WME_OUI_TYPE,
		WME_SUBTYPE_PARAM_IE,
		WME_VER,
		0,
		0,
		{
			{ EDCF_AC_BE_ACI_STA, EDCF_AC_BE_ECW_STA, HTOL16(EDCF_AC_BE_TXOP_STA) },
			{ EDCF_AC_BK_ACI_STA, EDCF_AC_BK_ECW_STA, HTOL16(EDCF_AC_BK_TXOP_STA) },
			{ EDCF_AC_VI_ACI_STA, EDCF_AC_VI_ECW_STA, HTOL16(EDCF_AC_VI_TXOP_STA) },
			{ EDCF_AC_VO_ACI_STA, EDCF_AC_VO_ECW_STA, HTOL16(EDCF_AC_VO_TXOP_STA) }
		}
	};

	STATIC_ASSERT(sizeof(*pe) == WME_PARAM_IE_LEN);
	memcpy(pe, &stadef, sizeof(*pe));
}
#endif /* WME */

/**
 * Check if this TLVS entry is a WME IE or params IE entry.
 * If not, move the tlvs pointer/length to indicate the next TLVS entry.
 */
bcm_tlv_t *
wlc_find_wme_ie(uint8 *tlvs, uint tlvs_len)
{
	bcm_tlv_t *wme_ie;

	if ((wme_ie =
	     bcm_find_wmeie(tlvs, tlvs_len, WME_SUBTYPE_IE, WME_IE_LEN)) != NULL)
		return wme_ie;

	if ((wme_ie =
	     bcm_find_wmeie(tlvs, tlvs_len, WME_SUBTYPE_PARAM_IE, WME_PARAM_IE_LEN)) != NULL)
		return wme_ie;

	return NULL;
}

void
wlc_fragthresh_set(wlc_info_t *wlc, uint fifo, uint16 thresh)
{
	wlc->fragthresh[fifo] = thresh;
}

static void
wlc_edcf_setparams(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint aci, bool suspend)
{
	uint i;
	edcf_acparam_t *acp;

	/*
	 * EDCF params are getting updated, so reset the frag thresholds for ACs,
	 * if those were changed to honor the TXOP settings
	 */
	for (i = 0; i < NFIFO; i++)
		wlc_fragthresh_set(wlc, i, wlc->usr_fragthresh);

	ASSERT(aci < AC_COUNT);

	acp = &cfg->wme->wme_param_ie.acparam[aci];

	wlc_edcf_acp_set_sw(wlc, cfg, acp);

	/* Only apply params to h/w if the core is out of reset and has clocks */
	if (!wlc->clk || !cfg->associated)
		return;

	if (suspend)
		wlc_suspend_mac_and_wait(wlc);

	wlc_edcf_acp_set_hw(wlc, cfg, acp);

	if (BSSCFG_AP(cfg)) {
		WL_APSTA_BCN(("wl%d.%d: wlc_edcf_setparams() -> wlc_update_beacon()\n",
		              wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, FALSE);
	}

	if (suspend)
		wlc_enable_mac(wlc);
}

/* Extract one AC param set from the WME IE acparam 'acp'
 * and use them to update WME into in 'cfg'
 */
static void
wlc_edcf_acp_set_sw(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	uint aci;
	wlc_wme_t *wme = cfg->wme;

	/* find out which ac this set of params applies to */
	aci = (acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT;
	ASSERT(aci < AC_COUNT);
	/* set the admission control policy for this AC */
	wme->wme_admctl &= ~(1 << aci);
	if (acp->ACI & EDCF_ACM_MASK) {
		wme->wme_admctl |= (1 << aci);
	}
	/* convert from units of 32us to us for ucode */
	wme->edcf_txop[aci] = EDCF_TXOP2USEC(ltoh16(acp->TXOP));

	WL_INFORM(("wl%d.%d: Setting %s: admctl 0x%x txop 0x%x\n",
	           wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	           aci_names[aci], wme->wme_admctl, wme->edcf_txop[aci]));
}

/* Extract one AC param set from the WME IE acparam 'acp'
 * and plumb them into h/w
 */
static void
wlc_edcf_acp_set_hw(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	uint aci, j;
	shm_acparams_t acp_shm;
	uint16 *shm_entry;
	uint offset;

	bzero((char*)&acp_shm, sizeof(shm_acparams_t));
	/* find out which ac this set of params applies to */
	aci = (acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT;
	ASSERT(aci < AC_COUNT);

	/* fill in shm ac params struct */
	acp_shm.txop = ltoh16(acp->TXOP);
	/* convert from units of 32us to us for ucode */

	acp_shm.txop = EDCF_TXOP2USEC(acp_shm.txop);

	acp_shm.aifs = (acp->ACI & EDCF_AIFSN_MASK);

	if (acp_shm.aifs < EDCF_AIFSN_MIN || acp_shm.aifs > EDCF_AIFSN_MAX) {
		WL_ERROR(("wl%d.%d: %s: Setting %s: bad aifs %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		          aci_names[aci], acp_shm.aifs));
		return;
	}


	/* CWmin = 2^(ECWmin) - 1 */
	acp_shm.cwmin = EDCF_ECW2CW(acp->ECW & EDCF_ECWMIN_MASK);
	/* CWmax = 2^(ECWmax) - 1 */
	acp_shm.cwmax =
	        EDCF_ECW2CW((acp->ECW & EDCF_ECWMAX_MASK) >> EDCF_ECWMAX_SHIFT);
	acp_shm.cwcur = acp_shm.cwmin;
	acp_shm.bslots = R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random) & acp_shm.cwcur;
	acp_shm.reggap = acp_shm.bslots + acp_shm.aifs;
	/* Indicate the new params to the ucode */
	offset = M_EDCF_QINFO + wme_shmemacindex(aci) * M_EDCF_QLEN + M_EDCF_STATUS_OFF;
	acp_shm.status = wlc_read_shm(wlc, offset);
	acp_shm.status |= WME_STATUS_NEWAC;

	/* Fill in shm acparam table */
	shm_entry = (uint16*)&acp_shm;
	for (j = 0; j < (int)sizeof(shm_acparams_t); j += 2) {
		offset = M_EDCF_QINFO + wme_shmemacindex(aci) * M_EDCF_QLEN + j;
		wlc_write_shm(wlc, offset, *shm_entry++);
	}

	WL_INFORM(("wl%d.%d: Setting %s: txop 0x%x cwmin 0x%x cwmax 0x%x "
	           "cwcur 0x%x aifs 0x%x bslots 0x%x reggap 0x%x status 0x%x\n",
	           wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	           aci_names[aci], acp_shm.txop, acp_shm.cwmin,
	           acp_shm.cwmax, acp_shm.cwcur, acp_shm.aifs,
	           acp_shm.bslots, acp_shm.reggap, acp_shm.status));
}

/** Extract all AC param sets from WME IE 'ie' and write them into host struct 'acp' */
static void
_wlc_edcf_acp_get_all(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wme_param_ie_t *ie, edcf_acparam_t *acp)
{
	uint i;
	edcf_acparam_t *acp_ie = ie->acparam;

	for (i = 0; i < AC_COUNT; i++, acp_ie++, acp ++) {
		acp->ACI = acp_ie->ACI;
		acp->ECW = acp_ie->ECW;
		/* convert to host order */
		acp->TXOP = ltoh16(acp_ie->TXOP);
	}
}

/** Extract all AC param sets used by the BSS and write them into host struct 'acp' */
static void
wlc_edcf_acp_get_all(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	_wlc_edcf_acp_get_all(wlc, cfg, &cfg->wme->wme_param_ie, acp);
}

/** Extract all AC param sets used in BCN/PRBRSP and write them into host struct 'acp' */
static void
wlc_edcf_acp_ad_get_all(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	ASSERT(BSSCFG_AP(cfg));
	_wlc_edcf_acp_get_all(wlc, cfg, cfg->wme->wme_param_ie_ad, acp);
}

/** Extract one set of AC params from host struct 'acp' and write them into WME IE 'ie' */
static int
_wlc_edcf_acp_set_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp, wme_param_ie_t *ie)
{
	uint aci_in;
	edcf_acparam_t *acp_ie;

	/* Determine requested entry */
	aci_in = (acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT;
	if (aci_in >= AC_COUNT) {
		WL_ERROR(("Set of AC Params with bad ACI %d\n", aci_in));
		return BCME_RANGE;
	}

	/* Set the contents as specified */
	acp_ie = &ie->acparam[aci_in];
	acp_ie->ACI = acp->ACI;
	acp_ie->ECW = acp->ECW;
	/* convert to network order */
	acp_ie->TXOP = htol16(acp->TXOP);

	WL_INFORM(("wl%d.%d: setting %s RAW AC Params ACI 0x%x ECW 0x%x TXOP 0x%x\n",
	           wlc->pub->unit, WLC_BSSCFG_IDX(cfg), aci_names[aci_in],
	           acp_ie->ACI, acp_ie->ECW, acp_ie->TXOP));

	/* APs need to notify any clients */
	if (BSSCFG_AP(cfg)) {
		/* Increment count field to notify associated
		 * STAs of parameter change
		 */
		int count = ie->qosinfo & WME_QI_AP_COUNT_MASK;
		ie->qosinfo &= ~WME_QI_AP_COUNT_MASK;
		ie->qosinfo |= (count + 1) & WME_QI_AP_COUNT_MASK;
	}

	return BCME_OK;
}

/* Extract one set of AC params from host struct 'acp'
 * and put them into WME IE whose AC params are used by h/w.
 */
static int
wlc_edcf_acp_set_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	return _wlc_edcf_acp_set_ie(wlc, cfg, acp, &cfg->wme->wme_param_ie);
}

/* Extract one set of AC params from host struct 'acp'
 * and write them into WME IE used in BCN/PRBRSP.
 */
static int
wlc_edcf_acp_ad_set_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	ASSERT(BSSCFG_AP(cfg));
	return _wlc_edcf_acp_set_ie(wlc, cfg, acp, cfg->wme->wme_param_ie_ad);
}

/* Extract one set of AC params from host struct 'acp',
 * write them into the WME IE whose AC param are used by h/w,
 * and them to the h/w'.
 * It also updates other WME info in 'cfg' based on 'acp',
 */
static int
wlc_edcf_acp_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp, bool suspend)
{
	int err;
	uint aci;

	err = wlc_edcf_acp_set_ie(wlc, cfg, acp);
	if (err != BCME_OK)
		return err;

	aci = (acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT;
	ASSERT(aci < AC_COUNT);

	wlc_edcf_setparams(wlc, cfg, aci, suspend);
	return BCME_OK;
}

/* APIs to external users including ioctl/iovar handlers... */

/* Function used by ioctl/iovar code.
 * Extract one set of AC params from host struct 'acp'
 * and write them into the WME IE whose AC params are used by h/w
 * and update the h/w.
 */
static int
wlc_edcf_acp_set_one(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp, bool suspend)
{
	uint aci = (acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT;
#ifndef WLP2P_UCODE_MACP
	int idx;
	wlc_bsscfg_t *bc;
	int err;
#endif

	if (aci >= AC_COUNT)
		return BCME_RANGE;

#ifndef WLP2P_UCODE_MACP
	/* duplicate to all BSSs if the h/w supports only a single set of WME params... */
	FOREACH_BSS(wlc, idx, bc) {
		if (bc == cfg)
			continue;
		WL_INFORM(("wl%d: Setting %s in cfg %d...\n",
		           wlc->pub->unit, aci_names[aci], WLC_BSSCFG_IDX(bc)));
		err = wlc_edcf_acp_set_ie(wlc, bc, acp);
		if (err != BCME_OK)
			return err;
	}
#endif /* !WLP2P_UCODE_MACP */

	return wlc_edcf_acp_set(wlc, cfg, acp, suspend);
}

/* Function used by ioctl/iovar code.
 * Extract one set of AC params from host struct 'acp'
 * and write them into the WME IE used in BCN/PRBRSP
 * and update the h/w.
 */
static int
wlc_edcf_acp_ad_set_one(wlc_info_t *wlc, wlc_bsscfg_t *cfg, edcf_acparam_t *acp)
{
	uint aci = (acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT;
#ifndef WLP2P_UCODE_MACP
	int idx;
	wlc_bsscfg_t *bc;
#endif

	if (aci >= AC_COUNT)
		return BCME_RANGE;

#ifndef WLP2P_UCODE_MACP
	/* duplicate to all BSSs if the h/w supports only a single set of WME params... */
	FOREACH_AP(wlc, idx, bc) {
		if (bc == cfg)
			continue;
		WL_INFORM(("wl%d: Setting advertised %s in cfg %d...\n",
		           wlc->pub->unit, aci_names[aci], WLC_BSSCFG_IDX(bc)));
		wlc_edcf_acp_ad_set_ie(wlc, bc, acp);
	}
#endif /* !WLP2P_UCODE_MACP */

	return wlc_edcf_acp_ad_set_ie(wlc, cfg, acp);
}

/** Apply all AC param sets stored in WME IE in 'cfg' to the h/w */
int
wlc_edcf_acp_apply(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool suspend)
{
	edcf_acparam_t acp[AC_COUNT];
	uint i;
	int err = BCME_OK;
#ifndef WLP2P_UCODE_MACP
	int idx;
	wlc_bsscfg_t *tplt;
#endif

#ifndef WLP2P_UCODE_MACP
	/* find the first STA (primary STA has priority) and use its AC params */
	FOREACH_AS_STA(wlc, idx, tplt) {
		if (BSS_WME_AS(wlc, tplt))
			break;
	}
	if (tplt != NULL) {
		WL_INFORM(("wl%d: Using cfg %d's AC params...\n",
		           wlc->pub->unit, WLC_BSSCFG_IDX(tplt)));
		cfg = tplt;
	}
#endif

	wlc_edcf_acp_get_all(wlc, cfg, acp);
	for (i = 0; i < AC_COUNT; i++) {
		err = wlc_edcf_acp_set_one(wlc, cfg, &acp[i], suspend);
		if (err != BCME_OK)
			return err;
	}
	return BCME_OK;
}

bool
BCMATTACHFN(wlc_timers_init)(wlc_info_t *wlc, int unit)
{
	if (!(wlc->wdtimer = wl_init_timer(wlc->wl, wlc_watchdog_timer, wlc, "watchdog"))) {
		WL_ERROR(("wl%d: wdtimer failed\n", unit));
		goto fail;
	}

	if (!(wlc->radio_timer = wl_init_timer(wlc->wl, wlc_radio_timer, wlc, "radio"))) {
		WL_ERROR(("wl%d: radio_timer failed\n", unit));
		goto fail;
	}

#ifdef STA
	if (!(wlc->iscan_timer = wl_init_timer(wlc->wl, wlc_iscan_timeout, wlc, "iscan_timeout"))) {
		WL_ERROR(("wl%d: iscan_timeout failed\n", unit));
		goto fail;
	}

	if (!(wlc->pm2_radio_shutoff_dly_timer = wl_init_timer(wlc->wl,
		wlc_pm2_radio_shutoff_dly_timer, wlc, "radioshutoffdly"))) {
		WL_ERROR(("wl%d: radio_shutoff_dly_timer failed\n", unit));
		goto fail;
	}

	if (!(wlc->wake_event_timer = wl_init_timer(wlc->wl, wlc_wake_event_timer, wlc,
		"wakeeventtimer"))) {
		WL_ERROR(("wl%d: wl_init_timer for wake_event_timer failed\n", unit));
		goto fail;
	}
#endif	/* STA */
#ifdef WLRXOV
	if (WLRXOV_ENAB(wlc->pub)) {
		if (!(wlc->rxov_timer = wl_init_timer(wlc->wl, wlc_rxov_timer, wlc, "rxov"))) {
			WL_ERROR(("wl%d: rxov_timer failed\n", unit));
			goto fail;
		}
	}
#endif
	return TRUE;

fail:
	return FALSE;
}

/** Initialize wlc_info default values. May get overrides later in this function. */
void
BCMATTACHFN(wlc_info_init)(wlc_info_t *wlc, int unit)
{
	int i;

	/* Assume the device is there until proven otherwise */
	wlc->device_present = TRUE;

	/* set default power output percentage to 100 percent */
	wlc->txpwr_percent = 100;

	/* Save our copy of the chanspec */
	wlc->chanspec = CH20MHZ_CHSPEC(1);

	wlc->legacy_probe = TRUE;

	/* various 802.11g modes */
	wlc->shortslot = FALSE;
	wlc->shortslot_override = WLC_SHORTSLOT_AUTO;

	wlc->stf->ant_rx_ovr = ANT_RX_DIV_DEF;
	wlc->stf->txant = ANT_TX_DEF;

	wlc->prb_resp_timeout = WLC_PRB_RESP_TIMEOUT;

	wlc->usr_fragthresh = DOT11_DEFAULT_FRAG_LEN;
	for (i = 0; i < NFIFO; i++)
		wlc->fragthresh[i] = DOT11_DEFAULT_FRAG_LEN;
	wlc->RTSThresh = DOT11_DEFAULT_RTS_LEN;

#if defined(AP) && !defined(STA)
	wlc->pub->_ap = TRUE;
#endif


	/* default rate fallback retry limits */
	wlc->SFBL = RETRY_SHORT_FB;
	wlc->LFBL = RETRY_LONG_FB;

	/* default mac retry limits */
	wlc->SRL = RETRY_SHORT_DEF;
	wlc->LRL = RETRY_LONG_DEF;

	/* init PM state */
	wlc->PMpending = FALSE;		/* Tracks whether STA indicated PM in the last attempt */

	wlc->excess_pm_period = 0; /* Disable the excess PM notify */
	wlc->excess_pm_percent = 100;

	/* Init wme queuing method */
	wlc->wme_prec_queuing = FALSE;

	/* Overrides for the core to stay awake under zillion conditions Look for STAY_AWAKE */
	wlc->wake = FALSE;
	/* Are we waiting for a response to PS-Poll that we sent */
	wlc->PSpoll = FALSE;

	/* APSD defaults */
#ifdef STA
	wlc->apsd_sta_usp = FALSE;
#endif

	/* WME QoS mode is Auto by default */
#if defined(WME)
	wlc->pub->_wme = AUTO;
#endif

#ifdef BCMSDIODEV_ENABLED
	wlc->pub->_priofc = TRUE;	/* enable priority flow control for sdio dongle */
#endif

#ifdef WLAMPDU
	/* enable AMPDU by default */
	wlc->pub->_ampdu_tx = TRUE;
	wlc->pub->_ampdu_rx = TRUE;
#endif
#ifdef WLAMPDU_HOSTREORDER
#ifdef WLAMPDU_HOSTREORDER_DISABLED
	wlc->pub->_ampdu_hostreorder = FALSE;
#else
	wlc->pub->_ampdu_hostreorder = TRUE;
#endif /* WLAMPDU_HOSTREORDER_DISABLED */
#endif /* WLAMPDU_HOSTREORDER */
#ifdef WLAMSDU
	/* AMSDU agg/deagg will be rechecked later */
	wlc->pub->_amsdu_tx = FALSE;
	wlc->_amsdu_rx = FALSE;
	wlc->_rx_amsdu_in_ampdu = FALSE;
#endif

	/* AMSDU monitor packet queue */
	wlc->monitor_amsdu_pkts = NULL;


#ifdef WLTDLS
	/* disable TDLS by default */
	wlc->pub->_tdls = FALSE;
#ifndef WLTDLS_DISABLED
	wlc->pub->_tdls_support = TRUE;
#endif /* !defined(WLTDLS_DISABLED) */
#endif /* WLTDLS */

#if defined(WLFBT) && !defined(WLFBT_DISABLED)
	wlc->pub->_fbt = TRUE;
#endif

#if defined(DMATXRC) && !defined(DMATXRC_DISABLED)
	wlc->pub->_dmatxrc = TRUE;
#endif

#if defined(TINY_PKTJOIN) && !defined(TINY_PKTJOIN_DISABLED)
	wlc->pub->_tiny_pktjoin = TRUE;
#endif

#if defined(WL_RXEARLYRC) && !defined(WL_RXEARLYRC_DISABLED)
	wlc->pub->_wl_rxearlyrc = TRUE;
#endif

#if defined(WLRXOV) && !defined(WLRXOV_DISABLED)
	wlc->pub->_rxov = TRUE;
#endif

#if defined(IBSS_PEER_GROUP_KEY) && !defined(IBSS_PEER_GROUP_KEY_DISABLED)
	wlc->pub->_ibss_peer_group_key = TRUE;
#endif

#if defined(IBSS_PEER_DISCOVERY_EVENT) && !defined(IBSS_PEER_DISCOVERY_EVENT_DISABLED)
	wlc->pub->_ibss_peer_discovery_event = TRUE;
#endif

#if defined(IBSS_PEER_MGMT) && !defined(IBSS_PEER_MGMT_DISABLED)
	wlc->pub->_ibss_peer_mgmt = TRUE;
#endif

#if !defined(WLNOEIND) && !defined(WLEIND_DISABLED)
	wlc->pub->_wleind = TRUE;
#endif

#if defined(PROP_TXSTATUS) && defined(PROP_TXSTATUS_ENABLED)
	wlc->pub->_proptxstatus = TRUE;
#endif

#if defined(WL11K) && !defined(WL11K_DISABLED)
	wlc->pub->_rrm = TRUE;
#endif

#if defined(WLWNM) && !defined(WLWNM_DISABLED)
	wlc->pub->_wnm = TRUE;
#endif

#if defined(WLOVERTHRUSTER) && !defined(WLOVERTHRUSTER_DISABLED)
	wlc->pub->_wloverthruster = TRUE;
#endif

	wlc->pub->bcmerror = 0;

	wlc->ibss_allowed = TRUE;
	wlc->ibss_coalesce_allowed = TRUE;

	if (N_ENAB(wlc->pub))
		wlc->pub->_coex = ON;

#ifdef STA
	wlc->pm2_radio_shutoff_pending = FALSE;
	wlc->pm2_radio_shutoff_dly = 0; /* disable radio shutoff delay feature */

	/* initialize mpc delay (it should be non-zero value) */
	wlc->mpc_delay_off = wlc->mpc_dlycnt = WLC_MPC_THRESHOLD;
#endif

	wlc->rfaware_lifetime = WLC_RFAWARE_LIFETIME_DEFAULT;

	wlc->pr80838_war = TRUE;
#ifdef STA
#if defined(AP_KEEP_ALIVE)
	wlc_ap_keep_alive_count_default(wlc);
#endif 
#endif /* STA */
	wlc->roam_rssi_cancel_hysteresis = ROAM_RSSITHRASHDIFF;

	/* Init flag to prevent multiple executions thru wlc_send_q */
	wlc->in_send_q = FALSE;
#ifdef STA
	wlc->seq_reset = TRUE;          /* reset the sequence number register in SCR to 0 */
#endif
#if defined(WLFMC) && !defined(WLFMC_DISABLED)
	wlc->pub->_fmc = TRUE;
#endif
#if defined(WLRCC) && !defined(WLRCC_DISABLED)
	wlc->pub->_rcc = TRUE;
#endif

#if defined(WLNON_CLEARING_RESTRICTED_CHANNEL_BEHAVIOR)
	wlc->pub->_clear_restricted_chan = FALSE;
#else
	wlc->pub->_clear_restricted_chan = TRUE;
#endif
	/* init BCMC wait time to disabled value */
	wlc->pm_bcmc_wait = 0xFFFF;
#if defined(WL_EVDATA_BSSINFO) && !defined(WL_EVDATA_BSSINFO_DISABLED)
	wlc->pub->_evdata_bssinfo = TRUE;
#endif

#ifdef WLPM_BCNRX
	wlc->pub->_pm_bcnrx = TRUE;
#endif
#if defined(WLABT) && !defined(WLABT_DISABLED)
	wlc->pub->_abt = TRUE;
#endif
#if defined(WLSCAN_PS) && !defined(WLSCAN_PS_DISABLED)
	wlc->pub->_scan_ps = TRUE;
#endif
	/* It is enabled dynamically when PSM watchdog is hit */
	wlc->psm_watchdog_debug = FALSE;
#if defined(WLIPFO) && !defined(WLIPFO_DISABLED)
	wlc->pub->_ipfo = TRUE;
#endif
#if defined(WL_FRWD_REORDER) && !defined(WL_FRWD_REORDER_DISABLED)
	wlc->pub->_frwd_reorder = TRUE;
#endif
	wlc->watchdog_on_bcn = TRUE;
#ifdef BCMPCIEDEV_ENABLED
	wlc->pub->_pciedev = TRUE;
#endif
}

#if defined(WLC_LOW) && defined(DMATXRC)
int
BCMATTACHFN(wlc_phdr_attach)(wlc_info_t *wlc)
{
	int err, n = 0, len = ROUNDUP(TXOFF, 4);
	uint32 phdr_list_sz;

	wlc->phdr_len = PHDR_POOL_LEN;
	phdr_list_sz = wlc->phdr_len * sizeof(void *);

	if ((wlc->phdr_list = MALLOC(wlc->osh, phdr_list_sz)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate peek list\n",
			wlc->pub->unit, __FUNCTION__));
		err = BCME_ERROR;
		goto exit;
	}
	bzero(wlc->phdr_list, phdr_list_sz);

	if ((wlc->pktpool_phdr = MALLOC(wlc->osh, sizeof(pktpool_t))) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate phdr\n",
			wlc->pub->unit, __FUNCTION__));
		MFREE(wlc->osh, wlc->phdr_list, phdr_list_sz);
		wlc->phdr_list = NULL;
		err = BCME_ERROR;
		goto exit;
	}
	bzero(wlc->pktpool_phdr, sizeof(pktpool_t));

	len += sizeof(txrc_ctxt_t);
	len = ROUNDUP(len, 4);

	err = pktpool_init(wlc->osh, PHDR_POOL(wlc), &n, len, TRUE, lbuf_basic);

	if (err == BCME_ERROR) {
		WL_ERROR(("wl%d: %s: failed to initialize phdr poolidx\n",
		          wlc->pub->unit, __FUNCTION__));
		MFREE(wlc->osh, wlc->pktpool_phdr, sizeof(pktpool_t));
		MFREE(wlc->osh, wlc->phdr_list, phdr_list_sz);
		wlc->pktpool_phdr = NULL;
		wlc->phdr_list = NULL;
		goto exit;
	}
exit:
	if (err)
		WL_ERROR(("phdr attach err:%d\n", err));
	return err;
}

void
BCMATTACHFN(wlc_phdr_detach)(wlc_info_t *wlc)
{
	uint32 phdr_list_sz = wlc->phdr_len * sizeof(void *);
	pktpool_t * pktpool_phdr = PHDR_POOL(wlc);

	if (wlc->phdr_list) {
		MFREE(wlc->osh, wlc->phdr_list, phdr_list_sz);
		wlc->phdr_list = NULL;
	}

	if (pktpool_phdr) {
		pktpool_deinit(wlc->osh, pktpool_phdr);

		MFREE(wlc->osh, pktpool_phdr, sizeof(pktpool_t));
		wlc->pktpool_phdr = NULL;
	}
}

void*
wlc_phdr_get(wlc_info_t *wlc)
{
	txrc_ctxt_t *rctxt;
	int len;
	void *p = pktpool_get(PHDR_POOL(wlc));

	if (p == NULL) {
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return NULL;
	}

	len = ROUNDUP(TXOFF, 4);
	PKTSETLEN(wlc->osh, p, len);
	PKTPULL(wlc->osh, p, len);

	ASSERT(PKTTAILROOM(wlc->osh, p) >= sizeof(txrc_ctxt_t));

	rctxt = TXRC_PKTTAIL(wlc->osh, p);
	ASSERT(ISALIGNED(rctxt, 4));
	bzero(rctxt, sizeof(txrc_ctxt_t));

	TXRC_SETMARKER(rctxt);

	return p;
}

/* Incrementally add to pkt header pool when memory
 * is available after reclaim
 */
void
wlc_phdr_fill(wlc_info_t* wlc)
{
	osl_t *osh = wlc->osh;
	void *p;
	int len;

	len = pktpool_len(PHDR_POOL(wlc));

	for (; len < wlc->phdr_len; len++) {
		p = PKTGET(osh, pktpool_plen(PHDR_POOL(wlc)), TRUE);
		if (p == NULL)
			break;

		if (pktpool_add(PHDR_POOL(wlc), p) != BCME_OK) {
			PKTFREE(osh, p, FALSE);
			break;
		}
	}
}
#endif /* WLC_LOW && DMATXRC */

static int
BCMATTACHFN(wlc_register_iovt_all)(wlc_info_t *wlc, wlc_iocv_info_t *ii)
{
	phy_info_t *pi, *prev_pi = NULL;
	int err;
	uint i;

	if ((err = wlc_bmac_register_iovt(wlc->hw, ii)) != BCME_OK) {
		WL_ERROR(("%s: wlc_bmac_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	for (i = 0; i < MAXBANDS; i++) {
		wlcband_t *band = wlc->bandstate[i];

		if (band == NULL)
			continue;

		pi = (phy_info_t *)WLC_PI_BANDUNIT(wlc, i);
		if (pi == NULL)
			continue;

		if (pi == prev_pi)
			continue;

		if ((err = phy_high_register_iovt_all(band->phytype, ii)) != BCME_OK) {
			WL_ERROR(("%s: phy_high_register_iovt_all failed\n", __FUNCTION__));
			goto fail;
		}

		prev_pi = pi;
	}

	return BCME_OK;

fail:
	return err;
}

static int
BCMATTACHFN(wlc_register_ioct_all)(wlc_info_t *wlc, wlc_iocv_info_t *ii)
{
	phy_info_t *pi, *prev_pi = NULL;
	int err;
	uint i;

	for (i = 0; i < MAXBANDS; i++) {
		wlcband_t *band = wlc->bandstate[i];

		if (band == NULL)
			continue;

		pi = (phy_info_t *)WLC_PI_BANDUNIT(wlc, i);
		if (pi == NULL)
			continue;

		if (pi == prev_pi)
			continue;

		if ((err = phy_high_register_ioct_all(band->phytype, ii)) != BCME_OK) {
			WL_ERROR(("%s: phy_high_register_ioct_all failed\n", __FUNCTION__));
			goto fail;
		}

		prev_pi = pi;
	}

	return BCME_OK;

fail:
	return err;
}


/* ATTENTION!! INCREMENT the below when adding a module (or new error) */
/* and use new value */
#define MODULE_LAST_GUID 137

/* the err msgs help ensure valid uid is used */
#define MODULE_ATTACH(var, attach_func, num) \
	if (num > MODULE_LAST_GUID) \
		WL_ERROR(("ERROR: %s: invalid module/err num=%d last valid=%d\n", \
			__FUNCTION__, num, MODULE_LAST_GUID)); \
	if (prev_num >= num) \
		WL_ERROR(("ERROR: %s: module/err num seq: prev_num=%d num=%d\n", \
			__FUNCTION__, prev_num, num)); \
	prev_num = num; \
	if ((wlc->var = attach_func(wlc)) == NULL) { \
		WL_ERROR(("%s: %s failed\n", __FUNCTION__, # attach_func));  \
		err = num; \
		goto fail; \
	}

/* When adding a module -- */
/* increment MODULE_LAST_GUID, and use the new value in MODULE_ATTACH call */
static uint
BCMATTACHFN(wlc_attach_module)(wlc_info_t *wlc)
{
	uint err = 0;
	uint prev_num = 0;
#ifdef BCMDBG_ERR
	uint unit = wlc->pub->unit;
#endif
	BCM_REFERENCE(prev_num);
	/* Attach the notifier first, since other modules may create a notification
	 * server in their module-attach functions.
	 */
	if ((wlc->notif = bcm_notif_attach(wlc->osh, wlc->mem_pool_mgr,
	                                   wlc->pub->tunables->max_notif_servers,
	                                   wlc->pub->tunables->max_notif_clients)) == NULL) {
		WL_ERROR(("wl%d: %s: bcm_notif_attach failed\n", unit, __FUNCTION__));
		err = 500;
		goto fail;
	}
	/* Attach iovar/ioctl table (pointer) WLC registry */
	MODULE_ATTACH(iocvi, wlc_iocv_high_attach, 1);
	if (wlc_register_iovt_all(wlc, wlc->iocvi) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_register_iovt_all failed\n",
		          unit, __FUNCTION__));
		err = 501;
		goto fail;
	}
	if (wlc_register_ioct_all(wlc, wlc->iocvi) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_register_ioct_all failed\n",
		          unit, __FUNCTION__));
		err = 502;
		goto fail;
	}
	MODULE_ATTACH(hrti, wlc_hrt_attach, 2);
	MODULE_ATTACH(pcb, wlc_pcb_attach, 3);
	MODULE_ATTACH(bcmh, wlc_bsscfg_attach, 4);
	MODULE_ATTACH(scbstate, wlc_scb_attach, 5); /* scbstate before wlc_ap_attach() */
	MODULE_ATTACH(iemi, wlc_iem_attach, 6);
	MODULE_ATTACH(ieri, wlc_ier_attach, 7);
#ifdef WLOLPC
	MODULE_ATTACH(olpc_info, wlc_olpc_eng_attach, 8 /* olpc */);
#endif /* OPEN LOOP POWER CAL */

	/* TODO: Move these IE mgmt callbacks registration to their own modules */

	/* Register IE mgmt callbacks. */
	if (wlc_register_iem_fns(wlc) != BCME_OK ||
	    wlc_assoc_register_iem_fns(wlc) != BCME_OK ||
	    wlc_bcn_register_iem_fns(wlc) != BCME_OK ||
	    wlc_prq_register_iem_fns(wlc) != BCME_OK ||
	    wlc_auth_register_iem_fns(wlc) != BCME_OK ||
	    wlc_scan_register_iem_fns(wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_register_iem_fns failed\n", unit, __FUNCTION__));
		err = 9;
		goto fail;
	}

	/* TODO: Move these IE mgmt registry creation to their own modules */

	/* Create IE mgmt callback registry */
#if defined(WLTDLS) && !defined(WLTDLS_DISABLED)
	/* TDLS Setup Request Action Frame */
	if ((wlc->ier_tdls_srq = wlc_ier_create_registry(wlc->ieri,
		IEM_TDLS_SRQ_BUILD_CB_MAX, IEM_TDLS_SRQ_PARSE_CB_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, tdls setup req\n",
			unit, __FUNCTION__));
		err = 10;
		goto fail;
	}
	/* TDLS Setup Response Action Frame */
	if ((wlc->ier_tdls_srs = wlc_ier_create_registry(wlc->ieri,
		IEM_TDLS_SRS_BUILD_CB_MAX, IEM_TDLS_SRS_PARSE_CB_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, tdls setup resp\n",
			unit, __FUNCTION__));
		err = 11;
		goto fail;
	}
	/* TDLS Setup Confirm Action Frame */
	if ((wlc->ier_tdls_scf = wlc_ier_create_registry(wlc->ieri,
		IEM_TDLS_SCF_BUILD_CB_MAX, IEM_TDLS_SCF_PARSE_CB_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, tdls setup confirm\n",
			unit, __FUNCTION__));
		err = 12;
		goto fail;
	}
	/* TDLS Discovery Response Action Frame */
	if ((wlc->ier_tdls_drs = wlc_ier_create_registry(wlc->ieri,
		IEM_TDLS_DRS_BUILD_CB_MAX, IEM_TDLS_DRS_PARSE_CB_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, tdls disc resp\n",
			unit, __FUNCTION__));
		err = 13;
		goto fail;
	}
#endif /* defined(WLTDLS) && !defined(WLTDLS_DISABLED) */
#ifdef WL11AC
	/* CS Wrapper IE */
	if ((wlc->ier_csw = wlc_ier_create_registry(wlc->ieri, 2, 2)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, cs wrapper ie\n",
		          unit, __FUNCTION__));
		err = 14;
		goto fail;
	}
#endif
#if (defined(WLFBT) && !defined(WLFBT_DISABLED))
	/* FBT over the DS Action frame */
	if ((wlc->ier_fbt = wlc_ier_create_registry(wlc->ieri, 3, 3)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, fbt over the ds\n",
		          unit, __FUNCTION__));
		err = 15;
		goto fail;
	}
	/* FBT RIC request */
	if ((wlc->ier_ric = wlc_ier_create_registry(wlc->ieri, 2, 0)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, fbt ric request\n",
		          unit, __FUNCTION__));
		err = 16;
		goto fail;
	}
#endif /* WLFBT && !WLFBT_DISABLED */

	/* Get notified when an SCB is freed */
	wlc_scb_cubby_reserve(wlc, 0, wlc_txq_scb_init, wlc_txq_scb_deinit, NULL, wlc);
	/* Register the 1st txmod */
	wlc_txmod_fn_register(wlc, TXMOD_TRANSMIT, wlc, txq_txmod_fns);

#ifdef NEW_TXQ
	if ((wlc->txqi = wlc_txq_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_txq_attach failed\n", unit, __FUNCTION__));
		err = 306;
		goto fail;
	}

#ifdef USING_MUX
	MODULE_ATTACH(tx_mux, wlc_mux_module_attach, 9);
	MODULE_ATTACH(tx_scbq, wlc_scbq_module_attach, 10);
#endif

#endif /* NEW_TXQ */

#ifdef WLBMON
	MODULE_ATTACH(bmon, wlc_bmon_attach, 17);
#endif

#ifdef WLPROBRESP_SW
	MODULE_ATTACH(mprobresp, wlc_probresp_attach, 18);
#endif

	MODULE_ATTACH(prot, wlc_prot_attach, 19);
	MODULE_ATTACH(prot_g, wlc_prot_g_attach, 20);
	MODULE_ATTACH(prot_n, wlc_prot_n_attach, 21);

	MODULE_ATTACH(ap, wlc_ap_attach, 22); /* ap_attach() depends on wlc->scbstate */
#ifdef WLLED
	MODULE_ATTACH(ledh, wlc_led_attach, 23);
#endif
#ifdef WL11N
	MODULE_ATTACH(asi, wlc_antsel_attach, 24);
#endif
#if defined(STA) && defined(WLRM) && !defined(WLRM_DISABLED)
	MODULE_ATTACH(rm_info, wlc_rm_attach, 25);
#endif

#ifdef WET
	MODULE_ATTACH(weth, wlc_wet_attach, 27);
#endif
#ifdef WET_TUNNEL
	MODULE_ATTACH(wetth, wlc_wet_tunnel_attach, 28);
#endif
#ifdef WLAMSDU
	/*
	 * assume core->xmtfifo_sz have been initialized and is not band specfic
	 *   otherwise need to call wlc_amsdu_agg_limit_upd() whenever that changes
	 */
	MODULE_ATTACH(ami, wlc_amsdu_attach, 29);
#endif
#ifdef WLAMPDU
	MODULE_ATTACH(ampdu_tx, wlc_ampdu_tx_attach, 30);
	MODULE_ATTACH(ampdu_rx, wlc_ampdu_rx_attach, 31);
	if (wlc_ampdu_init(wlc)) {
		err = 32;
		goto fail;
	}
#endif
#ifdef WMF
	MODULE_ATTACH(wmfi, wlc_wmf_attach, 34);
#endif
	MODULE_ATTACH(wrsi, wlc_scb_ratesel_attach, 35);

#ifdef WLPLT
	if (WLPLT_ENAB(wlc->pub) && ((wlc->plt = wlc_plt_attach(wlc)) == NULL)) {
		WL_ERROR(("wl%d: %s: wlc_pltu_attach failed\n", unit, __FUNCTION__));
		err = 36;
		goto fail;
	}
#endif

	/* Initialize event queue; needed before following calls */
	wlc->eventq = wlc_eventq_attach(wlc->pub, wlc, wlc->wl, wlc_process_eventq);
	if (wlc->eventq == NULL) {
		WL_ERROR(("wl%d: %s: wlc_eventq_attachfailed\n", unit, __FUNCTION__));
		err = 37;
		goto fail;
	}
	ASSERT(wlc->eventq != NULL);


#if defined(WLCAC) && !defined(WLCAC_DISABLED)
	MODULE_ATTACH(cac, wlc_cac_attach, 41);
#endif
	/* allocate the sequence commands info struct */
	if ((wlc->seq_cmds_info = wlc_seq_cmds_attach(wlc, wlc_ioctl)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_seq_cmds_attach failed\n", unit, __FUNCTION__));
		err = 42;
		goto fail;
	}

#if defined(WLMCNX) && !defined(WLMCNX_DISABLED)
	MODULE_ATTACH(mcnx, wlc_mcnx_attach, 43);
	MODULE_ATTACH(tbtt, wlc_tbtt_attach, 44);
#endif
#if defined(WLP2P) && !defined(WLP2P_DISABLED)
	MODULE_ATTACH(p2p, wlc_p2p_attach, 45);
#endif
#ifdef WOWL
	/* If hw is capable, then attempt to start wowl feature */
	if (wlc_wowl_cap(wlc)) {
		MODULE_ATTACH(wowl, wlc_wowl_attach, 46);
		wlc->pub->_wowl = TRUE;
	} else
		wlc->pub->_wowl = FALSE;
#endif

#ifdef WLEXTLOG
	if ((wlc->extlog = wlc_extlog_attach(wlc->osh, wlc)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_extlog_attach failed!\n", unit, __FUNCTION__));
		err = 47;
		goto fail;
	}
#endif

#if defined(BCMAUTH_PSK) && !defined(BCMAUTH_PSK_DISABLED)
	MODULE_ATTACH(authi, wlc_auth_attach, 48);
	wlc->pub->_bcmauth_psk = TRUE;
#endif

#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR)
	MODULE_ATTACH(rwl, wlc_rwl_attach, 49);
#endif

#if defined(WLC_HOSTOID) && !defined(HOSTOIDS_DISABLED)
	if (wlc_hostoid_attach(wlc) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_hostoid_attach failed\n", unit));
		err = 50;
		goto fail;
	}
#endif

#if defined(NDIS) || (defined(WLC_HOSTOID) && !defined(HOSTOIDS_DISABLED))
	if (wlc_ndis_iovar_attach(wlc) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_ndis_iovar_attach failed\n", unit));
		err = 51;
		goto fail;
	}
#endif
#if defined(WL11K) && !defined(WL11K_DISABLED)
	MODULE_ATTACH(rrm_info, wlc_rrm_attach, 52);
#endif
#if defined(WLWNM) && !defined(WLWNM_DISABLED)
	MODULE_ATTACH(wnm_info, wlc_wnm_attach, 53);
#endif
	MODULE_ATTACH(hti, wlc_ht_attach, 54);
#ifdef WL11AC
	MODULE_ATTACH(vhti, wlc_vht_attach, 55);
#endif

	if ((wlc_stf_attach(wlc) != 0)) {
		WL_ERROR(("wl%d: %s: wlc_stf_attach failed\n", unit, __FUNCTION__));
		err = 56;
		goto fail;
	}

	if ((wlc_lq_attach(wlc) != 0)) {
		WL_ERROR(("wl%d: %s: wlc_lq_attach failed\n", unit, __FUNCTION__));
		err = 57;
		goto fail;
	}

#if defined(WLMCHAN) && !defined(WLMCHAN_DISABLED)
	MODULE_ATTACH(mchan, wlc_mchan_attach, 58);
#endif
#ifdef WL11H
	MODULE_ATTACH(m11h, wlc_11h_attach, 59);
#ifdef WLCSA
	MODULE_ATTACH(csa, wlc_csa_attach, 60);
#endif
#ifdef WLQUIET
	MODULE_ATTACH(quiet, wlc_quiet_attach, 61);
#endif
#ifdef WLDFS
	MODULE_ATTACH(dfs, wlc_dfs_attach, 62);
#endif
#endif /* WL11H */

#ifdef WLTPC
	MODULE_ATTACH(tpc, wlc_tpc_attach, 63);
#endif
#if defined(WLTDLS) && !defined(WLTDLS_DISABLED)
	MODULE_ATTACH(tdls, wlc_tdls_attach, 64);
#endif /* defined(WLTDLS) && !defined(WLTDLS_DISABLED) */

#if defined(L2_FILTER) && !defined(L2_FILTER_DISABLED)
	MODULE_ATTACH(l2_filter, wlc_l2_filter_attach, 65);
	wlc->pub->_l2_filter = TRUE;
#endif /* L2_FILTER && !L2_FILTER_DISABLED */

#ifdef WL11D
	MODULE_ATTACH(m11d, wlc_11d_attach, 66);
#endif
#ifdef WLCNTRY
	MODULE_ATTACH(cntry, wlc_cntry_attach, 67);
#endif
	if (D11REV_GE(wlc->pub->corerev, 15) &&
		(CHIPID(wlc->pub->sih->chip) != BCM4312_CHIP_ID)) {
#ifdef CCA_STATS
		MODULE_ATTACH(cca_info, wlc_cca_attach, 68);
#ifdef ISID_STATS
		MODULE_ATTACH(itfr_info, wlc_itfr_attach, 69);
#endif
#endif /* CCA_STATS */
	}
#ifdef WL11U
	MODULE_ATTACH(m11u, wlc_11u_attach, 71);
#endif
#ifdef WLDLS
	MODULE_ATTACH(dls, wlc_dls_attach, 72);
#endif
#if defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT)
	MODULE_ATTACH(mbssload, wlc_bssload_attach, 73);
#endif
#ifdef PSTA
	MODULE_ATTACH(psta, wlc_psta_attach, 74);
#endif /* PSTA */
	MODULE_ATTACH(txc, wlc_txc_attach, 75);
#if defined(WOWLPF)
	/* If hw is capable, then attempt to start wowl feature */
	if (WOWLPF_ENAB(wlc->pub)) {
		if (wlc_wowlpf_cap(wlc)) {
			MODULE_ATTACH(wowlpf, wlc_wowlpf_attach, 77);
			wlc->pub->_wowlpf = TRUE;
		} else
			wlc->pub->_wowlpf = FALSE;
	}
#endif
#ifdef WLOTA_EN
	if ((wlc->ota_info = wlc_ota_test_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wlc_attach: wlc_ota_test_attach failed\n", unit));
		err = 78;
		goto fail;
	}
#endif /* WLOTA_EN */

#ifdef WL_LPC
	/* attach power sel only if the rate sel is already up */
	MODULE_ATTACH(wlpci, wlc_scb_lpc_attach, 79);
#endif

#ifdef TRAFFIC_MGMT
	MODULE_ATTACH(trf_mgmt_ctxt, wlc_trf_mgmt_attach, 80);
#endif


#if defined(MFP) && !defined(MFP_DISABLED)
	MODULE_ATTACH(mfp, wlc_mfp_attach, 81);
#endif

	MODULE_ATTACH(macfltr, wlc_macfltr_attach, 82);

#ifdef WLNAR
	MODULE_ATTACH(nar_handle, wlc_nar_attach, 83);
#endif
#if defined(WL_RELMCAST) && !defined(WL_RELMCAST_DISABLED)
	MODULE_ATTACH(rmc, wlc_rmc_attach, 105 /* reliable multicast */);
#endif

#if defined(WL_PROXDETECT) && !defined(WL_PROXDETECT_DISABLED)
	/* Attach proxmity detection service to wlc */
	MODULE_ATTACH(pdsvc_info, wlc_proxd_attach, 130);
#endif

#if (defined(BCMSUP_PSK) && defined(BCMINTSUP) && !defined(BCMSUP_PSK_DISABLED))
	MODULE_ATTACH(idsup, wlc_sup_attach, 85);
	wlc->pub->_sup_enab = TRUE;
#endif /* (BCMSUP_PSK && BCMINTSUP && !BCMSUP_PSK_DISABLED) */

	MODULE_ATTACH(akmi, wlc_akm_attach, 86);

#ifdef STA
	MODULE_ATTACH(hs20, wlc_hs20_attach, 87);
#endif	/* STA */

#if (defined(WLFBT) && !defined(WLFBT_DISABLED))
	MODULE_ATTACH(fbt, wlc_fbt_attach, 88);
#endif

	MODULE_ATTACH(pmkid_info, wlc_pmkid_attach, 90);

	MODULE_ATTACH(btch, wlc_btc_attach, 91);

#ifdef WL_BCN_COALESCING
	MODULE_ATTACH(bc, wlc_bcn_clsg_attach, 92);
#endif /* WL_BCN_COALESCING */

#ifdef WL_STAPRIO
	MODULE_ATTACH(staprio, wlc_staprio_attach, 93);
#endif /* WL_STAPRIO */

#if defined(WL_MONITOR) || defined(BCMSPACE)
	/* Attach monitor module to wlc */
	MODULE_ATTACH(mon_info, wlc_monitor_attach, 94);
#endif 
#ifdef WL_STA_MONITOR
	MODULE_ATTACH(stamon_info, wlc_stamon_attach, 95);
#endif /* WL_STA_MONITOR */

#ifdef WDS
	MODULE_ATTACH(mwds, wlc_wds_attach, 96);
#endif /* WDS */

#ifdef WLOFFLD
	if (wlc_ol_cap(wlc)) {
		MODULE_ATTACH(ol, wlc_ol_attach, 97);
	}
#endif
#if (defined(WL_OKC) && !defined(WL_OKC_DISABLED)) || (defined(WLRCC) && \
	!defined(WLRCC_DISABLED))
	MODULE_ATTACH(okc_info, wlc_okc_attach, 98 /* OKC */);
#endif /*  defined(WL_OKC) || defined(WLRCC) */
#if defined(WLAIBSS) && !defined(WLAIBSS_DISABLED)
	MODULE_ATTACH(aibss_info, wlc_aibss_attach, 99);
#endif /* defined(WLAIBSS) && !defined(WLAIBSS_DISABLED) */

#if defined(WL_BEAMFORMING) && !defined(WLTXBF_DISABLED)
	MODULE_ATTACH(txbf, wlc_txbf_attach, 100);
#endif /* WL_BEAMFORMING */

#if defined(WL_STATS) && !defined(WL_STATS_DISABLED)
	MODULE_ATTACH(stats_info, wlc_stats_attach, 101);
#endif /* WL_STATS && WL_STATS_DISABLED */

#if defined(WLWSEC) && !defined(WLWSEC_DISABLED)
	MODULE_ATTACH(keymgmt, wlc_keymgmt_attach, 102);
#endif /* WLWSEC && !WLWSEC_DISABLED */

#if defined(SCB_BS_DATA)
	MODULE_ATTACH(bs_data_handle, wlc_bs_data_attach, 103);
#endif

	MODULE_ATTACH(obss, wlc_obss_attach, 104);

#ifdef WLDURATION
	MODULE_ATTACH(dur_info, wlc_duration_attach, 105);
#endif /* WLDURATION */

#ifdef NET_DETECT
	MODULE_ATTACH(net_detect_ctxt, wlc_net_detect_attach, 106);
#endif

#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
	MODULE_ATTACH(prot_obss, wlc_prot_obss_attach, 128);
#endif

#if defined(WL_PWRSTATS) && !defined(WL_PWRSTATS_DISABLED)
	MODULE_ATTACH(pwrstats, wlc_pwrstats_attach, 107);
#endif /* PWRSTATS */

#if defined(WLIPFO) && !defined(WLIPFO_DISABLED)
	MODULE_ATTACH(ipfo_info, wlc_ipfo_attach, 129);
#endif /* defined(WLIPFO) && !defined(WLIPFO_DISABLED) */

#if defined(MBSS) && !defined(MBSS_DISABLED)
	MODULE_ATTACH(mbss, wlc_mbss_attach, 130);
#endif /* MBSS && !MBSS_DISABLED */

#if defined(WLRSDB) && !defined(WLRSDB_DISABLED)
	if ((wlc_rsdb_attach(wlc) != 0)) {
		WL_ERROR(("wl%d: %s: wlc_rsdb_attach failed\n", unit, __FUNCTION__));
		err = 108;
		goto fail;
	}
#endif /* WLRSDB && !WLRSDB_DISABLED */

#if defined(BCMLTECOEX) && !defined(BCMLTECOEX_DISABLED)
	MODULE_ATTACH(ltecx, wlc_ltecx_attach, 131);
#endif /* defined(BCMLTECOEX) && !defined(BCMLTECOEX_DISABLED) */

#ifdef WLPROBRESP_MAC_FILTER
	MODULE_ATTACH(mprobresp_mac_filter, wlc_probresp_mac_filter_attach, 132);
#endif

#if defined(WL_BWTE) && !defined(WL_BWTE_DISABLED)
	MODULE_ATTACH(bwte_info, wlc_bwte_attach, 133);
	wlc->pub->_bwte = TRUE;
#endif /* defined(WL_BWTE) && !defined(WL_BWTE_DISABLED) */

#if defined(APCS) && !defined(APCS_DISABLED)
	MODULE_ATTACH(cs, wlc_apcs_attach, 134);
#endif /* APCS && !APCS_DISABLED */

#ifdef WL_LTR
	MODULE_ATTACH(ltr_info, wlc_ltr_attach, 135);
#endif /* WL_LTR */

#if defined(WL_TBOW) && !defined(WL_TBOW_DISABLED)
	MODULE_ATTACH(tbow_info, wlc_tbow_attach, 136);
	wlc->pub->_tbow = TRUE;
#endif /* defined(WL_TBOW) && !defined(WL_TBOW_DISABLED) */

#ifdef WL_PM_MUTE_TX
	MODULE_ATTACH(pm_mute_tx, wlc_pm_mute_tx_attach, 137);
#endif


fail:
#ifdef BCMDBG
	/* this assert ensures that MODULE_LAST_GUID is used for any new */
	/* err codes as well as new modules placed in this function */
	if (err > MODULE_LAST_GUID) {
		WL_ERROR(("ERROR: %s: errno too large!\n", __FUNCTION__));
	}
#endif /* BCMDBG */
	return err;
}

wlc_pub_t *
wlc_pub(void *wlc)
{
	return ((wlc_info_t *)wlc)->pub;
}

void wlc_get_override_vendor_dev_id(void *wlc, uint16 *vendorid, uint16 *devid)
{
	*vendorid = ((wlc_info_t *)wlc)->vendorid;
	*devid = ((wlc_info_t *)wlc)->deviceid;
}

#define CHIP_BCM5356_NON_NMODE(sih) \
	(CHIPID((sih)->chip) == BCM5356_CHIP_ID && ((sih)->chippkg & BCM5356_PKG_NONMODE) != 0)

#define CHIP_SUPPORTS_11N(wlc) \
	(!CHIP_BCM5356_NON_NMODE((wlc)->pub->sih))

static const char BCMATTACHDATA(rstr_devid)[] = "devid";
static const char BCMATTACHDATA(rstr_11n_disable)[] = "11n_disable";
static const char BCMATTACHDATA(rstr_tx_duty_cycle_ofdm)[] = "tx_duty_cycle_ofdm";
static const char BCMATTACHDATA(rstr_tx_duty_cycle_cck)[] = "tx_duty_cycle_cck";
static const char BCMATTACHDATA(rstr_tx_duty_cycle_ofdm_40_5g)[] = "tx_duty_cycle_ofdm_40_5g";
static const char BCMATTACHDATA(rstr_tx_duty_cycle_thresh_40_5g)[] = "tx_duty_cycle_thresh_40_5g";
static const char BCMATTACHDATA(rstr_tx_duty_cycle_ofdm_80_5g)[] = "tx_duty_cycle_ofdm_80_5g";
static const char BCMATTACHDATA(rstr_tx_duty_cycle_thresh_80_5g)[] = "tx_duty_cycle_thresh_80_5g";
static const char BCMATTACHDATA(rstr_pktc_policy)[] = "pktc_policy";
static const char BCMATTACHDATA(rstr_sdmaxblk)[] = "sdmaxblk";
static const char BCMATTACHDATA(rstr_sdrev)[] = "sdrev";
static const char BCMATTACHDATA(rstr_NVRAMRev)[] = "NVRAMRev";
static const char BCMATTACHDATA(rstr_aa2g)[] = "aa2g";
static const char BCMATTACHDATA(rstr_aa5g)[] = "aa5g";
static const char BCMATTACHDATA(rstr_aga0)[] = "aga0";
static const char BCMATTACHDATA(rstr_agbg0)[] = "agbg0";
static const char BCMATTACHDATA(rstr_aga1)[] = "aga1";
static const char BCMATTACHDATA(rstr_agbg1)[] = "agbg1";
static const char BCMATTACHDATA(rstr_aga2)[] = "aga2";
static const char BCMATTACHDATA(rstr_agbg2)[] = "agbg2";
static const char BCMATTACHDATA(rstr_aa0)[] = "aa0";
static const char BCMATTACHDATA(rstr_aa1)[] = "aa1";
static const char BCMATTACHDATA(rstr_ag0)[] = "ag0";
static const char BCMATTACHDATA(rstr_ag1)[] = "ag1";
static const char BCMATTACHDATA(rstr_sar5g)[] = "sar5g";
static const char BCMATTACHDATA(rstr_sar2g)[] = "sar2g";

static int
BCMATTACHFN(wlc_attach_cb_init)(wlc_info_t *wlc)
{
	int code = 0;

	/* register module IOCTL handlers */
	if (wlc_module_add_ioctl_fn(wlc->pub, wlc, _wlc_ioctl, 0, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		code = 10;
		goto fail;
	}

#ifdef WIFI_ACT_FRAME
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_AF, wlc_actionframetx_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set(af) failed\n",
		          wlc->pub->unit, __FUNCTION__));
		code = 11;
		goto fail;
	}
#endif
#ifdef STA
	/* register packet class callback */
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_PSP, wlc_sendpspoll_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set(pspoll) failed\n",
		          wlc->pub->unit, __FUNCTION__));
		code = 12;
		goto fail;
	}
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_TKIP_CM, wlc_tkip_countermeasures) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set(tkip) failed\n",
		          wlc->pub->unit, __FUNCTION__));
		code = 13;
		goto fail;
	}
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_RATE_PRB, wlc_rateprobe_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set(rate) failed\n",
		          wlc->pub->unit, __FUNCTION__));
		code = 14;
		goto fail;
	}
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_PM_NOTIF, wlc_pm_notif_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set(pmchg) failed\n",
		          wlc->pub->unit, __FUNCTION__));
		code = 15;
		goto fail;
	}
#endif /* STA */

	return BCME_OK;

fail:
	WL_ERROR(("wl%d: %s: code %d\n", wlc->pub->unit, __FUNCTION__, code));
	BCM_REFERENCE(code);
	return BCME_ERROR;
}

/** The common driver entry routine. Error codes should be unique */
void *
BCMATTACHFN(wlc_attach)(void *wl, uint16 vendor, uint16 device, uint unit, bool piomode,
                      osl_t *osh, void *regsva, uint bustype, void *btparam, void *objr, uint *perr)
{
	wlc_info_t *wlc = NULL;
	uint err = 0;
	uint j;
	wlc_pub_t *pub;
	wlc_txq_info_t *qi;
	uint n_disabled;
	wlc_bmac_state_t state_bmac;
	char *nvramrev;
	si_t *sih = NULL;
	char *vars = NULL;
	uint vars_size = 0;
#if defined(DONGLEBUILD) && !defined(PREATTACH_NORECLAIM) && !defined(BCMUCDOWNLOAD)
	uint8 ucode_reclaimed = 0;
#endif /* defined(DONGLEBUILD) && !defined(PREATTACH_NORECLAIM) && !defined(BCMUCDOWNLOAD) */

	WL_TRACE(("wl%d: %s: vendor 0x%x device 0x%x\n", unit, __FUNCTION__, vendor, device));

	/* some code depends on packed structures */
	STATIC_ASSERT(sizeof(struct ether_addr) == ETHER_ADDR_LEN);
	STATIC_ASSERT(sizeof(struct ether_header) == ETHER_HDR_LEN);
	STATIC_ASSERT(sizeof(d11regs_t) == SI_CORE_SIZE);
	STATIC_ASSERT(sizeof(ofdm_phy_hdr_t) == D11_PHY_HDR_LEN);
	STATIC_ASSERT(sizeof(cck_phy_hdr_t) == D11_PHY_HDR_LEN);
	STATIC_ASSERT(sizeof(d11txh_t) == D11_TXH_LEN);
	STATIC_ASSERT(sizeof(d11actxh_t) == D11AC_TXH_LEN);
	STATIC_ASSERT(sizeof(d11rxhdr_t) == RXHDR_LEN);
	STATIC_ASSERT(sizeof(wlc_d11rxhdr_t) == WRXHDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_llc_snap_header) == DOT11_LLC_SNAP_HDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_header) == DOT11_A4_HDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_rts_frame) == DOT11_RTS_LEN);
	STATIC_ASSERT(sizeof(struct dot11_cts_frame) == DOT11_CTS_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ack_frame) == DOT11_ACK_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ps_poll_frame) == DOT11_PS_POLL_LEN);
	STATIC_ASSERT(sizeof(struct dot11_cf_end_frame) == DOT11_CS_END_LEN);
	STATIC_ASSERT(sizeof(struct dot11_management_header) == DOT11_MGMT_HDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_auth) == DOT11_AUTH_FIXED_LEN);
	STATIC_ASSERT(sizeof(struct dot11_bcn_prb) == DOT11_BCN_PRB_LEN);
	STATIC_ASSERT(sizeof(brcm_ie_t) == BRCM_IE_LEN);
	STATIC_ASSERT(sizeof(ht_add_ie_t) == HT_ADD_IE_LEN);
	STATIC_ASSERT(sizeof(ht_cap_ie_t) == HT_CAP_IE_LEN);
	STATIC_ASSERT(OFFSETOF(wl_scan_params_t, channel_list) == WL_SCAN_PARAMS_FIXED_SIZE);
	STATIC_ASSERT(TKIP_MIC_SIZE == (2 * sizeof(uint32)));	/* some tkip code expects this */
	STATIC_ASSERT(ISPOWEROF2(MA_WINDOW_SZ));
	STATIC_ASSERT(OSL_PKTTAG_SZ >= sizeof(wlc_pkttag_t));
	STATIC_ASSERT(BRCM_IE_LEN <= WLC_MAX_BRCM_ELT);

	STATIC_ASSERT(sizeof(wlc_d11rxhdr_t) <= WL_HWRXOFF);
	STATIC_ASSERT(M_EDCF_QLEN == sizeof(shm_acparams_t));

	STATIC_ASSERT(ISALIGNED(OFFSETOF(wlc_pub_t, cur_etheraddr), sizeof(uint16)));
	STATIC_ASSERT(ISALIGNED(OFFSETOF(struct scb, ea), sizeof(uint16)));
	STATIC_ASSERT(ISALIGNED(OFFSETOF(wlc_bsscfg_t, cur_etheraddr), sizeof(uint16)));
	STATIC_ASSERT(ISALIGNED(OFFSETOF(wlc_bsscfg_t, BSSID), sizeof(uint16)));

#ifdef STA
	ASSERT(OFFSETOF(wl_assoc_params_t, chanspec_list) -
	       OFFSETOF(wl_assoc_params_t, chanspec_num) ==
	       OFFSETOF(chanspec_list_t, list) - OFFSETOF(chanspec_list_t, num) &&
	       OFFSETOF(wl_assoc_params_t, chanspec_list[1]) -
	       OFFSETOF(wl_assoc_params_t, chanspec_list[0]) ==
	       OFFSETOF(chanspec_list_t, list[1]) - OFFSETOF(chanspec_list_t, list[0]));
#endif

#ifndef PREATTACH_NORECLAIM
#ifdef DONGLEBUILD
	/* 3 stage reclaim
	 * download the ucode during attach just before allocating wlc_info
	 * & reclaim the ucode fw & its associated functions, so that
	 * attach phase malloc failure can be avoided.
	*/
#ifndef BCMUCDOWNLOAD
	wl_isucodereclaimed(&ucode_reclaimed);
	if (!ucode_reclaimed) {
		if (wlc_bmac_process_ucode_sr(device, osh, regsva, bustype, btparam)) {
			WL_ERROR(("wl%d: wlc_attach: backplane attach failed\n", unit));
			err = 1;
			goto fail;
		}
		wl_reclaim();
	}
#endif /* BCMUCDOWNLOAD */
#endif /* DONGLEBUILD */
#endif /* PREATTACH_NORECLAIM */

	sih = wlc_bmac_si_attach((uint)device, osh, regsva, bustype, btparam,
		&vars, &vars_size);
	if (sih == NULL) {
		WL_ERROR(("wl%d: %s: si_attach failed\n", unit, __FUNCTION__));
		err = 1;
		goto fail;
	}
	if (vars) {
		char *var;
		if ((var = getvar(vars, rstr_devid))) {
			uint16 devid = (uint16)bcm_strtoul(var, NULL, 0);

			WL_ERROR(("wl%d: %s: Overriding device id: 0x%x instead of 0x%x\n",
				unit, __FUNCTION__, devid, device));
			device = devid;
		}
	}

	/* allocate wlc_info_t state and its substructures */
	if ((wlc = (wlc_info_t*) wlc_attach_malloc(osh, unit, &err, device, objr)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_attach_malloc failed\n", unit, __FUNCTION__));
		goto fail;
	}
	wlc->osh = osh;
	pub = wlc->pub;

	/* stash sih, vars and vars_size in pub now */
	pub->sih = sih;
	sih = NULL;
	pub->vars = vars;
	vars = NULL;
	pub->vars_size = vars_size;
	vars_size = 0;

#ifdef BPRESET
	/* enable bpreset by default */
	pub->_bpreset = TRUE;
#endif

#if defined(BCMDBG)
	wlc_info_dbg = wlc;
#endif

	wlc->stf->onechain = -1;
	wlc->core = wlc->corestate;
	wlc->wl = wl;
	pub->unit = unit;
	pub->osh = osh;
	pub->cmn->driverrev = EPI_VERSION_NUM;
	wlc->btparam = btparam;
	wlc->brcm_ie = 1;


	pub->_piomode = piomode;
	wlc->bandinit_pending = FALSE;

	/* populate wlc_info_t with default values  */
	wlc_info_init(wlc, unit);

	/* Create the memory pool manager. */
	if (bcm_mpm_init(wlc->osh, wlc->pub->tunables->max_mempools,
	                 &wlc->mem_pool_mgr) != BCME_OK) {
		WL_ERROR(("wl%d: %s: failed to init memory pool manager\n", unit, __FUNCTION__));
		goto fail;
	}

	/* Register dump handler for memory pool manager. */
#if defined(BCMDBG)
	wlc_dump_register(pub, "mempool", wlc_dump_mempool, wlc);
#endif   

	/* update sta/ap related parameters */
	wlc_ap_upd(wlc, NULL);

	/* construct scan data */
	wlc->scan = wlc_scan_attach(wlc, wl, osh, unit);
	if (!wlc->scan) {
		WL_ERROR(("wl%d: %s: failed to malloc scan struct\n", unit, __FUNCTION__));
		err = 15;
		goto fail;
	}

	/* register a module (to handle iovars) */
	if (wlc_module_register(pub, wlc_iovars, "wlc", wlc, wlc_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          pub->unit, __FUNCTION__));
		err = 16;
		goto fail;
	}

#if defined(BCMDBG) || defined(WLTEST)
	wlc_dump_register(pub, "list", (dump_fn_t)wlc_dump_list, (void *)wlc);
#endif

#if defined(WL_OBJ_REGISTRY) && defined(BCMDBG)
	wlc_dump_register(pub, "objreg", (dump_fn_t)wlc_dump_objr, (void *)(wlc->objr));
#endif 

#if defined(BCMDBG)
	wlc_dump_register(pub, "default", (dump_fn_t)wlc_dump_default, (void *)wlc);
	wlc_dump_register(pub, "all", (dump_fn_t)wlc_dump_all, (void *)wlc);
	wlc_dump_register(pub, "wlc", (dump_fn_t)wlc_dump_wlc, (void *)wlc);
	wlc_dump_register(pub, "ratestuff", (dump_fn_t)wlc_dump_ratestuff, (void *)wlc);
	wlc_dump_register(pub, "bssinfo", (dump_fn_t)wlc_bssinfo_dump, (void *)wlc);
	wlc_dump_register(pub, "locale", wlc_channel_dump_locale, wlc);
	wlc_dump_register(pub, "wme", (dump_fn_t)wlc_dump_wme, (void *)wlc);
	wlc_dump_register(pub, "mac", (dump_fn_t)wlc_dump_mac, (void *)wlc);
	wlc_dump_register(pub, "pio", (dump_fn_t)wlc_dump_pio, (void *)wlc);
	wlc_dump_register(pub, "dma", (dump_fn_t)wlc_dump_dma, (void *)wlc);
	wlc_dump_register(pub, "stats", (dump_fn_t)wlc_dump_stats, (void *)wlc);
#ifdef BCMDBG_CTRACE
	wlc_dump_register(pub, "ctrace", (dump_fn_t)wlc_pkt_ctrace_dump, (void *)wlc);
#endif
	wlc_dump_register(pub, "bcntpl", (dump_fn_t)wlc_dump_bcntpls, (void *)wlc);
#endif 


#ifdef WLTINYDUMP
	wlc_dump_register(pub, "tiny", (dump_fn_t)wlc_tinydump, (void *)wlc);
#endif
#if defined(WLTEST)
	wlc_dump_register(pub, "nvram", (dump_fn_t)wlc_nvram_dump, (void *)wlc);
#endif
#if defined(BCMDBG)
	wlc_dump_register(pub, "pm", (dump_fn_t)wlc_dump_pm, (void *)wlc);
#endif 

#if defined(WLC_LOW) && defined(WLC_HIGH) && (defined(BCMDBG) || \
	defined(MCHAN_MINIDUMP))
	wlc_dump_register(pub, "chanswitch", (dump_fn_t)wlc_dump_chanswitch, (void *)wlc);
#endif

#if defined(WLC_HIGH_ONLY) && defined(BCMDBG)
	wlc_dump_register(pub, "rpcpkt", (dump_fn_t)wlc_dump_rpcpktlog, (void *)wlc);
	wlc_dump_register(pub, "rpc", (dump_fn_t)wlc_dump_rpc, (void *)wlc);
#endif /* WLC_HIGH_ONLY && BCMDBG */

#if defined(BCMDBG)
	wlc_dump_register(pub, "perf_stats", (dump_fn_t)wlc_dump_perf_stats, (void *)wlc);
#endif /* BCMDBG */

	/* low level attach steps(all hw accesses go inside, no more in rest of the attach) */
	err = wlc_bmac_attach(wlc, vendor, device, unit, piomode, osh, regsva, bustype, btparam);
	if (err != 0) {
		WL_ERROR(("wl%d: %s: wlc_bmac_attach failed\n", unit, __FUNCTION__));
		goto fail;
	}

#ifndef WLC_HIGH_ONLY
	wlc->fabid = si_fabid(wlc->hw->sih);
#endif /* WLC_LOW */

	wlc_pi_band_update(wlc, IS_SINGLEBAND_5G(wlc->deviceid) ? BAND_5G_INDEX : BAND_2G_INDEX);

	/* PULL BMAC states
	 * for some states, due to different info pointer(e,g, wlc, wlc_hw) or master/slave split,
	 * HIGH driver(both monolithic and HIGH_ONLY) needs to sync states FROM BMAC portion driver
	 */
	if (wlc_bmac_state_get(wlc->hw, &state_bmac) != 0) {
		WL_ERROR(("wl%d: %s: wlc_bmac_state_get failed\n", unit, __FUNCTION__));
		err = 20;
		goto fail;
	}
	wlc->machwcap = state_bmac.machwcap;

	for (j = 0; j < NFIFO; j++) {
		uint fifo_size;
		wlc_xmtfifo_sz_get(wlc, j, &fifo_size);
		wlc_xmtfifo_sz_upd_high(wlc, j, (uint16)fifo_size);
	}

	if (D11REV_LT(wlc->pub->corerev, 30))
		pub->m_coremask_blk_wowl = M_COREMASK_BLK_WOWL_L30;
	else if (D11REV_GE(wlc->pub->corerev, 42))
		pub->m_coremask_blk_wowl = D11AC_M_COREMASK_BLK_WOWL;
	else
		pub->m_coremask_blk_wowl = M_COREMASK_BLK_WOWL;

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		pub->bcn_tmpl_len = D11AC_BCN_TMPL_LEN;
		pub->d11tpl_phy_hdr_len = D11AC_PHY_HDR_LEN;
		pub->max_addrma_idx = AMT_SIZE;
		pub->m_coremask_blk = M_COREMASK_BLK;
		wlc_mfbr_retrylimit_upd(wlc);
	} else {
		pub->bcn_tmpl_len = BCN_TMPL_LEN;
		pub->d11tpl_phy_hdr_len = D11_PHY_HDR_LEN;
		pub->max_addrma_idx = RCMTA_SIZE;
		pub->m_coremask_blk = D11_PRE40_M_COREMASK_BLK;
	}

	wlc->tcpack_fast_tx = FALSE;

	pub->phy_11ncapable = WLC_PHY_11N_CAP(wlc->band);
	pub->phy_bw40_capable = WLC_40MHZ_CAP_PHY(wlc);
	pub->phy_bw80_capable = WLC_80MHZ_CAP_PHY(wlc);
	pub->phy_bw8080_capable = WLC_8080MHZ_CAP_PHY(wlc);
	pub->phy_bw160_capable = WLC_160MHZ_CAP_PHY(wlc);

#ifdef WLC_HIGH_ONLY
	/* propagate *vars* from BMAC driver to high driver */
	wlc_bmac_copyfrom_vars(wlc->hw, &pub->vars, &pub->vars_size);
#endif

	/* 11n_disable nvram */
	n_disabled = getintvar(pub->vars, rstr_11n_disable);

	if ((pub->sih->boardtype == BCM943224X21) ||
	    (pub->sih->boardvendor == VENDOR_APPLE &&
	     ((pub->sih->boardtype == BCM943224X21_FCC) ||
	      (pub->sih->boardtype == BCM943224X21B)))) {
		wlc->stf->tx_duty_cycle_pwr = WLC_DUTY_CYCLE_PWR_DEF;
		wlc->stf->pwr_throttle = AUTO;
		/* Thermal throttling is disabled for x21 */
		wlc->stf->tx_duty_cycle_thermal = NO_DUTY_THROTTLE;
		wlc->stf->tempsense_disable = TRUE;
	} else if ((pub->sih->boardvendor == VENDOR_APPLE) &&
	           ((pub->sih->boardtype == BCM94331X19) ||
	            (pub->sih->boardtype == BCM94331X19C) ||
	            (pub->sih->boardtype == BCM94331X29B) ||
	            (pub->sih->boardtype == BCM94331X29D) ||
	            (pub->sih->boardtype == BCM94331X33) ||
	            (pub->sih->boardtype == BCM94331X28) ||
	            (pub->sih->boardtype == BCM94331X28B) ||
	            (pub->sih->boardtype == BCM94360X29C) ||
	            (pub->sih->boardtype == BCM94360X29CP2) ||
	            (pub->sih->boardtype == BCM94360X29CP3) ||
	            (pub->sih->boardtype == BCM94360X52C) ||
	            (pub->sih->boardtype == BCM94360X51) ||
	            (pub->sih->boardtype == BCM94360X51B) ||
	            (pub->sih->boardtype == BCM94360X51A) ||
	            (pub->sih->boardtype == BCM943602X87) ||
	            (pub->sih->boardtype == BCM943602X238) ||
	            (pub->sih->boardtype == BCM94350X14))) {
	        wlc->stf->tx_duty_cycle_thermal = WLC_DUTY_CYCLE_THERMAL_50;
		/* duty cycle throttling disabled for pwr throttling */
		wlc->stf->tx_duty_cycle_pwr = NO_DUTY_THROTTLE;
		wlc->stf->pwr_throttle = AUTO;
	} else if ((pub->sih->boardvendor == VENDOR_BROADCOM) &&
	           (pub->sih->boardtype == BCM94331CD_SSID)) {
	  /* dutycycle throttling disabled for both thermal and pwr throttling */
		wlc->stf->tx_duty_cycle_thermal = NO_DUTY_THROTTLE;
		wlc->stf->tx_duty_cycle_pwr = NO_DUTY_THROTTLE;
		wlc->stf->pwr_throttle = AUTO;
	}

	/* set maximum allowed duty cycle */
	wlc->stf->tx_duty_cycle_ofdm = (uint16)getintvar(pub->vars, rstr_tx_duty_cycle_ofdm);
	wlc->stf->tx_duty_cycle_cck = (uint16)getintvar(pub->vars, rstr_tx_duty_cycle_cck);
#ifdef WL11AC
	wlc->stf->tx_duty_cycle_ofdm_40_5g = (uint16)getintvar(pub->vars,
		rstr_tx_duty_cycle_ofdm_40_5g);
	wlc->stf->tx_duty_cycle_thresh_40_5g = (uint16)getintvar(pub->vars,
		rstr_tx_duty_cycle_thresh_40_5g);
	wlc->stf->tx_duty_cycle_ofdm_80_5g = (uint16)getintvar(pub->vars,
		rstr_tx_duty_cycle_ofdm_80_5g);
	wlc->stf->tx_duty_cycle_thresh_80_5g = (uint16)getintvar(pub->vars,
		rstr_tx_duty_cycle_thresh_80_5g);
	/* Make sure tx duty cycle is set to smaller value when tempsense throttle is active */
	if ((wlc->stf->tx_duty_cycle_ofdm_40_5g &&
		wlc->stf->tx_duty_cycle_pwr > wlc->stf->tx_duty_cycle_ofdm_40_5g) ||
		(wlc->stf->tx_duty_cycle_ofdm_80_5g &&
		wlc->stf->tx_duty_cycle_pwr > wlc->stf->tx_duty_cycle_ofdm_80_5g)) {
		WL_ERROR(("wl%d: %s: tx_duty_cycle_ofdm (40 %d, 80 %d) must be larger than"
			" tx_duty_cycle_pwr %d.\n", unit, __FUNCTION__,
			wlc->stf->tx_duty_cycle_ofdm_40_5g, wlc->stf->tx_duty_cycle_ofdm_80_5g,
			wlc->stf->tx_duty_cycle_pwr));
		err = 23;
		goto fail;
	}
#endif /* WL11AC */

	wlc_stf_phy_chain_calc(wlc);

	/* txchain 1: txant 0, txchain 2: txant 1 */
	if (WLCISNPHY(wlc->band) && (wlc->stf->txstreams == 1))
		wlc->stf->txant = wlc->stf->hw_txchain - 1;

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc->toe_capable = TRUE;
		wlc->toe_bypass = FALSE;
	}

	/* push to BMAC driver */
	wlc_stf_chain_init(wlc);

#ifdef PKTC
	/* initialize default pktc policy */
	wlc->pktc_info->policy = getintvar(pub->vars, rstr_pktc_policy);
#endif
#if defined(PKTC_DONGLE) || (defined(__NetBSD__) && defined(PKTC))
	wlc->pktc_info->policy = PKTC_POLICY_SENDUPUC;
#endif

#ifdef WLC_LOW
	/* pull up some info resulting from the low attach */
	wlc_hw_get_txavail(wlc->hw, wlc->core->txavail);
#endif /* WLC_LOW */

	wlc->stf->ipaon = wlc_phy_txpower_ipa_ison(WLC_PI(wlc));

	wlc_bmac_hw_etheraddr(wlc->hw, &wlc->perm_etheraddr);

	bcopy((char *)&wlc->perm_etheraddr, (char *)&pub->cur_etheraddr, ETHER_ADDR_LEN);
#ifdef WLOLPC
	pub->_olpc = FALSE; /* try to enable in loop below */
#endif /* WLOLPC */

	for (j = 0; j < NBANDS(wlc); j++) {
		/* Use band 1 for single band 11a */
		if (IS_SINGLEBAND_5G(wlc->deviceid))
			j = BAND_5G_INDEX;

		wlc_pi_band_update(wlc, j);

		if (!wlc_attach_stf_ant_init(wlc)) {
			WL_ERROR(("wl%d: %s: wlc_attach_stf_ant_init failed\n",
			          unit, __FUNCTION__));
			err = 24;
			goto fail;
		}

		/* default contention windows size limits */
		wlc->band->CWmin = APHY_CWMIN;
		wlc->band->CWmax = PHY_CWMAX;

		/* set default roam parameters */
		wlc_roam_defaults(wlc, wlc->band, &wlc->band->roam_trigger_def,
			&wlc->band->roam_delta_def);
		wlc->band->roam_trigger_init_def = wlc->band->roam_trigger_def;
		wlc->band->roam_trigger = wlc->band->roam_trigger_def;
		wlc->band->roam_delta = wlc->band->roam_delta_def;

		/* init gmode value */
		if (BAND_2G(wlc->band->bandtype)) {
			wlc->band->gmode = GMODE_AUTO;
		}

		/* init _n_enab supported mode */
		if (WLC_PHY_11N_CAP(wlc->band) && CHIP_SUPPORTS_11N(wlc)) {
			if (n_disabled & WLFEATURE_DISABLE_11N) {
				pub->_n_enab = OFF;
			} else {
				pub->_n_enab = SUPPORT_11N;
				if ((WLCISHTPHY(wlc->band)) && HTREV_LT(wlc->band->phyrev, 2))
					pub->_n_enab = SUPPORT_HT;
			}
		}

		if (WLC_PHY_VHT_CAP(wlc->band)) {
			/* Setup default VHT feature set for device */
			WLC_VHT_FEATURES_DEFAULT(pub, wlc->bandstate, wlc->nbands);
			if (wlc->pub->_n_enab == OFF) {
				/* If _n_enab is off, _vht_enab should also be off */
				wlc->pub->_vht_enab = 0;
			} else {
				wlc->pub->_vht_enab = 1;
			}
#ifdef WLOLPC
			/* open loop pwr cal only available on AC phys */
			pub->_olpc = TRUE;
#endif /* WLOLPC */
		}  else {
			wlc->pub->_vht_enab = 0;
#ifdef WL11AC
			wlc->pub->vht_features = 0;
#endif
		};

		/* init per-band default rateset, depend on band->gmode */
		wlc_default_rateset(wlc, &wlc->band->defrateset);

		/* fill in hw_rateset (used early by WLC_SET_RATESET) */
		wlc_rateset_filter(&wlc->band->defrateset, &wlc->band->hw_rateset, FALSE,
		                   WLC_RATES_CCK_OFDM, RATE_MASK, wlc_get_mcsallow(wlc, NULL));
	}

	/* update antenna config due to wlc->stf->txant/txchain/ant_rx_ovr change */
	wlc_stf_phy_txant_upd(wlc);

	/* set maximum packet length */
	wlc->pkteng_maxlen = PKTBUFSZ - TXOFF;

	/* attach each modules */
	err = wlc_attach_module(wlc);
	if (err != 0) {
		WL_ERROR(("wl%d: %s: wlc_attach_module failed\n", unit, __FUNCTION__));
		goto fail;
	}

	wlc_prot_n_cfg_set(wlc->prot_n, WLC_PROT_N_PAM_OVR, (int8)state_bmac.preamble_ovr);
	/* TODO: VSDB/P2P/MCNX bmac update ? */

#ifndef ATE_BUILD
	/* Timers are not required for the ATE firmware */
	if (!wlc_timers_init(wlc, unit)) {
		WL_ERROR(("wl%d: %s: wlc_init_timer failed\n", unit, __FUNCTION__));
		err = 1000;
		goto fail;
	}
#endif /* !ATE_BUILD */

	/* depend on rateset, gmode */
	wlc->cmi = wlc_channel_mgr_attach(wlc);
	if (!wlc->cmi) {
		WL_ERROR(("wl%d: %s: wlc_channel_mgr_attach failed\n", unit, __FUNCTION__));
		err = 1001;
		goto fail;
	}

	/* init default when all parameters are ready, i.e. ->rateset */
	wlc_bss_default_init(wlc);

	/*
	 * Complete the wlc default state initializations..
	 */

	/* allocate our initial queue */
	qi = wlc_txq_alloc(wlc, osh);
	if (qi == NULL) {
		WL_ERROR(("wl%d: %s: failed to malloc tx queue\n", unit, __FUNCTION__));
		err = 1003;
		goto fail;
	}
	wlc->active_queue = qi;

#ifdef WL_MULTIQUEUE
	wlc->primary_queue = qi;

	/* allocate the excursion queue */
	qi = wlc_txq_alloc(wlc, osh);
	if (qi == NULL) {
		WL_ERROR(("wl%d: %s: failed to malloc excursion queue\n", unit, __FUNCTION__));
		err = 1004;
		goto fail;
	}
	wlc->excursion_queue = qi;
	pub->_mqueue = TRUE;
#endif /* WL_MULTIQUEUE */

	/* allocate/init the primary bsscfg */
	if (wlc_bsscfg_primary_init(wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: init primary bsscfg failed\n", unit, __FUNCTION__));
		err = 1005;
		goto fail;
	}


#if defined(BCMDBG) || defined(INTERNAL) || defined(MCHAN_MINIDUMP)
	for (j = 0; j < CHANSWITCH_LAST; j++) {
		wlc->chansw_hist[j] =
			(chanswitch_times_t *)MALLOCZ(osh, sizeof(chanswitch_times_t));
		if (NBANDS(wlc) > 1) {
			wlc->bandsw_hist[j] =
				(chanswitch_times_t *)MALLOCZ(osh, sizeof(chanswitch_times_t));
		}
	}
#endif
	wlc_set_default_txmaxpkts(wlc);

	WLCNTSET(pub->_cnt->version, WL_CNT_T_VERSION);
	WLCNTSET(pub->_cnt->length, sizeof(wl_cnt_t));

	WLCNTSET(pub->_wme_cnt->version, WL_WME_CNT_VERSION);
	WLCNTSET(pub->_wme_cnt->length, sizeof(wl_wme_cnt_t));

#ifdef WLAMSDU
#ifndef WLC_HIGH_ONLY
	if (AMSDU_TX_SUPPORT(wlc->pub)) {
		/* set AMSDU agg based on core rev */
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			/* enable AMSDU for core rev 40+ to allow large aggs */
			wlc->pub->_amsdu_tx = TRUE;
		}
		if ((CHIPID(pub->sih->chip) == BCM4345_CHIP_ID) ||
			(CHIPID(pub->sih->chip) == BCM4335_CHIP_ID)) {
			wlc->pub->_amsdu_tx = FALSE;
		}
	}
#endif /* !WLC_HIGH_ONLY */
	/* check hw AMSDU deagg capability */
	if (wlc_amsdurx_cap(wlc->ami))
		wlc->_amsdu_rx = TRUE;
	/* enable A-MSDU in A-MPDU if we have the HW (corerev 40+)
	* and we have ample rx buffers and amsdu tx is ON.
	*/
	if (D11REV_GE(wlc->pub->corerev, 40) &&
		wlc->pub->tunables->nrxbufpost >= 64 &&
		wlc->pub->_amsdu_tx) {
		wlc->_rx_amsdu_in_ampdu = TRUE;
	}
#endif	/* WLAMSDU */
	wlc_ht_init_defaults(wlc->hti);
	wlc_vht_init_defaults(wlc->vhti);
	wlc_ht_nvm_overrides(wlc->hti, n_disabled);
	/* allocate the perm scbs after everybody has reserved their cubby
	 *  can this be merged with early loop, where gmode/nmode has to before all module attach ?
	 */
	for (j = 0; j < NBANDS(wlc); j++) {
		static const struct ether_addr ether_local = {{2, 0, 0, 0, 0, 0}};

		/* Use band 1 for single band 11a */
		if (IS_SINGLEBAND_5G(wlc->deviceid))
			j = BAND_5G_INDEX;

		if (wlc->bandstate[j]->hwrs_scb == NULL) {
			wlc->bandstate[j]->hwrs_scb = wlc_internalscb_alloc(wlc, wlc->cfg,
				&ether_local, wlc->bandstate[j]);
		}
		if (!wlc->bandstate[j]->hwrs_scb) {
			WL_ERROR(("wl%d: %s: wlc_internalscb_alloc failed\n", unit, __FUNCTION__));
			err = 1100;
			goto fail;
		}
		bcopy(&wlc->bandstate[j]->hw_rateset, &wlc->bandstate[j]->hwrs_scb->rateset,
			sizeof(wlc_rateset_t));
	}

	if ((wlc->ebd = MALLOCZ(wlc->osh,
		sizeof(wlc_early_bcn_detect_t))) == NULL) {
		WL_ERROR(("wl%d: wlc_early_bcn_detect_t alloc failed\n", unit));
		err = 1101;
		goto fail;
	}
#ifdef WL_EXCESS_PMWAKE
	if ((wlc->excess_pmwake = MALLOCZ(wlc->osh,
		sizeof(wlc_excess_pm_wake_info_t))) == NULL) {
		WL_ERROR(("wl%d: wlc_excess_pm_wake_info_t alloc failed\n", unit));
		err = 1102;
		goto fail;
	}
#endif /* WL_EXCESS_PMWAKE */

#ifdef BCMUSBDEV_ENABLED
	/* USB does not have native per-AC flow control */
	pub->wlfeatureflag |= WL_SWFL_FLOWCONTROL;
#endif
#ifdef DONGLEBUILD
	/* Disable HW radio monitor for dongle builds */
	pub->wlfeatureflag |= WL_SWFL_NOHWRADIO;
#endif

#ifdef STA
	/* initialize radio_mpc_disable according to wlc->mpc */
	wlc_radio_mpc_upd(wlc);
#ifdef PCOEM_LINUXSTA
	wlc_radio_hwdisable_upd(wlc);
	if (wlc->pub->radio_disabled & WL_RADIO_HW_DISABLE) {
		wlc_radio_monitor_start(wlc);
	}
#endif /* PCOEM_LINUXSTA */
#endif /* STA */

#if defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND)
	wlc->pub->txdelay_params.period = 0;
	wlc->pub->txdelay_params.cnt = TXDELAY_SAMPLE_CNT;
	wlc->pub->txdelay_params.ratio = TXDELAY_RATIO;
#endif /* defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND) */
	if (WLANTSEL_ENAB(wlc)) {
		if ((pub->sih->chip == BCM43234_CHIP_ID) ||
		    (pub->sih->chip == BCM43235_CHIP_ID) ||
		    (pub->sih->chip == BCM43236_CHIP_ID) ||
		    (pub->sih->chip == BCM43238_CHIP_ID) ||
		    ((CHIPID(pub->sih->chip)) == BCM5357_CHIP_ID) ||
		    ((CHIPID(pub->sih->chip)) == BCM43237_CHIP_ID) ||
		    ((CHIPID(pub->sih->chip)) == BCM4749_CHIP_ID)) {
			if ((getintvar(pub->vars, rstr_aa2g) == 7) ||
			    (getintvar(pub->vars, rstr_aa5g) == 7)) {
				wlc_bmac_antsel_set(wlc->hw, 1);
			}
		}
		else
			wlc_bmac_antsel_set(wlc->hw, wlc->asi->antsel_avail);
	}


#ifdef WLATF
	/* Default for airtime fairness is in wlc_cfg.h */
	wlc->atf = WLC_ATF_ENABLE_DEFAULT;
#endif

	if (wlc_attach_cb_init(wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_attach_cb_init failed\n", unit, __FUNCTION__));
		err = 2000;
		goto fail;
	}

	nvramrev = getvar(wlc->pub->vars, rstr_NVRAMRev);
	if (nvramrev)
	{
		while (*nvramrev && !bcm_isdigit(*nvramrev)) nvramrev++;
		wlc->nvramrev = bcm_strtoul(nvramrev, NULL, 10);
	}
	WL_MPC(("wl%d: ATTACHED\n", unit));

	wl_indicate_maccore_state(wlc->wl, LTR_SLEEP);
	wlc->cmn->hostmem_access_enabled = TRUE;

	if (perr)
		*perr = 0;

	return ((void*)wlc);

fail:
	WL_ERROR(("wl%d: %s: failed with err %d\n", unit, __FUNCTION__, err));
	if (sih) {
		wlc_bmac_si_detach(osh, sih);
		sih = NULL;
	}
	if (vars) {
		MFREE(osh, vars, vars_size);
		vars = NULL;
		vars_size = 0;
	}
	if (wlc)
		wlc_detach(wlc);

	if (perr)
		*perr = err;
	return (NULL);
}

#ifdef WL_ANTGAIN
#define NUM_ANT      3
#define MAXNUM_ANT_ENTRY	(sizeof(ant_default)/sizeof(ant_default[0]))
#define WLC_TXMINGAIN 0
typedef struct {
	uint8	band2g [WLC_TXCORE_MAX];
	uint8	band5g [WLC_TXCORE_MAX];
} ant_gain_limit;

const struct {
	uint16	boardtype;
	ant_gain_limit antgain;
} ant_default [] = {
	{BCM94360X51P2,  {{0xc5, 0x83, 6, WLC_TXMINGAIN}, /* 2g ant gain 5.75 3.5 6 */
	{0xc7, 0xc7, 7, WLC_TXMINGAIN}}}, /* 5g ant gain 7.75 7.75 7 */
	{BCM94360X51P3,  {{6, 0x85, 0x83, WLC_TXMINGAIN}, /* 2g ant gain 6 5.5 3.5 */
	{0x83, 0x83, 5, WLC_TXMINGAIN}}}, /* 5g ant gain 3.5 3.5 5 */
	{BCM94360X29CP3,  {{0xc5, 0xc5, 0xc5, WLC_TXMINGAIN}, /* 2g ant gain 5.75 5.75 5.75 */
	{0xc6, 0xc6, 0xc6, WLC_TXMINGAIN}}}, /* 5g ant gain 6.75 6.75 6.75 */
};
#endif /* WL_ANTGAIN */

static int8
wlc_antgain_calc(int8 ag)
{
	int8 gain, fract;
	/* Older sroms specified gain in whole dbm only.  In order
	 * be able to specify qdbm granularity and remain backward compatible
	 * the whole dbms are now encoded in only low 6 bits and remaining qdbms
	 * are encoded in the hi 2 bits. 6 bit signed number ranges from
	 * -32 - 31. Examples: 0x1 = 1 db,
	 * 0xc1 = 1.75 db (1 + 3 quarters),
	 * 0x3f = -1 (-1 + 0 quarters),
	 * 0x7f = -.75 (-1 in low 6 bits + 1 quarters in hi 2 bits) = -3 qdbm.
	 * 0xbf = -.50 (-1 in low 6 bits + 2 quarters in hi 2 bits) = -2 qdbm.
	 */
	gain = ag & 0x3f;
	gain <<= 2;	/* Sign extend */
	gain >>= 2;
	fract = (ag & 0xc0) >> 6;
	return (4 * gain + fract);
}

static void
BCMATTACHFN(wlc_attach_antgain_init)(wlc_info_t *wlc)
{
	int8 antgain = wlc->band->antgain;

	if ((antgain == -1) && (wlc->pub->sromrev == 1)) {
		/* default antenna gain for srom rev 1 is 2 dBm (8 qdbm) */
		antgain = 8;
	} else if (antgain == -1) {
		WL_ERROR(("wl%d: %s: Invalid antennas available in srom, using 2dB\n",
		          wlc->pub->unit, __FUNCTION__));
		antgain = 8;
	} else if (wlc->pub->sromrev < 11) {
		antgain = wlc_antgain_calc(antgain);
	} else if (wlc->pub->sromrev >= 11) {
		char *vars = wlc->pub->vars;
		int bandtype = wlc->band->bandtype;
		int8 ag[3], max_ag = 0;
		uint i;

		ag[0] = (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_aga0 : rstr_agbg0));
		if (CHIPID(wlc->pub->sih->chip) != BCM4335_CHIP_ID) {
			ag[1] = (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_aga1 : rstr_agbg1));
			ag[2] = (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_aga2 : rstr_agbg2));
#ifdef WL_ANTGAIN
			for (i = 0; i < MAXNUM_ANT_ENTRY; i++) {
				if (ant_default[i].boardtype == wlc->pub->sih->boardtype) {
					uint j;
					for (j = 0; j < NUM_ANT; j++) {
					ag[j] = BAND_5G(bandtype) ?
						ant_default[i].antgain.band5g[j]:
						ant_default[i].antgain.band2g[j];
					}
				break;
				}
			}
#endif
			for (i = 0; i < 3; i++) {
				ag[i] = wlc_antgain_calc(ag[i]);
				max_ag = MAX(ag[i], max_ag);
			}
		} else {
			max_ag = wlc_antgain_calc(ag[0]);
		}

		antgain = max_ag;
		WL_INFORM(("wl%d: %s: SROM %s antenna gain is %d\n", wlc->pub->unit, __FUNCTION__,
			BAND_5G(bandtype) ? "5G" : "2G", antgain));
	}
	wlc->band->antgain = antgain;
}

static void
BCMATTACHFN(wlc_attach_sar_init)(wlc_info_t *wlc)
{
#define MAX_5G_SAR	(15 * WLC_TXPWR_DB_FACTOR)
#define MAX_2G_SAR	(18 * WLC_TXPWR_DB_FACTOR)
	bool apply_sar = FALSE;
	int8 sar, fract;

	if (((wlc->pub->sih->boardvendor == VENDOR_APPLE) &&
	     ((wlc->pub->sih->boardtype == BCM94331X19) ||
	      (wlc->pub->sih->boardtype == BCM94331X19C) ||
	      (wlc->pub->sih->boardtype == BCM94331X29B) ||
	      (wlc->pub->sih->boardtype == BCM94331X29D))) ||
	    (wlc->pub->sih->boardtype == BCM94331CS_SSID) ||
	    (wlc->pub->sih->boardtype == BCM94331HM_SSID))
		apply_sar = TRUE;

	if (!apply_sar) {
		wlc->band->sar = WLC_TXPWR_MAX;
		return;
	}

	if (wlc->band->sar == -1)
		wlc->band->sar = (BAND_5G(wlc->band->bandtype) ? MAX_5G_SAR : MAX_2G_SAR);
	else {
		/* use same format as antgain
		 * lower 6 bits is whole number
		 * upper 2 bits is fraction (qdb)
		 */
		sar = wlc->band->sar & 0x3f;
		sar <<= 2;	/* Sign extend */
		sar >>= 2;
		fract = (wlc->band->sar & 0xc0) >> 6;
		wlc->band->sar = WLC_TXPWR_DB_FACTOR * sar + fract;
	}

	ASSERT(wlc->band->sar >= (BCM94331X19_MINTXPOWER * WLC_TXPWR_DB_FACTOR));
	if (wlc->band->sar <= (BCM94331X19_MINTXPOWER * WLC_TXPWR_DB_FACTOR))
		wlc->band->sar = (BAND_5G(wlc->band->bandtype) ? MAX_5G_SAR : MAX_2G_SAR);

	WL_INFORM(("%s: Update %s SAR %d\n", __FUNCTION__,
		(BAND_5G(wlc->band->bandtype) ? "5G" : "2G"), wlc->band->sar));
}

static bool
BCMATTACHFN(wlc_attach_stf_ant_init)(wlc_info_t *wlc)
{
	int aa;
	char *vars;
	int bandtype;

	vars = wlc->pub->vars;
	bandtype = wlc->band->bandtype;

	/* get antennas available */
	aa = (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_aa5g : rstr_aa2g));
	if (aa == 0)
		aa = (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_aa1 : rstr_aa0));
	if ((aa < 1) || (aa > 15)) {
		WL_ERROR(("wl%d: %s: Invalid antennas available in srom (0x%x), using 3.\n",
			wlc->pub->unit, __FUNCTION__, aa));
		aa = 3;
	}

	if (CHIPID(wlc->pub->sih->chip) == BCM4325_CHIP_ID)
		aa = 1;

	/* reset the defaults if we have a single antenna */
	if (aa == 1) {
		WL_INFORM(("wl%d: %s: Single antenna available (aa = %d) forcing 0(main)\n",
			wlc->pub->unit, __FUNCTION__, aa));
		wlc->stf->ant_rx_ovr = ANT_RX_DIV_FORCE_0;
		wlc->stf->txant = ANT_TX_FORCE_0;
	} else if (aa == 2) {
		WL_INFORM(("wl%d: %s: Single antenna available (aa = %d) forcing 1(aux)\n",
			wlc->pub->unit, __FUNCTION__, aa));
		wlc->stf->ant_rx_ovr = ANT_RX_DIV_FORCE_1;
		wlc->stf->txant = ANT_TX_FORCE_1;
	} else  {
	}

	/* Compute Antenna Gain */
	if (CHIPID(wlc->pub->sih->chip) != BCM4335_CHIP_ID) {
		wlc->band->antgain =
		        (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_ag1 : rstr_ag0));
	}
	wlc_attach_antgain_init(wlc);

	wlc->band->sar = (int8)getintvar(vars, (BAND_5G(bandtype) ? rstr_sar5g : rstr_sar2g));
	wlc_attach_sar_init(wlc);
	wlc->band->sar_cached = wlc->band->sar;

#define AA2DBI		(2 * WLC_TXPWR_DB_FACTOR)
	if (wlc->pub->sih->boardvendor == VENDOR_APPLE) {
		if (wlc->band->antgain > AA2DBI)
			goto exit;

		if ((wlc->pub->sih->boardtype == BCM943224X21_FCC) ||
		    (wlc->pub->sih->boardtype == BCM943224X21B)) {
			/* antenna gain for X21 is 6.75dBi (5G), 6.0dBi (2.4G) */
			wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
				((6 * WLC_TXPWR_DB_FACTOR) + 3) : (6 * WLC_TXPWR_DB_FACTOR);
		} else if ((wlc->pub->sih->boardtype == BCM943224M93A) ||
			(wlc->pub->sih->boardtype == BCM943224M93)) {
			/* antenna gain for X21b is 6.25dBi (5G), 6.0dBi (2.4G) */
			wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
				((6 * WLC_TXPWR_DB_FACTOR) + 3) : (6 * WLC_TXPWR_DB_FACTOR);
		} else if (wlc->pub->sih->boardtype == BCM94322X9) {
			/* antenna gain for X9 is 6.75dBi (5G), 3.25dBi (2.4G) */
			wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
				((6 * WLC_TXPWR_DB_FACTOR) + 3) : ((3 * WLC_TXPWR_DB_FACTOR) + 1);
		} else if (wlc->pub->sih->boardtype == BCM943224X16) {
			if ((wlc->pub->boardrev & 0x0fff) < 0x0411) {
				/* antenna gain for X16 is 6.75dBi (5G), 6.0dBi (2.4G) */
				wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
					((6 * WLC_TXPWR_DB_FACTOR) + 3) : (6 * WLC_TXPWR_DB_FACTOR);
			} else {
				/* antenna gain for X16HG is 6.0dBi (5G), 6.75dBi (2.4G) */
				wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
					(6 * WLC_TXPWR_DB_FACTOR) : ((6 * WLC_TXPWR_DB_FACTOR) + 3);
			}
		} else if (wlc->pub->sih->boardtype == BCM94322M35e) {
			/* antenna gain for M35e is 4.75dBi (5G), 4.0dBi (2.4G) */
			wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
				((4 * WLC_TXPWR_DB_FACTOR) + 3) : (4 * WLC_TXPWR_DB_FACTOR);
		} else if ((wlc->pub->sih->boardtype == BCM94331X19) ||
		           (wlc->pub->sih->boardtype == BCM94331X28)) {
			wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
				(7 * WLC_TXPWR_DB_FACTOR) : ((5 * WLC_TXPWR_DB_FACTOR) + 3);
		}
	}
exit:
	/* This fixup pre-production modules which have antenna
	 * gain programmed as 2dBi. (7 dBi (5G), 5.75 dBi (2G))
	 */
	if (((wlc->pub->sih->boardtype == BCM94331HM_SSID) ||
	     (wlc->pub->sih->boardtype == BCM94331CS_SSID)) &&
	    (wlc->band->antgain <= AA2DBI)) {
		wlc->band->antgain = BAND_5G(wlc->band->bandtype) ?
			(7 * WLC_TXPWR_DB_FACTOR) : ((5 * WLC_TXPWR_DB_FACTOR) + 3);
	}
	return TRUE;
}

static void
BCMATTACHFN(wlc_timers_deinit)(wlc_info_t *wlc)
{
	/* free timer state */
	if (wlc->wdtimer) {
		wl_free_timer(wlc->wl, wlc->wdtimer);
		wlc->wdtimer = NULL;
	}

	if (wlc->radio_timer) {
		wl_free_timer(wlc->wl, wlc->radio_timer);
		wlc->radio_timer = NULL;
	}

#ifdef STA
	if (wlc->iscan_timer) {
		wl_free_timer(wlc->wl, wlc->iscan_timer);
		wlc->iscan_timer = NULL;
	}

	if (wlc->pm2_radio_shutoff_dly_timer) {
		wl_free_timer(wlc->wl, wlc->pm2_radio_shutoff_dly_timer);
		wlc->pm2_radio_shutoff_dly_timer = NULL;
	}

	if (wlc->wake_event_timer) {
		wl_free_timer(wlc->wl, wlc->wake_event_timer);
		wlc->wake_event_timer = NULL;
	}

#ifdef WLRXOV
	if (WLRXOV_ENAB(wlc->pub) && wlc->rxov_timer) {
		wl_free_timer(wlc->wl, wlc->rxov_timer);
		wlc->rxov_timer = NULL;
	}
#endif
#endif /* STA */
}

#define MODULE_DETACH(var, detach_func) \
	if (wlc->var) { \
		detach_func(wlc->var); \
		wlc->var = NULL; \
	}

static void
BCMATTACHFN(wlc_detach_module)(wlc_info_t *wlc)
{
#if defined(APCS) && !defined(APCS_DISABLED)
	MODULE_DETACH(cs, wlc_apcs_detach);
#endif /* APCS && !APCS_DISABLED */

#ifdef WLPROBRESP_MAC_FILTER
	MODULE_DETACH(mprobresp_mac_filter, wlc_probresp_mac_filter_detach);
#endif
	MODULE_DETACH(macfltr, wlc_macfltr_detach);

#if defined(MBSS) && !defined(MBSS_DISABLED)
	MODULE_DETACH(mbss, wlc_mbss_detach);
#endif /* MBSS && !MBSS_DISABLED */
#if defined(MFP) && !defined(MFP_DISABLED)
	MODULE_DETACH(mfp, wlc_mfp_detach);
#endif
	MODULE_DETACH(txc, wlc_txc_detach);
#if defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT)
	MODULE_DETACH(mbssload, wlc_bssload_detach);
#endif
#ifdef WLDLS
	MODULE_DETACH(dls, wlc_dls_detach);
#endif
#ifdef WL11U
	MODULE_DETACH(m11u, wlc_11u_detach);
#endif
#if defined(BCMAUTH_PSK) && !defined(BCMAUTH_PSK_DISABLED)
	MODULE_DETACH(authi, wlc_auth_detach);
#endif
#ifdef WL11N
	MODULE_DETACH(asi, wlc_antsel_detach);
#endif
#ifdef WET
	MODULE_DETACH(weth, wlc_wet_detach);
#endif
#ifdef WET_TUNNEL
	MODULE_DETACH(wetth, wlc_wet_tunnel_detach);
#endif
#ifdef WLAMSDU
	MODULE_DETACH(ami, wlc_amsdu_detach);
#endif
#ifdef WLAMPDU
	MODULE_DETACH(ampdu_tx, wlc_ampdu_tx_detach);
	MODULE_DETACH(ampdu_rx, wlc_ampdu_rx_detach);
	wlc_ampdu_deinit(wlc);
#endif
#ifdef WLNAR
	MODULE_DETACH(nar_handle, wlc_nar_detach);
#endif

#if defined(WLMCHAN) && !defined(WLMCHAN_DISABLED)
	MODULE_DETACH(mchan, wlc_mchan_detach);
#endif
#if defined(WLP2P) && !defined(WLP2P_DISABLED)
	MODULE_DETACH(p2p, wlc_p2p_detach);
#endif
#if defined(WLMCNX) && !defined(WLMCNX_DISABLED)
	MODULE_DETACH(tbtt, wlc_tbtt_detach);
	MODULE_DETACH(mcnx, wlc_mcnx_detach);
#endif
	MODULE_DETACH(wrsi, wlc_scb_ratesel_detach);

#ifdef PSTA
	MODULE_DETACH(psta, wlc_psta_detach);
#endif

#ifdef WLPLT
	if (WLPLT_ENAB(wlc->pub)) {
		MODULE_DETACH(plt, wlc_plt_detach);
	}
#endif
#ifdef WOWL
	MODULE_DETACH(wowl, wlc_wowl_detach);
#endif

	MODULE_DETACH(seq_cmds_info, wlc_seq_cmds_detach);
#if defined(WLCAC) && !defined(WLCAC_DISABLED)
	MODULE_DETACH(cac, wlc_cac_detach);
#endif
#ifdef WMF
	MODULE_DETACH(wmfi, wlc_wmf_detach);
#endif
#ifdef WLEXTLOG
	MODULE_DETACH(extlog, wlc_extlog_detach);
#endif
#if defined(WL11K) && !defined(WL11K_DISABLED)
	MODULE_DETACH(rrm_info, wlc_rrm_detach);
#endif
#if defined(WLWNM) && !defined(WLWNM_DISABLED)
	MODULE_DETACH(wnm_info, wlc_wnm_detach);
#endif
#if defined(WLC_HOSTOID) && !defined(HOSTOIDS_DISABLED)
	wlc_hostoid_detach(wlc);
#endif
#if defined(NDIS) || (defined(WLC_HOSTOID) && !defined(HOSTOIDS_DISABLED))
	wlc_ndis_iovar_detach(wlc);
#endif
	wlc_stf_detach(wlc);
	wlc_lq_detach(wlc);
#if defined(STA) && defined(WLRM) && !defined(WLRM_DISABLED)
	MODULE_DETACH(rm_info, wlc_rm_detach);
#endif
#if defined(WLWSEC) && !defined(WLWSEC_DISABLED)
	MODULE_DETACH(keymgmt, wlc_keymgmt_detach);
#endif /* WLWSEC && !WLWSEC_DISABLED */
#ifdef WL11H
	MODULE_DETACH(m11h, wlc_11h_detach);
#ifdef WLCSA
	MODULE_DETACH(csa, wlc_csa_detach);
#endif
#ifdef WLQUIET
	MODULE_DETACH(quiet, wlc_quiet_detach);
#endif
#ifdef WL_PM_MUTE_TX
	if (wlc->pm_mute_tx != NULL) {
		wlc_pm_mute_tx_detach(wlc->pm_mute_tx);
		wlc->pm_mute_tx = NULL;
	}
#endif
#ifdef WLDFS
	MODULE_DETACH(dfs, wlc_dfs_detach);
#endif
#endif /* WL11H */

#ifdef WLTPC
	MODULE_DETACH(tpc, wlc_tpc_detach);
#endif
#if defined(WLTDLS) && !defined(WLTDLS_DISABLED)
	MODULE_DETACH(tdls, wlc_tdls_detach);
#endif /* defined(WLTDLS) && !defined(WLTDLS_DISABLED) */

#if defined(L2_FILTER) && !defined(L2_FILTER_DISABLED)
	MODULE_DETACH(l2_filter, wlc_l2_filter_detach);
#endif /* L2_FILTER && !L2_FILTER_DISABLED */

#ifdef WL11D
	MODULE_DETACH(m11d, wlc_11d_detach);
#endif
#ifdef WLCNTRY
	MODULE_DETACH(cntry, wlc_cntry_detach);
#endif
#ifdef CCA_STATS
#ifdef ISID_STATS
	MODULE_DETACH(itfr_info, wlc_itfr_detach);
#endif
	MODULE_DETACH(cca_info, wlc_cca_detach);
#endif /* CCA_STATS */
	MODULE_DETACH(prot, wlc_prot_detach);
	MODULE_DETACH(prot_g, wlc_prot_g_detach);
	MODULE_DETACH(prot_n, wlc_prot_n_detach);
#ifdef WLPROBRESP_SW
	MODULE_DETACH(mprobresp, wlc_probresp_detach);
#endif
#ifdef WOWLPF
	MODULE_DETACH(wowlpf, wlc_wowlpf_detach);
#endif

#ifdef TRAFFIC_MGMT
	MODULE_DETACH(trf_mgmt_ctxt, wlc_trf_mgmt_detach);
#endif
#ifdef NET_DETECT
	MODULE_DETACH(net_detect_ctxt, wlc_net_detect_detach);
#endif
#ifdef WL_LPC
	MODULE_DETACH(wlpci, wlc_scb_lpc_detach);
#endif
#ifdef WLBMON
	MODULE_DETACH(bmon, wlc_bmon_detach);
#endif

#if defined(NEW_TXQ) && defined(USING_MUX)
	MODULE_DETACH(tx_scbq, wlc_scbq_module_detach);
	MODULE_DETACH(tx_mux, wlc_mux_module_detach);
#endif

#ifdef WL11AC
	MODULE_DETACH(vhti, wlc_vht_detach);
#endif
	MODULE_DETACH(obss, wlc_obss_detach);
	MODULE_DETACH(hti, wlc_ht_detach);

	MODULE_DETACH(akmi, wlc_akm_detach);
#ifdef STA
	MODULE_DETACH(hs20, wlc_hs20_detach);
#endif	/* STA */

#if (defined(WLFBT) && !defined(WLFBT_DISABLED))
	MODULE_DETACH(fbt, wlc_fbt_detach);
#endif
#if (defined(BCMSUP_PSK) && defined(BCMINTSUP) && !defined(BCMSUP_PSK_DISABLED))
	MODULE_DETACH(idsup, wlc_sup_detach);
#endif
	MODULE_DETACH(pmkid_info, wlc_pmkid_detach);

#ifdef WLTDLS
	if (wlc->ier_tdls_drs != NULL)
		wlc_ier_destroy_registry(wlc->ier_tdls_drs);
	if (wlc->ier_tdls_scf != NULL)
		wlc_ier_destroy_registry(wlc->ier_tdls_scf);
	if (wlc->ier_tdls_srs != NULL)
		wlc_ier_destroy_registry(wlc->ier_tdls_srs);
	if (wlc->ier_tdls_srq != NULL)
		wlc_ier_destroy_registry(wlc->ier_tdls_srq);
#endif
#ifdef WL11AC
	if (wlc->ier_csw != NULL)
		wlc_ier_destroy_registry(wlc->ier_csw);
#endif
#if (defined(WLFBT) && !defined(WLFBT_DISABLED))
	if (wlc->ier_fbt != NULL)
		wlc_ier_destroy_registry(wlc->ier_fbt);

	if (wlc->ier_ric != NULL)
		wlc_ier_destroy_registry(wlc->ier_ric);
#endif

#ifdef WLOTA_EN
	MODULE_DETACH(ota_info, wlc_ota_test_detach);
#endif

#if defined(WL_PROXDETECT) && !defined(WL_PROXDETECT_DISABLED)
	MODULE_DETACH(pdsvc_info, wlc_proxd_detach);
#endif

#if defined(WL_RELMCAST) && !defined(WL_RELMCAST_DISABLED)
	MODULE_DETACH(rmc, wlc_rmc_detach);
#endif

	MODULE_DETACH(btch, wlc_btc_detach);
#ifdef WLOLPC
	MODULE_DETACH(olpc_info, wlc_olpc_eng_detach);
#endif /* OPEN LOOP POWER CAL */

#ifdef WL_BCN_COALESCING
	MODULE_DETACH(bc, wlc_bcn_clsg_detach);
#endif /* WL_BCN_COALESCING */

#ifdef WL_STAPRIO
	MODULE_DETACH(staprio, wlc_staprio_detach);
#endif /* WL_STAPRIO */

#ifdef WL_STA_MONITOR
	MODULE_DETACH(stamon_info, wlc_stamon_detach);
#endif /* WL_STA_MONITOR */
#if defined(WL_MONITOR) || defined(BCMSPACE)
	/* Detach monitor module from wlc */
	MODULE_DETACH(mon_info, wlc_monitor_detach);
#endif 

#ifdef WDS
	MODULE_DETACH(mwds, wlc_wds_detach);
#endif /* WDS */

#ifdef WLOFFLD
	MODULE_DETACH(ol, wlc_ol_detach);
#endif
#if (defined(WL_OKC) && !defined(WL_OKC_DISABLED)) || (defined(WLRCC) && \
	!defined(WLRCC_DISABLED))
	MODULE_DETACH(okc_info, wlc_okc_detach);
#endif /* WL_OKC || WLRCC */
#if defined(WL_BEAMFORMING) && !defined(WLTXBF_DISABLED)
	MODULE_DETACH(txbf, wlc_txbf_detach);
#endif /* WL_BEAMFORMING */
	if (wlc->ebd) {
		MFREE(wlc->osh, wlc->ebd, sizeof(wlc_early_bcn_detect_t));
		wlc->ebd = NULL;
	}

#if defined(WL_STATS) && !defined(WL_STATS_DISABLED)
	MODULE_DETACH(stats_info, wlc_stats_detach);
#endif /* WL_STATS && WL_STATS_DISABLED */

#ifdef WLDURATION
	MODULE_DETACH(dur_info, wlc_duration_detach);
#endif /* WLDURATION */

#if defined(WLAIBSS) && !defined(WLAIBSS_DISABLED)
	MODULE_DETACH(aibss_info, wlc_aibss_detach);
#endif /* defined(WLAIBSS) && !defined(WLAIBSS_DISABLED) */

#if defined(SCB_BS_DATA)
	MODULE_DETACH(bs_data_handle, wlc_bs_data_detach);
#endif

#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
	MODULE_DETACH(prot_obss, wlc_prot_obss_detach);
#endif

#ifdef WL_LTR
	MODULE_DETACH(ltr_info, wlc_ltr_detach);
#endif

	/* Keep these the last and in the this order since other modules
	 * may reference them.
	 */
	MODULE_DETACH(ieri, wlc_ier_detach);
	MODULE_DETACH(iemi, wlc_iem_detach);
	MODULE_DETACH(scbstate, wlc_scb_detach);
	MODULE_DETACH(bcmh, wlc_bsscfg_detach);
	MODULE_DETACH(pcb, wlc_pcb_detach);
	MODULE_DETACH(hrti, wlc_hrt_detach);
	MODULE_DETACH(iocvi, wlc_iocv_high_detach);
	MODULE_DETACH(notif, bcm_notif_detach);
#ifdef NEW_TXQ
	MODULE_DETACH(txqi, wlc_txq_detach);
#endif /* NEW_TXQ */
#if defined(BCMLTECOEX) && !defined(BCMLTECOEX_DISABLED)
	MODULE_DETACH(ltecx, wlc_ltecx_detach);
#endif /* defined(BCMLTECOEX) && !defined(BCMLTECOEX_DISABLED) */
#if defined(WL_PWRSTATS) && !defined(WL_PWRSTATS_DISABLED)
	MODULE_DETACH(pwrstats, wlc_pwrstats_detach);
#endif /* PWRSTATS */
#ifdef WL_EXCESS_PMWAKE
	if (wlc->excess_pmwake) {
		MFREE(wlc->osh, wlc->excess_pmwake, sizeof(wlc_excess_pm_wake_info_t));
		wlc->excess_pmwake = NULL;
	}
#endif /* WL_EXCESS_PMWAKE */
#if defined(WLRSDB) && !defined(WLRSDB_DISABLED)
	wlc_rsdb_detach(wlc);
#endif /* WLRSDB && !WLRSDB_DISABLED */

#if defined(WL_TBOW) && !defined(WL_TBOW_DISABLED)
	MODULE_DETACH(tbow_info, wlc_tbow_detach);
#endif /* defined(WL_TBOW) && !defined(WL_TBOW_DISABLED) */

#if defined(WL_BWTE) && !defined(WL_BWTE_DISABLED)
	MODULE_DETACH(bwte_info, wlc_bwte_detach);
#endif /* defined(WL_BWTE) && !defined(WL_BWTE_DISABLED) */
}

/**
 * Return a count of the number of driver callbacks still pending.
 *
 * General policy is that wlc_detach can only dealloc/free software states. It can NOT
 *  touch hardware registers since the d11core may be in reset and clock may not be available.
 *    One exception is sb register access, which is possible if crystal is turned on
 * After "down" state, driver should avoid software timer with the exception of radio_monitor.
 */
uint
BCMATTACHFN(wlc_detach)(wlc_info_t *wlc)
{
	uint i;
	uint callbacks = 0;
	wlc_bsscfg_t *bsscfg;

	if (wlc == NULL)
		return 0;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	ASSERT(!wlc->pub->up);

	if (SRHWVSDB_ENAB(wlc->pub)) {
		wlc_srvsdb_reset_engine(wlc); /* resets software context, not hardware */
		wlc_bmac_deactivate_srvsdb(wlc->hw);
	}

#ifdef WLLED
	if (wlc->ledh) {
		wlc_led_deinit(wlc->ledh);
	}
#endif

	/* delete software timers */
	if (!wlc_radio_monitor_stop(wlc))
		callbacks++;

#ifdef STA
	/* ...including any wake_event timer */
	if (wlc->wake_event_timer) {
		if (!wl_del_timer(wlc->wl, wlc->wake_event_timer))
			callbacks ++;
	}

	/* free bcn_prb in wlc_bss_info_t in case they haven't been */
	wlc_bss_list_free(wlc, wlc->scan_results);
	wlc_bss_list_free(wlc, wlc->as_info->join_targets);
	wlc_bss_list_free(wlc, wlc->custom_scan_results);

	if (wlc->iscan_ignore_list)
		MFREE(wlc->osh, wlc->iscan_ignore_list, WLC_ISCAN_IGNORE_LIST_SZ);
#endif /* STA */

	if (wlc->eventq) {
		wlc_eventq_detach(wlc->eventq);
		wlc->eventq = NULL;
	}

#ifdef WLLED
	if (wlc->ledh) {
		callbacks += wlc_led_detach(wlc->ledh);
		wlc->ledh = NULL;
	}
#endif

	/* free the internal scb */
	for (i = 0; i < NBANDS(wlc); i++) {
		wlcband_t *bandstate = (NBANDS(wlc) == 1) ? wlc->band:wlc->bandstate[i];

		if (bandstate->hwrs_scb) {
			WL_INFORM(("hwrs_scb: band %d: free internal scb for 0x%p\n",
				i, bandstate->hwrs_scb));
			wlc_internalscb_free(wlc, bandstate->hwrs_scb);
		}
		bandstate->hwrs_scb = NULL;
	}

	/* Free memory for bsscfg structs */
	FOREACH_BSS(wlc, i, bsscfg) {
		wlc_bsscfg_free(wlc, bsscfg);
	}

	wlc_channel_mgr_detach(wlc->cmi);

	wlc_timers_deinit(wlc);

#ifdef WL_MULTIQUEUE
	wlc->txfifo_detach_transition_queue = NULL;
#endif
	while (wlc->tx_queues != NULL) {
		wlc_txq_free(wlc, wlc->osh, wlc->tx_queues);
	}

	wlc_detach_module(wlc);

	/* free all h/w states */
	callbacks += wlc_bmac_detach(wlc);

	/* free the sih now */
	if (wlc->pub->sih) {
		wlc_bmac_si_detach(wlc->osh, wlc->pub->sih);
		wlc->pub->sih = NULL;
	}

	/* free other state */

	/* free vars now */
	if (wlc->pub->vars) {
		MFREE(wlc->osh, wlc->pub->vars, wlc->pub->vars_size);
		wlc->pub->vars = NULL;
	}

	{
	/* free dumpcb list */
	dumpcb_t *prev, *ptr;
	prev = ptr = wlc->dumpcb_head;
	while (ptr) {
		ptr = prev->next;
		MFREE(wlc->osh, prev, sizeof(dumpcb_t));
		prev = ptr;
	}
	wlc->dumpcb_head = NULL;
	}

	/* Remove IOCTL handler */
	wlc_module_remove_ioctl_fn(wlc->pub, (void *)wlc);

	/* Detach from iovar manager */
	wlc_module_unregister(wlc->pub, "wlc", wlc);

	if (wlc->scan) {
		wlc_scan_detach(wlc->scan);
		wlc->scan = NULL;
	}

	if (wlc->ap) {
		wlc_ap_detach(wlc->ap);
		wlc->ap = NULL;
	}


#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR)
	if (wlc->rwl) {
		wlc_rwl_detach(wlc->rwl);
		wlc->rwl = NULL;
	}
#endif /* RWL_WIFI || WIFI_REFLECTOR */

#if defined(BCMDBG) || defined(INTERNAL) || defined(MCHAN_MINIDUMP)
	for (i = 0; i < CHANSWITCH_LAST; i++) {
		if (wlc->chansw_hist[i]) {
			MFREE(wlc->osh, wlc->chansw_hist[i], sizeof(chanswitch_times_t));
			wlc->chansw_hist[i] = NULL;
		}
		if (wlc->bandsw_hist[i]) {
			MFREE(wlc->osh, wlc->bandsw_hist[i], sizeof(chanswitch_times_t));
			wlc->bandsw_hist[i] = NULL;
		}
	}
#endif /* defined(BCMDBG) || defined(INTERNAL) */

	/*
	 * consistency check: wlc_module_register/wlc_module_unregister calls
	 * should match therefore nothing should be left here.
	 */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (wlc->modulecb[i].name[0] == 0)
			continue;
		if (obj_registry_islast(wlc->objr)) {
			WL_ERROR(("wl%d: module %s is still registered\n", wlc->pub->unit,
				wlc->modulecb[i].name));
			ASSERT(wlc->modulecb[i].name[0] == '\0');
		}
	}

	WL_MPC(("wl%d: DETACHED, callbacks %d\n", wlc->pub->unit, callbacks));

	/* Free the memory pool manager. */
	bcm_mpm_deinit(&wlc->mem_pool_mgr);

	wlc_detach_mfree(wlc, wlc->osh);

	return (callbacks);
}

#if defined(WLC_LOW) && defined(WLC_HIGH) && (defined(BCMDBG) || \
	defined(MCHAN_MINIDUMP))
void
chanswitch_history(wlc_info_t *wlc, chanspec_t from, chanspec_t to, uint32 start,
	uint32 end, int context)
{
	chanswitch_times_t *history;
	int i;

	ASSERT(context >= 0 && context < CHANSWITCH_LAST);

	if (CHSPEC_WLCBANDUNIT(from) == CHSPEC_WLCBANDUNIT(to))
		history = wlc->chansw_hist[context];
	else
		history = wlc->bandsw_hist[context];

	ASSERT(history);
	if (!history) return;
	i = history->index;
	history->entry[i].from = from;
	history->entry[i].to = to;
	history->entry[i].start = start;
	history->entry[i].end = end;

	history->index = (i + 1) % CHANSWITCH_TIMES_HISTORY;
}
static int
wlc_dump_chanswitch(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int i, j, nsamps, contexts;
	uint band;
	chanswitch_times_t *history = NULL;
	uint32 diff, total;
	uint us, ms, ts;
	char chanbuf[CHANSPEC_STR_LEN];
	char chanbuf1[CHANSPEC_STR_LEN];
	const char *chanswitch_history_names[] = {
		"wlc_set_chanspec",
		"wlc_adopt_context",
		"wlc_preppm_context"
	};

	if (ARRAYSIZE(chanswitch_history_names) != CHANSWITCH_LAST) {
		WL_ERROR(("%s: num_labels needs to match num of events!\n", __FUNCTION__));
		return -1;
	}

	for (contexts = 0; contexts < CHANSWITCH_LAST; contexts++) {
		bcm_bprintf(b, "**** %s  **** \n", chanswitch_history_names[contexts]);

		for (band = 0; band < NBANDS(wlc); band++) {
			if (band == 0) {
				history = wlc->chansw_hist[contexts];
				bcm_bprintf(b, "Channelswitch:      Duration"
					"          timestamp\n");
			} else {
				history = wlc->bandsw_hist[contexts];
				bcm_bprintf(b, "Bandswitch:         Duration"
					"          timestamp\n");
			}
			ASSERT(history);
			j = history->index % CHANSWITCH_TIMES_HISTORY;
			total = 0;
			nsamps = 0;
			for  (i = 0; i < CHANSWITCH_TIMES_HISTORY; i++) {
				diff = history->entry[j].end - history->entry[j].start;
				if (diff) {
					us = (diff % TSF_TICKS_PER_MS) * 1000 / TSF_TICKS_PER_MS;
					ms = diff / TSF_TICKS_PER_MS;
					total += diff;
					nsamps++;

					ts = history->entry[j].start / TSF_TICKS_PER_MS;

					bcm_bprintf(b, "%-6s => %-6s"
					"      %2.2d.%03u             %03u\n",
					wf_chspec_ntoa(history->entry[j].from, chanbuf),
					wf_chspec_ntoa(history->entry[j].to, chanbuf1),
					ms, us, ts);
				}
				j = (j + 1) % CHANSWITCH_TIMES_HISTORY;
			}
			if (nsamps) {
				total /= nsamps;
				us = (total % TSF_TICKS_PER_MS) * 1000 / TSF_TICKS_PER_MS;
				ms = total / TSF_TICKS_PER_MS;
				bcm_bprintf(b, "%s: %s: Avg %d.%03u Millisecs, %d Samples\n\n",
					chanswitch_history_names[contexts],
					band ? "Bandswitch" : "Channelswitch",
					ms, us, nsamps);
			} else {
				bcm_bprintf(b, "  -                   -                   -\n");
			}
		}
	}
	return 0;
}
#endif 

/** update state that depends on the current value of "ap" */
void
wlc_ap_upd(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg)
{
	/* force AUTO where AP uses it. */
	if (bsscfg && BSSCFG_AP(bsscfg))
		/* AP: short not allowed, but not enforced */
		bsscfg->PLCPHdr_override = WLC_PLCP_AUTO;

	/* disable vlan_mode on AP since some legacy STAs cannot rx tagged pkts */
	wlc->vlan_mode = AP_ENAB(wlc->pub) ? OFF : AUTO;

	/* fixup mpc */
	wlc->mpc = wlc_mpccap(wlc);

	/* always disable oper_mode on STA/AP switch */
	if (bsscfg)
		bsscfg->oper_mode_enabled = FALSE;
}

/** read hwdisable state and propagate to wlc flag */
static void
wlc_radio_hwdisable_upd(wlc_info_t* wlc)
{
	if ((wlc->pub->wlfeatureflag & WL_SWFL_NOHWRADIO) ||
	    wlc->pub->hw_off)
		return;

	if (wlc_bmac_radio_read_hwdisabled(wlc->hw)) {
#ifdef WLEXTLOG
		if (!mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE))
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_HW_OFF_ID,
				WL_LOG_LEVEL_ERR, 0, 0, NULL);
#endif
		mboolset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE);
	} else {
#ifdef WLEXTLOG
		if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE))
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_HW_ON_ID,
				WL_LOG_LEVEL_ERR, 0, 0, NULL);
#endif
		mboolclr(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE);
	}
}

static bool
wlc_mpccap(wlc_info_t* wlc)
{
	bool mpc_capable = TRUE;


	if ((D11REV_IS(wlc->pub->corerev, 4)) && (wlc->deviceid == BCM4306_D11DUAL_ID))
		mpc_capable = FALSE;

#ifdef PSTA
	if (PSTA_ENAB(wlc->pub))
		mpc_capable = FALSE;
#endif
	/* MPC cannot be tested on QT builds. disable it by default */
	if (ISSIM_ENAB(wlc->pub->sih))
		mpc_capable = FALSE;

	return mpc_capable;
}

/* return TRUE if Minimum Power Consumption should be entered, FALSE otherwise */
#ifdef STA
static bool
wlc_is_non_delay_mpc(wlc_info_t* wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

	/* mpc feature must be enabled when this function is called! */
	ASSERT(wlc->mpc);

	/* force mpc off when any of these global conditions meet */
	if (wlc->pub->associated ||
		SCAN_IN_PROGRESS(wlc->scan) || MONITOR_ENAB(wlc) ||
		wlc->mpc_out || wlc->mpc_scan || wlc->mpc_join ||
		wlc->mpc_oidscan || wlc->mpc_oidjoin || wlc->mpc_oidnettype ||
		wlc->pub->delayed_down || BTA_IN_PROGRESS(wlc) ||
		wlc->mpc_off_req) {

		return FALSE;
	}

	/* force mpc off when any of these per bsscfg conditions meet */
	FOREACH_BSS(wlc, idx, cfg) {
		if ((BSSCFG_STA(cfg) &&
			(cfg->assoc->state != AS_IDLE || cfg->assoc->rt ||
			wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, cfg))) ||
		    (BSSCFG_AP(cfg) && wlc_bss_wdsscb_getcnt(wlc, cfg) > 0))
		    return FALSE;
	}

	return TRUE;
}

bool
wlc_ismpc(wlc_info_t* wlc)
{
	return ((wlc->mpc_delay_off == 0) && (wlc_is_non_delay_mpc(wlc)));
}

void
wlc_radio_mpc_upd(wlc_info_t* wlc)
{
	bool radio_state;

	/*
	* Clear the WL_RADIO_MPC_DISABLE bit when mpc feature is disabled
	* in case the WL_RADIO_MPC_DISABLE bit was set. Stop the radio
	* monitor also when WL_RADIO_MPC_DISABLE is the only reason that
	* the radio is going down.
	*/
	if (!wlc->mpc) {
		WL_MPC(("wl%d: radio_disabled %x radio_monitor %d\n", wlc->pub->unit,
			wlc->pub->radio_disabled, wlc->radio_monitor));
		if (!wlc->pub->radio_disabled)
			return;
		wlc->mpc_dur += OSL_SYSUPTIME() - wlc->mpc_laston_ts;
		mboolclr(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE);
		wlc_radio_upd(wlc);
		if (!wlc->pub->radio_disabled)
			wlc_radio_monitor_stop(wlc);
		return;
	}

	/*
	 * sync ismpc logic with WL_RADIO_MPC_DISABLE bit in wlc->pub->radio_disabled
	 * to go ON, always call radio_upd synchronously
	 * to go OFF, postpone radio_upd to later when context is safe(e.g. watchdog)
	 */
	radio_state = (mboolisset(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE) ? OFF : ON);

	WL_MPC(("radio_state %d, non-del-mpc %d, delay_off %d, delay_cnt %d, off_time %u\n",
		radio_state, wlc_is_non_delay_mpc(wlc),
		wlc->mpc_delay_off, wlc->mpc_dlycnt,
		OSL_SYSUPTIME() - wlc->mpc_off_ts));

	if (wlc_ismpc(wlc) == TRUE) {
		/* Just change the state here. Let later context bring down the radio */
		mboolset(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE);
	} else {
		if (radio_state == OFF) {
			/* Clear the state and bring up radio immediately */
			mboolclr(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE);
			wlc_radio_upd(wlc);

			/* Calculate the mpc_dlycnt based on how long radio was off */
			if (OSL_SYSUPTIME() - wlc->mpc_off_ts < WLC_MPC_THRESHOLD * 1000) {
				wlc->mpc_dlycnt = WLC_MPC_MAX_DELAYCNT;
			} else	{
				wlc->mpc_dlycnt = WLC_MPC_MIN_DELAYCNT;
			}

			WL_MPC(("wl%d: mpc delay %d\n", wlc->pub->unit, wlc->mpc_dlycnt));
			wlc->mpc_dur += OSL_SYSUPTIME() - wlc->mpc_laston_ts;
		}
		if (wlc_is_non_delay_mpc(wlc) == FALSE)
			wlc->mpc_delay_off = wlc->mpc_dlycnt;
	}
}

int wlc_mpc_off_req_set(wlc_info_t *wlc, mbool mask, mbool val)
{
	int ret = BCME_OK;

	/* set the MPC request mask and adjust the radio if necessary */
	wlc->mpc_off_req = (wlc->mpc_off_req & ~mask) | (val & mask);

	/* update radio MPC state from this setting */
	wlc_radio_mpc_upd(wlc);

	return ret;
}


static bool
wlc_scb_wds_cb(struct scb *scb)
{
	return SCB_WDS(scb) != NULL ? TRUE : FALSE;
}

static uint8
wlc_bss_wdsscb_getcnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	return wlc_bss_scb_getcnt(wlc, cfg, wlc_scb_wds_cb);
}
#endif /* STA */

static bool
wlc_scb_ps_cb(struct scb *scb)
{
	return (SCB_PS(scb) != 0) ? TRUE : FALSE;
}

uint8
wlc_bss_psscb_getcnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	return wlc_bss_scb_getcnt(wlc, cfg, wlc_scb_ps_cb);
}

/**
 * centralized radio disable/enable function,
 * invoke radio enable/disable after updating hwradio status
 */
void
wlc_radio_upd(wlc_info_t* wlc)
{
	bool changed = FALSE;
	WL_MPC(("wl%d, wlc_radio_upd, radio_disabled 0x%x\n", wlc->pub->unit,
		wlc->pub->radio_disabled));

	if (wlc->pub->radio_disabled) {
		wlc->mpc_laston_ts = OSL_SYSUPTIME();
		changed = wlc_radio_disable(wlc);
	} else
		changed = wlc_radio_enable(wlc);

	if (wlc->pub->last_radio_disabled != wlc->pub->radio_disabled) {
		changed = TRUE;
		wlc->pub->last_radio_disabled = wlc->pub->radio_disabled;
	}

	/* Send RADIO indications only if radio state actually changed */
	if (changed) {
#ifdef STA
		/* signal event to OS dependent layer */
		wlc_mac_event(wlc, WLC_E_RADIO, NULL, 0, 0, 0, 0, 0);
#endif
	}
}

/** maintain LED behavior in down state */
static void
wlc_down_led_upd(wlc_info_t *wlc)
{
#ifdef STA
	int idx;
	wlc_bsscfg_t *cfg;
#endif
	ASSERT(!wlc->pub->up);

#ifdef STA
#ifdef WLC_HIGH_ONLY
	if (bcm_rpc_is_asleep(wlc->rpc) == TRUE)
		return;
#endif

	/* maintain LEDs while in down state, turn on sbclk if not available yet */
	FOREACH_BSS(wlc, idx, cfg) {
		if (BSSCFG_STA(cfg)) {
			/* turn on sbclk if necessary */
			wlc_pllreq(wlc, TRUE, WLC_PLLREQ_FLIP);

#ifdef WLLED
			wlc_led_event(wlc->ledh);
#endif

			wlc_pllreq(wlc, FALSE, WLC_PLLREQ_FLIP);

			break;
		}
	}
#endif /* STA */
}

/** Returns TRUE if function actually brings down the nic or radio */
bool
wlc_radio_disable(wlc_info_t *wlc)
{
	if (!wlc->pub->up) {
		wlc_down_led_upd(wlc);
		return FALSE;
	}

	/* Check if are already going down */
	if (wlc->going_down) {
		return FALSE;
	}

	WL_MPC(("wl%d: wlc_radio_disable, radio off\n", wlc->pub->unit));
	wlc_radio_monitor_start(wlc);
	WL_APSTA_UPDN(("wl%d: wlc_radio_disable() -> wl_down()\n", wlc->pub->unit));
	wl_down(wlc->wl);
	return TRUE;
}

/** Returns True if function actually brings up the nic or radio */
static bool
wlc_radio_enable(wlc_info_t *wlc)
{
	if (wlc->pub->up)
		return FALSE;

	if (DEVICEREMOVED(wlc))
		return FALSE;

	if (!wlc->down_override) {	/* imposed by wl down/out ioctl */
		WL_MPC(("wl%d: wlc_radio_enable, radio on\n", wlc->pub->unit));
		WL_APSTA_UPDN(("wl%d: wlc_radio_enable() -> wl_up()\n", wlc->pub->unit));
		wl_up(wlc->wl);
	}
	return TRUE;
}

/** periodical query hw radio button while driver is "down" */
static void
wlc_radio_timer(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	/* validate all the reasons driver could be down and running this radio_timer */
	ASSERT(wlc->pub->radio_disabled || wlc->down_override);
	wlc_radio_hwdisable_upd(wlc);
	wlc_radio_upd(wlc);
}

static bool
wlc_radio_monitor_start(wlc_info_t *wlc)
{
	wlc->mpc_off_ts = OSL_SYSUPTIME();

	/* Don't start the timer if HWRADIO feature is disabled */
	if (wlc->radio_monitor || (wlc->pub->wlfeatureflag & WL_SWFL_NOHWRADIO))
		return TRUE;

	wlc->radio_monitor = TRUE;
#ifdef WLC_LOW
	wlc_pllreq(wlc, TRUE, WLC_PLLREQ_RADIO_MON);
#endif /* WLC_HIGH_ONLY */
	wl_add_timer(wlc->wl, wlc->radio_timer, TIMER_INTERVAL_RADIOCHK, TRUE);
	return TRUE;
}

bool
wlc_radio_monitor_stop(wlc_info_t *wlc)
{
	if (!wlc->radio_monitor)
		return TRUE;

	ASSERT((wlc->pub->wlfeatureflag & WL_SWFL_NOHWRADIO) != WL_SWFL_NOHWRADIO);

	wlc->radio_monitor = FALSE;
#ifdef WLC_LOW
	wlc_pllreq(wlc, FALSE, WLC_PLLREQ_RADIO_MON);
#endif /* WLC_HIGH_ONLY */
	return (wl_del_timer(wlc->wl, wlc->radio_timer));
}

bool
wlc_down_for_mpc(wlc_info_t *wlc)
{
	if (!wlc->pub->up && !wlc->down_override &&
	    mboolisset(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE) &&
	    !mboolisset(wlc->pub->radio_disabled, ~WL_RADIO_MPC_DISABLE))
		return TRUE;

	return FALSE;
}

/** bring the driver down, but don't reset hardware */
void
wlc_out(wlc_info_t *wlc)
{
	wlc_bmac_set_noreset(wlc->hw, TRUE);
	wlc_radio_upd(wlc);
	wl_down(wlc->wl);
	wlc_bmac_set_noreset(wlc->hw, FALSE);

	/* core clk is TRUE in BMAC driver due to noreset, need to mirror it in HIGH */
	wlc->clk = TRUE;

	/* This will make sure that when 'up' is done
	 * after 'out' it'll restore hardware (especially gpios)
	 */
	wlc->pub->hw_up = FALSE;
}

#if !defined(NEW_TXQ)
#ifdef BCMASSERT_SUPPORT
/* Sanity check for tx_prec_map and fifo synchup
 * Either there are some packets pending for the fifo, else if fifo is empty then
 * all the corresponding precmap bits should be set
 */
#define WLC_TX_FIFO_CHECK(wlc, fifo) (\
	TXPKTPENDGET(wlc, fifo) > 0 || \
	(TXPKTPENDGET(wlc, fifo) == 0 && \
	 ((wlc)->tx_prec_map & (wlc)->fifo2prec_map[fifo]) == (wlc)->fifo2prec_map[fifo]) || \
	(PIO_ENAB((wlc)->pub) && \
	 ((wlc)->tx_suspended || !wlc_pio_txavailable(WLC_HW_PIO(wlc, fifo), 1, 1))) \
)

/* Verify the sanity of wlc->tx_prec_map. This can be done only by making sure that
 * if there is no packet pending for the FIFO, then the corresponding prec bits should be set
 * in prec_map. Of course, ignore this rule when block_datafifo is set
 */
static bool
wlc_tx_prec_map_verify(wlc_info_t *wlc)
{
	/* For non-WME, both fifos have overlapping prec_map. So it's an error only if both
	 * fail the check.
	 */
	if (!EDCF_ENAB(wlc->pub)) {
		if (!(WLC_TX_FIFO_CHECK(wlc, TX_DATA_FIFO) ||
		      WLC_TX_FIFO_CHECK(wlc, TX_CTL_FIFO)))
			return FALSE;
		else
			return TRUE;
	}

	return (WLC_TX_FIFO_CHECK(wlc, TX_AC_BK_FIFO) && WLC_TX_FIFO_CHECK(wlc, TX_AC_BE_FIFO) &&
	        WLC_TX_FIFO_CHECK(wlc, TX_AC_VI_FIFO) && WLC_TX_FIFO_CHECK(wlc, TX_AC_VO_FIFO));
}
#endif /* BCMASSERT_SUPPORT */
#endif /* !NEW_TXQ */

static void
wlc_watchdog_timer(void *arg)
{
#ifdef STA
	wlc_info_t *wlc = (wlc_info_t *)arg;
#endif /* STA */

	wlc_watchdog(arg);

#ifdef STA
	if (WLC_WATCHDOG_TBTT(wlc)) {
		/* set to normal osl watchdog period */
		wl_del_timer(wlc->wl, wlc->wdtimer);
		wl_add_timer(wlc->wl, wlc->wdtimer, TIMER_INTERVAL_WATCHDOG, TRUE);
	}
#endif /* STA */
}

#ifdef STA
void
wlc_handle_ap_lost(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_roam_t *roam = cfg->roam;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	int reason;

	/*
	 * Clear software BSSID information so that we do not
	 * process future beacons from this AP so that the roam
	 * code will continue attempting to join
	 */
	wlc_bss_clear_bssid(cfg);

	/* If we are in spectrum management mode and this channel is
	 * subject to radar detection rules, or this is a restricted
	 * channel, restore the channel's quiet bit until we
	 * reestablish contact with our AP
	 */
	if ((WL11H_ENAB(wlc) && wlc_radar_chanspec(wlc->cmi, current_bss->chanspec)) ||
	     wlc_restricted_chanspec(wlc->cmi, current_bss->chanspec))
		wlc_set_quiet_chanspec(wlc->cmi, current_bss->chanspec);

	/* Restore the frequency tracking bandwidth if we lost our AP */
	wlc_freqtrack_reset(wlc);

	WL_ASSOC(("wl%d: ROAM: time_since_bcn %d > bcn_timeout %d : link down\n",
		WLCWLUNIT(wlc), roam->time_since_bcn, roam->bcn_timeout));

	WL_ASSOC(("wl%d: original roam reason %d\n", WLCWLUNIT(wlc), roam->original_reason));
	switch (roam->original_reason) {
		case WLC_E_REASON_DEAUTH:
		case WLC_E_REASON_DISASSOC:
		case WLC_E_REASON_MINTXRATE:
		case WLC_E_REASON_DIRECTED_ROAM:
		case WLC_E_REASON_TSPEC_REJECTED:
		case WLC_E_REASON_BETTER_AP:
			reason = WLC_E_LINK_DISASSOC;
			break;
		case WLC_E_REASON_BCNS_LOST:
			reason = WLC_E_LINK_BCN_LOSS;
			break;
		default:
			reason = WLC_E_LINK_BCN_LOSS;
	}
	if (reason == WLC_E_LINK_BCN_LOSS) {
		WL_APSTA_UPDN(("wl%d: Reporting link down on config 0 (STA lost beacons)\n",
			WLCWLUNIT(wlc)));
	} else {
		WL_APSTA_UPDN(("wl%d: Reporting link down on config 0 (link down reason %d)\n",
			WLCWLUNIT(wlc), reason));
	}
#if defined(MACOSX)
	wlc->cacheRssiVal = wlc->cfg->link->rssi;
#endif
	wlc_link(wlc, FALSE, &cfg->prev_BSSID, cfg, reason);
	roam->bcns_lost = TRUE;

	/* reset rssi moving average */
	wlc_lq_rssi_snr_noise_reset_ma(wlc, cfg, WLC_RSSI_INVALID,
		WLC_SNR_INVALID, WLC_NOISE_INVALID);
	wlc_lq_rssi_event_update(cfg);

}

static void
wlc_link_monitor_watchdog(wlc_info_t *wlc)
{
	int i;
	wlc_bsscfg_t *cfg;

	/* link monitor, roam, ... */
	FOREACH_AS_STA(wlc, i, cfg)	{
		wlc_roam_t *roam = cfg->roam;
		wlc_assoc_t *assoc = cfg->assoc;
		wlc_bss_info_t *bss = cfg->current_bss;
		wlc_cxn_t *cxn = cfg->cxn;


#ifdef PSTA
		/* No need to monitor for proxy stas */
		if (PSTA_ENAB(wlc->pub) && BSSCFG_PSTA(cfg))
			continue;
#endif /* PSTA */

		/* Monitor connection to a network */
		if (cfg->BSS) {
			struct scb *scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);

#ifdef WL_BCN_COALESCING
			bool flushed = FALSE;
#endif /* WL_BCN_COALESCING */

			/* Increment the time since the last full scan, if caching is desired */
			if (roam->partialscan_period && roam->active) {
				roam->time_since_upd++;
			}

			if (roam->motion_rssi_delta > 0 && scb != NULL &&
			    (wlc->pub->now - scb->assoctime) > WLC_ROAM_SCAN_PERIOD) {
				/* Don't activate motion detection code until we are at a
				 * "moderate" RSSI or lower
				 */
				if (!roam->RSSIref && bss->RSSI < MOTION_DETECT_RSSI) {
					WL_ASSOC(("wl%d: %s: Setting reference RSSI in this bss "
					          "to %d\n", wlc->pub->unit, __FUNCTION__,
					          bss->RSSI));
					roam->RSSIref = bss->RSSI;
				}
				if (roam->RSSIref)
					wlc_roam_motion_detect(cfg);
			}

#ifdef WL_BCN_COALESCING
			if (wlc_bcn_clsg_in_ucode(wlc->bc, roam->time_since_bcn, &flushed) &&
			    (!roam->old_bcn)) {
				roam->time_since_bcn = 0;
			}
			if (flushed)
				roam->old_bcn = TRUE;

#endif /* WL_BCN_COALESCING */
#ifdef WLOFFLD
			if (WLOFFLD_BCN_ENAB(wlc->pub)) {
				if (wlc_ol_time_since_bcn(wlc->ol))
					roam->time_since_bcn = 0;
			}
#endif
			/* Handle Roaming if beacons are lost */
			if (roam->time_since_bcn > 0) {

				uint roam_time_thresh = wlc->pub->roam_time_thresh; /* millisec */
				wlc_pm_st_t *pm = cfg->pm;
				uint32 bp = bss->beacon_period;

#ifdef BCMDBG
				if (ETHER_ISNULLADDR(&cfg->BSSID)) {
					WL_ASSOC(("wl%d.%d: time_since_bcn %d\n", WLCWLUNIT(wlc),
						WLC_BSSCFG_IDX(cfg), roam->time_since_bcn));
				} else {
					WL_ERROR(("wl%d.%d: time_since_bcn %d\n", WLCWLUNIT(wlc),
						WLC_BSSCFG_IDX(cfg), roam->time_since_bcn));
				}
#endif /* BCMDBG */

				/* convert from Kusec to millisec */
				bp = (bp << 10)/1000;

				if (roam_time_thresh < roam->max_roam_time_thresh) {
					/* bcn_timeout should be nonzero */
					ASSERT(roam->bcn_timeout != 0);
					roam_time_thresh = MIN(roam->max_roam_time_thresh,
						(roam->bcn_timeout*1000)/2);
				}

				/* No beacon seen for a while, check unaligned beacon
				 * at UATBTT_TO_ROAM_BCN time before roam threshold time
				 */
				if (roam->time_since_bcn*1000u + bp * UATBTT_TO_ROAM_BCN >=
					roam_time_thresh) {

					if (pm->PMenabled && !pm->PM_override &&
						!ETHER_ISNULLADDR(&cfg->BSSID)) {
						WL_ASSOC(("wl%d: ROAM: check for unaligned TBTT, "
							"time_since_bcn %d Sec\n",
							WLCWLUNIT(wlc), roam->time_since_bcn));
						wlc_set_uatbtt(cfg, TRUE);
						ASSERT(STAY_AWAKE(wlc));
					}
					if (roam->timer_active == FALSE) {
						wl_add_timer(wlc->wl, roam->timer,
							bp * UATBTT_TO_ROAM_BCN, FALSE);
						roam->timer_active = TRUE;
					}
				}

				if ((roam->time_since_bcn*1000u) < roam_time_thresh) {
					/* clear the consec_roam_bcns_lost counter */
					/* because we've seen at least 1 beacon since */
					/* we last called wlc_roam_bcns_lost() */
					roam->consec_roam_bcns_lost = 0;
				}
			}


			/*
			 * Too many lost beacons. Perhaps, this is one of those old
			 * 802.11b Linksys APs which has a carrier frequency jitter.
			 * If we are currently associated with a B/G band AP and the
			 * link hasn't gone down yet, increase the frequency tracking
			 * bandwidth of the 802.11b demodulator and check if we get
			 * any beacons.
			 */
			if (roam->time_since_bcn > WLC_FREQTRACK_THRESHOLD &&
			    !roam->bcns_lost)
				wlc_freqtrack(wlc);

			/* If the link was down and we got a beacon, mark it 'up' */
			/* bcns_lost indicates whether the link is marked 'down' or not */
			if (roam->time_since_bcn == 0 &&
			    roam->bcns_lost) {
				roam->bcns_lost = FALSE;
				bcopy(&cfg->prev_BSSID, &cfg->BSSID, ETHER_ADDR_LEN);
				bcopy(&cfg->prev_BSSID, &bss->BSSID,
				      ETHER_ADDR_LEN);
				WL_APSTA_UPDN(("wl%d: Reporting link up on config 0 (STA"
				               " recovered beacons)\n", WLCWLUNIT(wlc)));
				wlc_link(wlc, TRUE, &cfg->BSSID, cfg, 0);
				WL_ASSOC(("wl%d: ROAM: new beacon: called link_up() \n",
				            WLCWLUNIT(wlc)));
			}

#if defined(DBG_BCN_LOSS)
			if (roam->time_since_bcn > roam->bcn_timeout) {
				struct scb *ap_scb = wlc_scbfindband(wlc, cfg, &cfg->BSSID,
					CHSPEC_WLCBANDUNIT(bss->chanspec));
				char eabuf[ETHER_ADDR_STR_LEN];
				char *phy_buf;
				struct bcmstrbuf b;

				if (ap_scb) {
					bcm_ether_ntoa(&cfg->BSSID, eabuf);
					WL_ERROR(("bcn_loss:\tLost beacon: %d AP:%s\n",
						roam->time_since_bcn, eabuf));
					WL_ERROR(("bcn_loss:\tNow: %d AP RSSI: %d BI: %d\n",
						wlc->pub->now, wlc->cfg->link->rssi,
						bss->beacon_period));

					WL_ERROR(("bcn_loss:\tTime of Assoc: %d"
						" Last pkt from AP: %d RSSI: %d BCN_RSSI: %d"
						" Last pkt to AP: %d\n",
						ap_scb->assoctime,
						ap_scb->dbg_bcn.last_rx,
						ap_scb->dbg_bcn.last_rx_rssi,
						ap_scb->dbg_bcn.last_bcn_rssi,
						ap_scb->dbg_bcn.last_tx));

					#define PHYCAL_DUMP_LEN 300
					phy_buf = MALLOC(wlc->osh, PHYCAL_DUMP_LEN);
					if (phy_buf != NULL) {
						bcm_binit(&b, phy_buf, PHYCAL_DUMP_LEN);
						(void)wlc_bmac_dump(wlc->hw, "phycalrxmin", &b);

						WL_ERROR(("bcn_loss:\tphydebug: 0x%x "
							"psmdebug: 0x%x psm_brc: 0x%x\n%s",
							R_REG(wlc->osh, &wlc->regs->phydebug),
							R_REG(wlc->osh, &wlc->regs->psmdebug),
							R_REG(wlc->osh, &wlc->regs->psm_brc),
							phy_buf));
						MFREE(wlc->osh, phy_buf, PHYCAL_DUMP_LEN);
					} else {
						WL_ERROR(("wl%d:%s: out of memory for "
							"dbg bcn loss\n",
							wlc->pub->unit, __FUNCTION__));
					}
				}
			}
#endif /* DBG_BCN_LOSS */

			/* No beacon for too long time. indicate the link is down */
			if (roam->time_since_bcn > roam->bcn_timeout &&
				!roam->bcns_lost && assoc->type == AS_IDLE) {
				wlc_handle_ap_lost(wlc, cfg);
			}

			if (roam->scan_block)
				roam->scan_block--;

			if (roam->ratebased_roam_block)
				roam->ratebased_roam_block--;

			if (!AS_IN_PROGRESS(wlc) && !SCAN_IN_PROGRESS(wlc->scan) &&
				roam->reassoc_param) {

				if ((wlc_reassoc(cfg, roam->reassoc_param)) != 0) {
					WL_INFORM(("%s: Delayed reassoc attempt failed\r\n",
						__FUNCTION__));
				}
			}
		}

		/* monitor IBSS link state */
		if (!cfg->BSS) {
#if defined(IBSS_PEER_MGMT)
			if (IBSS_PEER_MGMT_ENAB(wlc->pub)) {
				struct scb *scb;
				struct scb_iter scbiter;
				int32 idx;
				wlc_bsscfg_t *bsscfg;

				/* age out inactive IBSS peers */
				FOREACH_IBSS(wlc, idx, bsscfg) {
					FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
						if (!SCB_IS_IBSS_PEER(scb) ||
						    ((wlc->pub->now - scb->used) <=
						     SCB_ACTIVITY_TIME))
							continue;

						SCB_UNSET_IBSS_PEER(scb);
						wlc_scb_disassoc_cleanup(wlc, scb);
						wlc_disassoc_complete(cfg, WLC_E_STATUS_SUCCESS,
						                      &scb->ea,
						                      DOT11_RC_INACTIVITY,
						                      DOT11_BSSTYPE_INDEPENDENT);
					}
				}
			}
#endif /* defined(IBSS_PEER_MGMT) */

			/* if no beacon for too long time, indicate the link is down */
			if (roam->time_since_bcn > roam->bcn_timeout &&
			    !roam->bcns_lost) {
#ifdef WLAMPDU
				/* detect link up/down for IBSS so that ba sm can be cleaned up */
				scb_ampdu_cleanup_all(wlc, cfg);
#endif
				wlc_bss_mac_event(wlc, cfg, WLC_E_BEACON_RX, NULL,
				                  WLC_E_STATUS_FAIL, 0, 0, 0, 0);
				roam->bcns_lost = TRUE;
			}
		}

		/* Increment time since last bcn if we are associated (Infra or IBSS)
		 * Avoid wrapping by maxing count to bcn_timeout + 1
		 * Do not increment during an assoc recreate since we have not yet
		 * reestablished the connection to the AP.
		 */
		if (cxn->ign_bcn_lost_det == 0 &&
		    roam->time_since_bcn <= roam->bcn_timeout &&
		    !(ASSOC_RECREATE_ENAB(wlc->pub) && assoc->type == AS_RECREATE) &&
		    (FALSE ||
#ifdef BCMQT_CPU
		     TRUE ||
#endif
		     !ISSIM_ENAB(wlc->pub->sih))) {
#ifdef BCMDBG
			if (roam->time_since_bcn > 0) {
#ifdef WLMCNX
				if (MCNX_ENAB(wlc->pub)) {
					WL_ASSOC(("wl%d.%d: ROAM: time_since_bcn %d\n",
					          wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
					          roam->time_since_bcn));
				}
				else
#endif
				if (cfg == wlc->cfg) {
#ifdef WLCNT
					WL_ASSOC(("wl%d.%d: "
					          "ROAM: time_since_bcn %d, tbtt %u\n",
					          wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
					          roam->time_since_bcn, wlc->pub->_cnt->tbtt));
#endif /* WLCNT */
					WL_ASSOC(("wl%d.%d: "
					          "TSF: 0x%08x 0x%08x CFPSTART: 0x%08x "
					          "CFPREP: 0x%08x\n",
					          wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
					          R_REG(wlc->osh, &wlc->regs->tsf_timerhigh),
					          R_REG(wlc->osh, &wlc->regs->tsf_timerlow),
					          R_REG(wlc->osh, &wlc->regs->tsf_cfpstart),
					          R_REG(wlc->osh, &wlc->regs->tsf_cfprep)));
				}
			}
#endif	/* BCMDBG */

			roam->time_since_bcn++;
		}

		if (roam->thrash_block_active) {
			bool still_blocked = FALSE;
			int j;
			for (j = 0; j < (int) roam->cache_numentries; j++) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
				char eabuf[ETHER_ADDR_STR_LEN];
#endif
				if (roam->cached_ap[j].time_left_to_next_assoc == 0)
					continue;
				roam->cached_ap[j].time_left_to_next_assoc--;
				if (roam->cached_ap[j].time_left_to_next_assoc == 0) {
					WL_ASSOC(("wl%d: %s: ROAM: AP with BSSID %s on "
						  "chanspec 0x%x available for "
						  "reassociation\n", wlc->pub->unit,
						  __FUNCTION__,
						  bcm_ether_ntoa(&roam->cached_ap[j].BSSID, eabuf),
						  roam->cached_ap[j].chanspec));
				} else {
					WL_ASSOC(("wl%d: %s: ROAM: AP with BSSID %s on "
						  "chanspec 0x%x blocked for %d seconds\n",
						  wlc->pub->unit, __FUNCTION__,
						  bcm_ether_ntoa(&roam->cached_ap[j].BSSID, eabuf),
						  roam->cached_ap[j].chanspec,
						  roam->cached_ap[j].time_left_to_next_assoc));
					still_blocked = TRUE;
					break;
				}
			}
			if (!still_blocked)
				roam->thrash_block_active = FALSE;
		}
	}
}

#if defined(AP_KEEP_ALIVE)
void
wlc_ap_keep_alive_count_default(wlc_info_t *wlc)
{
	wlc->keep_alive_time = AP_KEEP_ALIVE_INTERVAL;
	wlc->keep_alive_count = wlc->keep_alive_time;
}

void
wlc_ap_keep_alive_count_update(wlc_info_t *wlc, uint16 keep_alive_time)
{
	wlc->keep_alive_time = keep_alive_time;
	wlc->keep_alive_count = MIN(wlc->keep_alive_time, wlc->keep_alive_count);
}

static void
wlc_ap_keep_alive(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg = wlc->cfg;

		if (wlc->keep_alive_count) {
			wlc->keep_alive_count--;
		}
		if (wlc->keep_alive_count == 0) {
			/* set priority '-1' : null data frame (without QoS) */
			if (!(wlc_sendnulldata(wlc, cfg, &cfg->BSSID, 0, 0, -1,
				NULL, NULL)))
				WL_INFORM(("wl%d: wlc_ccx_keep_alive: keep-alive pkt failed\n",
					wlc->pub->unit));
			/* reload count */
			wlc->keep_alive_count = wlc->keep_alive_time;
		}
}
#endif /* defined(AP_KEEP_ALIVE) || defined(CCX) */

#if defined(WL_PWRSTATS)
void
wlc_connect_time_upd(wlc_info_t *wlc)
{
#ifdef WL_EXCESS_PMWAKE
	uint32 connect_time;
	connect_time = wlc_pwrstats_connect_time_upd(wlc->pwrstats);
	if (connect_time)
		wlc_epm_roam_time_upd(wlc, connect_time);
#endif /* WL_EXCESS_PMWAKE */
}
#endif /* WL_PWRSTATS */
#endif /* STA */

#ifdef WLOFFLD

static void wlc_collect_rates(wlc_info_t *wlc)
{
	wl_cnt_t *cnt = wlc->pub->_cnt;
	wlc_rate_measurement_t *rates = wlc->_wlc_rate_measurement;

	if (cnt->txbyte > rates->txbytes_prev) {
		rates->txbytes_rate = cnt->txbyte - rates->txbytes_prev;
	} else {
		rates->txbytes_rate = 0;
	}
	rates->txbytes_prev = cnt->txbyte;

	if (cnt->rxbyte > rates->rxbytes_prev) {
		rates->rxbytes_rate = cnt->rxbyte - rates->rxbytes_prev;
	} else {
		rates->rxbytes_rate = 0;
	}
	rates->rxbytes_prev = cnt->rxbyte;

	if (cnt->rxframe > rates->rxframes_prev) {
		rates->rxframes_rate = cnt->rxbyte - rates->rxframes_prev;
	} else {
		rates->rxframes_rate = 0;
	}
	rates->rxframes_prev = cnt->rxbyte;

	if (cnt->txframe > rates->txframes_prev) {
		rates->txframes_rate = cnt->txbyte - rates->txframes_prev;
	} else {
		rates->txframes_rate = 0;
	}
	rates->txframes_prev = cnt->txbyte;
}

#endif /* WLOFFLD */

/** common watchdog code */
void
wlc_watchdog(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	int i;
#ifdef STA
	wlc_bsscfg_t *cfg;
#endif /* STA */
#if defined(STA) && defined(AP_KEEP_ALIVE)
	bool ap_keep_alive = FALSE;
#endif

	WL_TRACE(("wl%d: wlc_watchdog\n", wlc->pub->unit));

	wlc->wd_waitfor_bcn = FALSE;

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

#if defined(WLTEST)
	if (wlc->watchdog_disable)
		return;
#endif

	WLDURATION_ENTER(wlc, DUR_WATCHDOG);

#ifdef WAR4360_UCODE
	if (wlc->hw->need_reinit) {
		wlc->pub->_cnt->reinitreason[REINITREASONIDX(wlc->hw->need_reinit)]++;
		WL_ERROR(("wl%d: %s: need to reinit() %d, Big Hammer\n",
			wlc->pub->unit, __FUNCTION__, wlc->hw->need_reinit));
		wlc->hw->need_reinit = 0;
		wlc_fatal_error(wlc);	/* big hammer */
	}

	if (R_REG(wlc->osh, &wlc->hw->regs->psm_pc_reg_0) & 0x40) {
		WL_ERROR(("wl%d:%s: ucode stack error, Big Hammer\n",
			wlc->pub->unit, __FUNCTION__));
		wlc_fatal_error(wlc);	/* big hammer */
	}
#endif /* WAR4360_UCODE */
#ifndef WLC_DISABLE_ACI
	wlc_weakest_link_rssi_chan_stats_upd(wlc);
#endif

#ifdef WLOFFLD
	wlc_collect_rates(wlc);
#endif /* WLOFFLD */

#ifdef WLC_LOW
	if (wlc->blocked_for_slowcal &&
	    !(R_REG(wlc->osh, &wlc->hw->regs->maccommand) & MCMD_SLOWCAL)) {
		wlc_suspend_mac_and_wait(wlc);
		wlc->blocked_for_slowcal = FALSE;
		wlc_phy_block_bbpll_change(WLC_PI(wlc), FALSE, FALSE);
		wlc_enable_mac(wlc);
	}
#endif /* WLC_LOW */

	/* increment second count */
	wlc->pub->now++;

	/* Get Noise Est from Phy */
	wlc_lq_noise_update_ma(wlc,
		wlc_phy_noise_avg((wlc_phy_t *)WLC_PI(wlc)));

#ifdef ROUTER_TINY
	if (pktq_full(&wlc->active_queue->q)) {
		WL_ERROR(("wl%d: %s: max txq full. HAMMERING!\n",
			wlc->pub->unit, __FUNCTION__));
		wlc_fatal_error(wlc);
		goto exit;
	}
#endif
#ifdef DNGL_WD_KEEP_ALIVE
	if (wlc->dngl_wd_keep_alive) {
		wlc_bmac_dngl_wd_keep_alive(wlc->hw, TIMER_INTERVAL_DNGL_WATCHDOG);
	}
#endif

#if defined(PKTC) || defined(PKTC_DONGLE)
	if (PKTC_ENAB(wlc->pub)) {
		struct scb *scb;
		struct scb_iter scbiter;
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			scb->pktc_pps = 0;
		}
	}
#endif

#ifdef STA
	/* delay radio disable */
	if (wlc->mpc && wlc_is_non_delay_mpc(wlc)) {
		if (wlc->mpc_delay_off)
			wlc->mpc_delay_off--;
	}

	WL_MPC(("wlc_watchdog: wlc->mpc_delay_off %d\n", wlc->mpc_delay_off));

	/* mpc sync */
	wlc_radio_mpc_upd(wlc);
#endif /* STA */

	/* radio sync: sw/hw/mpc --> radio_disable/radio_enable */
	wlc_radio_hwdisable_upd(wlc);
	wlc_radio_upd(wlc);
#if defined(STA)
	/* if ismpc, driver should be in down state if up/down is allowed */
	if (wlc->mpc && wlc_ismpc(wlc))
		ASSERT(!wlc->pub->up);
#endif 
	/* if radio is disable, driver may be down, quit here */
	if (wlc->pub->radio_disabled)
		goto exit;
#if defined(STA)
#ifdef WL_EXCESS_PMWAKE
	if (!AP_ACTIVE(wlc) && (wlc->excess_pm_period || wlc->excess_pmwake->ca_thresh) &&
	    ((wlc->cfg->pm->PM && wlc->stas_associated) ||
	     0))
		wlc_check_excess_pm_awake(wlc);
#endif /* WL_EXCESS_PMWAKE */

#ifdef DEBUG_TBTT
	{
	uint32 tsf_l, tsf_h;

	/* read the tsf from our chip */
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);

	WL_ASSOC(("wl%d: TBTT: wlc_watchdog: tsf time: %08x:%08x\n", WLCWLUNIT(wlc), tsf_h, tsf_l));
	}
#endif /* DEBUG_TBTT */
#endif /* STA */

#ifdef WLC_LOW
#if defined(BCMECICOEX)
	/*
	* These BTCX parameters need to be read out from SHM before mac suspend
	* (which might be called in one or more of the watchdog routines). This is because
	* resuming after a mac suspend causes some of the SHM parameters to be reset
	*/
	wlc_btcx_read_btc_params(wlc);
#endif /* defined(BCMECICOEX) */
	wlc_bmac_watchdog(wlc);
#endif /* WLC_LOW */
#ifdef WLC_HIGH_ONLY
	/* maintenance */
	wlc_bmac_rpc_watchdog(wlc);
#endif

#if defined(STA) && defined(AP)
	/* start AP if operation were pending on SCAN_IN_PROGRESS() or WLC_RM_IN_PROGRESS() */
	/* Find AP's that are enabled but not up to restart */
	if (AP_ENAB(wlc->pub) && APSTA_ENAB(wlc->pub) && wlc_apup_allowed(wlc)) {
		bool startap = FALSE;

		FOREACH_AP(wlc, i, cfg) {
			/* find the first ap that is enabled but not up */
			if (cfg->enable && !cfg->up) {
				startap = TRUE;
				break;
			}
		}

		if (startap) {
			WL_APSTA_UPDN(("wl%d: wlc_watchdog -> restart downed ap\n",
			       wlc->pub->unit));
			wlc_restart_ap(wlc->ap);
		}
	}
#endif /* STA && AP */

#if defined(DELTASTATS)
	/* check if delta stats enabled */
	if (wlc->delta_stats->interval != 0) {

		/* update mac stats counters at every watchdog */
		wlc_statsupd(wlc);

		/* check if interval has elapsed */
		if (wlc->delta_stats->seconds % wlc->delta_stats->interval == 0) {
			wlc_delta_stats_update(wlc);
		}

		/* seconds is zeroed only when delta stats is enabled so
		 * that it can be used to determine when delta stats are valid
		 */
		wlc->delta_stats->seconds++;
	} else
#endif /* DELTASTATS */
	if ((WLC_UPDATE_STATS(wlc)) && (!(wlc->pub->now % SW_TIMER_MAC_STAT_UPD))) {
		/* occasionally sample mac stat counters to detect 16-bit counter wrap */
		wlc_statsupd(wlc);
	}

	wlc_lq_chanim_update(wlc, wlc->chanspec, CHANIM_WD);

#ifdef STA
	/* If we had increased the frequency tracking b/w to track beacons,
	 * verify that it is working
	 */
	if (wlc->stas_associated > 0) {
		if ((wlc->freqtrack_attempts > 0) &&
		    (wlc->freqtrack_attempts <= WLC_FREQTRACK_MIN_ATTEMPTS))
			wlc_freqtrack_verify(wlc);
	}

	/* Ensure that the PM state is in sync. between the STA and AP */
	FOREACH_AS_STA(wlc, i, cfg) {
		if (cfg->pm->PMpending && PS_ALLOWED(cfg)) {
			if (!cfg->BSS ||
			    !cfg->pm->PMpending ||
			    !WLC_BSS_CONNECTED(cfg))
				continue;
			WL_RTDC(wlc, "wlc_watchdog: tx PMep=%02u AW=%02u",
			        (cfg->pm->PMenabled ? 10 : 0) | cfg->pm->PMpending,
			        (PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
			WL_RTDC(wlc, "            : PMb=0x%x", cfg->pm->PMblocked, 0);
			/* send another NULL data frame to communicate PM state */
			if (!(wlc_sendnulldata(wlc, cfg, &cfg->BSSID, 0, 0, -1,
				NULL, NULL)))
				WL_ERROR(("wl%d: %s: PM state pkt failed\n",
				          wlc->pub->unit, __FUNCTION__));
		}
	}

#if defined(WLAIBSS)
	if (AIBSS_ENAB(wlc->pub)) {
		/* check beacon timeout is reached to send fail event */
		FOREACH_AS_STA(wlc, i, cfg) {
			if (BSSCFG_IBSS(cfg)) {
				struct scb_iter scbiter;
				struct scb *scb;
				FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
					aibss_tx_monitor_t	*aibss_scb_tx_stats =
						SCB_CUBBY((scb),
						WLC_AIBSS_INFO_SCB_HDL(wlc->aibss_info));
					/*
					 * Current TX FAIL CONDITION if there is no BEACON
					 * for a N seconds
					 */
					 if (aibss_scb_tx_stats->bcn_count == 0) {
						aibss_scb_tx_stats->no_bcn_counter++;
						wlc_aibss_check_txfail(wlc->aibss_info, cfg, scb);
					 }
					 else  {
						aibss_scb_tx_stats->no_bcn_counter = 0;
						aibss_scb_tx_stats->bcn_count = 0;
					 }
				}
			}
		}
	}
#endif /* WLAIBSS */

	wlc_link_monitor_watchdog(wlc);

	/* periodic IBSS gmode rate probing */
	if (wlc->band->gmode && (wlc->pub->now % SW_TIMER_IBSS_GMODE_RATEPROBE) == 0) {
		FOREACH_AS_STA(wlc, i, cfg) {
			if (cfg->BSS)
				continue;
			wlc_rateprobe_scan(cfg);
		}
	}

#if defined(AP_KEEP_ALIVE)
#ifdef AP_KEEP_ALIVE
	ap_keep_alive = TRUE;
#endif /* AP_KEEP_ALIVE */
	if (ap_keep_alive) {
		cfg = wlc->cfg;
		if (BSSCFG_STA(cfg) && cfg->BSS) {
			if (cfg->associated && !ETHER_ISNULLADDR(&cfg->BSSID)) {
				wlc_ap_keep_alive(wlc);
			}
		}
	}

#endif 

	if (wlc->reset_triggered_pmoff) {
		FOREACH_AS_STA(wlc, i, cfg) {
			if (cfg->up) {
				/* resync pm mode */
				wlc_set_pm_mode(wlc, cfg->pm->PM, cfg);
				WL_PS(("wl%d.%d: resync PM mode to %d\n", wlc->pub->unit,
				       WLC_BSSCFG_IDX(cfg), cfg->pm->PM != PM_OFF));
			}
		}
	}
#endif /* STA */

	if (wlc->pm_dur_clear_timeout)
		wlc->pm_dur_clear_timeout--;
	else
		wlc_get_accum_pmdur(wlc);

	/* Call any registered watchdog handlers */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (wlc->modulecb[i].watchdog_fn == NULL)
			continue;
		WL_TRACE(("wl%d: %s: calling WATCHDOG callback for \"%s\"\n",
			wlc->pub->unit, __FUNCTION__, wlc->modulecb[i].name));
		wlc->modulecb[i].watchdog_fn(wlc->modulecb_data[i].hdl);
	}

#ifdef WL11N
#if defined(WLC_LOW) && defined(WLC_HIGH)
	wlc_stf_pwrthrottle_upd(wlc);
#endif
#endif /* WL11N */

#ifdef STA
	/* clear reset variable */
	wlc->reset_triggered_pmoff = FALSE;
#endif /* STA */

#ifdef WL11N
	wlc_stf_tempsense_upd(wlc);
#endif /* WL11N */

#ifdef WLC_LOW
	/* BMAC_NOTE: for HIGH_ONLY driver, this seems being called after RPC bus failed */
	ASSERT(wlc_bmac_taclear(wlc->hw, TRUE));
#endif

#if !defined(NEW_TXQ)
	/* Verify that tx_prec_map and fifos are in sync to avoid lock ups */
	ASSERT(wlc_tx_prec_map_verify(wlc));
#else
	/* Verify that txq stopped conditions and pkt counts are in sync to avoid lock ups */
	ASSERT(wlc_txq_fc_verify(wlc->txqi, wlc->active_queue->low_txq));
#endif /* !NEW_TXQ */

	ASSERT(wlc_ps_check(wlc));


	if ((wlc->cfg) && (wlc->cfg->pm) && (wlc->cfg->pm->PM == PM_OFF) && (wlc->cfg->associated))
		wl_indicate_maccore_state(wlc->wl, LTR_ACTIVE);

	wlc_scb_restrict_wd(wlc);

exit:
	WLDURATION_EXIT(wlc, DUR_WATCHDOG);
	return;
}

/** make interface operational */
int
BCMINITFN(wlc_up)(wlc_info_t *wlc)
{
	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

	/* HW is turned off so don't try to access it */
	if (wlc->pub->hw_off || DEVICEREMOVED(wlc))
		return BCME_RADIOOFF;

	if (!wlc->pub->hw_up) {
#ifdef WLLED
		/* Do basic GPIO initializations for LED. This just sets up LED mask */
		wlc_led_init(wlc->ledh);
#endif
		wlc_bmac_hw_up(wlc->hw);
		wlc->pub->hw_up = TRUE;

		/* Initialize LTR and set latency to idle */
#ifdef WL_LTR
		if (LTR_ENAB(wlc->pub)) {
			wlc_ltr_up_prep(wlc, LTR_SLEEP);
		}
#endif /* WL_LTR */
	}

	wlc_bmac_4331_epa_init(wlc->hw);

	wlc_btc_4313_gpioctrl_init(wlc);

	{
		/* disable/enable RSSI power down feature */
		uint16 flags = wlc->stf->rssi_pwrdn_disable ? MHF5_HTPHY_RSSI_PWRDN : 0;
		wlc_mhf(wlc, MHF5, MHF5_HTPHY_RSSI_PWRDN, flags, WLC_BAND_ALL);
	}

	/*
	 * Need to read the hwradio status here to cover the case where the system
	 * is loaded with the hw radio disabled. We do not want to bring the driver up in this case.
	 * if radio is disabled, abort up, lower power, start radio timer and return 0(for NDIS)
	 * don't call radio_update to avoid looping wlc_up.
	 *
	 * wlc_bmac_up_prep() returns either 0 or BCME_RADIOOFF only
	 */
	if (!wlc->pub->radio_disabled) {
		int status = wlc_bmac_up_prep(wlc->hw);
		if (status == BCME_RADIOOFF) {
			if (!mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE)) {
				int idx;
				wlc_bsscfg_t *bsscfg;
#ifdef WLEXTLOG
				WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_HW_OFF_ID,
					WL_LOG_LEVEL_ERR, 0, 0, NULL);
#endif
				mboolset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE);

				FOREACH_BSS(wlc, idx, bsscfg) {
					/* all bsscfg including AP's for Win7 */
					if ((!WLEXTSTA_ENAB(wlc->pub) && !BSSCFG_STA(bsscfg)) ||
					    !bsscfg->enable || !bsscfg->BSS)
						continue;
					WL_ERROR(("wl%d.%d: wlc_up: rfdisable -> "
						"wlc_bsscfg_disable()\n",
						wlc->pub->unit, idx));
					wlc_bsscfg_disable(wlc, bsscfg);
				}
			}
		} else
			ASSERT(!status);
	}

#if defined(SAVERESTORE) && defined(MACOSX)
	if (wlc_bmac_radio_read_hwdisabled(wlc->hw)) {
#else
	if (wlc->pub->radio_disabled) {
#endif
		wlc_radio_monitor_start(wlc);
		return 0;
	}

	wlc_bmac_set_ctrl_ePA(wlc->hw);
	if (wlc->hw->btswitch_ovrd_state == AUTO)
		wlc_bmac_set_ctrl_bt_shd0(wlc->hw, TRUE);
	else
		wlc_bmac_set_btswitch(wlc->hw, wlc->hw->btswitch_ovrd_state);

	/* wlc_bmac_up_prep has done wlc_corereset(). so clk is on, set it */
	wlc->clk = TRUE;

	wlc_radio_monitor_stop(wlc);

	/* Set EDCF hostflags */
	if (EDCF_ENAB(wlc->pub)) {
		wlc_mhf(wlc, MHF1, MHF1_EDCF, MHF1_EDCF, WLC_BAND_ALL);
#if defined(WME_PER_AC_TX_PARAMS)
		{
			int ac;
			bool enab = FALSE;

			for (ac = 0; ac < AC_COUNT; ac++) {
				if (wlc->wme_max_rate[ac] != 0) {
					enab = TRUE;
					break;
				}
			}
			WL_RATE(("Setting per ac maxrate to %d \n", enab));
			wlc->pub->_per_ac_maxrate = enab;
		}
#endif /* WME_PER_AC_TX_PARAMS */
	} else {
		wlc_mhf(wlc, MHF1, MHF1_EDCF, 0, WLC_BAND_ALL);
	}

	if (WLC_WAR16165(wlc))
		wlc_mhf(wlc, MHF2, MHF2_PCISLOWCLKWAR, MHF2_PCISLOWCLKWAR, WLC_BAND_ALL);

#if defined(BCMDBG) && defined(WLC_LOW) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
	if (wlc_info_time_dbg == NULL) {
	    wlc_info_time_dbg = wlc;
	}
#endif /* BCMDBG && WLC_LOW && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

	wl_init(wlc->wl);
	wlc->pub->up = TRUE;

#if defined(AP) && defined(RADAR)
	wlc_phy_first_radar_indicator(wlc->band->pi);
#endif
	if (wlc->bandinit_pending) {
		wlc_suspend_mac_and_wait(wlc);
		wlc_set_chanspec(wlc, wlc->default_bss->chanspec);
		wlc->bandinit_pending = FALSE;
		wlc_enable_mac(wlc);
	}

	/* Ser the default mode of probe req reception to the host */
	wlc_enable_probe_req(wlc, PROBE_REQ_EVT_MASK, wlc->prb_req_enable_mask);
	wlc_config_ucode_probe_resp(wlc);

	/* Propagate rfaware_lifetime setting to ucode */
	wlc_rfaware_lifetime_set(wlc, wlc->rfaware_lifetime);

	wlc_bmac_up_finish(wlc->hw);

	/* other software states up after ISR is running */
	/* start APs that were to be brought up but are not up  yet */
	if (AP_ENAB(wlc->pub))
		wlc_restart_ap(wlc->ap);

#ifdef STA
	if (ASSOC_RECREATE_ENAB(wlc->pub)) {
		int idx;
		wlc_bsscfg_t *cfg;
		FOREACH_BSS(wlc, idx, cfg) {
			if (BSSCFG_STA(cfg) && cfg->enable && (cfg->flags & WLC_BSSCFG_PRESERVE)) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
				char ssidbuf[SSID_FMT_BUF_LEN];
				wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
#endif
				WL_ASSOC(("wl%d: wlc_up: restarting STA bsscfg 0 \"%s\"\n",
				          wlc->pub->unit, ssidbuf));
				wlc_join_recreate(wlc, cfg);
			}
		}
	}
#endif /* STA */

	/* Program the TX wme params with the current settings */
	wlc_wme_retries_write(wlc);

	/* sanitize any existing scb rates */
	wlc_scblist_validaterates(wlc);

#ifndef ATE_BUILD
	/* start one second watchdog timer */
	ASSERT(!wlc->WDarmed);
	wl_add_timer(wlc->wl, wlc->wdtimer, TIMER_INTERVAL_WATCHDOG, TRUE);
	wlc->WDarmed = TRUE;
#endif

	/* ensure txc is up to date */
	if (WLC_TXC_ENAB(wlc))
		wlc_txc_upd(wlc->txc);

	/* ensure antenna config is up to date */
	wlc_stf_phy_txant_upd(wlc);

#ifdef WL11AC
	/* Updated duty cycle for band 40 and 80 */
	wlc_stf_chanspec_upd(wlc);
#endif /* WL11AC */

	/* ensure the brcm ie is up to date */
	wlc_update_brcm_ie(wlc);
	WL_APSTA_BCN(("wl%d: wlc_up() -> wlc_update_beacon()\n", wlc->pub->unit));
	wlc_update_beacon(wlc);
	wlc_update_probe_resp(wlc, TRUE);

#ifdef WLLED
	/* start led timer module */
	wlc_led_up(wlc);
	wlc_led_event(wlc->ledh);
#endif

	/* toe enabled ? */
	wlc->toe_bypass = TRUE;
#ifdef WLTOEHW
	if (wlc->toe_capable) {
		uint32 toe_ctl;

		toe_ctl = R_REG(wlc->osh, &wlc->regs->u.d11acregs.ToECTL);

#if defined(WLCSO) || defined(WLAMPDU)
		/* WLAMPDU needs TOE header for the Epoch bit. Header cache also need it (?) */
		/* TOE engine needs to be enabled for CSO, TSO, or AMPDU for d11 rev40 */
		if ((toe_ctl & 1) != 0) {
			toe_ctl &= 0xFFFE;
			W_REG(wlc->osh, &wlc->regs->u.d11acregs.ToECTL, toe_ctl);
		}
#else
		/* TOE engine can be disabled if not needed */
		if ((toe_ctl & 1) == 0) {
			toe_ctl |= 1;
			W_REG(wlc->osh, &wlc->regs->u.d11acregs.ToECTL, toe_ctl);
		}
#endif /* (WLCSO || WLAMPDU) */

		if (toe_ctl & 1)
			wlc->toe_bypass = TRUE;
		else
			wlc->toe_bypass = FALSE;
	}
#endif /* WLTOEHW */

#ifdef STA
	/* BMAC_NOTE: Ignore freqtrack support in LOW driver since it's for GPHY only */
	/* If the user wants wideband freq tracking to be always ON, set the PHY regs */
	if ((wlc->freqtrack_override == FREQTRACK_ON) && WLCISGPHY(wlc->band))
		wlc_phy_freqtrack_start(WLC_PI(wlc));
#endif /* STA */

#ifdef WLC_LOW
	if ((CHIPID(wlc->pub->sih->chip) == BCM4345_CHIP_ID) ||
		(CHIPID(wlc->pub->sih->chip) == BCM4335_CHIP_ID)) {
		/* Initiate the slow calibration */
		OR_REG(wlc->osh, &wlc->regs->maccommand, MCMD_SLOWCAL);
		/* disable scan till the slow cal is over. */
		wlc_phy_block_bbpll_change(WLC_PI(wlc), TRUE, FALSE);
		wlc->blocked_for_slowcal = TRUE;
	}
#endif /* WLC_LOW */

	/* sync up btc mode between high and low driver */
	wlc_btc_mode_sync(wlc);

	/* wl_init() or other wlc_up() code may have set latency to active,
	 * set it back to idle
	 */
#ifdef WL_LTR
	if (LTR_ENAB(wlc->pub)) {
		wlc_ltr_hwset(wlc->hw, wlc->regs, LTR_SLEEP);
	}
#endif /* WL_LTR */
#ifdef BCMPCIEDEV_ENABLED
	wlc_bmac_enable_rx_hostmem_access(wlc->hw, wlc->cmn->hostmem_access_enabled);
#endif
	WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_DRIVER_UP_ID, WL_LOG_LEVEL_INFO, 0, 0, NULL);

	if (D11REV_GE(wlc->pub->corerev, 40))
		wlc_write_shm(wlc, M_BCMC_TIMEOUT, (uint16)wlc->pm_bcmc_wait);

	WL_MPC(("wl%d: UP\n", wlc->pub->unit));
	return (0);
}

/** Initialize the base precedence map for dequeueing from txq based on WME settings */
static void
BCMINITFN(wlc_tx_prec_map_init)(wlc_info_t *wlc)
{
	wlc->tx_prec_map = WLC_PREC_BMP_ALL;
	bzero(wlc->fifo2prec_map, sizeof(uint16) * NFIFO);

	/* For non-WME, both fifos have overlapping MAXPRIO. So just disable all precedences
	 * if either is full.
	 */
	if (!EDCF_ENAB(wlc->pub)) {
		wlc->fifo2prec_map[TX_DATA_FIFO] = WLC_PREC_BMP_ALL;
		wlc->fifo2prec_map[TX_CTL_FIFO] = WLC_PREC_BMP_ALL;
	} else {
		wlc->fifo2prec_map[TX_AC_BK_FIFO] = WLC_PREC_BMP_AC_BK;
		wlc->fifo2prec_map[TX_AC_BE_FIFO] = WLC_PREC_BMP_AC_BE;
		wlc->fifo2prec_map[TX_AC_VI_FIFO] = WLC_PREC_BMP_AC_VI;
		wlc->fifo2prec_map[TX_AC_VO_FIFO] = WLC_PREC_BMP_AC_VO;
	}
}

static uint
BCMUNINITFN(wlc_down_del_timer)(wlc_info_t *wlc)
{
	uint callbacks = 0;

#ifdef STA
	if (!wl_del_timer(wlc->wl, wlc->iscan_timer))
		callbacks++;

	/* cancel any radio_shutoff_dly timer */
	if (!wl_del_timer(wlc->wl, wlc->pm2_radio_shutoff_dly_timer))
		callbacks ++;

#endif /* STA */
#ifdef PSPRETEND
	if (wlc->pps_info && !wl_del_timer(wlc->wl, wlc->pps_info->ps_pretend_probe_timer)) {
		callbacks++;
	}
#endif

	return callbacks;
}


/**
 * Mark the interface nonoperational, stop the software mechanisms,
 * disable the hardware, free any transient buffer state.
 * Return a count of the number of driver callbacks still pending.
 */
uint
BCMUNINITFN(wlc_down)(wlc_info_t *wlc)
{

	uint callbacks = 0;
	int i;
	bool dev_gone = FALSE;
	wlc_bsscfg_t *bsscfg;
	wlc_txq_info_t *qi;

	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

	/* check if we are already in the going down path */
	if (wlc->going_down) {
		WL_ERROR(("wl%d: %s: Driver going down so return\n", wlc->pub->unit, __FUNCTION__));
		return 0;
	}
	if (!wlc->pub->up)
		return 0;

	/* Update the pm_dur value before going down */
	wlc_get_accum_pmdur(wlc);

#if defined(WLTDLS)
	if (TDLS_SUPPORT(wlc->pub) && TDLS_ENAB(wlc->pub)) {
		wlc_tdls_down(wlc->tdls);
		OSL_DELAY(1000);
	}
#endif /* defined(WLTDLS) */

	/* in between, mpc could try to bring down again.. */
	wlc->going_down = TRUE;

#ifdef WLC_LOW
	if (CHIPID(wlc->pub->sih->chip) == BCM4335_CHIP_ID) {
		wlc_phy_block_bbpll_change(WLC_PI(wlc), FALSE, TRUE);
	}
#endif

#ifdef DNGL_WD_KEEP_ALIVE
	/* disable dngl watchdog timer while going down */
	wlc_bmac_dngl_wd_keep_alive(wlc->hw, 0);
#endif

	callbacks += wlc_bmac_down_prep(wlc->hw);

	dev_gone = DEVICEREMOVED(wlc);
	if (!dev_gone) {
		/* abort any scan in progress */
		if (SCAN_IN_PROGRESS(wlc->scan))
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);

		FOREACH_BSS(wlc, i, bsscfg) {
			/* down any AP BSSs, but leave them enabled */
			if (BSSCFG_AP(bsscfg)) {
				if (bsscfg->up) {
					WL_APSTA_UPDN(("wl%d: wlc_down -> wlc_bsscfg_down %p "
					               "(%d)\n", wlc->pub->unit, bsscfg, i));

					callbacks += wlc_bsscfg_down(wlc, bsscfg);
				}
				continue;
			}

#ifdef STA
			/* Perform STA down operations if needed */

			/* abort any association in progress */
			callbacks += wlc_assoc_abort(bsscfg);

			/* if config is enabled, take care of deactivating */
			if (!bsscfg->enable)
				continue;

			/* For WOWL or Assoc Recreate, don't disassociate,
			 * just down the bsscfg.
			 * Otherwise, disable the config (STA requires active restart)
			 */
			/* WOWL_ACTIVE does not mean association needs to be recreated!
			 * ASSOC_RECREATE_ENAB is meant for that
			 */
			if (
#ifdef WOWL
			    WOWL_ACTIVE(wlc->pub) ||
#endif
			    (ASSOC_RECREATE_ENAB(wlc->pub) &&
			     (bsscfg->flags & WLC_BSSCFG_PRESERVE))) {
				WL_APSTA_UPDN(("wl%d: %s: wlc_bsscfg_down(STA)\n",
				               wlc->pub->unit, __FUNCTION__));

				callbacks += wlc_bsscfg_down(wlc, bsscfg);

				/* reset the quiet channels vector to the union
				 * of the restricted and radar channel sets
				 */
				wlc_quiet_channels_reset(wlc->cmi);

				if (WLEXTSTA_ENAB(wlc->pub) &&
				    !WOWL_ACTIVE(wlc->pub)) {
					struct scb *scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
					if (scb) {
						wlc_scb_disassoc_cleanup(wlc, scb);
						if (SCB_ASSOCIATED(scb)) {
							wlc_scb_clearstatebit(scb, ASSOCIATED);
							wlc_bss_mac_event(wlc, bsscfg,
								WLC_E_DISASSOC,
								&bsscfg->BSSID,
								WLC_E_STATUS_SUCCESS,
								DOT11_RC_DISASSOC_LEAVING,
								WLC_DOT11_BSSTYPE(bsscfg->BSS),
								0, 0);
						}
						bsscfg->ar_disassoc = TRUE;
					}
				}
			} else {
				WL_APSTA_UPDN(("wl%d: %s: wlc_bsscfg_disable(STA)\n",
				               wlc->pub->unit, __FUNCTION__));

				/* delete all keys in bsscfg if it is still there */
				wlc_keymgmt_reset(wlc->keymgmt, bsscfg, NULL);

				callbacks += wlc_bsscfg_disable(wlc, bsscfg);

				/* allow time for disassociation packet to
				 * notify associated AP of our departure
				 */
				OSL_DELAY(4 * 1000);
			}
#endif /* STA */
		}
	}

	/* Call any registered down handlers */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (wlc->modulecb[i].down_fn == NULL)
			continue;
		WL_INFORM(("wl%d: %s: calling DOWN callback for \"%s\"\n",
		           wlc->pub->unit, __FUNCTION__, wlc->modulecb[i].name));
		callbacks += wlc->modulecb[i].down_fn(wlc->modulecb_data[i].hdl);
	}

	/* cancel the watchdog timer */
	if (wlc->WDarmed) {
		if (!wl_del_timer(wlc->wl, wlc->wdtimer))
			callbacks++;
		wlc->WDarmed = FALSE;
	}
	/* cancel all other timers */
	callbacks += wlc_down_del_timer(wlc);

#ifdef WLC_LOW
	/* interrupt must have been blocked */
	ASSERT((wlc_hw_get_macintmask(wlc->hw) == 0) || !wlc->pub->up);
#endif

	wlc->pub->up = FALSE;

	wlc_phy_mute_upd(WLC_PI(wlc), FALSE, PHY_MUTE_ALL);

#ifdef WLRXOV
	if (WLRXOV_ENAB(wlc->pub) && wlc->rxov_timer) {
		if (!wl_del_timer(wlc->wl, wlc->rxov_timer))
			callbacks++;
	}
#endif

	wlc_monitor_down(wlc);

#ifdef WLLED
	/* this has to be done after state change above for LEDs to turn off properly */
	callbacks += wlc_led_down(wlc);
#endif

	/* clear txq flow control */
	wlc_txflowcontrol_reset(wlc);

	/* flush tx queues */
	for (qi = wlc->tx_queues; qi != NULL; qi = qi->next) {
		pktq_flush(wlc->osh, &qi->q, TRUE, NULL, 0);
		ASSERT(pktq_empty(&qi->q));
	}

	/* flush event queue.
	 * Should be the last thing done after all the events are generated
	 * Just delivers the events synchronously instead of waiting for a timer
	 */
	if (wlc->eventq)
		callbacks += wlc_eventq_down(wlc->eventq);

#if defined(BCMDBG) && defined(WLC_LOW) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
	if (wlc == wlc_info_time_dbg) {
		wlc_info_time_dbg = NULL;
	}
#endif /* BCMDBG && WLC_LOW && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

	/* Increase latency tolerance */
#ifdef WL_LTR
	if (LTR_ENAB(wlc->pub)) {
		wlc_ltr_hwset(wlc->hw, wlc->regs, LTR_SLEEP);
	}
#endif /* WL_LTR */

	callbacks += wlc_bmac_down_finish(wlc->hw);

	/* wlc_bmac_down_finish has done wlc_coredisable(). so clk is off */
	wlc->clk = FALSE;

#ifdef WLC_HIGH_ONLY
	wlc_rpctx_txreclaim(wlc->rpctx);
#endif

	/* Verify all packets are flushed from the driver */
	if (PKTALLOCED(wlc->osh) > 0) {
		WL_ERROR(("wl%d: %d packets not freed at wlc_down. MBSS=%d\n",
		          wlc->pub->unit, PKTALLOCED(wlc->osh), MBSS_ENAB(wlc->pub)));
		PKTLIST_DUMP(wlc->osh, NULL);

#if defined(MBSS) && defined(BCMDBG)
		FOREACH_BSS(wlc, i, bsscfg) {
			wlc_mbss_dump_spt_pkt_state(wlc, bsscfg, i);
		}
#endif /* defined(MBSS) && defined (BCMDBG) */

#ifdef DMATXRC
		if (!PHDR_ENAB(wlc))
#endif
		ASSERT(PKTALLOCED(wlc->osh) == 0);
	}

	if (SRHWVSDB_ENAB(wlc->pub)) {
		wlc_srvsdb_force_set(wlc, 0);
	}

	WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_DRIVER_DOWN_ID, WL_LOG_LEVEL_INFO, 0, 0, NULL);

	WL_MPC(("wl%d: DOWN  callbacks %d\n", wlc->pub->unit, callbacks));
	wlc->going_down = FALSE;
	return (callbacks);
} /* wlc_down */

#ifdef WLC_HIGH_ONLY
int
BCMINITFN(wlc_sleep)(wlc_info_t *wlc)
{
	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));
	return bcm_rpc_sleep(wlc->rpc);
}

int
BCMINITFN(wlc_resume)(wlc_info_t *wlc)
{
	int err, fw_reload;
	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));
	err = bcm_rpc_resume(wlc->rpc, &fw_reload);
	if (fw_reload) {
		wlc_stf_chain_init(wlc);
	}
	return err;
}
#endif	/* WLC_HIGH_ONLY */

/** Set the current gmode configuration */
int
wlc_set_gmode(wlc_info_t *wlc, uint8 gmode, bool config)
{
	int ret = 0;
	uint i;
	wlc_rateset_t rs;
	/* Default to 54g Auto */
	int8 shortslot = WLC_SHORTSLOT_AUTO; /* Advertise and use shortslot (-1/0/1 Auto/Off/On) */
	bool shortslot_restrict = FALSE; /* Restrict association to stations that support shortslot
					  */
	bool ignore_bcns = TRUE;		/* Ignore legacy beacons on the same channel */
	bool ofdm_basic = FALSE;		/* Make 6, 12, and 24 basic rates */
	int preamble = WLC_PLCP_LONG; /* Advertise and use short preambles (-1/0/1 Auto/Off/On) */
	wlcband_t* band;

	/* if N-support is enabled, allow Gmode set as long as requested
	 * Gmode is not GMODE_LEGACY_B
	 */
	if (N_ENAB(wlc->pub) && gmode == GMODE_LEGACY_B)
		return BCME_UNSUPPORTED;

#ifdef WLP2P
	/* if P2P is enabled no GMODE_LEGACY_B is allowed */
	if (P2P_ACTIVE(wlc) && gmode == GMODE_LEGACY_B)
		return BCME_UNSUPPORTED;
#endif

	/* verify that we are dealing with 2G band and grab the band pointer */
	if (wlc->band->bandtype == WLC_BAND_2G)
		band = wlc->band;
	else if ((NBANDS(wlc) > 1) &&
	         (wlc->bandstate[OTHERBANDUNIT(wlc)]->bandtype == WLC_BAND_2G))
		band = wlc->bandstate[OTHERBANDUNIT(wlc)];
	else
		return BCME_BADBAND;

	/* update configuration value */
	if (config == TRUE)
		wlc_prot_g_cfg_set(wlc->prot_g, WLC_PROT_G_USER, gmode);

	/* Clear supported rates filter */
	bzero(&wlc->sup_rates_override, sizeof(wlc_rateset_t));

	/* Clear rateset override */
	bzero(&rs, sizeof(wlc_rateset_t));

	switch (gmode) {
	case GMODE_LEGACY_B:
		shortslot = WLC_SHORTSLOT_OFF;
		wlc_rateset_copy(&gphy_legacy_rates, &rs);
		break;

	case GMODE_LRS:
		if (AP_ENAB(wlc->pub))
			wlc_rateset_copy(&cck_rates, &wlc->sup_rates_override);
		break;

	case GMODE_AUTO:
		/* Accept defaults */
		break;

	case GMODE_ONLY:
		ofdm_basic = TRUE;
		preamble = WLC_PLCP_SHORT;
		break;


	case GMODE_PERFORMANCE:
		if (AP_ENAB(wlc->pub))	/* Put all rates into the Supported Rates element */
			wlc_rateset_copy(&cck_ofdm_rates, &wlc->sup_rates_override);

		shortslot = WLC_SHORTSLOT_ON;
		shortslot_restrict = TRUE;
		ofdm_basic = TRUE;
		preamble = WLC_PLCP_SHORT;
		break;

	default:
		/* Error */
		WL_ERROR(("wl%d: %s: invalid gmode %d\n", wlc->pub->unit, __FUNCTION__, gmode));
		return BCME_UNSUPPORTED;
	}

	WL_INFORM(("wl%d: %s: 0x%x 0x%x\n", wlc->pub->unit, __FUNCTION__, gmode, config));

	/*
	 * If we are switching to gmode == GMODE_LEGACY_B,
	 * clean up rate info that may refer to OFDM rates.
	 */
	if ((gmode == GMODE_LEGACY_B) && (band->gmode != GMODE_LEGACY_B)) {
		band->gmode = gmode;
		wlc_scb_reinit(wlc);
		if (band->rspec_override && !IS_CCK(band->rspec_override)) {
			band->rspec_override = 0;
			wlc_reprate_init(wlc);
		}
		if (band->mrspec_override && !IS_CCK(band->mrspec_override)) {
			band->mrspec_override = 0;
		}
	}

	band->gmode = gmode;

	wlc->ignore_bcns = ignore_bcns;

	wlc->shortslot_override = shortslot;
	if (AP_ENAB(wlc->pub))
		wlc->ap->shortslot_restrict = shortslot_restrict;

	if (AP_ENAB(wlc->pub)) {
		wlc_bsscfg_t *bsscfg;
		FOREACH_BSS(wlc, i, bsscfg) {
			bsscfg->PLCPHdr_override =
				(preamble != WLC_PLCP_LONG) ? WLC_PLCP_SHORT : WLC_PLCP_AUTO;
		}
	}

	if ((AP_ENAB(wlc->pub) && preamble != WLC_PLCP_LONG) || preamble == WLC_PLCP_SHORT)
		wlc->default_bss->capability |= DOT11_CAP_SHORT;
	else
		wlc->default_bss->capability &= ~DOT11_CAP_SHORT;

	/* Update shortslot capability bit for AP and IBSS */
	if ((AP_ENAB(wlc->pub) && shortslot == WLC_SHORTSLOT_AUTO) ||
		shortslot == WLC_SHORTSLOT_ON)
		wlc->default_bss->capability |= DOT11_CAP_SHORTSLOT;
	else
		wlc->default_bss->capability &= ~DOT11_CAP_SHORTSLOT;

	/* Use the default 11g rateset */
	if (!rs.count)
		wlc_rateset_copy(&cck_ofdm_rates, &rs);

	if (ofdm_basic) {
		for (i = 0; i < rs.count; i++) {
			if (rs.rates[i] == WLC_RATE_6M || rs.rates[i] == WLC_RATE_12M ||
			    rs.rates[i] == WLC_RATE_24M)
				rs.rates[i] |= WLC_RATE_FLAG;
		}
	}

	/* Set default bss rateset */
	wlc->default_bss->rateset.count = rs.count;
	bcopy((char*)rs.rates, (char*)wlc->default_bss->rateset.rates,
		sizeof(wlc->default_bss->rateset.rates));
	band->defrateset.count = rs.count;
	bcopy((char*)rs.rates, (char*)band->defrateset.rates,
	      sizeof(band->defrateset.rates));

	wlc_update_brcm_ie(wlc);

	return ret;
}

static int
wlc_set_rateset(wlc_info_t *wlc, wlc_rateset_t *rs_arg)
{
	wlc_rateset_t rs, new;
	uint bandunit;

	bcopy((char*)rs_arg, (char*)&rs, sizeof(wlc_rateset_t));

	/* check for bad count value */
	if ((rs.count == 0) || (rs.count > WLC_NUMRATES))
		return BCME_BADRATESET;

	/* try the current band */
	bandunit = wlc->band->bandunit;
	bcopy((char*)&rs, (char*)&new, sizeof(wlc_rateset_t));
	if (wlc_rate_hwrs_filter_sort_validate(&new, &wlc->bandstate[bandunit]->hw_rateset, TRUE,
		wlc->stf->txstreams))
		goto good;

	/* try the other band */
	if (IS_MBAND_UNLOCKED(wlc)) {
		bandunit = OTHERBANDUNIT(wlc);
		bcopy((char*)&rs, (char*)&new, sizeof(wlc_rateset_t));
		if (wlc_rate_hwrs_filter_sort_validate(&new,
			&wlc->bandstate[bandunit]->hw_rateset, TRUE,
			wlc->stf->txstreams))
			goto good;
	}

	return BCME_ERROR;

good:
	/* apply new rateset */
	bcopy((char*)&new, (char*)&wlc->default_bss->rateset, sizeof(wlc_rateset_t));
	bcopy((char*)&new, (char*)&wlc->bandstate[bandunit]->defrateset, sizeof(wlc_rateset_t));

	if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype)) {
		/* update wlc->vht_cap. mcs map (802.11 IE format) */
		wlc_vht_update_mcs_cap(wlc->vhti);
	}

	return BCME_OK;
}

#ifdef BCMDBG
void
wlc_rateset_show(wlc_info_t *wlc, wlc_rateset_t *rs, struct ether_addr *ea)
{
	uint idx;
	uint r;
	bool b;

	if (WL_RATE_ON()) {
		if (ea != NULL) {
			char eabuf[ETHER_ADDR_STR_LEN];
			WL_RATE(("wl%d: %s: %s: ", wlc->pub->unit, __FUNCTION__,
			         bcm_ether_ntoa(ea, eabuf)));
		}
		WL_RATE(("[ "));
		for (idx = 0; idx < rs->count; idx++) {
			r = rs->rates[idx] & RATE_MASK;
			b = rs->rates[idx] & (~RATE_MASK);
			if (r == 0)
				break;
			WL_RATE(("%d%s%s ", (r / 2), (r % 2)?".5":"", b?"(b)":""));
		}
		WL_RATE(("]\n"));
		WL_RATE(("VHT mcsmap (rateset format): %04x\n", rs->vht_mcsmap));
	}
}
#endif /* BCMDBG */

/** bandlock ioctl */
int
wlc_bandlock(wlc_info_t *wlc, int val)
{
	chanspec_t chspec;
	bool move;
	uint bandunit, j;

	WL_TRACE(("wl%d: wlc_bandlock: wlc %p val %d\n", wlc->pub->unit, wlc, val));

	/* sanity check arg */
	if ((val < WLC_BAND_AUTO) || (val > WLC_BAND_2G))
		return BCME_RANGE;

	/* single band is easy */
	if (NBANDS(wlc) == 1) {
		if ((val != WLC_BAND_AUTO) && (val != wlc->band->bandtype))
			return BCME_BADBAND;

		wlc->bandlocked = (val == WLC_BAND_AUTO)? FALSE : TRUE;
#ifdef WLLED
		if (wlc->pub->up)
			wlc_led_event(wlc->ledh);
#endif
		return 0;
	}

	/* multiband */
	move = (wlc->band->bandtype != val);
	bandunit = (val == WLC_BAND_5G) ? 1 : 0;

	switch (val) {
	case WLC_BAND_AUTO:
		wlc->bandlocked = FALSE;
		break;

	case WLC_BAND_5G:
	case WLC_BAND_2G: {
		bool valid_channels = FALSE;

		/* make sure new band has at least one hw supported channel */
		for (j = 0; j < MAXCHANNEL; j++) {
			if (VALID_CHANNEL20_IN_BAND(wlc, bandunit, j)) {
				valid_channels = TRUE;
				break;
			}
		}
		if (valid_channels == FALSE) {
			WL_ERROR(("wl%d: can't change band since no valid channels in new band\n",
				wlc->pub->unit));
			return BCME_BADBAND;
		}

		/* prepare to set one band, allow core switching */
		wlc->bandlocked = FALSE;

		if (wlc->pub->up)
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);

		if (move) {
#ifdef STA
			if (wlc->pub->up) {
				int idx;
				wlc_bsscfg_t *cfg;
				FOREACH_BSS(wlc, idx, cfg) {
					if (BSSCFG_STA(cfg) && cfg->enable) {
						WL_APSTA_UPDN(("wl%d: wlc_bandlock() ->"
						               " wlc_bsscfg_disable()\n",
						               wlc->pub->unit));
						wlc_disassociate_client(cfg, FALSE);
						wlc_bsscfg_disable(wlc, cfg);
					}
				}
			}
#endif /* STA */
#ifdef AP
			if (wlc->pub->up && AP_ENAB(wlc->pub) && wlc->pub->associated) {
				/* do not switch band if the interface is associated in AP mode.
				 * We need do wlc_BSSinit() to sanitize the rateset
				 * otherwise we will fail the wlc_valid_rate checking
				 */
				WL_ERROR(("wl%d: can't change band when associated\n",
					wlc->pub->unit));
				return BCME_ASSOCIATED;
			}
#endif
#ifdef WDS
			/* On changing the band, delete the WDS nodes by freeing the scb
			 * and deleting the wds interface (done by wlc_scbfree). This is
			 * required because the scb is looked up off of the hash table
			 * associated with band and wds scbs will not be found on a band
			 * change in wlc_send.
			 */
			if (AP_ENAB(wlc->pub) && WLWDS_CAP(wlc))
				wlc_scb_wds_free(wlc);
#endif

			/* switch to first channel in the new band */
			wlc_pi_band_update(wlc, bandunit);
			chspec = wlc_default_chanspec(wlc->cmi, FALSE);
			ASSERT(chspec != INVCHANSPEC);
			wlc_pi_band_update(wlc, OTHERBANDUNIT(wlc));
			wlc->home_chanspec = chspec;

			if (wlc->pub->up) {
				wlc_suspend_mac_and_wait(wlc);
				wlc_set_chanspec(wlc, chspec);
				wlc_enable_mac(wlc);
			} else {
				/* In down state, only update the software chanspec. Don't call
				 * wlc_set_chanspec(), which touches the hardware. In high driver,
				 * there's no concept of band switch, which is encapsulated inside
				 * the chanspec change.
				 */
				wlc_pi_band_update(wlc, bandunit);
				/* sync up phy/radio chanspec */
				wlc_set_phy_chanspec(wlc, chspec);
			}

		}

		/* If the default bss chanspec is now invalid then pick a valid one */
		if (!wlc_valid_chanspec(wlc->cmi, wlc->default_bss->chanspec))
			wlc->default_bss->chanspec = wlc_default_chanspec(wlc->cmi, TRUE);

		/* Update bss rates to the band specific default rate set */
		wlc_default_rateset(wlc, &wlc->default_bss->rateset);

		wlc->bandlocked = TRUE;
		break;
	}
	default:
		ASSERT(0);
		break;
	}

#ifdef WLLED
	/* fixup LEDs */
	if (wlc->pub->up)
		wlc_led_event(wlc->ledh);
#endif

	return 0;
}

/** simplified integer set interface for common ioctl handler */
int
wlc_set(wlc_info_t *wlc, int cmd, int arg)
{
	return wlc_ioctl(wlc, cmd, (void *)&arg, sizeof(arg), NULL);
}

/** simplified integer get interface for common ioctl handler */
int
wlc_get(wlc_info_t *wlc, int cmd, int *arg)
{
	return wlc_ioctl(wlc, cmd, arg, sizeof(int), NULL);
}

/** This function is always forced to RAM to support WLTEST_DISABLED */
static int
wlc_ioctl_filter(wlc_info_t *wlc, int cmd, void *arg, int len)
{
#if defined(OPENSRC_IOV_IOCTL) && !defined(BCMDBG)
	/* List of commands supported by linux sta hybrid module */
	switch (cmd) {
	case WLC_SET_DTIMPRD:
	case WLC_SET_REGULATORY:
	case WLC_SET_CLOSED:
	case WLC_SET_COUNTRY:
	case WLC_FREQ_ACCURACY:
	case WLC_CARRIER_SUPPRESS:
	case WLC_GET_PHYREG:
	case WLC_SET_PHYREG:
	case WLC_GET_RADIOREG:
	case WLC_SET_RADIOREG:
	case WLC_SET_MACMODE:
	case WLC_SET_WDSLIST:
	case WLC_SET_CLK:
	case WLC_SET_SUP_RATESET_OVERRIDE:
	case WLC_SET_ASSOC_PREFER:
	case WLC_SET_BAD_FRAME_PREEMPT:
	case WLC_SET_SPECT_MANAGMENT:
		return BCME_UNSUPPORTED;
	}
#endif /* OPENSRC_IOV_IOCTL && !BCMDBG */

#ifdef WLTEST_DISABLED
	/* ioctls encapsulated by WLTEST */
	switch (cmd) {
	case WLC_GET_PHYREG:
	case WLC_SET_PHYREG:
	case WLC_GET_TSSI:
	case WLC_GET_ATTEN:
	case WLC_SET_ATTEN:
	case WLC_LONGTRAIN:
	case WLC_EVM:
	case WLC_FREQ_ACCURACY:
	case WLC_CARRIER_SUPPRESS:
	case WLC_GET_PWRIDX:
	case WLC_SET_PWRIDX:
	case WLC_GET_MSGLEVEL:
	case WLC_SET_MSGLEVEL:
	case WLC_GET_UCANTDIV:
	case WLC_SET_UCANTDIV:
	case WLC_SET_SROM:
	case WLC_NVRAM_GET:
#if !defined(WL_EXPORT_CURPOWER)
	case WLC_CURRENT_PWR:
#endif 
	case WLC_OTPW:
	case WLC_NVOTPW:
		return BCME_UNSUPPORTED;
	}
#endif /* WLTEST_DISABLED */

	return 0;
}

void
wlc_if_event(wlc_info_t *wlc, uint8 what, struct wlc_if *wlcif)
{
	wlc_event_t *e;
	wlc_bsscfg_t *cfg;
	wl_event_data_if_t *d;
	uint8 r;

	if ((cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_find_by_wlcif failed\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		return;
	}

	/* Don't do anything if the eventing system has been shutdown */
	if (wlc->eventq == NULL) {
		WL_ERROR(("wl%d: %s: eventing subsystem is down\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		return;
	}

	e = wlc_event_alloc(wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: %s: wlc_event_alloc failed\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		return;
	}

	bcopy(&cfg->cur_etheraddr.octet, e->event.addr.octet, ETHER_ADDR_LEN);
	e->addr = &(e->event.addr);

	e->event.event_type = WLC_E_IF;
	e->event.datalen = sizeof(wl_event_data_if_t);
	e->data = MALLOC(wlc->osh, e->event.datalen);
	if (e->data == NULL) {
		WL_ERROR(("wl%d: %s MALLOC failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		wlc_event_free(wlc->eventq, e);
		return;
	}

	d = (wl_event_data_if_t *)e->data;
	d->ifidx = wlcif->index;
	d->opcode = what;

	d->reserved = 0;
	if (BSSCFG_HAS_NOIF(cfg)) {
		d->reserved |= WLC_E_IF_FLAGS_BSSCFG_NOIF;
	}

	d->bssidx = WLC_BSSCFG_IDX(cfg);
	if (wlcif->type == WLC_IFTYPE_BSS) {
		if (BSSCFG_AP(cfg)) {
#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg))
				r = WLC_E_IF_ROLE_P2P_GO;
			else
#endif
				r = WLC_E_IF_ROLE_AP;
		} else {
#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg))
				r = WLC_E_IF_ROLE_P2P_CLIENT;
			else
#endif
				r = WLC_E_IF_ROLE_STA;
		}
	}
	else
		r = WLC_E_IF_ROLE_WDS;
	d->role = r;

	/* always send the WLC_E_IF event to the main interface */
	_wlc_event_if(wlc, e, cfg, cfg->wlcif, wlc->cfg->wlcif);

	wlc_process_event(wlc, e);
}

static void
wlc_ofdm_rateset_war(wlc_info_t *wlc)
{
	uint8 r;
	bool war = FALSE;

	if (wlc->cfg->associated)
		r = wlc->cfg->current_bss->rateset.rates[0];
	else
		r = wlc->default_bss->rateset.rates[0];

	if (WLCISGPHY(wlc->band) && (r != 0x82) && (r != 0x84) && (r != 0x8b) && (r != 0x96))
		war = TRUE;

	wlc_phy_ofdm_rateset_war(WLC_PI(wlc), war);

	return;
}

#if defined(WLC_PATCH_IOCTL)
#if !defined(WLC_IOCTL_PATCHMOD_INCLUDED)
int BCMRAMFN(wlc_ioctl_patchmod)(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	return (BCME_UNSUPPORTED);
}
#endif /* WLC_IOCTL_PATCHMOD_INCLUDED */
#endif /* WLC_PATCH_IOCTL */

int
wlc_ioctl(wlc_info_t *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	int bcmerror;

#if defined(WLC_PATCH_IOCTL)
	bcmerror = wlc_ioctl_patchmod(wlc, cmd, arg, len, wlcif);

	/* BCME_DISABLED is an internal error code (only returned by patch handlers) that indicates
	 * that the requested IOCTL is not included in the target. It was either unconditionally
	 * removed (after a ROM tape-out), or is not supported based upon #ifdefs. Map this back to
	 * the default error (BCME_UNSUPPORTED) so that users of this API don't have to deal with a
	 * new return code.
	 */
	if (bcmerror == BCME_DISABLED) {
		return (BCME_UNSUPPORTED);
	}

	/* If the IOCTL was handled by the patch handler function, return the result. Otherwise,
	 * invoke the main IOCTL handling function wlc_module_do_ioctl().
	 */
	if (bcmerror != BCME_UNSUPPORTED) {
		return bcmerror;
	}
#endif /* WLC_PATCH_IOCTL */

	bcmerror = wlc_module_do_ioctl(wlc, cmd, arg, len, wlcif);

	return bcmerror;
}

#ifdef STA
int
wlc_set_pm_mode(wlc_info_t *wlc, int val, wlc_bsscfg_t *bsscfg)
{
	wlc_pm_st_t *pm = bsscfg->pm;

	if ((val < PM_OFF) || (val > PM_FAST)) {
		return BCME_ERROR;
	}

	WL_INFORM(("wl%d.%d: setting PM from %d to %d\n",
		WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), pm->PM, val));

	if (pm->PM == (uint8)val)
		return BCME_OK;

	pm->PM = (uint8)val;
#ifdef WLC_LOW
	if (D11REV_GE(wlc->pub->corerev, 41)) {
		if (val == PM_OFF) {
			wlc_bmac_core_phypll_ctl(wlc->hw, TRUE);
		} else {
			wlc_bmac_core_phypll_ctl(wlc->hw, FALSE);
		}
	}
#endif /* WLC_LOW */

	/* if entering or exiting Fast PM mode, reset the PM2 features,
	 * stop any timers, and restore the TBTT wakeup interval.
	 */
	if (wlc->pub->up) {
		wlc_pm2_sleep_ret_timer_stop(bsscfg);
		if (PM2_RCV_DUR_ENAB(bsscfg)) {
			wlc_pm2_rcv_reset(bsscfg);
		}
		WL_RTDC(wlc, "WLC_SET_PM: %d\n", pm->PM, 0);
	}
	/* Set pmstate only if different from current state and */
	/* coming out of PM mode or PM is allowed */
	if ((!pm->WME_PM_blocked || val == PM_OFF))
		wlc_set_pmstate(bsscfg, val != PM_OFF);
	/* Change watchdog driver to align watchdog with tbtt if possible */
	wlc_watchdog_upd(bsscfg, PS_ALLOWED(bsscfg));

#ifdef WLAMPDU_MAC
	wlc_ampdu_upd_pm(wlc, pm->PM);
#endif

#ifdef WL_BCN_COALESCING
	wlc_bcn_clsg_disable(wlc->bc, BCN_CLSG_PM_MASK,
		val == PM_OFF ? 0: BCN_CLSG_PM_MASK);
#endif /* WL_BCN_COALESCING */

	return 0;
}

#endif /* STA */

void
wlc_do_down(wlc_info_t *wlc)
{
	int idx;
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
	wlc_info_t *wlc_iter;
	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {

		WL_APSTA_UPDN(("wl%d: wlc_ioctl(WLC_DOWN) -> wl_down()\n",
			wlc_iter->pub->unit));
		/* Don't access the hardware if hw is off */
		if (wlc_iter->pub->hw_off)
			continue;

		wlc_iter->down_override = TRUE;
#ifdef WL_DUALNIC_RSDB
		wlc_down(wlc_iter);
#else
		wl_down(wlc_iter->wl);
#endif
		wlc_iter->pub->hw_up = FALSE;
	}
}

/* common ioctl handler. return: 0=ok, -1=error, positive=particular error */
static int
_wlc_ioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	int val, *pval;
	bool bool_val;
	int bcmerror;
	d11regs_t *regs;
	uint i;
	struct scb *scb = NULL;
	scb_val_t *scb_val = (scb_val_t*)arg;
	bool ta_ok;
	uint band;
	wlc_bsscfg_t *bsscfg;
	struct scb_iter scbiter;
	osl_t *osh;
	int seq_cmd_rc;
	wlc_bss_info_t *current_bss;
	wlc_assoc_t *as;
	wlc_phy_t *pi = WLC_PI(wlc);
#ifdef STA
	wlc_roam_t *roam;
	wlc_pm_st_t *pm;
#endif
#if defined(BCMDBG) || defined(BCMQT)
	rw_reg_t *r; /* Used in WLC_R_REG and WLC_W_REG cases only */
#endif

	/* update bsscfg pointer */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	current_bss = bsscfg->current_bss;
	as = bsscfg->assoc;
#ifdef STA
	roam = bsscfg->roam;
	pm = bsscfg->pm;
#endif

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* initialize the following to get rid of compiler warning */
	ta_ok = FALSE;
	BCM_REFERENCE(ta_ok);
	band = 0;
	BCM_REFERENCE(band);

	/* If the IOCTL is part of a "batched" sequence command, simply queue it
	 * for future execution, and return a dummy success response. If not,
	 * process it immediately.
	 */
	seq_cmd_rc = wlc_seq_cmds_process(wlc->seq_cmds_info, cmd, arg, len, wlcif);
	if (SEQ_CMDS_BUFFERED == seq_cmd_rc)
		return (BCME_OK);
	else if (SEQ_CMDS_NOT_BUFFERED != seq_cmd_rc) {
		bcmerror = seq_cmd_rc;
		goto done;
	}

	bcmerror = wlc_ioctl_filter(wlc, cmd, arg, len);
	if (bcmerror < 0)
		return bcmerror;

	/* If the device is turned off, then it's not "removed" */
	if (!wlc->pub->hw_off && DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return BCME_ERROR;
	}

	ASSERT(!(wlc->pub->hw_off && wlc->pub->up));

	/* default argument is generic integer */
	pval = (int *) arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	/* bool conversion to avoid duplication below */
	bool_val = (val != 0);

	WL_TRACE(("wl%d: wlc_ioctl: interface %s cmd %d val 0x%x (%d) len %d\n",
		wlc->pub->unit, wl_ifname(wlc->wl, wlcif->wlif), cmd, (uint)val, val, len));
	if (!WL_TRACE_ON()) {
		WL_OID(("wl%d: wlc_ioctl: interface %s cmd %d val 0x%x (%d) len %d\n",
		        wlc->pub->unit, wl_ifname(wlc->wl, wlcif->wlif), cmd,
		        (uint)val, val, len));
	}

	bcmerror = 0;
	regs = wlc->regs;
	osh = wlc->osh;

	/* A few commands don't need any arguments; all the others do. */
	switch (cmd) {
	case WLC_UP:
	case WLC_OUT:
	case WLC_DOWN:
	case WLC_DISASSOC:
	case WLC_TERMINATED:
	case WLC_REBOOT:
	case WLC_START_CHANNEL_QA:
	case WLC_INIT:
		break;

	default:
		if ((arg == NULL) || (len <= 0)) {
			WL_ERROR(("wl%d: %s: Command %d needs arguments\n",
			          wlc->pub->unit, __FUNCTION__, cmd));
			bcmerror = BCME_BADARG;
			goto done;
		}
	}

	switch (cmd) {

	case WLC_GET_MAGIC:
		*pval = WLC_IOCTL_MAGIC;
		break;

	case WLC_GET_VERSION:
		*pval = WLC_IOCTL_VERSION;
		break;

	case WLC_UP:
		/* Don't access the hardware if hw is off */
		if (wlc->pub->hw_off)
			return BCME_RADIOOFF;

		wlc->down_override = FALSE;
		wlc->mpc_out = FALSE;
#ifdef STA
		wlc_radio_mpc_upd(wlc);
#endif /* STA */

#ifdef WLC_LOW
		/* BMAC_NOTES, TBD */
		wlc_radio_hwdisable_upd(wlc);
#endif

		wlc_radio_upd(wlc);

		if (!wlc->pub->up) {
			if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) ||
			    mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE)) {
				bcmerror = BCME_RADIOOFF;
			} else if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE)) {
				WL_INFORM(("wl%d: up failed: mpc radio is disabled\n",
					wlc->pub->unit));
			} else if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_COUNTRY_DISABLE)) {
				WL_INFORM(("wl%d: up failed: radio is disabled for country %s\n",
					wlc->pub->unit, wlc_channel_country_abbrev(wlc->cmi)));
			} else {
				/* something is wrong */
				bcmerror = BCME_ERROR;
#ifdef WLC_LOW /* for BMAC dongle WLC_UP  may fail if timeout when resume */
				ASSERT(0);
#endif /* WLC_HIGH_ONLY */
			}
		}
		break;

	case WLC_OUT:

		/* Don't access the hardware if hw is off */
		if (wlc->pub->hw_off)
			return BCME_RADIOOFF;

		/* make MPC transparent to "wl out", bring driver up if MPC made it down */
		wlc->mpc_out = TRUE;
#ifdef STA
		wlc_radio_mpc_upd(wlc);	/* this may cause interrupts and queued dpc */
		if (!wlc->pub->up)
			WL_MPC(("wl%d: WLC_OUT, driver was down, but not due to MPC\n",
			        wlc->pub->unit));
#endif /* STA */

		wlc->down_override = TRUE;
		wlc_out(wlc);
		break;

	case WLC_DOWN:
	case WLC_REBOOT:
		wlc_do_down(wlc);
		break;

	case WLC_GET_UP:
		*pval = (int)wlc->pub->up;
		break;

	case WLC_GET_CLK:
		*pval = wlc->clk;
		break;

	case WLC_SET_CLK:
		if (wlc->pub->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		/* Don't access the hardware if hw is off */
		if (wlc->pub->hw_off) {
			bcmerror = BCME_RADIOOFF;
			break;
		}

#ifdef BCMRECLAIM
		if (bcmreclaimed) {
			bcmerror = val ? 0 : BCME_EPERM;
			break;
		}
#endif /* BCMRECLAIM */

		wlc_bmac_set_clk(wlc->hw, bool_val);
		wlc->clk = bool_val;

		break;

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_MSGLEVEL:
		*pval = wl_msg_level;
		break;

	case WLC_SET_MSGLEVEL:
		wl_msg_level = val;
		break;
#endif /* BCMDBG || WLTEST */

#if defined(WL_PROMISC) || defined(BCMSPACE)
	case WLC_GET_PROMISC:
		*pval = (int)wlc->pub->promisc;
		break;

	case WLC_SET_PROMISC:
		wlc->pub->promisc = (val != 0);

		if (!wlc->clk) {
			bcmerror = 0;
			break;
		}

		wlc_mac_promisc(wlc);
		break;
#endif /* WL_PROMISC || BCMSPACE */

#if defined(WL_MONITOR) || defined(BCMSPACE)
	case WLC_GET_MONITOR:
		*pval = (int)wlc->monitor;
		break;

	case WLC_SET_MONITOR:
		wlc->monitor = val;
		wlc->bcnmisc_monitor = (wlc->monitor != 0);
#ifdef STA
		/* disable mpc if monitor is on, enable mpc if monitor is off */
		wlc_radio_mpc_upd(wlc);
#endif
		if (!wlc->clk) {
			bcmerror = 0;
			break;
		}

		wlc_monitor_promisc_enable(wlc->mon_info, (wlc->monitor != 0));
		wl_set_monitor(wlc->wl, val);

#ifdef STA
		/* Update PM state based on Monitor mode */
		wlc_set_wake_ctrl(wlc);
#endif /* STA */
		break;
#endif 

	case WLC_GET_RATE: {
		/* return raw datarate in units of 500 Kbit/s, other parameters are translated */
		ratespec_t rspec = wlc_get_rspec_history(bsscfg);
		*pval = RSPEC2KBPS(rspec)/500;
		break;
	}

	case WLC_GET_MAX_RATE: {
		/* return max raw datarate in units of 500 Kbit/s */
		ratespec_t rspec = wlc_get_current_highest_rate(bsscfg);
		*pval = RSPEC2KBPS(rspec)/500;
		break;
	}

	case WLC_GET_INSTANCE:
		*pval = wlc->pub->unit;
		break;

	case WLC_GET_INFRA:
		*pval = bsscfg->BSS;
		break;

	case WLC_SET_INFRA:
		if (BSSCFG_AP(bsscfg) && val == 0) {
			bcmerror = BCME_NOTSTA;
			break;
		}
		wlc->default_bss->infra = val?1:0;
		bsscfg->BSS = val?1:0;
		break;

	case WLC_GET_AUTH:
		*pval = DOT11AUTH2WLAUTH(bsscfg);
		break;

	case WLC_SET_AUTH:
		if (val == WL_AUTH_OPEN_SYSTEM || val == WL_AUTH_SHARED_KEY) {
			bsscfg->auth = WLAUTH2DOT11AUTH(val);
			bsscfg->openshared = 0;
		} else {
#ifdef WL_AUTH_METHOD_SHARED_OPEN
			bsscfg->auth = DOT11_SHARED_KEY;
#else
			bsscfg->auth = DOT11_OPEN_SYSTEM;
#endif /* WL_AUTH_METHOD_SHARED_OPEN */
			bsscfg->openshared = 1;
		}
		break;

	case WLC_GET_BSSID:

		/* Report on primary config */
		if ((BSSCFG_STA(bsscfg) && !bsscfg->associated)) {
			bcmerror = BCME_NOTASSOCIATED;
			break;
		}
		if (len < ETHER_ADDR_LEN) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		bcopy(bsscfg->BSSID.octet, arg, ETHER_ADDR_LEN);
		break;

	case WLC_SET_BSSID:
		if (bsscfg->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		if (len < ETHER_ADDR_LEN) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		bcopy(arg, bsscfg->BSSID.octet, ETHER_ADDR_LEN);
		if (!wlc->clk) {
			bcmerror = 0;
			break;
		}

		wlc_set_bssid(bsscfg);
		break;

	case WLC_GET_SSID: {
		wlc_ssid_t *ssid = (wlc_ssid_t *) arg;
		if (len < (int)sizeof(wlc_ssid_t)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		bzero((char*)ssid, sizeof(wlc_ssid_t));
		if (BSSCFG_AP(bsscfg)) {
			ssid->SSID_len = bsscfg->SSID_len;
			bcopy(bsscfg->SSID, ssid->SSID, ssid->SSID_len);
		} else if (bsscfg->associated) {
			ssid->SSID_len = current_bss->SSID_len;
			bcopy(current_bss->SSID, ssid->SSID, current_bss->SSID_len);
		} else {
			ssid->SSID_len = wlc->default_bss->SSID_len;
			bcopy(wlc->default_bss->SSID, ssid->SSID, wlc->default_bss->SSID_len);
		}

		break;
	}

	case WLC_SET_SSID: {
		wlc_ssid_t *ssid = (wlc_ssid_t *)arg;

		if (len < (int)sizeof(ssid->SSID_len)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		if (len < (int)(sizeof(ssid->SSID_len))+(int)ssid->SSID_len) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		if (ssid->SSID_len > DOT11_MAX_SSID_LEN) {
			bcmerror = BCME_BADSSIDLEN;
			break;
		}

		/* keep old behavior of WLC_SET_SSID by restarting the AP's BSS
		 * if the SSID is non-null
		 */
		if (BSSCFG_AP(bsscfg)) {
			wlc_bsscfg_SSID_set(bsscfg, ssid->SSID, ssid->SSID_len);

			WL_APSTA_UPDN(("wl%d: SET_SSID -> wlc_bsscfg_disable()\n",
			               wlc->pub->unit));
			wlc_bsscfg_disable(wlc, bsscfg);

			/* only bring up the BSS if SSID is non-null */
			if (bsscfg->SSID_len != 0) {
				WL_APSTA_UPDN(("wl%d: SET_SSID -> wlc_bsscfg_enable()\n",
					wlc->pub->unit));
				bcmerror = wlc_bsscfg_enable(wlc, bsscfg);
				if (bcmerror)
					break;
#if defined(RADAR)
				if (WL11H_AP_ENAB(wlc)) {
					/* no radar detection if sta is associated since
					 * AP shares the same channel which is selected
					 * by an external AP sta associates with
					 */
					if (!(BSSCFG_SRADAR_ENAB(bsscfg) ||
					      BSSCFG_AP_NORADAR_CHAN_ENAB(bsscfg)))
						wlc_set_dfs_cacstate(wlc->dfs, ON);
				}
#endif /* defined(RADAR) */
			}
		}

#ifdef STA
		/* Behavior of WLC_SET_SSID for STA is different from that for AP */
		if (BSSCFG_STA(bsscfg)) {
			wl_join_assoc_params_t *assoc_params = NULL;
			int assoc_params_len = 0;

			if ((uint)len >= WL_JOIN_PARAMS_FIXED_SIZE) {
				bool reset_chanspec_num = FALSE;

				assoc_params = &((wl_join_params_t *)arg)->params;
				assoc_params_len = len - OFFSETOF(wl_join_params_t, params);
#ifdef WLRCC
				if (WLRCC_ENAB(wlc->pub) &&
					(bsscfg->roam->rcc_mode != RCC_MODE_FORCE))
					rcc_update_channel_list(bsscfg->roam, ssid, assoc_params);
#endif
				/* When we intend to use bsslist, we set bssid_cnt to the count
				* of bssids and chanspec (they are the same)
				* and we set chanspec_num to zero.
				* This is to  make sure that we do not
				* unintentionally use any uninitialized bssid_cnt
				* as bssid_cnt is a new field .
				*/
				if ((assoc_params->bssid_cnt) && (assoc_params->chanspec_num == 0))
				{
					reset_chanspec_num = TRUE;
					assoc_params->chanspec_num = assoc_params->bssid_cnt;
				}

				bcmerror = wlc_assoc_chanspec_sanitize(wlc,
				        (chanspec_list_t *)&assoc_params->chanspec_num,
				        len - OFFSETOF(wl_join_params_t, params.chanspec_num));
				if (reset_chanspec_num)
					assoc_params->chanspec_num = 0;
				if (bcmerror != BCME_OK)
					break;
			}
			WL_APSTA_UPDN(("wl%d: SET_SSID(STA) -> wlc_join()\n",
			               wlc->pub->unit));
			bcmerror = wlc_join(wlc, bsscfg, ssid->SSID, ssid->SSID_len,
			         NULL,
			         assoc_params, assoc_params_len);
		}
#endif /* STA */

		break;
	}


	case WLC_GET_CHANNEL: {
		channel_info_t *ci = (channel_info_t*)arg;
		int chspec;

		ASSERT(len > (int)sizeof(ci));

		ci->hw_channel = wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC);

		bcmerror = wlc_iovar_op(wlc, "chanspec",
		                        NULL, 0, &chspec, sizeof(int), IOV_GET, wlcif);
		if (bcmerror != BCME_OK)
			break;
		ci->target_channel = wf_chspec_ctlchan((chanspec_t)chspec);

		ci->scan_channel = 0;

		if (SCAN_IN_PROGRESS(wlc->scan)) {
			ci->scan_channel =
				wf_chspec_ctlchan(wlc_scan_get_current_chanspec(wlc->scan));
		}
		break;
	}

	case WLC_SET_CHANNEL: {
		int chspec = CH20MHZ_CHSPEC(val);

		if (val < 0 || val > MAXCHANNEL) {
			bcmerror = BCME_OUTOFRANGECHAN;
			break;
		}
		bcmerror = wlc_iovar_op(wlc, "chanspec",
		                        NULL, 0, &chspec, sizeof(int), IOV_SET, wlcif);
		break;
	}

	case WLC_GET_PWROUT_PERCENTAGE:
		*pval = wlc->txpwr_percent;
		break;

	case WLC_SET_PWROUT_PERCENTAGE:
		if ((uint)val > 100) {
			bcmerror = BCME_RANGE;
			break;
		}

		wlc->txpwr_percent = (uint8)val;
		wlc_bmac_set_txpwr_percent(wlc->hw, (uint8)val);
		if (wlc->pub->up) {
			uint8 constraint;

			if (SCAN_IN_PROGRESS(wlc->scan)) {
				WL_INFORM(("wl%d: Scan in progress, skipping txpower control\n",
					wlc->pub->unit));
				break;
			}

			wlc_suspend_mac_and_wait(wlc);

			/* Set the power limits for this locale after computing
			 * any 11h local tx power constraints.
			 */
			constraint = wlc_tpc_get_local_constraint_qdbm(wlc->tpc);
			wlc_channel_set_txpower_limit(wlc->cmi, constraint);

			wlc_enable_mac(wlc);
		}
		break;

#if defined(BCMDBG) || defined(BCMQT)
	case WLC_GET_UCFLAGS:
		if (!wlc->pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		/* optional band is stored in the second integer of incoming buffer */
		band = (len < (int)(2 * sizeof(int))) ? WLC_BAND_AUTO : ((int *)arg)[1];

		/* bcmerror checking */
		if ((bcmerror = wlc_iocregchk(wlc, band)))
			break;
		if (val >= MHFMAX) {
			bcmerror = BCME_RANGE;
			break;
		}

		*pval = wlc_bmac_mhf_get(wlc->hw, (uint8)val, WLC_BAND_AUTO);
		break;

	case WLC_SET_UCFLAGS:
		if (!wlc->pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		/* optional band is stored in the second integer of incoming buffer */
		band = (len < (int)(2 * sizeof(int))) ? WLC_BAND_AUTO : ((int *)arg)[1];

		/* bcmerror checking */
		if ((bcmerror = wlc_iocregchk(wlc, band)))
			break;
		i = (uint16)val;
		if (i >= MHFMAX) {
			bcmerror = BCME_RANGE;
			break;
		}

		wlc_mhf(wlc, (uint8)i, 0xffff, (uint16)(val >> NBITS(uint16)), WLC_BAND_AUTO);
		break;

	case WLC_GET_SHMEM:
		ta_ok = TRUE;

		/* optional band is stored in the second integer of incoming buffer */
		band = (len < (int)(2 * sizeof(int))) ? WLC_BAND_AUTO : ((int *)arg)[1];

		/* bcmerror checking */
		if ((bcmerror = wlc_iocregchk(wlc, band)))
			break;

		if (val & 1) {
			bcmerror = BCME_BADADDR;
			break;
		}

		*pval = wlc_read_shm(wlc, (uint16)val);
		break;

	case WLC_SET_SHMEM:
		ta_ok = TRUE;

		/* optional band is stored in the second integer of incoming buffer */
		band = (len < (int)(2 * sizeof(int))) ? WLC_BAND_AUTO : ((int *)arg)[1];

		/* bcmerror checking */
		if ((bcmerror = wlc_iocregchk(wlc, band)))
			break;

		if (val & 1) {
			bcmerror = BCME_BADADDR;
			break;
		}

		wlc_write_shm(wlc, (uint16)val, (uint16)(val >> NBITS(uint16)));
		break;

	case WLC_R_REG:	/* MAC registers */
		ta_ok = TRUE;
		r = (rw_reg_t*)arg;
		band = WLC_BAND_AUTO;

		if (len < (int)(sizeof(rw_reg_t) - sizeof(uint))) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		if (len >= (int)sizeof(rw_reg_t))
			band = r->band;

		/* bcmerror checking */
		if ((bcmerror = wlc_iocregchk(wlc, band)))
			break;

		if ((r->byteoff + r->size) > sizeof(d11regs_t)) {
			bcmerror = BCME_BADADDR;
			break;
		}
		if (r->size == sizeof(uint32))
			r->val = R_REG(osh, (uint32 *)((uchar *)(uintptr)regs + r->byteoff));
		else if (r->size == sizeof(uint16))
			r->val = R_REG(osh, (uint16 *)((uchar *)(uintptr)regs + r->byteoff));
		else
			bcmerror = BCME_BADADDR;
		break;

	case WLC_W_REG:
		ta_ok = TRUE;
		r = (rw_reg_t*)arg;
		band = WLC_BAND_AUTO;

		if (len < (int)(sizeof(rw_reg_t) - sizeof(uint))) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		if (len >= (int)sizeof(rw_reg_t))
			band = r->band;

		/* bcmerror checking */
		if ((bcmerror = wlc_iocregchk(wlc, band)))
			break;

		if (r->byteoff + r->size > sizeof(d11regs_t)) {
			bcmerror = BCME_BADADDR;
			break;
		}
		if (r->size == sizeof(uint32))
			W_REG(osh, (uint32 *)((uchar *)(uintptr)regs + r->byteoff), r->val);
		else if (r->size == sizeof(uint16))
			W_REG(osh, (uint16 *)((uchar *)(uintptr)regs + r->byteoff), r->val);
		else
			bcmerror = BCME_BADADDR;
		break;
#endif /* BCMDBG */

	case WLC_GET_TXANT:
		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}
		*pval = wlc->stf->txant;
		break;

	case WLC_SET_TXANT:
		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}
		bcmerror = wlc_stf_ant_txant_validate(wlc, (int8)val);
		if (bcmerror < 0)
			break;

		wlc->stf->txant = (int8)val;

		/* if down, we are done */
		if (!wlc->pub->up)
			break;

		wlc_suspend_mac_and_wait(wlc);

		wlc_stf_phy_txant_upd(wlc);
		wlc_beacon_phytxctl_txant_upd(wlc, wlc->bcn_rspec);

		wlc_enable_mac(wlc);

		break;

	case WLC_GET_ANTDIV: {
		uint8 phy_antdiv;

		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		/* return configured value if core is down */
		if (!wlc->pub->up) {
			*pval = wlc->stf->ant_rx_ovr;

		} else {
			phy_antdiv_get_rx((phy_info_t *)pi, &phy_antdiv);
		}

		break;
	}
	case WLC_SET_ANTDIV:
		/* values are -1=driver default, 0=force0, 1=force1, 2=start1, 3=start0 */
		if ((val < -1) || (val > 3)) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		if (val == -1)
			val = ANT_RX_DIV_DEF;

		wlc->stf->ant_rx_ovr = (uint8)val;
		bcmerror = phy_antdiv_set_rx((phy_info_t *)pi, (uint8)val);
		break;

	case WLC_GET_RX_ANT: {	/* get latest used rx antenna */
		uint16 rxstatus;

		if (!wlc->pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		rxstatus = R_REG(osh, &regs->phyrxstatus0);
		if (rxstatus == 0xdead || rxstatus == (uint16)-1) {
			bcmerror = BCME_ERROR;
			break;
		}
		*pval = (rxstatus & PRXS0_RXANT_UPSUBBAND) ? 1 : 0;
		break;
	}

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_UCANTDIV:
		if (!wlc->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		*pval = (wlc_bmac_mhf_get(wlc->hw, MHF1, WLC_BAND_AUTO) & MHF1_ANTDIV);
		break;

	case WLC_SET_UCANTDIV: {
		if (!wlc->pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		/* if multiband, band must be locked */
		if (IS_MBAND_UNLOCKED(wlc)) {
			bcmerror = BCME_NOTBANDLOCKED;
			break;
		}

		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		/* 4322 supports antdiv in phy, no need to set it to ucode */
		if (WLCISNPHY(wlc->band) && D11REV_IS(wlc->pub->corerev, 16)) {
			WL_ERROR(("wl%d: can't set ucantdiv for 4322\n", wlc->pub->unit));
			bcmerror = BCME_UNSUPPORTED;
		} else
			wlc_mhf(wlc, MHF1, MHF1_ANTDIV, (val? MHF1_ANTDIV : 0), WLC_BAND_AUTO);
		break;
	}
#endif /* BCMDBG || WLTEST */

	case WLC_GET_SRL:
		*pval = wlc->SRL;
		break;

	case WLC_SET_SRL:
		if (val >= 1 && val <= RETRY_SHORT_MAX) {
			int ac;
			wlc->SRL = (uint16)val;

			wlc_bmac_retrylimit_upd(wlc->hw, wlc->SRL, wlc->LRL);

			for (ac = 0; ac < AC_COUNT; ac++) {
				WLC_WME_RETRY_SHORT_SET(wlc, ac, wlc->SRL);
			}
			wlc_wme_retries_write(wlc);
		} else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_LRL:
		*pval = wlc->LRL;
		break;

	case WLC_SET_LRL:
		if (val >= 1 && val <= 255) {
			int ac;
			wlc->LRL = (uint16)val;

			wlc_bmac_retrylimit_upd(wlc->hw, wlc->SRL, wlc->LRL);

			for (ac = 0; ac < AC_COUNT; ac++) {
				WLC_WME_RETRY_LONG_SET(wlc, ac, wlc->LRL);
			}
			wlc_wme_retries_write(wlc);
		} else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_CWMIN:
		*pval = wlc->band->CWmin;
		break;

	case WLC_SET_CWMIN:
		if (!wlc->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		if (val >= 1 && val <= 255) {
			wlc_set_cwmin(wlc, (uint16)val);
		} else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_CWMAX:
		*pval = wlc->band->CWmax;
		break;

	case WLC_SET_CWMAX:
		if (!wlc->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		if (val >= 255 && val <= 2047) {
			wlc_set_cwmax(wlc, (uint16)val);
		} else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_PLCPHDR:
		*pval = bsscfg->PLCPHdr_override;
		break;

	case WLC_SET_PLCPHDR:
		/*
		 * Regardless of the value here, the MAC will always return ctrl/mgmt
		 * response frames with the same type preamble as received.
		 * So will the driver unless forced long (1) here (testing only).
		 * (-1) is to disable short preamble capability advertisement and
		 *      never to initiate a short preamble frame exchange sequence.
		 * (0) BSS STA: enable short preamble capability advertisement but
		 *              the driver will only send short preambles if the
		 *              AP also advertises the capability;
		 *     AP:      enable short preamble capability advertisement and will
		 *              deny association to STAs that do not advertise the capability.
		 *     IBSS STA:questionable but will generally work.
		 */
		if ((val == WLC_PLCP_AUTO) || (val == WLC_PLCP_SHORT) || (val == WLC_PLCP_LONG))
			bsscfg->PLCPHdr_override = (int8) val;
		else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_RADIO:	/* use mask if don't want to expose some internal bits */
		*pval = wlc->pub->radio_disabled;
		break;

	case WLC_SET_RADIO: { /* 32 bits input, higher 16 bits are mask, lower 16 bits are value to
			       * set
			       */
		uint16 radiomask, radioval;
		uint validbits = WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;
		mbool new = 0;

		radiomask = (val & 0xffff0000) >> 16;
		radioval = val & 0x0000ffff;

		if ((radiomask == 0) || (radiomask & ~validbits) || (radioval & ~validbits) ||
		    ((radioval & ~radiomask) != 0)) {
			WL_ERROR(("SET_RADIO with wrong bits 0x%x\n", val));
			bcmerror = BCME_RANGE;
			break;
		}

		new = (wlc->pub->radio_disabled & ~radiomask) | radioval;
#ifdef WLEXTLOG
		if (wlc->pub->radio_disabled != new) {
			if (!mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) &&
			    mboolisset(new, WL_RADIO_HW_DISABLE))
				WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_HW_OFF_ID,
					WL_LOG_LEVEL_ERR, 0, 0, NULL);
			if (!mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE) &&
			    mboolisset(new, WL_RADIO_SW_DISABLE))
				WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_SW_OFF_ID,
					WL_LOG_LEVEL_ERR, 0, 0, NULL);
			if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) &&
			    !mboolisset(new, WL_RADIO_HW_DISABLE))
				WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_HW_ON_ID,
					WL_LOG_LEVEL_ERR, 0, 0, NULL);
			if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE) &&
			    !mboolisset(new, WL_RADIO_SW_DISABLE))
				WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RADIO_SW_ON_ID,
					WL_LOG_LEVEL_ERR, 0, 0, NULL);
		}
#endif /* WLEXTLOG */
		wlc->pub->radio_disabled = new;
		WL_MPC(("wl%d: SET_RADIO, radio_disable vector 0x%x\n", wlc->pub->unit,
			wlc->pub->radio_disabled));

		wlc_radio_hwdisable_upd(wlc);
		wlc_radio_upd(wlc);
		break;
	}

	case WLC_GET_PHYTYPE:
		*pval = WLC_PHYTYPE(wlc->band->phytype);
		break;


#ifdef BCMDBG
	/* deprecated, use wl ratesel_xxx iovar */
	case WLC_SET_RATE_PARAMS:
		bcmerror = BCME_NOTFOUND;
		break;
#endif /* BCMDBG */

	case WLC_GET_WEP_RESTRICT:
		*pval = bsscfg->wsec_restrict;
		break;

	case WLC_SET_WEP_RESTRICT:
		bsscfg->wsec_restrict = bool_val;
		break;

#ifdef WLLED
	case WLC_SET_LED:
		if (len < (int)sizeof(wl_led_info_t)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		wlc_led_set(wlc->ledh, (wl_led_info_t *)arg);
		break;
#endif

	case WLC_SCAN:
		WL_ASSOC(("wl%d.%d: SCAN: WLC_SCAN custom scan\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg)));
		bcmerror = wlc_custom_scan(wlc, arg, len, 0, WLC_ACTION_SCAN, bsscfg);
		break;

	case WLC_SCAN_RESULTS: {
		/* IO buffer has a uint32 followed by variable data.
		 * The uint32 declares the size of the buffer on input and
		 * size written on output (or required if insufficient buf space).
		 * If successful, return the bss count in uint32, followed by
		 * variable length wl_bss_info_t structs.
		 * BCME_BUFTOOSHORT is used only to indicate results were partial.
		 */
		wl_scan_results_t *iob = (wl_scan_results_t*)arg;
		int buflen;

		if ((uint)len < WL_SCAN_RESULTS_FIXED_SIZE) {
			bcmerror = BCME_BADARG;
			break;
		}

		buflen = (int)iob->buflen;

		if ((uint)buflen < WL_SCAN_RESULTS_FIXED_SIZE) {
			bcmerror = BCME_BADARG;
			break;
		}

		if (buflen > len)
			buflen = len;

		buflen -= WL_SCAN_RESULTS_FIXED_SIZE;

		bcmerror = wlc_scan_results(wlc, iob, &buflen, wlc->custom_scan_results_state);
		break;
	}

#ifdef STA
	case WLC_DISASSOC:
		if (!wlc->clk) {
			if (!mboolisset(wlc->pub->radio_disabled, WL_RADIO_MPC_DISABLE)) {
				bcmerror = BCME_NOCLK;
			}
		} else if (BSSCFG_AP(bsscfg)) {
			bcmerror = BCME_NOTSTA;
		} else {
			/* translate the WLC_DISASSOC command to disabling the config
			 * disassoc packet will be sent if currently associated
			 */
			WL_APSTA_UPDN(("wl%d: WLC_DISASSOC -> wlc_bsscfg_disable()\n",
				wlc->pub->unit));
#ifdef ROBUST_DISASSOC_TX
			bsscfg->assoc->disassoc_tx_retry = 0;
			wlc_disassoc_tx(bsscfg, TRUE);
#else
			wlc_bsscfg_disable(wlc, bsscfg);
#endif

		}
		break;

	case WLC_REASSOC: {
		wl_reassoc_params_t reassoc_params;
		wl_reassoc_params_t *params;
		memset(&reassoc_params, 0, sizeof(wl_reassoc_params_t));

		if (!wlc->pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		if ((uint)len < ETHER_ADDR_LEN) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		if ((uint)len >= WL_REASSOC_PARAMS_FIXED_SIZE) {
			params = (wl_reassoc_params_t *)arg;
			bcmerror = wlc_assoc_chanspec_sanitize(wlc,
			        (chanspec_list_t *)&params->chanspec_num,
			        len - OFFSETOF(wl_reassoc_params_t, chanspec_num));
			if (bcmerror != BCME_OK)
				break;
		} else {
			bcopy((void *)arg, (void *)&reassoc_params.bssid, ETHER_ADDR_LEN);
			/* scan on all channels */
			reassoc_params.chanspec_num = 0;
			params = &reassoc_params;

			len = (int)WL_REASSOC_PARAMS_FIXED_SIZE;
		}

		if ((bcmerror = wlc_reassoc(bsscfg, params)) != 0)
			WL_ERROR(("%s: wlc_reassoc fails (%d)\n", __FUNCTION__, bcmerror));

		if ((bcmerror == BCME_BUSY) && (BSSCFG_STA(bsscfg)) && (bsscfg == wlc->cfg)) {
			/* If REASSOC is blocked by other activity,
			   it will retry again from wlc_watchdog
			   so the param info needs to be stored, and return success
			*/
			if (roam->reassoc_param != NULL) {
				MFREE(wlc->osh, roam->reassoc_param, roam->reassoc_param_len);
				roam->reassoc_param = NULL;
			}
			roam->reassoc_param_len = (uint8)len;
			roam->reassoc_param = MALLOC(wlc->osh, len);

			if (roam->reassoc_param != NULL) {
				bcopy((char*)params, (char*)roam->reassoc_param, len);
			}
			bcmerror = 0;
		}
		break;
	}

	case WLC_GET_ROAM_TRIGGER:
		/* bcmerror checking */
		if ((bcmerror = wlc_iocbandchk(wlc, (int*)arg, len, &band, FALSE)))
			break;

		if (band == WLC_BAND_ALL) {
			bcmerror = BCME_BADOPTION;
			break;
		}

		/* Return value for specified band: current or other */
		if ((int)band == wlc->band->bandtype)
			*pval = wlc->band->roam_trigger;
		else
			*pval = wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_trigger;
		break;

	case WLC_SET_ROAM_TRIGGER: {
		int trigger_dbm;

		/* bcmerror checking */
		if ((bcmerror = wlc_iocbandchk(wlc, (int*)arg, len, &band, FALSE)))
			break;

		if ((val < -100) || (val > WLC_ROAM_TRIGGER_MAX_VALUE)) {
			bcmerror = BCME_RANGE;
			break;
		}

		/* set current band if specified explicitly or implicitly (via "all") */
		if ((band == WLC_BAND_ALL) || ((int)band == wlc->band->bandtype)) {
			/* roam_trigger is either specified as dBm (-1 to -99 dBm) or a
			 * logical value >= 0
			 */
			trigger_dbm = (val < 0) ? val :
			        wlc_roam_trigger_logical_dbm(wlc, wlc->band, val);
			wlc->band->roam_trigger_def = trigger_dbm;
			wlc->band->roam_trigger = trigger_dbm;
		}

		/* set other band if explicit or implicit (via "all") */
		if ((NBANDS(wlc) > 1) &&
		    ((band == WLC_BAND_ALL) || ((int)band != wlc->band->bandtype))) {
			trigger_dbm = (val < 0) ? val : wlc_roam_trigger_logical_dbm(wlc,
				wlc->bandstate[OTHERBANDUNIT(wlc)], val);
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_trigger_def = trigger_dbm;
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_trigger = trigger_dbm;
		}
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;
	}

	case WLC_GET_ROAM_DELTA:
		/* bcmerror checking */
		if ((bcmerror = wlc_iocbandchk(wlc, (int*)arg, len, &band, FALSE)))
			break;

		if (band == WLC_BAND_ALL) {
			bcmerror = BCME_BADOPTION;
			break;
		}

		/* Return value for specified band: current or other */
		if ((int)band == wlc->band->bandtype)
			*pval = wlc->band->roam_delta;
		else
			*pval = wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_delta;
		break;

	case WLC_SET_ROAM_DELTA:
		/* bcmerror checking */
		if ((bcmerror = wlc_iocbandchk(wlc, (int*)arg, len, &band, FALSE)))
			break;

		if ((val > 100) || (val < 0)) {
			bcmerror = BCME_BADOPTION;
			break;
		}

		/* set current band if specified explicitly or implicitly (via "all") */
		if ((band == WLC_BAND_ALL) || ((int)band == wlc->band->bandtype)) {
			wlc->band->roam_delta_def = val;
			wlc->band->roam_delta = val;
		}

		/* set other band if explicit or implicit (via "all") */
		if ((NBANDS(wlc) > 1) &&
		    ((band == WLC_BAND_ALL) || ((int)band != wlc->band->bandtype))) {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_delta_def = val;
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_delta = val;
		}
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;

	case WLC_GET_ROAM_SCAN_PERIOD:
		*pval = roam->partialscan_period;
		break;

	case WLC_SET_ROAM_SCAN_PERIOD:
		roam->partialscan_period = val;
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;
#endif /* STA */

	case WLC_GET_CLOSED:
		*pval = bsscfg->closednet_nobcnssid;
		break;

	case WLC_SET_CLOSED:
		bcmerror = wlc_iovar_op(wlc, "closednet", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	case WLC_GET_CURR_RATESET: {
		wl_rateset_t *ret_rs = (wl_rateset_t *)arg;
		wlc_rateset_t *rs;

		if (bsscfg->associated)
			rs = &current_bss->rateset;
		else
			rs = &wlc->default_bss->rateset;

		if (len < (int)(rs->count + sizeof(rs->count))) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		/* Copy only legacy rateset section */
		ret_rs->count = rs->count;
		bcopy(&rs->rates, &ret_rs->rates, rs->count);
		break;
	}

	case WLC_GET_RATESET: {
		wlc_rateset_t rs;
		wl_rateset_t *ret_rs = (wl_rateset_t *)arg;

		bzero(&rs, sizeof(wlc_rateset_t));
		wlc_default_rateset(wlc, (wlc_rateset_t*)&rs);

		if (len < (int)(rs.count + sizeof(rs.count))) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		/* Copy only legacy rateset section */
		ret_rs->count = rs.count;
		bcopy(&rs.rates, &ret_rs->rates, rs.count);
		break;
	}

	case WLC_SET_RATESET: {
		wlc_rateset_t rs;
		wl_rateset_t *in_rs = (wl_rateset_t *)arg;

		if (len < (int)(in_rs->count + sizeof(in_rs->count))) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		if (in_rs->count > WLC_NUMRATES) {
			bcmerror = BCME_BUFTOOLONG;
			break;
		}

		bzero(&rs, sizeof(wlc_rateset_t));

		/* Copy only legacy rateset section */
		rs.count = in_rs->count;
		bcopy(&in_rs->rates, &rs.rates, rs.count);

		/* merge rateset coming in with the current mcsset */
		if (N_ENAB(wlc->pub)) {
			if (bsscfg->associated)
				bcopy(&current_bss->rateset.mcs[0], rs.mcs, MCSSET_LEN);
			else
				bcopy(&wlc->default_bss->rateset.mcs[0], rs.mcs, MCSSET_LEN);
		}

#if defined(WL11AC)
		if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype)) {
				rs.vht_mcsmap = (bsscfg->associated)
				? current_bss->rateset.vht_mcsmap
				: wlc->default_bss->rateset.vht_mcsmap;
		}
#endif /* WL11AC */

		bcmerror = wlc_set_rateset(wlc, &rs);

		if (!bcmerror)
			wlc_ofdm_rateset_war(wlc);

		break;
	}

	case WLC_GET_DESIRED_BSSID:
		bcopy((char*)wlc->desired_BSSID.octet, (char*)arg, ETHER_ADDR_LEN);
		break;

	case WLC_SET_DESIRED_BSSID:
		bcopy((char*)arg, (char*)wlc->desired_BSSID.octet, ETHER_ADDR_LEN);
		break;

	case WLC_GET_BCNPRD:
#ifdef STA
		if (BSSCFG_STA(bsscfg) && bsscfg->BSS && bsscfg->associated)
			*pval = current_bss->beacon_period;
		else
#endif
			*pval = wlc->default_bss->beacon_period;
		break;

	case WLC_SET_BCNPRD:
		/* range [1, 0xffff] */
		if (val >= DOT11_MIN_BEACON_PERIOD && val <= DOT11_MAX_BEACON_PERIOD) {
			wlc->default_bss->beacon_period = (uint16) val;
		} else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_DTIMPRD:
#ifdef STA
		if (BSSCFG_STA(bsscfg) && bsscfg->BSS && bsscfg->associated)
			*pval = current_bss->dtim_period;
		else
#endif
			*pval = wlc->default_bss->dtim_period;
		break;

	case WLC_SET_DTIMPRD:
		/* range [1, 0xff] */
		if (val >= DOT11_MIN_DTIM_PERIOD && val <= DOT11_MAX_DTIM_PERIOD) {
			wlc->default_bss->dtim_period = (uint8)val;
		} else
			bcmerror = BCME_RANGE;
		break;

	case WLC_GET_SROM: {
		srom_rw_t *s = (srom_rw_t *)arg;
		if (s->byteoff == 0x55ab) {	/* Hack for srcrc */
			s->byteoff = 0;
			bcmerror = wlc_iovar_op(wlc, "srcrc", arg, len, arg, len, IOV_GET, wlcif);
		} else
			bcmerror = wlc_iovar_op(wlc, "srom", arg, len, arg, len, IOV_GET, wlcif);
		break;
	}

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_SET_SROM: {
		bcmerror = wlc_iovar_op(wlc, "srom", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	}
#endif	

#ifdef STA
	case WLC_GET_PM:
		*pval = pm->PM;
		break;

	case WLC_SET_PM:
		/* save host requested pmstate */
		if (wlc->cfg == bsscfg)
			wlc_btc_save_host_requested_pm(wlc, (uint8)val);
		if (val == PM_FORCE_OFF) {
			val = PM_OFF;
		}
#ifdef BTCX_PM0_IDLE_WAR
		else {
			if (wlc_btc_get_bth_active(wlc) && (PM_OFF == val))
				val = PM_FAST;
		}
#endif	/* BTCX_PM0_IDLE_WAR */
		bcmerror = wlc_set_pm_mode(wlc, val, bsscfg);

		break;

#ifdef BCMDBG
	case WLC_GET_WAKE:
		*pval = wlc->wake;
		break;

	case WLC_SET_WAKE:
		WL_PS(("wl%d: setting WAKE to %d\n", WLCWLUNIT(wlc), val));
		wlc->wake = val ? TRUE : FALSE;

		/* apply to the mac */
		wlc_set_wake_ctrl(wlc);
		break;
#endif /* BCMDBG */
#endif /* STA */

	case WLC_GET_REVINFO:
		bcmerror = wlc_get_revision_info(wlc, arg, (uint)len);
		break;

	case WLC_SET_AP: {
	        bool wasup = wlc->pub->up;
#ifndef AP
	        if (val)
	                bcmerror = BCME_NOTAP;
#endif /* AP */
#ifndef STA
	        if (!val)
	                bcmerror = BCME_NOTSTA;
#endif /* STA */
	        /* APSTA mode takes precedence */
	        if (APSTA_ENAB(wlc->pub) && !val)
	                bcmerror = BCME_EPERM;


	        if (bcmerror || (AP_ENAB(wlc->pub) == bool_val))
	                break;

	        if (wasup) {
	                WL_APSTA_UPDN(("wl%d: WLC_SET_AP -> wl_down()\n", wlc->pub->unit));
	                wl_down(wlc->wl);
	        }

	        wlc->pub->_ap = bool_val;
		bcmerror = wlc_bsscfg_reinit(wlc, bsscfg, bool_val);
		if (bcmerror)
			break;

	        wlc_ap_upd(wlc, bsscfg);

	        /* always turn off WET when switching mode */
	        wlc->wet = FALSE;
	        /* always turn off MAC_SPOOF when switching mode */
	        wlc->mac_spoof = FALSE;

	        if (wasup) {
	                WL_APSTA_UPDN(("wl%d: WLC_SET_AP -> wl_up()\n", wlc->pub->unit));
	                bcmerror = wl_up(wlc->wl);
	        }
#ifdef STA
	        wlc_radio_mpc_upd(wlc);
#endif /* STA */
	        break;
	}

	case WLC_GET_AP:
	        *pval = (int)AP_ENAB(wlc->pub);
	        break;

	case WLC_GET_EAP_RESTRICT:
		bcmerror = wlc_iovar_op(wlc, "eap_restrict", NULL, 0, arg, len, IOV_GET, wlcif);
		break;

	case WLC_SET_EAP_RESTRICT:
		bcmerror = wlc_iovar_op(wlc, "eap_restrict", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	case WLC_SCB_DEAUTHENTICATE:
		/* Supply a reason in val */
		val = DOT11_RC_INACTIVITY;
		/* Fall through */

	case WLC_SCB_DEAUTHENTICATE_FOR_REASON:
		if (cmd == WLC_SCB_DEAUTHENTICATE_FOR_REASON) {
			/* point arg at MAC addr */
			if (len < (int)sizeof(scb_val_t)) {
				bcmerror = BCME_BUFTOOSHORT;
				break;
			}
			arg = &((scb_val_t *)arg)->ea;
			/* reason stays in `val' */
		}
#ifdef STA
		if (BSSCFG_STA(bsscfg)) {
			struct ether_addr *apmac = (struct ether_addr *)arg;
			if (ETHER_ISMULTI(apmac)) {
				WL_ERROR(("wl%d: bc/mc deauth%s on STA BSS?\n", wlc->pub->unit,
					(cmd == WLC_SCB_DEAUTHENTICATE) ? "" : "_reason"));
				apmac = &bsscfg->BSSID;
			} else if ((scb = wlc_scbfind(wlc, bsscfg, (struct ether_addr *)arg)) &&
			           (scb->flags & SCB_MYAP))
				wlc_scb_resetstate(scb);
			if (bsscfg->BSS) {
				wlc_disassociate_client(bsscfg, FALSE);
				if (scb)
					(void)wlc_senddeauth(wlc, bsscfg, scb, apmac, apmac,
					                     &bsscfg->cur_etheraddr, (uint16)val);
				WL_APSTA_UPDN(("wl%d: SCB DEAUTH: wlc_bsscfg_disable()\n",
				            wlc->pub->unit));
				wlc_bsscfg_disable(wlc, bsscfg);
			}
#if defined(IBSS_PEER_MGMT)
			else if (IBSS_PEER_MGMT_ENAB(wlc->pub)) {
				(void)wlc_senddeauth(wlc, bsscfg, scb, arg, &bsscfg->BSSID,
				                     &bsscfg->cur_etheraddr, (uint16)val);
				if (scb) {
					wlc_scb_disassoc_cleanup(wlc, scb);
					SCB_UNSET_IBSS_PEER(scb);
				}
			}
#endif /* defined(IBSS_PEER_MGMT) */
			break;
		}
#endif /* STA */
		/* fall thru */
	case WLC_SCB_AUTHORIZE:
	case WLC_SCB_DEAUTHORIZE: {
		uint32 flag;
		int rc = val;
		bool enable = (cmd == WLC_SCB_AUTHORIZE);

		/* ethernet address must be specified - otherwise we assert in dbg code  */
		if (!arg) {
			bcmerror = BCME_BADARG;
			break;
		}

		if (cmd == WLC_SCB_AUTHORIZE || cmd == WLC_SCB_DEAUTHORIZE)
			flag = AUTHORIZED;
		else
			flag = AUTHENTICATED;

		/* (de)authorize/authenticate all stations in this BSS */
		if (ETHER_ISBCAST(arg)) {

			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				/* skip if the scb is not valid or is WDS */
				if (SCB_LEGACY_WDS(scb))
					continue;

				/* don't authorize an unassociated scb */
				if (flag == AUTHORIZED && !SCB_ASSOCIATED(scb))
					continue;

				/* don't deauthenticate an unauthenticated scb */
				if (flag == AUTHENTICATED && !SCB_AUTHENTICATED(scb))
					continue;

				wlc_scb_set_auth(wlc, bsscfg, scb, enable, flag, rc);
			}
		}
		/* (de)authorize/authenticate a single station */
		else if ((scb = wlc_scbfind(wlc, bsscfg, (struct ether_addr *) arg))) {
#ifdef BCMDBG
			if (scb->bsscfg != bsscfg) {
				const char *cmdname;
				char eabuf[ETHER_ADDR_STR_LEN];
				if (cmd == WLC_SCB_AUTHORIZE)
					cmdname = "WLC_SCB_AUTHORIZE";
				else if (cmd == WLC_SCB_DEAUTHORIZE)
					cmdname = "WLC_SCB_DEAUTHORIZE";
				else if (cmd == WLC_SCB_DEAUTHENTICATE)
					cmdname = "WLC_SCB_DEAUTHENTICATE";
				else cmdname = "WLC_SCB_DEAUTHENTICATE_FOR_REASON";
				WL_ERROR(("wl%d: %s on i/f %s for non-local scb %s\n",
					wlc->pub->unit, cmdname, wl_ifname(wlc->wl, wlcif->wlif),
					bcm_ether_ntoa((struct ether_addr*)arg, eabuf)));
			}
#endif /* BCMDBG */
			wlc_scb_set_auth(wlc, bsscfg, scb, enable, flag, rc);
#ifdef DWDS
			/* Create dwds interface for this authorized scb if security
			 * is enabled and security is using AES or TKIP ciphers.
			 */
			if (enable && DWDS_ENAB(bsscfg) &&
				SCB_DWDS_CAP(scb) && SCB_ASSOCIATED(scb) &&
				BSSCFG_AP(bsscfg) && (cmd == WLC_SCB_AUTHORIZE) &&
				(WSEC_TKIP_ENABLED(bsscfg->wsec) ||
				WSEC_AES_ENABLED(bsscfg->wsec)))
				wlc_wds_create(wlc, scb, WDS_DYNAMIC);
#endif /* DWDS */

		}
	}
		break;

#ifdef STA
	case WLC_GET_ATIM:
		if (bsscfg->associated)
			*pval = (int) current_bss->atim_window;
		else
			*pval = (int) wlc->default_bss->atim_window;
		break;

	case WLC_SET_ATIM:
		wlc->default_bss->atim_window = (uint32) val;
		break;
#endif /* STA */

	case WLC_GET_RSSI:
		if (!scb_val) {
			bcmerror = BCME_BADARG;
			break;
		}

		if (len == sizeof(scb_val_t)) {
			/* User *possibly* provided a MAC address to use for STA specific RSSI */
			if (!ETHER_ISMULTI(&scb_val->ea)) {
				scb = wlc_scbfind(wlc, bsscfg, &scb_val->ea);
				if (scb != NULL) {
					if (BSSCFG_AP(SCB_BSSCFG(scb))) {
						/* enable per-scb collection on first request */
						wlc_scb_rssi_update_enable(scb, TRUE,
							RSSI_UPDATE_FOR_WLC);
						scb_val->val = wlc_scb_rssi(scb);
					} else {
						/* IBSS or STA */
#if defined(WLOFFLD)
						/* Check if we have any pkt received in ast tick. */
						/* if not then return offload rssi value else */
						/* fall back to original value */
						if (WLOFFLD_CAP(wlc)) {
						if (wlc_ol_bcn_is_enable(wlc->ol) &&
						    (wlc->_wlc_rate_measurement->rxframes_rate <
							    MA_WINDOW_SZ)) {
								scb_val->val =
								    wlc_ol_rssi_get_value(wlc->ol);
								break;
						    }
						    wlc_ol_inc_rssi_cnt_host(wlc->ol);
						}
#endif
						scb_val->val = bsscfg->link->rssi;
					}
				} else if (BSSCFG_STA(bsscfg)) {
					/* IBSS or STA */
#if defined(WLOFFLD)
					/* Check if we have any packets received in ast tick. */
					/* if not then return offload engines rssi value else */
					/* fall back to original value */
					if (WLOFFLD_CAP(wlc)) {
						if (wlc_ol_bcn_is_enable(wlc->ol) &&
						    (wlc->_wlc_rate_measurement->rxframes_rate <
							    MA_WINDOW_SZ)) {
							scb_val->val =
								wlc_ol_rssi_get_value(wlc->ol);
							break;
						}
						wlc_ol_inc_rssi_cnt_host(wlc->ol);
					}
#endif
					scb_val->val = bsscfg->link->rssi;
				} else {
					bcmerror = BCME_NOTFOUND;
				}
			} else {
				/* User isn't allowed to ask for RSSI for Multicast address */
				bcmerror = BCME_BADARG;
			}
		} else if ((len == sizeof(int)) && (BSSCFG_STA(bsscfg))) {
			/* User may have asked for bsscfg->link->rssi with an int sized buff */
			/* IBSS or STA */
#if defined(WLOFFLD)
			/* Check if we have any packets received in ast tick. */
			/* if not then return offload engines rssi value else */
			/* fall back to original value */
			if (WLOFFLD_CAP(wlc)) {
			    if (wlc_ol_bcn_is_enable(wlc->ol)) {
				if (wlc->_wlc_rate_measurement->rxframes_rate < MA_WINDOW_SZ) {
					scb_val->val = wlc_ol_rssi_get_value(wlc->ol);
					break;
				}
			    }
			    wlc_ol_inc_rssi_cnt_host(wlc->ol);
			}
#endif
			scb_val->val = bsscfg->link->rssi;
		} else {
			bcmerror = BCME_BADARG;
		}
		break;

	case WLC_GET_PKTCNTS: {
		get_pktcnt_t *pktcnt = (get_pktcnt_t*)pval;
		if (WLC_UPDATE_STATS(wlc))
			wlc_statsupd(wlc);
		pktcnt->rx_good_pkt = WLCNTVAL(wlc->pub->_cnt->rxframe);
		pktcnt->rx_bad_pkt = WLCNTVAL(wlc->pub->_cnt->rxerror);
		pktcnt->tx_good_pkt = WLCNTVAL(wlc->pub->_cnt->txfrmsnt);
		pktcnt->tx_bad_pkt = WLCNTVAL(wlc->pub->_cnt->txerror) +
			WLCNTVAL(wlc->pub->_cnt->txfail);
		if (len >= (int)sizeof(get_pktcnt_t)) {
			/* Be backward compatible - only if buffer is large enough  */
			pktcnt->rx_ocast_good_pkt = WLCNTVAL(wlc->pub->_cnt->rxmfrmocast);
		}
		break;
	}

	case WLC_GET_WSEC:
		bcmerror = wlc_iovar_op(wlc, "wsec", NULL, 0, arg, len, IOV_GET, wlcif);
		break;

	case WLC_SET_WSEC:
		bcmerror = wlc_iovar_op(wlc, "wsec", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	case WLC_GET_WPA_AUTH:
		*pval = (int) bsscfg->WPA_auth;
		break;

	case WLC_SET_WPA_AUTH:
		bcmerror = wlc_iovar_op(wlc, "wpa_auth", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	case WLC_GET_BSS_INFO: {
		wl_bss_info_t *wl_bi;
		int datalen = sizeof(uint32) + sizeof(wl_bss_info_t);

		if (val > len)
			val = len;
		if (val < datalen) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		/* Update the noise value of the current_bss structure */
		current_bss->phy_noise = (int8)wlc_lq_noise_update_ma(wlc,
			wlc_phy_noise_avg(pi));

		/* convert the wlc_bss_info_t, writing into buffer */
		wl_bi = (wl_bss_info_t*)((char*)arg + sizeof(uint32));
		wlc_bss2wl_bss(wlc, current_bss, wl_bi, val - sizeof(uint32), FALSE);
		/* If the requested BSS config is an AP, get the SSID and BSSID
		 * from that configuration
		 */
		*pval = (int)datalen + wl_bi->ie_length; /* byte count for return */

		if (BSSCFG_AP(bsscfg)) {
			bzero(wl_bi->SSID, sizeof(wl_bi->SSID));
			wl_bi->SSID_len	= bsscfg->SSID_len;
			bcopy(bsscfg->SSID, wl_bi->SSID, wl_bi->SSID_len);
			bcopy(&bsscfg->BSSID, &wl_bi->BSSID, sizeof(wl_bi->BSSID));
		}
#if defined(WLOFFLD)
		/* Check if we have any packets received in ast tick. */
		/* if not then return offload engines rssi value else */
		/* fall back to original value */
		if (WLOFFLD_CAP(wlc)) {
			if (wlc_ol_bcn_is_enable(wlc->ol)) {
				if (wlc->_wlc_rate_measurement->rxframes_rate < MA_WINDOW_SZ) {
					wl_bi->RSSI = wlc_ol_rssi_get_value(wlc->ol);
					break;
				}
			}
			wlc_ol_inc_rssi_cnt_host(wlc->ol);
		}
#endif
		break;
	}

	case WLC_GET_BANDLIST:
		/* count of number of bands, followed by each band type */
		*pval++ = NBANDS(wlc);
		*pval++ = wlc->band->bandtype;
		if (NBANDS(wlc) > 1)
			*pval++ = wlc->bandstate[OTHERBANDUNIT(wlc)]->bandtype;
			break;

	case WLC_GET_BAND:
		*pval = wlc->bandlocked ? wlc->band->bandtype : WLC_BAND_AUTO;
		break;

	case WLC_SET_BAND:
		bcmerror = wlc_bandlock(wlc, (uint)val);
		break;

	case WLC_GET_PHYLIST:
	{
		uchar *cp = arg;
		uint phy_type;

		if (len < 3) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		if (NBANDS(wlc) == 1) {
			phy_type = wlc->band->phytype;
			*cp++ = (phy_type == PHY_TYPE_A) ? 'a' :
				(phy_type == PHY_TYPE_G) ? 'g' :
			        (phy_type == PHY_TYPE_LP) ? 'l' :
			        (phy_type == PHY_TYPE_N) ? 'n' :
			        (phy_type == PHY_TYPE_AC) ? 'v' :
				(phy_type == PHY_TYPE_HT) ? 'h' :
			        (phy_type == PHY_TYPE_LCN) ? 'c' :
			        (phy_type == PHY_TYPE_LCN40) ? 'd' :
				'?';
		} else if (WLCISACPHY(wlc->band)) {
			*cp++ = 'v';
		} else if (WLCISHTPHY(wlc->band)) {
			*cp++ = 'h';
		} else if (WLCISNPHY(wlc->band)) {
			*cp++ = 'n';
		} else if (WLCISLPPHY(wlc->band)) {
			*cp++ = 'l';
		} else if (WLCISSSLPNPHY(wlc->band)) {
			*cp++ = 's';
		} else if (WLCISLCNPHY(wlc->band)) {
			*cp++ = 'c';
		} else if (WLCISLCN40PHY(wlc->band)) {
			*cp++ = 'd';
		} else {
			*cp++ = 'g';
			*cp++ = 'a';
		}
		*cp = '\0';
		break;
	}

	case WLC_GET_SHORTSLOT:
		*pval = wlc->shortslot;
		break;

	case WLC_GET_SHORTSLOT_OVERRIDE:
		*pval = wlc->shortslot_override;
		break;

	case WLC_SET_SHORTSLOT_OVERRIDE:
		if ((val != WLC_SHORTSLOT_AUTO) &&
		    (val != WLC_SHORTSLOT_OFF) &&
		    (val != WLC_SHORTSLOT_ON)) {
			bcmerror = BCME_RANGE;
			break;
		}

		wlc->shortslot_override = (int8)val;

		/* shortslot is an 11g feature, so no more work if we are
		 * currently on the 5G band
		 */
		if (BAND_5G(wlc->band->bandtype))
			break;

		if (wlc->pub->up && wlc->pub->associated) {
			/* let watchdog or beacon processing update shortslot */
		} else if (wlc->pub->up) {
			/* unassociated shortslot is off */
			wlc_switch_shortslot(wlc, FALSE);
		} else {
			/* driver is down, so just update the wlc_info value */
			if (wlc->shortslot_override == WLC_SHORTSLOT_AUTO) {
				wlc->shortslot = FALSE;
			} else {
				wlc->shortslot = (wlc->shortslot_override == WLC_SHORTSLOT_ON);
			}
		}

		break;

	case WLC_GET_GMODE:
		if (wlc->band->bandtype == WLC_BAND_2G)
			*pval = wlc->band->gmode;
		else if (NBANDS(wlc) > 1)
			*pval = wlc->bandstate[OTHERBANDUNIT(wlc)]->gmode;
		break;

	case WLC_SET_GMODE:
		if (!wlc->pub->associated)
			bcmerror = wlc_set_gmode(wlc, (uint8)val, TRUE);
		else {
			bcmerror = BCME_ASSOCIATED;
			break;
		}
		break;

	case WLC_SET_SUP_RATESET_OVERRIDE: {
		wlc_rateset_t rs, new;

		/* copyin */
		if (len < (int)sizeof(wl_rateset_t)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		STATIC_ASSERT(sizeof(wl_rateset_t) <= sizeof(wlc_rateset_t));

		bzero(&rs, sizeof(wlc_rateset_t));
		bcopy((char*)arg, (char*)&rs, sizeof(wl_rateset_t));

		/* check for bad count value */
		if (rs.count > WLC_NUMRATES) {
			bcmerror = BCME_BADRATESET;	/* invalid rateset */
			break;
		}

		/* this command is only appropriate for gmode operation */
		if (!(wlc->band->gmode ||
		      ((NBANDS(wlc) > 1) && wlc->bandstate[OTHERBANDUNIT(wlc)]->gmode))) {
			bcmerror = BCME_BADBAND;	/* gmode only command when not in gmode */
			break;
		}

		/* check for an empty rateset to clear the override */
		if (rs.count == 0) {
			bzero(&wlc->sup_rates_override, sizeof(wlc_rateset_t));
			break;
		}

		bzero(&new, sizeof(wlc_rateset_t));
		/* validate rateset by comparing pre and post sorted against 11g hw rates */
		wlc_rateset_filter(&rs, &new, FALSE, WLC_RATES_CCK_OFDM, RATE_MASK,
		                   wlc_get_mcsallow(wlc, NULL));
		wlc_rate_hwrs_filter_sort_validate(&new, &cck_ofdm_rates, FALSE,
			wlc->stf->txstreams);
		if (rs.count != new.count) {
			bcmerror = BCME_BADRATESET;	/* invalid rateset */
			break;
		}

		/* apply new rateset to the override */
		bcopy((char*)&new, (char*)&wlc->sup_rates_override, sizeof(wlc_rateset_t));

		/* update bcn and probe resp if needed */
		if (wlc->pub->up && AP_ENAB(wlc->pub) && wlc->pub->associated) {
			WL_APSTA_BCN(("wl%d: Calling update from SET_SUP_RATESET_OVERRIDE\n",
			            WLCWLUNIT(wlc)));
			WL_APSTA_BCN(("wl%d: SET_SUP_RATESET_OVERRIDE -> wlc_update_beacon()\n",
			            WLCWLUNIT(wlc)));
			wlc_update_beacon(wlc);
			wlc_update_probe_resp(wlc, TRUE);
		}
		break;
	}

	case WLC_GET_SUP_RATESET_OVERRIDE:
		/* this command is only appropriate for gmode operation */
		if (!(wlc->band->gmode ||
		      ((NBANDS(wlc) > 1) && wlc->bandstate[OTHERBANDUNIT(wlc)]->gmode))) {
			bcmerror = BCME_BADBAND;	/* gmode only command when not in gmode */
			break;
		}
		if (len < (int)sizeof(wl_rateset_t)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		bcopy((char*)&wlc->sup_rates_override, (char*)arg, sizeof(wl_rateset_t));

		break;

#ifdef STA
	case WLC_SET_ASSOC_PREFER:
		if (!VALID_BAND(wlc, val)) {
			bcmerror = BCME_BADBAND; /* bad value */
			break;
		}
		bsscfg->join_pref->band = (uint8)val;
		wlc_join_pref_band_upd(bsscfg);
		break;

	case WLC_GET_ASSOC_PREFER:
		if (bsscfg && bsscfg->join_pref)
			*pval = bsscfg->join_pref->band;
		else
			*pval = 0;
		break;

#endif /* STA */
	case WLC_GET_PRB_RESP_TIMEOUT:
		*pval = wlc->prb_resp_timeout;
		break;

	case WLC_SET_PRB_RESP_TIMEOUT:
		if (wlc->pub->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
		if (val < 0 || val >= 0xFFFF) {
			bcmerror = BCME_RANGE; /* bad value */
			break;
		}
		wlc->prb_resp_timeout = (uint16)val;
		break;

#ifndef OPENSRC_IOV_IOCTL
	case WLC_SET_COUNTRY:
		bcmerror = wlc_iovar_op(wlc, "country", NULL, 0, arg, len, IOV_SET, wlcif);
		break;
#endif /* OPENSRC_IOV_IOCTL */

	case WLC_GET_COUNTRY:
		if (len < WLC_CNTRY_BUF_SZ) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		bcopy(wlc_channel_country_abbrev(wlc->cmi), (char*)arg, WLC_CNTRY_BUF_SZ);
		break;


#ifdef WLCQ
	case WLC_GET_CHANNEL_QA:
		*pval = wlc->channel_quality;
		break;

	case WLC_START_CHANNEL_QA:
		if (!wlc->pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		if (!WLCISGPHY(wlc->band) && !WLCISNPHY(wlc->band)) {
			bcmerror = BCME_BADBAND;
			break;
		}

		bcmerror = wlc_lq_channel_qa_start(wlc);
		break;
#endif /* WLCQ */

	/* NOTE that this only returns valid 20MHZ channels */
	case WLC_GET_VALID_CHANNELS: {
		wl_uint32_list_t *list = (wl_uint32_list_t *)arg;
		uint8 channels[MAXCHANNEL];
		uint count;

		/* make sure the io buffer has at least room for a uint32 count */
		if (len < (int)sizeof(uint32)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		/* trim count to the buffer size in case it is bogus */
		if (list->count > (len - sizeof(uint32))/sizeof(uint32))
			list->count = (len - sizeof(uint32))/sizeof(uint32);

		/* find all valid channels */
		count = 0;
		for (i = 0; i < MAXCHANNEL; i++) {
			if (wlc_valid_chanspec_db(wlc->cmi, CH20MHZ_CHSPEC(i)))
				channels[count++] = (uint8)i;
		}

		/* check for buffer size */
		if (list->count < count) {
			/* too short, need this much */
			list->count = count;
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		/* return the valid channels */
		for (i = 0; i < count; i++) {
			list->element[i] = channels[i];
		}
		list->count = count;
		break;
	}

	case WLC_GET_CHANNELS_IN_COUNTRY:
		bcmerror = wlc_get_channels_in_country(wlc, arg);
		break;

	case WLC_GET_COUNTRY_LIST:
		bcmerror = wlc_get_country_list(wlc, arg);
		break;

	case WLC_SET_BAD_FRAME_PREEMPT:
		wlc->pub->bf_preempt_4306 = bool_val;
		wlc_phy_bf_preempt_enable(pi, bool_val);
		break;

	case WLC_GET_BAD_FRAME_PREEMPT:
		*pval = wlc->pub->bf_preempt_4306 ? 1 : 0;
		break;
#ifdef WET
	case WLC_GET_WET:
		bcmerror = wlc_iovar_op(wlc, "wet", NULL, 0, arg, len, IOV_GET, wlcif);
		break;

	case WLC_SET_WET:
		bcmerror = AP_ONLY(wlc->pub) ?
		        BCME_BADARG :
		        wlc_iovar_op(wlc, "wet", NULL, 0, arg, len, IOV_SET, wlcif);
		break;
#endif	/* WET */

#ifndef BCMRECLAIM
	case WLC_INIT:
		wlc_fatal_error(wlc);
		break;
#endif

	case WLC_SET_VAR:
	case WLC_GET_VAR: {
		char *name;
		/* validate the name value */
		name = (char*)arg;
		for (i = 0; i < (uint)len && *name != '\0'; i++, name++)
			;

		if (i == (uint)len) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		i++; /* include the null in the string length */

		if (cmd == WLC_GET_VAR) {
			bcmerror = wlc_iovar_op(wlc, arg, (void*)((int8*)arg + i), len - i, arg,
				len, IOV_GET, wlcif);
		}
		else
			bcmerror = wlc_iovar_op(wlc, arg, NULL, 0, (void*)((int8*)arg + i), len - i,
				IOV_SET, wlcif);

		break;
	}

	case WLC_SET_WSEC_PMK:
#if defined(BCMSUP_PSK) || defined(WLFBT)
		/* Use the internal supplicant if it is enabled else use the FBT one */
#ifdef BCMSUP_PSK
		if (SUP_ENAB(wlc->pub) && BSSCFG_STA(bsscfg) &&
			BSS_SUP_ENAB_WPA(wlc->idsup, bsscfg)) {
			bcmerror = wlc_sup_set_pmk(wlc->idsup, bsscfg, (wsec_pmk_t *)pval,
				bsscfg->associated);
			break;
		} else
#endif /* BCMSUP_PSK */
#ifdef WLFBT
			if (WLFBT_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
				bcmerror = wlc_fbt_set_pmk(wlc->fbt, bsscfg, (wsec_pmk_t *)pval,
					bsscfg->associated);
				break;
			}
#endif /* WLFBT */
#endif /* BCMSUP_PSK || WLFBT */
#ifdef BCMAUTH_PSK
		if (BCMAUTH_PSK_ENAB(wlc->pub) && BSSCFG_AP(bsscfg)) {
			bcmerror = wlc_auth_set_pmk(bsscfg->authenticator, (wsec_pmk_t *)pval);
			break;
		}
#endif /* BCMAUTH_PSK */
		bcmerror = BCME_UNSUPPORTED;
		break;

#if	defined(WLTEST)
	case WLC_NVRAM_GET:
		bcmerror = wlc_iovar_op(wlc, "nvram_get", arg, len, arg, len, IOV_GET, wlcif);
		break;

#endif 

#ifdef BCMNVRAMW
	case WLC_OTPW:
	case WLC_NVOTPW: {
		if (len & 1) {
			bcmerror = BCME_BADARG;
			break;
		}
		bcmerror = wlc_iovar_op(wlc, (cmd == WLC_OTPW) ? "otpw" : "nvotpw",
			NULL, 0, arg, len, IOV_SET, wlcif);

		break;
	}
#endif /* BCMNVRAMW */

	case WLC_GET_BANDSTATES: {
		wl_band_t *wl_band = (wl_band_t*) pval;
		for (i = 0; i < MAXBANDS; i++, wl_band++) {
			wl_band->bandtype = (uint16) wlc->bandstate[i]->bandtype;
			wl_band->bandunit = (uint16) wlc->bandstate[i]->bandunit;
			wl_band->phytype = (uint16) wlc->bandstate[i]->phytype;
			wl_band->phyrev = (uint16) wlc->bandstate[i]->phyrev;
		}
		break;
	}
	case WLC_GET_WLC_BSS_INFO:
		memcpy(pval, current_bss, sizeof(wlc_bss_info_t));
		break;
	case WLC_GET_ASSOC_INFO:
		store32_ua((uint8 *)pval, as->req_len);
		pval ++;
		memcpy(pval, as->req, as->req_len);
		pval = (int *)((uint8 *)pval + as->req_len);
		store32_ua((uint8 *)pval, as->resp_len);
		pval ++;
		memcpy(pval, as->resp, as->resp_len);
		break;

	case WLC_GET_BSS_BCN_TS:
		if (wlc->pub->associated && current_bss && current_bss->bcn_prb) {
			memcpy(pval, current_bss->bcn_prb->timestamp,
				sizeof(current_bss->bcn_prb->timestamp));
		} else {
			bcmerror = BCME_NOTASSOCIATED;
		}
		break;
	case WLC_GET_BSS_WPA_RSN:
		memcpy(pval, &current_bss->wpa, sizeof(current_bss->wpa));
		break;
	case WLC_GET_BSS_WPA2_RSN:
		memcpy(pval, &current_bss->wpa2, sizeof(current_bss->wpa2));
		break;

#if defined(WL_EXPORT_CURPOWER)
	case WLC_CURRENT_PWR:
		bcmerror = wlc_iovar_op(wlc, "curpower", NULL, 0, arg, len, IOV_GET, wlcif);
		break;
#endif

	case WLC_TERMINATED:
#ifndef WLC_HIGH_ONLY
		si_watchdog_ms(wlc->pub->sih, 10);
#endif
		break;

#ifdef STA
	case WLC_GET_PHY_NOISE:
		*pval = wlc_lq_noise_update_ma(wlc, wlc_phy_noise_avg((wlc_phy_t *)WLC_PI(wlc)));
		break;
#endif

	case WLC_LAST:
	default:
		/* returns BCME_UNSUPPORTED */
		bcmerror = BCME_UNSUPPORTED;
		break;
	}
done:

	if (bcmerror) {
		if (VALID_BCMERROR(bcmerror))
			wlc->pub->bcmerror = bcmerror;
		else {
			bcmerror = 0;
		}

	}

	/* The sequence is significant.
	 *   Only if sbclk is TRUE, we can proceed with register access.
	 *   Even though ta_ok is TRUE, we still want to check(and clear) target abort
	 *   si_taclear returns TRUE if there was a target abort, In this case, ta_ok must be TRUE
	 *   to avoid assert
	 *   ASSERT and si_taclear are both under ifdef BCMDBG
	 */
#ifdef WLC_LOW
	/* BMAC_NOTE: for HIGH_ONLY driver, this seems being called after RPC bus failed */
	/* In hw_off condition, IOCTLs that reach here are deemed safe but taclear would
	 * certainly result in getting -1 for register reads. So skip ta_clear altogether
	 */
	if (!(wlc->pub->hw_off))
		ASSERT(wlc_bmac_taclear(wlc->hw, ta_ok) || !ta_ok);
#endif

	return (bcmerror);
}

#if defined(BCMDBG) || defined(WLTEST) || defined(BCMDBG_ERR) || defined(DBG_PHY_IOV)
/** consolidated register access ioctl error checking */
int
wlc_iocregchk(wlc_info_t *wlc, uint band)
{
	/* if band is specified, it must be the current band */
	if ((band != WLC_BAND_AUTO) && (band != (uint)wlc->band->bandtype))
		return (BCME_BADBAND);

	/* if multiband and band is not specified, band must be locked */
	if ((band == WLC_BAND_AUTO) && IS_MBAND_UNLOCKED(wlc))
		return (BCME_NOTBANDLOCKED);

	/* must have core clocks */
	if (!wlc->clk)
		return (BCME_NOCLK);

	return (0);
}
#endif 

#if defined(BCMDBG)
/** For some ioctls, make sure that the pi pointer matches the current phy */
int
wlc_iocpichk(wlc_info_t *wlc, uint phytype)
{
	if (wlc->band->phytype != phytype)
		return BCME_BADBAND;
	return 0;
}
#endif 

#if defined(BCMDBG) || defined(WLTEST) || defined(STA)
/**
 * Checking band specifications for ioctls.  See if there's a supplied band
 * arg and return it (ALL, band A, or band B) if it's acceptable (valid).
 * If no band is supplied, or if it is explicitly AUTO, return the "current
 * band" if that's definitive (single-band or band locked).  [AUTO on a
 * dual-band product without bandlocking is ambiguous, since the "current"
 * band may be temporary, e.g. during a roam scan.]
 */
int
wlc_iocbandchk(wlc_info_t *wlc, int *arg, int len, uint *bands, bool clkchk)
{
	uint band;

	/* get optional band */
	band = (len < (int)(2 * sizeof(int))) ? WLC_BAND_AUTO : arg[1];

	/* If band is not specified (AUTO), determine band to use.
	 * Only valid if single-band or bandlocked (else error).
	 */
	if (band == WLC_BAND_AUTO) {
		if (IS_MBAND_UNLOCKED(wlc))
			return (BCME_NOTBANDLOCKED);
		band = (uint) wlc->band->bandtype;
	}

	/* If band is specified, it must be either a valid band or "all" */
	if (band != WLC_BAND_ALL) {
		if ((band != WLC_BAND_2G) && (band != WLC_BAND_5G))
			return (BCME_BADBAND);

		if ((NBANDS(wlc) == 1) && (band != (uint) wlc->band->bandtype))
			return (BCME_BADBAND);
	}

	/* no need clocks */
	if (clkchk && !wlc->clk)
		return (BCME_NOCLK);

	if (bands)
		*bands = band;

	return BCME_OK;
}
#endif 

/** Look up the given var name in the given table */
const bcm_iovar_t*
wlc_iovar_lookup(const bcm_iovar_t *table, const char *name)
{
	const bcm_iovar_t *vi;
	const char *lookup_name;

	/* skip any ':' delimited option prefixes */
	lookup_name = strrchr(name, ':');
	if (lookup_name != NULL)
		lookup_name++;
	else
		lookup_name = name;

	ASSERT(table != NULL);

	for (vi = table; vi->name; vi++) {
		if (!strcmp(vi->name, lookup_name))
			return vi;
	}
	/* ran to end of table */

	return NULL; /* var name not found */
}

/** simplified integer get interface for common WLC_GET_VAR ioctl handler */
int
wlc_iovar_getint(wlc_info_t *wlc, const char *name, int *arg)
{
	return wlc_iovar_op(wlc, name, NULL, 0, arg, sizeof(int32), IOV_GET, NULL);
}

/** simplified integer set interface for common WLC_SET_VAR ioctl handler */
int
wlc_iovar_setint(wlc_info_t *wlc, const char *name, int arg)
{
	return wlc_iovar_op(wlc, name, NULL, 0, (void *)&arg, sizeof(arg), IOV_SET, NULL);
}

/** simplified int8 get interface for common WLC_GET_VAR ioctl handler */
int
wlc_iovar_getint8(wlc_info_t *wlc, const char *name, int8 *arg)
{
	int iovar_int;
	int err;

	err = wlc_iovar_op(wlc, name, NULL, 0, &iovar_int, sizeof(iovar_int), IOV_GET, NULL);
	if (!err)
		*arg = (int8)iovar_int;

	return err;
}

/** simplified bool get interface for common WLC_GET_VAR ioctl handler */
int
wlc_iovar_getbool(wlc_info_t *wlc, const char *name, bool *arg)
{
	int iovar_int;
	int err;

	err = wlc_iovar_op(wlc, name, NULL, 0, &iovar_int, sizeof(iovar_int), IOV_GET, NULL);
	if (!err)
		*arg = (bool)iovar_int;

	return err;
}

/**
 * register iovar table, watchdog and down handlers.
 * calling function must keep 'iovars' until wlc_module_unregister is called.
 * 'iovar' must have the last entry's name field being NULL as terminator.
 */
int
BCMNMIATTACHFN(WLC_MODULE_REGISTER)(wlc_pub_t *pub, const bcm_iovar_t *iovars,
	const char *name, void *hdl, iovar_fn_t i_fn,
	watchdog_fn_t w_fn, up_fn_t u_fn, down_fn_t d_fn
	WLC_MODULE_REG_PATCH_ARGS_DECL)
{
	wlc_info_t *wlc = (wlc_info_t *)pub->wlc;
	int i, found;

	ASSERT(name != NULL);
	ASSERT(i_fn != NULL || w_fn != NULL || u_fn != NULL || d_fn != NULL);

	/* find if module already registered */
	found = wlc_module_find(pub, name);
	if (found >= 0) {
		/* MUST be a call to register new instance */
		wlc->modulecb_data[found].hdl = hdl;
		wlc->modulecb[found].ref_cnt++;
		return BCME_OK;
	}
	/* find an empty entry and just add, no duplication check! */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (wlc->modulecb[i].name[0] == '\0') {
			strncpy(wlc->modulecb[i].name, name, sizeof(wlc->modulecb[i].name) - 1);
			wlc->modulecb[i].iovars = iovars;
			wlc->modulecb[i].iovar_fn = i_fn;
			wlc->modulecb[i].watchdog_fn = w_fn;
			wlc->modulecb[i].up_fn = u_fn;
			wlc->modulecb[i].down_fn = d_fn;
#if defined(WLC_PATCH_IOCTL)
			wlc->modulecb[i].patch.iovars = patch_iovars;
			wlc->modulecb[i].patch.iovar_fn = patch_iovar_fn;
#endif /* WLC_PATCH_IOCTL */
			wlc->modulecb[i].ref_cnt++;
			wlc->modulecb_data[i].hdl = hdl;
			return BCME_OK;
		}
	}

	/* it is time to increase the capacity */
	ASSERT(i < wlc->pub->max_modules);
	return BCME_NORESOURCE;
}

/** unregister module callbacks */
int
BCMATTACHFN(wlc_module_unregister)(wlc_pub_t *pub, const char *name, void *hdl)
{
	wlc_info_t *wlc;
	int i;

	if (pub == NULL)
		return BCME_NOTFOUND;

	wlc = (wlc_info_t *)pub->wlc;
	if (wlc == NULL)
		return BCME_NOTFOUND;

	ASSERT(name != NULL);

	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (!strcmp(wlc->modulecb[i].name, name) &&
		    (wlc->modulecb_data[i].hdl == hdl)) {
			wlc->modulecb[i].ref_cnt--;
			if (wlc->modulecb[i].ref_cnt == 0) {
				bzero(&wlc->modulecb[i], sizeof(modulecb_t));
				bzero(&wlc->modulecb_data[i], sizeof(modulecb_data_t));
			}
			return 0;
		}
	}

	/* table not found! */
	return BCME_NOTFOUND;
}

int
BCMNMIATTACHFN(wlc_module_find)(wlc_pub_t *pub, const char *name)
{
	wlc_info_t *wlc = (wlc_info_t *)pub->wlc;
	int i;
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (!strcmp(wlc->modulecb[i].name, name)) {
			return i;
		}
	}

	/* not registered yet */
	return BCME_NOTFOUND;
}
/* Add module IOCTL handler.
 * Specifies an IOCTL handler for the module
 *
 * Special case for (num_cmds == 0):
 * The ioctl table is not defined for the module (ioctls == NULL),
 * so the ioctl_fn handler will be called directly to determine
 * if an IOCTL cmd can be serviced.
 * ioctl_fn must return BCME_UNSUPPORTED if it cannot service an IOCTL cmd
 */
int
BCMNMIATTACHFN(wlc_module_add_ioctl_fn)(wlc_pub_t *pub, void *hdl,
	wlc_ioctl_fn_t ioctl_fn, int num_cmds, const wlc_ioctl_cmd_t *ioctls)
{
	wlc_info_t *wlc = (wlc_info_t *)pub->wlc;
	wlc_ioctl_cb_t *ptr, *prev;
	int err = BCME_OK;

	ASSERT(hdl != NULL);
	ASSERT(ioctl_fn != NULL);

	for (prev = NULL, ptr = wlc->ioctl_cb_head; ptr != NULL; ptr = ptr->next) {
		if (hdl == ptr->ioctl_hdl) {
			/* Already present, just update */
			break;
		}
		prev = ptr;
	}

	/* Check if we found an existing module */
	if (ptr == NULL) {
		/* Not in list, allocate new */
		if ((ptr = (wlc_ioctl_cb_t *)MALLOC(wlc->osh, sizeof(wlc_ioctl_cb_t))) == NULL) {
			err = BCME_NORESOURCE;
		} else {
			/* Put at end of the list */
			ptr->next = NULL;
			if (prev != NULL) {
				prev->next = ptr;
			} else {
				wlc->ioctl_cb_head = ptr;
			}
		}
	}

	if (ptr) {
		/* Update IOCTL info for module */
		ptr->ioctl_hdl = hdl;
		ptr->ioctl_fn = ioctl_fn;
		ptr->num_cmds = num_cmds;
		ptr->ioctl_cmds = ioctls;
	}

	return err;
}

/** Remove module IOCTL handler. */
int
BCMATTACHFN(wlc_module_remove_ioctl_fn)(wlc_pub_t *pub, void *hdl)
{
	wlc_info_t *wlc = (wlc_info_t *)pub->wlc;
	wlc_ioctl_cb_t *ptr, *prev;
	int err = BCME_NOTFOUND;

	ASSERT(hdl != NULL);

	for (prev = NULL, ptr = wlc->ioctl_cb_head; ptr != NULL; ptr = ptr->next) {
		if (hdl == ptr->ioctl_hdl) {
			/* Remove entry */
			if (prev != NULL) {
				prev->next = ptr->next;
			} else {
				wlc->ioctl_cb_head = ptr->next;
			}
			MFREE(wlc->osh, ptr, sizeof(wlc_ioctl_cb_t));
			err = BCME_OK;
			break;
		}
		prev = ptr;
	}

	/* Not found! */
	return err;
}

/** Check module IOCTL parameters. */
static int
wlc_module_ioctl_check(wlc_info_t *wlc, const wlc_ioctl_cmd_t *ci,
	void *arg, int len, wlc_if_t *wlcif)
{
	int err = BCME_OK;
	uint16 flags = ci->flags;

	/* Found module, check argument length */
	if (len < ci->min_len)
		err = BCME_BADARG;
	/* Check flags */
	else if ((flags & WLC_IOCF_DRIVER_DOWN) && wlc->pub->up)
		err = BCME_NOTDOWN;
	else if ((flags & WLC_IOCF_DRIVER_UP) && !wlc->pub->up)
		err = BCME_NOTUP;
	else if ((flags & WLC_IOCF_FIXED_BAND) && IS_MBAND_UNLOCKED(wlc))
		err = BCME_NOTBANDLOCKED;
	else if ((flags & WLC_IOCF_CORE_CLK) && !wlc->clk)
		err = BCME_NOCLK;

#if defined(OPENSRC_IOV_IOCTL) && !defined(BCMDBG)
	else if ((flags & IOVF_OPEN_ALLOW) == 0)
		err = BCME_UNSUPPORTED;
#endif

#ifdef WLTEST_DISABLED
	else if (flags & WLC_IOCF_MFG)
		err = BCME_UNSUPPORTED;
#endif

	else if ((flags & (WLC_IOCF_BSSCFG_STA_ONLY | WLC_IOCF_BSSCFG_AP_ONLY)) != 0) {
		wlc_bsscfg_t *cfg;
		if ((cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif)) == NULL)
			err = BCME_NOTFOUND;

		else if ((flags & WLC_IOCF_BSSCFG_STA_ONLY) && !BSSCFG_STA(cfg))
			err = BCME_NOTSTA;
		else if ((flags & WLC_IOCF_BSSCFG_AP_ONLY) && !BSSCFG_AP(cfg))
			err = BCME_NOTAP;
	}

	return err;
}

/** Perform module IOCTL handling. */
static int
wlc_module_do_ioctl(wlc_info_t *wlc, int cmd, void *arg, int len, wlc_if_t *wlcif)
{
	wlc_ioctl_cb_t *ptr;
	int i;
	int err = BCME_UNSUPPORTED;
	const wlc_ioctl_cmd_t *ci = NULL;

	for (ptr = wlc->ioctl_cb_head; ptr != NULL; ptr = ptr->next) {
		if (ptr->num_cmds == 0) {
			/* Dispatch to module IOCTL handler */
			err = ptr->ioctl_fn(ptr->ioctl_hdl, cmd, arg, len, wlcif);
			if (err == BCME_UNSUPPORTED) {
				/* IOCTL not serviced, go to next */
				continue;
			} else {
				/* IOCTL was serviced, done */
				break;
			}
		}
		for (i = 0; i < ptr->num_cmds; i++) {
			if (cmd == (int)ptr->ioctl_cmds[i].cmd) {
				/* Found module */
				ci = &ptr->ioctl_cmds[i];
				break;
			}
		}
		/* Check if we found a module handler */
		if (i < ptr->num_cmds) {
			/* Check arg and flags */
			ASSERT(ci != NULL);
			err = wlc_module_ioctl_check(wlc, ci, arg, len, wlcif);
			if (err == BCME_OK) {
				/* Dispatch to IOCTL handler */
				err = ptr->ioctl_fn(ptr->ioctl_hdl, cmd, arg, len, wlcif);
			}
			break;
		}
	}

	/* wlc tables have been exhausted but no match has been found! */
	if (ptr == NULL) {
		uint16 tid;

		/* find the command and the table */
		if ((ci = wlc_iocv_high_find_ioc(wlc->iocvi, (uint16)cmd, &tid)) == NULL) {
			WL_NONE(("%s: wlc_iocv_high_find_ioc failed\n", __FUNCTION__));
			goto fail;
		}

		/* check arg and flags */
		if ((err = wlc_module_ioctl_check(wlc, ci, arg, len, wlcif)) != BCME_OK) {
			WL_NONE(("%s: wlc_module_ioctl_check failed\n", __FUNCTION__));
			goto fail;
		}

		/* forward ioctl to PHY */
		if ((err = wlc_iocv_high_fwd_ioc(wlc->iocvi, tid, ci, arg, len)) != BCME_OK) {
			WL_NONE(("%s: wlc_iocv_high_fwd_ioc failed\n", __FUNCTION__));
			goto fail;
		}
	}

fail:
	/* module IOCTL not found! */
	return err;
}

/** Register dump name and handlers. Calling function must keep 'dump function' */
int
BCMINITFN(wlc_dump_register)(wlc_pub_t *pub, const char *name, dump_fn_t dump_fn, void *dump_fn_arg)
{
	wlc_info_t *wlc = (wlc_info_t *)pub->wlc;
	dumpcb_t *ptr, *prev;

	ASSERT(name != NULL);
	ASSERT(dump_fn != NULL);

	for (prev = NULL, ptr = wlc->dumpcb_head; ptr != NULL; ptr = ptr->next) {
		if (!strcmp(name, ptr->name) && (dump_fn == ptr->dump_fn) &&
			(dump_fn_arg == ptr->dump_fn_arg)) {
			return BCME_OK;  /* Already present */
		}
		prev = ptr;
	}

	/* Not already in list; allocate new and put at end of list */
	if ((ptr = (dumpcb_t *)MALLOC(wlc->osh, sizeof(dumpcb_t))) == NULL) {
		return BCME_NORESOURCE;
	}
	ptr->name = name;
	ptr->dump_fn = dump_fn;
	ptr->dump_fn_arg = dump_fn_arg;
	ptr->next = NULL;

	if (prev != NULL) {
		prev->next = ptr;
	} else {
		wlc->dumpcb_head = ptr;
	}

	return BCME_OK;
}


/** Write WME tunable parameters for retransmit/max rate from wlc struct to ucode */
static void
wlc_wme_retries_write(wlc_info_t *wlc)
{
	int ac;

	/* Need clock to do this */
	if (!wlc->clk)
		return;

	for (ac = 0; ac < AC_COUNT; ac++) {
		wlc_write_shm(wlc, M_AC_TXLMT_ADDR(ac), wlc->wme_retries[ac]);
	}
}

#if defined(WLTEST)
/** Get chip manufacture information */
static int
wlc_manf_info_get(wlc_info_t *wlc, char *buf, int len, struct wlc_if *wlcif)
{
	uint16 otp_data[MANF_INFO_LEN]; /* 2 rows in OTP, 64 bits each */
	struct bcmstrbuf b;
	int bcmerror = BCME_OK;
	const struct wlc_otp_manf_info *pinfo;
	uint32 data, i;
	uint16 bit_pos, row_index = 0, bit_val;

	/* read 1st 2 row from OTP */
	for (i = 0; i < 8; i++) {
		bcmerror = wlc_iovar_op(wlc, "otprawr", &i, sizeof(uint32), &data,
			sizeof(uint32), IOV_GET, wlcif);
		if (bcmerror) {
			WL_ERROR(("fail to read OTP\n"));
			return BCME_ERROR;
		}
		otp_data[i] = (uint16)data;
		WL_TRACE(("\nOTP data%x: %x", i, otp_data[i]));
	}

	bcm_binit(&b, buf, len);
	pinfo = wlc_manf_info;
	while (pinfo->name != NULL) {
		data = 0;
		ASSERT(pinfo->bit_pos_end == pinfo->bit_pos_start + pinfo->len - 1);
		for (bit_pos = row_index * MANF_INFO_ROW_WIDTH + pinfo->bit_pos_start, i = 0;
		     i < pinfo->len; i++, bit_pos++) {
			if ((bit_pos >> 4) > MANF_INFO_LEN) {
				return BCME_ERROR;
			}
			/* extract the bit from the half word array */
			bit_val = (otp_data[bit_pos >> 4] >> (bit_pos & 0xf)) & 0x1;
			data |= (bit_val << i);
		}

		WL_TRACE(("%s : 0x%x\n", pinfo->name, data));
		if (pinfo->bit_pos_end == (MANF_INFO_ROW_WIDTH - 1)) {
			row_index++;
		}
		bcm_bprintf(&b, "%s: 0x%x\n", pinfo->name, data);
		pinfo++;
	}
	bcm_bprintf(&b, "Package ID:%x\n", wlc->pub->sih->chippkg);
	return bcmerror;
}
#endif 

/* Get or set an iovar.  The params/p_len pair specifies any additional
 * qualifying parameters (e.g. an "element index") for a get, while the
 * arg/len pair is the buffer for the value to be set or retrieved.
 * Operation (get/set) is specified by the last argument.
 * interface context provided by wlcif
 *
 * All pointers may point into the same buffer.
 */
int
wlc_iovar_op(wlc_info_t *wlc, const char *name,
	void *params, int p_len, void *arg, int len,
	bool set, struct wlc_if *wlcif)
{
	int err = 0;
	int val_size;
	const bcm_iovar_t *vi = NULL;
	uint32 actionid;
	int i = wlc->pub->max_modules;
	bool iocv = FALSE;
	uint16 tid;
	iovar_fn_t iovar_fn = NULL;
#if defined(WLC_PATCH_IOCTL)
	bool iovar_removed = FALSE;
#endif
#ifdef WLRSDB
	int32 wlc_idx = -1;
	int idx;
	bool do_iov_on_all_wlcs = 0;
	wlc_info_t *wlc_update;
#endif
	wlc_txc_info_t *txc = wlc->txc;
	ASSERT(name != NULL);

	ASSERT(len >= 0);

	/* Get MUST have return space */
	ASSERT(set || (arg && len));

	ASSERT(!(wlc->pub->hw_off && wlc->pub->up));

	/* Set does NOT take qualifiers */
	ASSERT(!set || (params == NULL && p_len == 0));
	WL_INFORM(("wl%d: Invoke IOVAR for:%s\n", wlc->pub->unit, name));

#if defined(WLC_PATCH_IOCTL)
	/* Find the given IOVAR name */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		/* Search patch IOVAR table first. If an IOVAR is flagged as REMOVED, keep
		 * searching in case it was actually moved to a different IOVAR table.
		 */
		if (wlc->modulecb[i].patch.iovars != NULL) {
			if ((vi = wlc_iovar_lookup(wlc->modulecb[i].patch.iovars, name))) {
				if (vi->flags != IOVF_REMOVED) {
					iovar_removed = FALSE;
					iovar_fn = wlc->modulecb[i].patch.iovar_fn;
					break;
				}
				iovar_removed = TRUE;
			}
		}
		/* If the IOVAR table is not patched, then search the original IOVAR table.
		 * This may be in ROM (if it is unchanged since tape-out) or may be in RAM
		 * (if it has changed, but patching is disabled).
		 */
		else if (wlc->modulecb[i].iovars != NULL) {
			if ((vi = wlc_iovar_lookup(wlc->modulecb[i].iovars, name))) {
				iovar_removed = FALSE;
				iovar_fn = wlc->modulecb[i].iovar_fn;
				break;
			}
		}
	}

	/* Bail if the IOVAR has been removed since ROM tape-out. */
	if (iovar_removed) {
		err = BCME_UNSUPPORTED;
		goto exit;
	}

	/* IOVAR name found. */
	if (i < wlc->pub->max_modules)
		goto doiovar;

	/* IOVAR name not found. Search ROM tables. */
#endif /* WLC_PATCH_IOCTL */

	/* find the given iovar name */
	for (i = 0; i < wlc->pub->max_modules; i ++) {
		if (wlc->modulecb[i].iovars == NULL) {
			continue;
		}

#if defined(WLC_PATCH_IOCTL)
		/* Only search tables in ROM that have been patched. Others have
		 * already been searched.*
		 */
		if (wlc->modulecb[i].patch.iovars == NULL)
			continue;
#endif /* WLC_PATCH_IOCTL */

		if ((vi = wlc_iovar_lookup(wlc->modulecb[i].iovars, name))) {
			iovar_fn = wlc->modulecb[i].iovar_fn;
			break;
		}
	}

	/* forward to IOCV iovar handler to try... */
	if (i >= wlc->pub->max_modules) {
		if ((vi = wlc_iocv_high_find_iov(wlc->iocvi, name, &tid)) != NULL) {
			WL_OID(("wl%d: tid %u iovar \"%s\" %s len %d p_len %d\n",
			        wlc->pub->unit, tid, name, set ? "set" : "get",
			        len, p_len));
			iocv = TRUE;
			goto check;
		}
	}

	/* iovar name not found */
	if (i >= wlc->pub->max_modules) {
		err = BCME_UNSUPPORTED;
		WL_INFORM(("wl%d: unable to find iovar \"%s\"\n", wlc->pub->unit, name));
		goto exit;
	}
	ASSERT(wlc->modulecb[i].iovar_fn != NULL);
check:

	/* check for alignment only for integer IOVAR types */
	if (!set && BCM_IOVT_IS_INT(vi->type) && (len == sizeof(int)) &&
		!ISALIGNED(arg, sizeof(int))) {
		WL_ERROR(("wl%d: %s unaligned get ptr for %s\n",
			wlc->pub->unit, __FUNCTION__, name));
		ASSERT(0);
	}

#ifdef WLC_PATCH_IOCTL
doiovar:
#endif

	if (!iocv) {
		WL_OID(("wl%d: table \"%s\" iovar \"%s\" %s len %d p_len %d\n", wlc->pub->unit,
			wlc->modulecb[i].name, name, (set ? "set" : "get"), len, p_len));
	}

	/* set up 'params' pointer in case this is a set command so that
	 * the convenience int and bool code can be common to set and get
	 */
	if (params == NULL) {
		params = arg;
		p_len = len;
	}

	if (vi->type == IOVT_VOID) {
		val_size = 0;
	} else if (vi->type == IOVT_BUFFER) {
		val_size = len;
	} else {
		/* all other types are integer sized */
		val_size = sizeof(int);
	}
	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);
#ifdef WLRSDB
	/* handle the "wlc:" prefix and assign the correct wlc */
	if (RSDB_ENAB(wlc->pub) && !strncmp("wlc:", name, 4)) {
		if (p_len < (int)sizeof(wlc_idx)) {
			err = BCME_BUFTOOSHORT;
			goto exit;
		}
		bcopy(params, &wlc_idx, sizeof(wlc_idx));
		WL_INFORM(("WLC prefix parsed:%d\n", wlc_idx));
		if (wlc_idx >= MAX_RSDB_MAC_NUM) {
			err = BCME_BADARG;
			goto exit;
		}
		wlc = wlc->cmn->wlc[wlc_idx];
		WL_INFORM(("WLC[%d]=%p\n", wlc_idx, wlc));
		ASSERT(wlc != NULL);
		if (wlc == NULL) {
			err = BCME_BADARG;
			goto exit;
		}
		/* adjust the name and i/o pointers to skip over the wlc index
		 * parameters
		 */
		name += sizeof("wlc:") - 1;
		params = (int8*)params + sizeof(wlc_idx);
		p_len -= sizeof(wlc_idx);
		if (IOV_ISSET(actionid)) {
			arg = (int8*)arg + sizeof(wlc_idx);
			len -= sizeof(wlc_idx);
		}
	}
#endif /* WLRSDB */

	/* handle the "bsscfg:" prefix and index arg for multi-bsscfg cases */
	if (!strncmp("bsscfg:", name, 7)) {
		int32 bsscfg_idx;
		wlc_bsscfg_t *bsscfg;

		if (p_len < (int)sizeof(bsscfg_idx)) {
			err = BCME_BUFTOOSHORT;
			goto exit;
		}

		bcopy(params, &bsscfg_idx, sizeof(bsscfg_idx));

		bsscfg = wlc_bsscfg_find(wlc, bsscfg_idx, &err);
#ifdef AP
		if (err == BCME_NOTFOUND) {
			if (IOV_ISSET(actionid)) {
				bsscfg = wlc_bsscfg_alloc(wlc, bsscfg_idx, 0, NULL, TRUE);
				if (bsscfg == NULL) {
					err = BCME_NOMEM;
				} else if ((err = wlc_bsscfg_init(wlc, bsscfg))) {
					WL_ERROR(("wl%d: wlc_bsscfg_init failed (%d)\n",
					          wlc->pub->unit, err));
					wlc_bsscfg_free(wlc, bsscfg);
				}
			}
		}
#endif /* AP */
		if (err) {
			goto exit;
		}

		/* adjust the name and i/o pointers to skip over the bsscfg index
		 * parameters
		 */
		name += sizeof("bsscfg:") - 1;
		params = (int8*)params + sizeof(bsscfg_idx);
		p_len -= sizeof(bsscfg_idx);
		if (IOV_ISSET(actionid)) {
			arg = (int8*)arg + sizeof(bsscfg_idx);
			len -= sizeof(bsscfg_idx);
		}

		/* use the wlcif from possibly a different bsscfg! */
		ASSERT(bsscfg != NULL);
		wlcif = bsscfg->wlcif;
		ASSERT(wlcif != NULL);
#ifdef WLRSDB
		if (RSDB_ENAB(wlc->pub)) {
			/* RSDB: Fixup wlc from bsscfg */
			/* This _MUST_ override the -W parameter as well! */
			wlc_info_t *owlc = wlc;
			wlc = bsscfg->wlc;
			if (owlc != wlc) {
				WL_INFORM(("*** RSDB *** wl%d ==> wl%d\n",
					owlc->pub->unit, wlc->pub->unit));
			}
		}
#endif /* WLRSDB */
	}

	if ((err = wlc_iovar_check(wlc, vi, arg, len, IOV_ISSET(actionid), wlcif)) != 0) {
		goto exit;
	}

#if defined(WLRSDB)
	if (RSDB_ENAB(wlc->pub))
		do_iov_on_all_wlcs = wlc_rsdb_chkiovar(vi, actionid, wlc_idx);
#endif /* WLRSDB  */

	/* Do the actual parameter implementation */
	if (!iocv) {
		ASSERT(i < wlc->pub->max_modules);
		ASSERT(iovar_fn != NULL);
#if defined(WLRSDB)
		if (RSDB_ENAB(wlc->pub) && do_iov_on_all_wlcs) {
			FOREACH_WLC(wlc->cmn, idx, wlc_update) {
				if (wlc_update) {
					WL_NONE(("wl%d: %s: calling IOVAR callback for \"%s\","
						" name %s\n", idx, __FUNCTION__,
						wlc->modulecb[i].name, name));
					err = iovar_fn(wlc_update->modulecb_data[i].hdl, vi,
						actionid, name, params, p_len, arg, len, val_size,
						wlcif);
					if (err != BCME_OK)
						break;
				}
			}
		} else
#endif /* WLRSDB */
		{
			WL_NONE(("wl%d: %s: calling IOVAR callback for \"%s\", name %s\n",
				wlc->pub->unit, __FUNCTION__, wlc->modulecb[i].name, name));
			err = iovar_fn(wlc->modulecb_data[i].hdl, vi, actionid,
				name, params, p_len, arg, len, val_size, wlcif);
		}

		goto exit;
	}

	/* forward to IOCV iovar handler to try... */
	err = wlc_iocv_high_fwd_iov(wlc->iocvi, tid, actionid, vi,
	                            params, p_len, arg, len, val_size);

	if (IOV_ISSET(actionid) && WLC_TXC_ENAB(wlc)) {
		/* if cache is enabled, invalidate - ensure use NOW of iovar setting */
		if (TXC_CACHE_ENAB(txc)) {
			wlc_txc_inv_all(txc);
		}
	}

exit:
	if (err && VALID_BCMERROR(err)) {
		wlc->pub->bcmerror = err;
	}
	WL_NONE(("wl%d: %s: BCME %d (%s) %s\n", wlc->pub->unit, __FUNCTION__, err,
		bcmerrorstr(err), name));
	return err;
}

/** This function is always forced to RAM to support WLTEST_DISABLED */
static int
wlc_iovar_filter(wlc_info_t *wlc, const bcm_iovar_t *vi)
{
	BCM_REFERENCE(wlc);
	BCM_REFERENCE(vi);

#ifdef WLTEST_DISABLED
	if (vi->flags & IOVF_MFG)
		return BCME_UNSUPPORTED;
#endif

	return 0;
}

int
wlc_iovar_check(wlc_info_t *wlc, const bcm_iovar_t *vi, void *arg, int len, bool set,
	wlc_if_t *wlcif)
{
	int err = BCME_OK;
	int32 int_val = 0;
	uint16 flags;

	/* check generic condition flags */
	if (set) {
		if (((vi->flags & IOVF_SET_DOWN) && wlc->pub->up) ||
		    ((vi->flags & IOVF_SET_UP) && !wlc->pub->up)) {
			err = (wlc->pub->up ? BCME_NOTDOWN : BCME_NOTUP);
		}
		else if ((vi->flags & IOVF_SET_BAND) && IS_MBAND_UNLOCKED(wlc)) {
			err = BCME_NOTBANDLOCKED;
		}
		else if ((vi->flags & IOVF_SET_CLK) && !wlc->clk) {
			err = BCME_NOCLK;
		}
	} else {
		if (((vi->flags & IOVF_GET_DOWN) && wlc->pub->up) ||
		    ((vi->flags & IOVF_GET_UP) && !wlc->pub->up)) {
			err = (wlc->pub->up ? BCME_NOTDOWN : BCME_NOTUP);
		}
		else if ((vi->flags & IOVF_GET_BAND) && IS_MBAND_UNLOCKED(wlc)) {
			err = BCME_NOTBANDLOCKED;
		}
		else if ((vi->flags & IOVF_GET_CLK) && !wlc->clk) {
			err = BCME_NOCLK;
		}
	}

#if defined(OPENSRC_IOV_IOCTL) && !defined(BCMDBG)
	if ((vi->flags & IOVF_OPEN_ALLOW) == 0)
		err = BCME_UNSUPPORTED;
#endif

	if (err)
		goto exit;

	if ((err = wlc_iovar_filter(wlc, vi)) < 0)
		goto exit;

	if ((flags = (vi->flags & (IOVF_BSSCFG_STA_ONLY | IOVF_BSSCFG_AP_ONLY |
		IOVF_BSS_SET_DOWN))) != 0) {
		wlc_bsscfg_t *cfg;
		if ((cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif)) == NULL) {
			err = BCME_NOTFOUND;
			goto exit;
		}
		if (set && ((flags & IOVF_BSS_SET_DOWN) != 0) && cfg->up) {
			err = BCME_NOTDOWN;
			goto exit;
		}

		flags &= ~IOVF_BSS_SET_DOWN;

		switch (flags) {
		case IOVF_BSSCFG_STA_ONLY:
			if (!BSSCFG_STA(cfg)) {
				err = BCME_NOTSTA;
				goto exit;
			}
			break;
		case IOVF_BSSCFG_AP_ONLY:
			if (!BSSCFG_AP(cfg)) {
				err = BCME_NOTAP;
				goto exit;
			}
			break;
		case IOVF_BSSCFG_STA_ONLY|IOVF_BSSCFG_AP_ONLY:
			err = BCME_UNSUPPORTED;
			goto exit;
		default:
			break; /* okay */
		}
	}

	/* length check on io buf */
	if ((err = bcm_iovar_lencheck(vi, arg, len, set)) < 0)
		goto exit;

	/* On set, check value ranges for integer types */
	if (set) {
		switch (vi->type) {
		case IOVT_BOOL:
		case IOVT_INT8:
		case IOVT_INT16:
		case IOVT_INT32:
		case IOVT_UINT8:
		case IOVT_UINT16:
		case IOVT_UINT32:
			bcopy(arg, &int_val, sizeof(int));
			err = wlc_iovar_rangecheck(wlc, int_val, vi);
			break;
		}
	}
exit:
	return err;
} /* wlc_iovar_op */

/** return number of packets txmodule has currently; 0 if no pktcnt function */
uint
wlc_txmod_get_pkts_pending(struct wlc_info *wlc, scb_txmod_t fid)
{
	if (wlc->txmod_fns[fid].pktcnt_fn) {
		return wlc->txmod_fns[fid].pktcnt_fn(wlc->txmod_fns[fid].ctx);
	} else {
		return 0;
	}
}

/** Return total transmit packets held by the driver */
uint
wlc_txpktcnt(struct wlc_info *wlc)
{
	uint pktcnt = 0;
	int i;
	wlc_bsscfg_t *cfg;

	/* Software txmods */
	/* Call any pktcnt handlers of registered modules only */
	for (i = TXMOD_START; i < TXMOD_LAST; i++) {
		pktcnt += wlc_txmod_get_pkts_pending(wlc, i);
	}
	/* Hardware FIFO */
	pktcnt += TXPKTPENDTOT(wlc);

	/* For all BSSCFG */
	FOREACH_BSS(wlc, i, cfg) {
#ifdef WL_BSSCFG_TX_SUPR
		if (cfg->psq == NULL)
			continue;
		pktcnt += (uint)pktq_len(cfg->psq);
#endif
	}

	return pktcnt;
}

#ifdef PKTQ_LOG
/** de-allocate pktq stats log data */
void wlc_pktq_stats_free(wlc_info_t* wlc, struct pktq* q)
{
	if (q->pktqlog) {
		uint32 prec_mask = q->pktqlog->_prec_log;
		uint32 index;

		for (index = 0; index < PKTQ_MAX_PREC; index++) {

			pktq_counters_t* counter = q->pktqlog->_prec_cnt[index];

			if (prec_mask & (1 << index)) {
				ASSERT(counter != 0);
				MFREE(wlc->osh, counter, sizeof(*counter));
			}
			else {
				ASSERT(counter == 0);
			}
		}
		MFREE(wlc->osh, q->pktqlog, sizeof(*q->pktqlog));
		q->pktqlog = 0;
	}
}

/** enable PKTQ_LOG for a precedence */
static pktq_counters_t*
wlc_txq_prec_log_enable(wlc_info_t* wlc, struct pktq* q, uint32 i)
{
	pktq_counters_t* new = MALLOC(wlc->osh, sizeof(*new));
	/* check not already allocated */
	ASSERT(q->pktqlog);
	ASSERT(q->pktqlog->_prec_cnt[i] == NULL);
	ASSERT(i < q->num_prec);

	if (new) {
		bzero(new, sizeof(*new));

		new->max_avail = pktq_pavail(q, i);
		new->max_used = pktq_plen(q, i);

		q->pktqlog->_prec_cnt[i] = new;
		q->pktqlog->_prec_log |= (1 << i);
	}

	/* if this is HI-PREC, ensure standard prec is enabled */
	if (i & 1) {
		i &= ~1;
		if (q->pktqlog->_prec_cnt[i] == NULL) {
			wlc_txq_prec_log_enable(wlc, q, i);
		}
	}
	return new;
}

/** format data according to v05 of the interface */
static void
wlc_txq_prec_dump(wlc_info_t* wlc, struct pktq* q, wl_iov_pktq_log_t* pktq_log, uint8 index,
                  bool clear, uint32 time, uint32 prec_mask, uint32 pps_time)
{
	uint32 i;
	uint32 result_mask = 0;

	ASSERT(pktq_log->version == 0x05);

	pktq_log->pktq_log.v05.num_prec[index] = 0;

	if (prec_mask & PKTQ_LOG_AUTO) {
		result_mask |= PKTQ_LOG_AUTO;
	}

	if (!q) {
		/* Handle non-existent queues */
		pktq_log->pktq_log.v05.counter_info[index] = result_mask;
	    return;
	}

	if (!q->pktqlog) {
		/* Dynamically allocate memory for pktq stats logging */
		q->pktqlog = MALLOC(wlc->osh, sizeof(*q->pktqlog));

		if (!q->pktqlog) {
			return;
		}
		bzero(q->pktqlog, sizeof(*q->pktqlog));
	}

	pktq_log->pktq_log.v05.num_prec[index] = (uint8)q->num_prec;

	/* this is the 'auto' prec setting */
	if (prec_mask & PKTQ_LOG_AUTO) {
		q->pktqlog->_prec_log |= PKTQ_LOG_AUTO;

		if (prec_mask & PKTQ_LOG_DEF_PREC) {
			/* if was default (all prec) and AUTO mode is set,
			 * we can remove the "all prec"
			 */
			prec_mask &= ~0xFFFF;
		}
		else if (prec_mask & 0xFFFF) {
			/* this was not default, there is actual specified
			 * prec_mask, so use it rather than automatic mode
			 */
			prec_mask &= ~PKTQ_LOG_AUTO;
		}
	}

	for (i = 0; i < q->num_prec; i++) {

		/* check pktq stats data was requested and also available */
		if ((((prec_mask & (1 << i)) || (prec_mask & PKTQ_LOG_AUTO)) &&
		                             (q->pktqlog->_prec_log & (1 << i)))) {
			pktq_log_counters_v05_t* counters =
			                    &pktq_log->pktq_log.v05.counters[index][i];
			pktq_counters_t*         pktq_count = q->pktqlog->_prec_cnt[i];

			ASSERT(pktq_count);

			result_mask |= (1 << i);

			counters->throughput     = pktq_count->throughput;

			counters->requested      = pktq_count->requested;
			counters->stored         = pktq_count->stored;
			counters->saved          = pktq_count->saved;
			counters->selfsaved      = pktq_count->selfsaved;
			counters->full_dropped   = pktq_count->full_dropped;
			counters->dropped        = pktq_count->dropped;
			counters->sacrificed     = pktq_count->sacrificed;
			counters->busy           = pktq_count->busy;
			counters->retry          = pktq_count->retry;
			counters->ps_retry       = pktq_count->ps_retry;
			counters->suppress       = pktq_count->suppress;
			counters->retry_drop     = pktq_count->retry_drop;
			counters->max_avail      = pktq_count->max_avail;
			counters->max_used       = pktq_count->max_used;
			counters->rtsfail        = pktq_count->rtsfail;
			counters->acked          = pktq_count->acked;
			counters->queue_capacity = pktq_pmax(q, i);
			counters->txrate_succ    = pktq_count->txrate_succ;
			counters->txrate_main    = pktq_count->txrate_main;
			counters->airtime        = pktq_count->airtime;

			counters->time_delta     = time - pktq_count->_logtime;

			if (clear) {
				bzero(pktq_count, sizeof(*pktq_count));
				pktq_count->max_avail = pktq_pavail(q, i);
				pktq_count->max_used = pktq_plen(q, i);
			}

			pktq_count->_logtime    = time;
		}
		/* else check pktq stats data was requested but not yet allocated */
		else if ((prec_mask & (1 << i)) && !(q->pktqlog->_prec_log & (1 << i))) {
			wlc_txq_prec_log_enable(wlc, q, i);
		}
	}
	pktq_log->pktq_log.v05.counter_info[index] = result_mask;
#if defined(BCMDBG) && defined(PSPRETEND)
	if (pps_time != (uint32)-1) {
		pktq_log->pktq_log.v05.pspretend_time_delta[index] =
		                       pps_time - q->pktqlog->pps_time;
		q->pktqlog->pps_time = pps_time;
	}
	else
#endif
	{
		pktq_log->pktq_log.v05.pspretend_time_delta[index] = (uint32)-1;
	}
}


/** wlc_pktq_stats - return data according to revision v05 of the API */
static int
wlc_pktq_stats(wl_iov_mac_full_params_t* full_params, uint8 params_version, struct bcmstrbuf* b,
               wl_iov_pktq_log_t* iov_pktq, wlc_info_t* wlc, wlc_bsscfg_t *cfg)
{
	wl_iov_mac_params_t* params	= &full_params->params;
	wl_iov_mac_extra_params_t* extra_params = &full_params->extra_params;

	uint32 addrs_todo = (params->num_addrs <= WL_IOV_MAC_PARAM_LEN ?
	                     params->num_addrs : WL_IOV_MAC_PARAM_LEN);
	struct ether_addr* ea = params->ea;

	wlc_txq_info_t* qi = wlc->active_queue;

	char* time = "";
	uint32 tsf_time;

#if defined(BCMDBG) && defined(WLC_LOW) && !defined(BCMDONGLEHOST) && \
	!defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
	if (wl_msg_level2 & WL_TIMESTAMP_VAL) {
		time = wlc_dbg_get_hw_timestamp();
	}
#endif

	tsf_time = (R_REG(wlc->osh, &wlc->regs->tsf_timerlow));

	while (addrs_todo) {
		pktq_get_fn_t* pktq_func = NULL;
		char* queue_type = "";
		uint8 index = (uint8)(ea - params->ea);
		char marker = '0' + index;
		uint32 prec_mask;

		if (params_version == 4) {
			prec_mask = extra_params->addr_info[index];
		}
		else {
			prec_mask = 0xFFFF;
		}

		/* bit 7 (0x80) is used as a flag, so mask it off */
		switch (params->addr_type[index] & 0x7F) {
			case 'C':
			{
				bcm_bprintf(b, "[%c]%sCommon queue:\n[%c]", marker, time, marker);
				wlc_txq_prec_dump(wlc, &qi->q, iov_pktq, index, TRUE, tsf_time,
				                  prec_mask, (uint32)-1);
				break;
			}
			default:
			{
				bcm_bprintf(b, "[%c]%sUnknown or missing addr prefix\n[%c]",
				                marker, time, marker);
				iov_pktq->pktq_log.v05.num_prec[index] = 0;
				break;
			}
#ifdef AP
			case 'P':
			{
				pktq_func = wlc_apps_prec_pktq;
				queue_type = "Power save queue";
				break;
			}
#endif

#ifdef WLAMPDU
			case 'A':
			{
				pktq_func = scb_ampdu_prec_pktq;
				queue_type = "AMPDU queue";
				break;
			}
#endif
#ifdef WLNAR
			case 'N':
			{
				pktq_func = wlc_nar_prec_pktq;
				queue_type = "NAR queue";
				break;
			}
#endif /* WLNAR */
		}

		if (pktq_func) {
			struct scb *scb = NULL;

			bool addr_OK = (!ETHER_ISMULTI(ea) && (scb = wlc_scbfind(wlc, cfg, ea)));

			if (addr_OK) {
				uint32  pps_time = (uint32)-1;
				bcm_bprintf(b, "[%c]%s%s "MACF"\n[%c]",
				            marker, time, queue_type,
				            ETHERP_TO_MACF(ea), marker);

#if defined(BCMDBG) && defined(PSPRETEND)
				if (SCB_PS_PRETEND_ENABLED(scb) &&
				             (params->addr_type[index] & 0x7F) == 'P') {
					pps_time = scb->ps_pretend_total_time_in_pps;
				}
#endif
				wlc_txq_prec_dump(wlc, pktq_func(wlc, scb), iov_pktq, index,
				                  TRUE, tsf_time, prec_mask, pps_time);
			}
			else {
				iov_pktq->pktq_log.v05.num_prec[index] = 0;
			}

		}

		addrs_todo--;
		ea++;
	}
	return BCME_OK;
}
#endif /* PKTQ_LOG */

#ifdef BCMDBG
#ifdef STA
static void
wlc_rpmt_timer_cb(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	uint32 to = 0;

	if (cfg->rpmt_n_st == 1) {
		wlc_set_pmstate(cfg, TRUE);
		to = cfg->rpmt_1_prd;
		cfg->rpmt_n_st = 0;
	}
	else if (cfg->rpmt_n_st == 0) {
		wlc_set_pmstate(cfg, FALSE);
		to = cfg->rpmt_0_prd;
		cfg->rpmt_n_st = 1;
	}
	if (to == 0)
		return;
	wlc_hrt_add_timeout(cfg->rpmt_timer, to, wlc_rpmt_timer_cb, cfg);
}
#endif /* STA */
#endif /* BCMDBG */

#ifdef WME
/** Convert discard policy AC bitmap to Precedence bitmap */
static uint16 wlc_convert_acbitmap_to_precbitmap(ac_bitmap_t acbitmap)
{
	uint16 prec_bitmap = 0;

	if (AC_BITMAP_TST(acbitmap, AC_BE))
		prec_bitmap |= WLC_PREC_BMP_AC_BE;

	if (AC_BITMAP_TST(acbitmap, AC_BK))
		prec_bitmap |= WLC_PREC_BMP_AC_BK;

	if (AC_BITMAP_TST(acbitmap, AC_VI))
		prec_bitmap |= WLC_PREC_BMP_AC_VI;

	if (AC_BITMAP_TST(acbitmap, AC_VO))
		prec_bitmap |= WLC_PREC_BMP_AC_VO;

	return prec_bitmap;
}
#endif /* WME */

/**
 * handler for iovar table wlc_iovars
 * IMPLEMENTATION NOTE: In order to avoid checking for get/set in each
 * iovar case, the switch statement maps the iovar id into separate get
 * and set values.  If you add a new iovar to the switch you MUST use
 * IOV_GVAL and/or IOV_SVAL in the case labels to avoid conflict with
 * another case.
 * Please use params for additional qualifying parameters.
 */
int
wlc_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_info_t *wlc = hdl;
	wlc_phy_t  *pi	= WLC_PI(wlc);
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	struct maclist *maclist;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;
	struct bcmstrbuf b;
#ifdef STA
	wlc_assoc_t *as;
	wlc_roam_t *roam;
	wlc_pm_st_t *pm;
#endif /* STA */
	wlc_bss_info_t *current_bss;
#ifdef DNGL_WD_KEEP_ALIVE
	uint32 delay;
#endif
#ifdef WME
	wlc_wme_t *wme;
#endif

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

#ifdef STA
	as = bsscfg->assoc;
	roam = bsscfg->roam;
	pm = bsscfg->pm;
#endif
	current_bss = bsscfg->current_bss;
#ifdef WME
	wme = bsscfg->wme;
#endif

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val2);

	/* Do the actual parameter implementation */
	switch (actionid) {

	case IOV_GVAL(IOV_5G_RATE): {
		uint32 wlrspec;
		ratespec_t ratespec = wlc->bandstate[BAND_5G_INDEX]->rspec_override;

		err = wlc_ratespec2wlrspec(ratespec, &wlrspec);
		*ret_int_ptr = wlrspec;
		break;
	}

	case IOV_SVAL(IOV_5G_RATE):
		err = wlc_set_iovar_ratespec_override(wlc, WLC_BAND_5G, (uint32)int_val, FALSE);
		break;

	case IOV_GVAL(IOV_5G_MRATE): {
		uint32 wlrspec;
		ratespec_t ratespec = wlc->bandstate[BAND_5G_INDEX]->mrspec_override;

		err = wlc_ratespec2wlrspec(ratespec, &wlrspec);
		*ret_int_ptr = wlrspec;
		break;
	}

	case IOV_SVAL(IOV_5G_MRATE):
		err = wlc_set_iovar_ratespec_override(wlc, WLC_BAND_5G, (uint32)int_val, TRUE);
		break;

	case IOV_GVAL(IOV_2G_RATE): {
		uint32 wlrspec;
		ratespec_t ratespec = wlc->bandstate[BAND_2G_INDEX]->rspec_override;

		err = wlc_ratespec2wlrspec(ratespec, &wlrspec);
		*ret_int_ptr = wlrspec;
		break;
	}

	case IOV_SVAL(IOV_2G_RATE):
		err = wlc_set_iovar_ratespec_override(wlc, WLC_BAND_2G, (uint32)int_val, FALSE);
		break;

	case IOV_GVAL(IOV_2G_MRATE): {
		uint32 wlrspec;
		ratespec_t ratespec = wlc->bandstate[BAND_2G_INDEX]->mrspec_override;

		err = wlc_ratespec2wlrspec(ratespec, &wlrspec);
		*ret_int_ptr = wlrspec;
		break;
	}

	case IOV_SVAL(IOV_2G_MRATE):
		err = wlc_set_iovar_ratespec_override(wlc, WLC_BAND_2G, (uint32)int_val, TRUE);
		break;

	case IOV_GVAL(IOV_QTXPOWER): {
		uint qdbm;
		bool override;

		if ((err = wlc_phy_txpower_get(pi, &qdbm, &override)) != BCME_OK)
			return err;

		/* Return qdbm units */
		*ret_int_ptr = qdbm | (override ? WL_TXPWR_OVERRIDE : 0);
		break;
	}

	/* As long as override is false, this only sets the *user* targets.
	   User can twiddle this all he wants with no harm.
	   wlc_phy_txpower_set() explicitly sets override to false if
	   not internal or test.
	*/
	case IOV_SVAL(IOV_QTXPOWER): {
		uint8 qdbm;
		bool override;
		ppr_t *txpwr;

		/* Remove override bit and clip to max qdbm value */
		qdbm = (uint8)MIN((int_val & ~WL_TXPWR_OVERRIDE), 0xff);
		/* Extract override setting */
		override = (int_val & WL_TXPWR_OVERRIDE) ? TRUE : FALSE;

#if defined WLTXPWR_CACHE && defined(WLC_LOW)
		wlc_phy_txpwr_cache_invalidate(wlc_phy_get_txpwr_cache(WLC_PI(wlc)));
#endif	/* WLTXPWR_CACHE */
		if ((txpwr = ppr_create(wlc->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
			break;
		}

		wlc_channel_reg_limits(wlc->cmi, wlc->chanspec, txpwr);
		ppr_apply_max(txpwr, WLC_TXPWR_MAX);
		err = wlc_phy_txpower_set(pi, qdbm, override, txpwr);
		ppr_delete(wlc->osh, txpwr);
		break;
	}

#ifdef WL11N
	case IOV_GVAL(IOV_ANTSEL_TYPE):
		*ret_int_ptr = (int32) wlc_antsel_antseltype_get(wlc->asi);
		break;
#endif /* WL11N */

#ifdef STA
	case IOV_GVAL(IOV_ASSOC_INFO): {
		wl_assoc_info_t *assoc_info = (wl_assoc_info_t *)arg;

		uint32 flags = (as->req_is_reassoc) ? WLC_ASSOC_REQ_IS_REASSOC : 0;
		bcopy(&flags, &assoc_info->flags, sizeof(uint32));

		bcopy(&as->req_len, &assoc_info->req_len, sizeof(uint32));
		bcopy(&as->resp_len, &assoc_info->resp_len, sizeof(uint32));

		if (as->req_len && as->req) {
			bcopy((char*)as->req, &assoc_info->req,
			      sizeof(struct dot11_assoc_req));
			if (as->req_is_reassoc)
				bcopy((char*)(as->req+1), &assoc_info->reassoc_bssid.octet[0],
					ETHER_ADDR_LEN);
		}
		if (as->resp_len && as->resp)
			bcopy((char*)as->resp, &assoc_info->resp,
			      sizeof(struct dot11_assoc_resp));
		break;
	}

	case IOV_SVAL(IOV_ASSOC_INFO): {
		wl_assoc_info_t *assoc_info = (wl_assoc_info_t *)arg;
		as->capability = assoc_info->req.capability;
		as->listen = assoc_info->req.listen;
		bcopy(assoc_info->reassoc_bssid.octet, bsscfg->assoc->bssid.octet,
			ETHER_ADDR_LEN);

		break;
	}

	case IOV_GVAL(IOV_ASSOC_REQ_IES): {
		/* if a reassoc then skip the bssid */
		char *req_ies = (char*)(as->req+1);
		int req_ies_len = as->req_len - sizeof(struct dot11_assoc_req);

		if (!as->req_len) {
			ASSERT(!as->req);
			break;
		}
		ASSERT(as->req);
		if (as->req_is_reassoc) {
			req_ies_len -= ETHER_ADDR_LEN;
			req_ies += ETHER_ADDR_LEN;
		}
		if (len < req_ies_len)
			return BCME_BUFTOOSHORT;
		bcopy((char*)req_ies, (char*)arg, req_ies_len);
		break;
	}

	case IOV_SVAL(IOV_ASSOC_REQ_IES): {
		if (len) {
			if (bsscfg->assoc->ie)
				MFREE(wlc->osh, bsscfg->assoc->ie, bsscfg->assoc->ie_len);
			bsscfg->assoc->ie_len = len;
			if (!(bsscfg->assoc->ie = MALLOC(wlc->osh, bsscfg->assoc->ie_len))) {
				WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				break;
			}
			bcopy((int8 *) arg, bsscfg->assoc->ie, bsscfg->assoc->ie_len);
		}
		break;
	}

	case IOV_GVAL(IOV_ASSOC_RESP_IES):
		if (!as->resp_len) {
			ASSERT(!as->resp);
			break;
		}
		ASSERT(as->resp);
		if (len < (int)(as->resp_len - sizeof(struct dot11_assoc_resp)))
			return BCME_BUFTOOSHORT;
		bcopy((char*)(as->resp+1), (char*)arg, as->resp_len -
		      sizeof(struct dot11_assoc_resp));
		break;

	case IOV_GVAL(IOV_JOIN_PREF):
		if ((err = wlc_join_pref_build(bsscfg, arg, len))) {
			goto exit;
		}
		break;

	case IOV_SVAL(IOV_JOIN_PREF):
		if ((err = wlc_join_pref_parse(bsscfg, arg, len))) {
			goto exit;
		}
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;

	case IOV_GVAL(IOV_FREQTRACK):
		*ret_int_ptr = (int32)wlc->freqtrack_override;
		break;

	case IOV_SVAL(IOV_FREQTRACK):
		if ((int_val >= FREQTRACK_AUTO) && (int_val <= FREQTRACK_OFF)) {
			wlc->freqtrack_override = (int8)int_val;
		}
		else {
			err = BCME_RANGE;
			goto exit;
		}
		break;
#endif /* STA */

#ifndef WLP2P_UCODE_MACP
	case IOV_GVAL(IOV_WME_AC_STA): {
		edcf_acparam_t acp_all[AC_COUNT];

		if (!WME_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (len < (int)sizeof(acp_all)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (BSSCFG_STA(bsscfg))
			wlc_edcf_acp_get_all(wlc, bsscfg, acp_all);
		if (BSSCFG_AP(bsscfg))
			wlc_edcf_acp_ad_get_all(wlc, bsscfg, acp_all);
		memcpy(arg, acp_all, sizeof(acp_all));    /* Copy to handle unaligned */
		break;
	}
	case IOV_GVAL(IOV_WME_AC_AP):
#endif /* WLP2P_UCODE_MACP */
	case IOV_GVAL(IOV_EDCF_ACP): {
		edcf_acparam_t acp_all[AC_COUNT];

		if (!WME_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (len < (int)sizeof(acp_all)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		wlc_edcf_acp_get_all(wlc, bsscfg, acp_all);
		memcpy(arg, acp_all, sizeof(acp_all));    /* Copy to handle unaligned */
		break;
	}
#ifdef AP
	case IOV_GVAL(IOV_EDCF_ACP_AD): {
		edcf_acparam_t acp_all[AC_COUNT];

		if (!WME_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (len < (int)sizeof(acp_all)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (!BSSCFG_AP(bsscfg)) {
			err = BCME_ERROR;
			break;
		}

		wlc_edcf_acp_ad_get_all(wlc, bsscfg, acp_all);
		memcpy(arg, acp_all, sizeof(acp_all));    /* Copy to handle unaligned */
		break;
	}
#endif /* AP */

#ifndef WLP2P_UCODE_MACP
	case IOV_SVAL(IOV_WME_AC_STA): {
		edcf_acparam_t acp;

		if (!WME_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		memcpy(&acp, arg, sizeof(acp));
		if (BSSCFG_STA(bsscfg))
			err = wlc_edcf_acp_set_one(wlc, bsscfg, &acp, TRUE);
		if (BSSCFG_AP(bsscfg))
			err = wlc_edcf_acp_ad_set_one(wlc, bsscfg, &acp);
		break;
	}
	case IOV_SVAL(IOV_WME_AC_AP):
#endif /* WLP2P_UCODE_MACP */
	case IOV_SVAL(IOV_EDCF_ACP): {
		edcf_acparam_t acp;

		if (!WME_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		memcpy(&acp, arg, sizeof(acp));
		err = wlc_edcf_acp_set_one(wlc, bsscfg, &acp, TRUE);
		break;
	}
#ifdef AP
	case IOV_SVAL(IOV_EDCF_ACP_AD): {
		edcf_acparam_t acp;

		if (!WME_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (!BSSCFG_AP(bsscfg)) {
			err = BCME_ERROR;
			break;
		}

		memcpy(&acp, arg, sizeof(acp));
		err = wlc_edcf_acp_ad_set_one(wlc, bsscfg, &acp);
		break;
	}
#endif /* AP */

	case IOV_GVAL(IOV_STA_INFO):
		err = wlc_sta_info(wlc, bsscfg, (struct ether_addr *)params, arg, len);
		break;

	case IOV_GVAL(IOV_CAP):
		wlc_cap(wlc, (char*)arg, len);
		break;

	case IOV_GVAL(IOV_WPA_AUTH):
		*((uint*)arg) = bsscfg->WPA_auth;
		break;

	case IOV_SVAL(IOV_WPA_AUTH):
#ifdef STA
		/* change of WPA_Auth modifies the PS_ALLOWED state */
		if (BSSCFG_STA(bsscfg)) {
			bool prev_psallowed = PS_ALLOWED(bsscfg);

			bsscfg->WPA_auth = (uint16)int_val;
#ifdef WLFBT
			if (WLFBT_ENAB(wlc->pub)) {
				/* Reset the current state so as to do initial FT association
				 * on a wl join command
				 */
				wlc_fbt_reset_current_akm(wlc->fbt, bsscfg);
			}
#endif /* WLFBT */
			if (prev_psallowed != PS_ALLOWED(bsscfg))
				wlc_set_pmstate(bsscfg, pm->PMenabled);

			/* Don't clear the WPA join preferences if they are already set */
			if (bsscfg->join_pref->wpas == 0)
			wlc_join_pref_reset(bsscfg);

		} else
#endif	/* STA */
#ifdef AP
		if (BSSCFG_AP(bsscfg)) {
			if (bsscfg->up)
				err = BCME_UNSUPPORTED;
			else
				bsscfg->WPA_auth = (uint16)int_val;
		} else
#endif	/* AP */
		bsscfg->WPA_auth = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_MALLOCED):
		*((uint*)arg) = MALLOCED(wlc->osh);
		break;

	case IOV_GVAL(IOV_PER_CHAN_INFO): {
		*ret_int_ptr = wlc_get_chan_info(wlc, (uint16)int_val);
		break;
	}

	case IOV_GVAL(IOV_VNDR_IE):
	case IOV_GVAL(IOV_IE): {
		uint32 pktflag;
		uint8 ie_type;

		if (actionid == IOV_GVAL(IOV_IE)) {
			ie_getbuf_t *ie_getbufp = (ie_getbuf_t *)params;
			if (p_len < sizeof(ie_getbuf_t)) {
				err = BCME_BUFTOOSHORT;
				break;
			}
			bcopy((uint8 *)&ie_getbufp->pktflag, &pktflag, sizeof(pktflag));
			ie_type = ie_getbufp->id;
		} else {
			pktflag = -1;
			ie_type = DOT11_MNG_PROPR_ID;
		}

		if (ie_type == DOT11_MNG_PROPR_ID) {
			vndr_ie_buf_t *vndr_ie_getbufp;
			/* Extract the vndr ie into a memory-aligned structure, then bcopy
			* the structure into the ioctl output buffer pointer 'arg'.
			* This extra copying is necessary in case 'arg' has an unaligned
			* address due to a "bsscfg:N" prefix.
			*/
			if (!(vndr_ie_getbufp = MALLOC(wlc->osh, len))) {
				WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				err = BCME_NOMEM;
				break;
			}
			err = wlc_vndr_ie_get(bsscfg, vndr_ie_getbufp, len, pktflag);
			bcopy(vndr_ie_getbufp, (uint8*)arg, len);
			MFREE(wlc->osh, vndr_ie_getbufp, len);
		} else {
			int ie_len = 0;
			uint8 *ie_data;

			ie_data = wlc_bsscfg_get_ie(wlc, bsscfg, ie_type);

			if (ie_data) {
				ie_len = ie_data[TLV_LEN_OFF] + TLV_HDR_LEN;
				if (len < ie_len) {
					err = BCME_BUFTOOSHORT;
					break;
				}
				bcopy(ie_data, (uint8*)arg, ie_len);
			} else {
				err = BCME_NOTFOUND;
			}
		}

		break;
	}
	case IOV_SVAL(IOV_VNDR_IE):
	case IOV_SVAL(IOV_IE): {
		int rem = len;
		bool bcn_upd = FALSE;
		bool prbresp_upd = FALSE;
		vndr_ie_setbuf_t *vndr_ie_bufp = (vndr_ie_setbuf_t *)arg;
		uint8 *ie_data;

		if (len < (int)sizeof(ie_setbuf_t) - 1) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		ie_data = (uint8 *)&vndr_ie_bufp->vndr_ie_buffer.vndr_ie_list->vndr_ie_data;

		if ((ie_data[TLV_TAG_OFF] != DOT11_MNG_PROPR_ID) &&
		            (actionid == IOV_SVAL(IOV_IE))) {
			/* none vendor IE case */
			int iecount, i, total_len;
			bcopy((uint8 *)&vndr_ie_bufp->vndr_ie_buffer.iecount,
				&iecount, sizeof(iecount));
			total_len = sizeof(ie_setbuf_t) - sizeof(ie_info_t);
			i = 0;
			while (i < iecount && err == BCME_OK) {
				total_len += (TLV_HDR_LEN + sizeof(uint32));
				if (len < total_len) {
					err = BCME_BUFTOOSHORT;
					break;
				}
				total_len += ie_data[TLV_LEN_OFF];
				if (len < total_len) {
					err = BCME_BUFTOOSHORT;
					break;
				}
				err = wlc_bsscfg_set_ie(wlc, bsscfg, ie_data,
					&bcn_upd, &prbresp_upd);
				ie_data += (ie_data[TLV_LEN_OFF] + TLV_HDR_LEN +
					sizeof(uint32));
				i++;
			}
		} else {
			/* vendor IE case */
			/* Make a memory-aligned bcopy of the arguments in case they start
			* on an unaligned address due to a "bsscfg:N" prefix.
			*/
			if (len < (int)sizeof(vndr_ie_setbuf_t) - 1) {
				err = BCME_BUFTOOSHORT;
				break;
			}
			if (!(vndr_ie_bufp = MALLOC(wlc->osh, len))) {
				WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				err = BCME_NOMEM;
				break;
			}
			bcopy((uint8*)arg, vndr_ie_bufp, len);

			do {
				int buf_len;
				int bcn_len, prbresp_len;

				if ((buf_len = wlc_vndr_ie_buflen(&vndr_ie_bufp->vndr_ie_buffer,
					rem - VNDR_IE_CMD_LEN, &bcn_len, &prbresp_len)) < 0) {
					err = buf_len;
					break;
				}

				if (rem < VNDR_IE_CMD_LEN + buf_len) {
					err = BCME_BUFTOOSHORT;
					break;
				}

				if (!bcn_upd && bcn_len > 0)
					bcn_upd = TRUE;
				if (!prbresp_upd && prbresp_len > 0)
					prbresp_upd = TRUE;

				if (!strcmp(vndr_ie_bufp->cmd, "add")) {
					err = wlc_vndr_ie_add(bsscfg,
						&vndr_ie_bufp->vndr_ie_buffer,
						rem - VNDR_IE_CMD_LEN);
				}
				else if (!strcmp(vndr_ie_bufp->cmd, "del")) {
					err = wlc_vndr_ie_del(bsscfg,
						&vndr_ie_bufp->vndr_ie_buffer,
						rem - VNDR_IE_CMD_LEN);
				}
				else {
					err = BCME_BADARG;
				}
				if ((err != BCME_OK))
					break;

				rem -= VNDR_IE_CMD_LEN + buf_len;
				if (rem == 0)
					break;

				memmove((uint8 *)vndr_ie_bufp,
					(uint8 *)vndr_ie_bufp + VNDR_IE_CMD_LEN + buf_len, rem);

				if ((rem < (int) sizeof(vndr_ie_setbuf_t) - 1) ||
					(strcmp(vndr_ie_bufp->cmd, "add") &&
					strcmp(vndr_ie_bufp->cmd, "del"))) {
					break;
				}

			}
			while (TRUE);
			MFREE(wlc->osh, vndr_ie_bufp, len);
		}

		if (err == BCME_OK && bsscfg->up) {
			if (bcn_upd)
				wlc_bss_update_beacon(wlc, bsscfg);
			if (prbresp_upd)
				wlc_bss_update_probe_resp(wlc, bsscfg, TRUE);
		}

		break;
	}

	case IOV_GVAL(IOV_WSEC):
		*((uint*)arg) = bsscfg->wsec;
		break;

	case IOV_SVAL(IOV_WSEC):
		err = wlc_wsec(wlc, bsscfg, (uint32)int_val);
		break;

	case IOV_GVAL(IOV_WSEC_RESTRICT):
		*((uint*)arg) = bsscfg->wsec_restrict;
		break;

	case IOV_SVAL(IOV_WSEC_RESTRICT):
		bsscfg->wsec_restrict = bool_val;
		break;

	case IOV_GVAL(IOV_FRAGTHRESH):
		*((uint*)arg) = wlc->usr_fragthresh;
		break;

	case IOV_SVAL(IOV_FRAGTHRESH): {
		int i;
		uint16 frag = (uint16)int_val;
		if (frag < DOT11_MIN_FRAG_LEN || frag > DOT11_MAX_FRAG_LEN) {
			err = BCME_RANGE;
			break;
		}

		wlc->usr_fragthresh = frag;
		for (i = 0; i < NFIFO; i++)
			wlc_fragthresh_set(wlc, i, frag);

#ifdef WLAMSDU_TX
		wlc_amsdu_agglimit_frag_upd(wlc->ami);
#endif
		break;
	}

	/* Former offset-based items */
	case IOV_GVAL(IOV_RTSTHRESH):
		*ret_int_ptr = (int32)wlc->RTSThresh;
		break;

	case IOV_SVAL(IOV_RTSTHRESH):
		wlc->RTSThresh = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_D11_AUTH):
		*ret_int_ptr = DOT11AUTH2WLAUTH(bsscfg);
		break;

	case IOV_SVAL(IOV_D11_AUTH):
		if (int_val == WL_AUTH_OPEN_SYSTEM ||
			int_val == WL_AUTH_SHARED_KEY) {
			bsscfg->auth = WLAUTH2DOT11AUTH(int_val);
			bsscfg->openshared = 0;
		} else {
#ifdef WL_AUTH_METHOD_SHARED_OPEN
			bsscfg->auth = DOT11_SHARED_KEY;
#else
			bsscfg->auth = DOT11_OPEN_SYSTEM;
#endif /* WL_AUTH_METHOD_SHARED_OPEN */
			bsscfg->openshared = 1;
		}
		break;

#ifdef STA
	case IOV_GVAL(IOV_STA_RETRY_TIME):
		*ret_int_ptr = (int32)wlc->as_info->sta_retry_time;
		break;

	case IOV_SVAL(IOV_STA_RETRY_TIME):
		if (int_val < 0 || int_val > WLC_STA_RETRY_MAX_TIME) {
			err = BCME_RANGE;
			break;
		}
		wlc->as_info->sta_retry_time = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_ASSOC_RETRY_MAX):
		*ret_int_ptr = (int32)as->retry_max;
		break;

	case IOV_SVAL(IOV_ASSOC_RETRY_MAX):
		as->retry_max = (uint8)int_val;
		break;
#ifdef WLSCANCACHE
	case IOV_GVAL(IOV_ASSOC_CACHE_ASSIST):
		*ret_int_ptr = (int32)wlc->_assoc_cache_assist;
		break;

	case IOV_SVAL(IOV_ASSOC_CACHE_ASSIST):
		wlc->_assoc_cache_assist = bool_val;
		break;
#endif /* WLSCANCACHE */
#endif /* STA */

	case IOV_SVAL(IOV_VLAN_MODE):
		if (int_val >= AUTO && int_val <= ON)
			wlc->vlan_mode = int_val;
		break;

	case IOV_GVAL(IOV_VLAN_MODE):
		*ret_int_ptr = wlc->vlan_mode;
		break;

#ifdef WME
	case IOV_GVAL(IOV_WME):
		*ret_int_ptr = (int32)wlc->pub->_wme;
		break;

	case IOV_SVAL(IOV_WME):
		if (int_val < AUTO || int_val > ON) {
			err = BCME_RANGE;
			break;
		}

		/* For AP, AUTO mode is same as ON */
		if (AP_ENAB(wlc->pub) && int_val == AUTO)
			wlc->pub->_wme = ON;
		else
			wlc->pub->_wme = int_val;

#ifdef STA
		/* If not in AUTO mode, PM is always allowed
		 * In AUTO mode, PM is allowed on for UAPSD enabled AP
		 */
		if (int_val != AUTO) {
			int idx;
			wlc_bsscfg_t *bc;

			FOREACH_BSS(wlc, idx, bc) {
				if (!BSSCFG_STA(bc))
					continue;
				bc->pm->WME_PM_blocked = FALSE;
			}
		}
#endif

		break;

	case IOV_GVAL(IOV_WME_BSS_DISABLE):
		*ret_int_ptr = ((bsscfg->flags & WLC_BSSCFG_WME_DISABLE) != 0);
		break;

	case IOV_SVAL(IOV_WME_BSS_DISABLE):
		WL_INFORM(("%s(): set IOV_WME_BSS_DISABLE to %s\n", __FUNCTION__,
			bool_val ? "TRUE" : "FALSE"));
		if (bool_val) {
			bsscfg->flags |= WLC_BSSCFG_WME_DISABLE;
		} else {
			bsscfg->flags &= ~WLC_BSSCFG_WME_DISABLE;
		}
		break;

	case IOV_GVAL(IOV_WME_NOACK):
		*ret_int_ptr = (int32)wme->wme_noack;
		break;

	case IOV_SVAL(IOV_WME_NOACK):
		wme->wme_noack = bool_val;
		break;

	case IOV_GVAL(IOV_WME_APSD):
		*ret_int_ptr = (int32)wme->wme_apsd;
		break;

	case IOV_SVAL(IOV_WME_APSD):
		wme->wme_apsd = bool_val;
		if (BSSCFG_AP(bsscfg) && wlc->clk) {
			wlc_bss_update_beacon(wlc, bsscfg);
			wlc_bss_update_probe_resp(wlc, bsscfg, TRUE);
		}
		break;

	case IOV_SVAL(IOV_SEND_FRAME): {
		bool status;
		osl_t *osh = wlc->osh;
		void *pkt;

		/* Reject runts and jumbos */
		if (p_len < ETHER_MIN_LEN || p_len > ETHER_MAX_LEN || params == NULL) {
			err = BCME_BADARG;
			break;
		}
		pkt = PKTGET(osh, p_len + TXOFF, TRUE);
		if (pkt == NULL) {
			err = BCME_NOMEM;
			break;
		}

		PKTPULL(osh, pkt, TXOFF);
		bcopy(params, PKTDATA(osh, pkt), p_len);
		PKTSETLEN(osh, pkt, p_len);
		status = wlc_sendpkt(wlc, pkt, wlcif);
		if (status)
			err = BCME_NORESOURCE;
		break;
	}

#ifdef STA
	case IOV_GVAL(IOV_WME_APSD_TRIGGER):
	        *ret_int_ptr = (int32)pm->apsd_trigger_timeout;
		break;

	case IOV_SVAL(IOV_WME_APSD_TRIGGER):
	        /*
		 * Round timeout up to an even number because we set
		 * the timer at 1/2 the timeout period.
		 */
		pm->apsd_trigger_timeout = ROUNDUP((uint32)int_val, 2);
		wlc_apsd_trigger_upd(bsscfg, TRUE);
		break;

	case IOV_GVAL(IOV_WME_TRIGGER_AC):
		*ret_int_ptr = (int32)wme->apsd_trigger_ac;
		break;

	case IOV_SVAL(IOV_WME_TRIGGER_AC):
		if (int_val <= AC_BITMAP_ALL)
			wme->apsd_trigger_ac = (ac_bitmap_t)int_val;
		else
			err = BCME_BADARG;
		break;

	case IOV_GVAL(IOV_WME_AUTO_TRIGGER):
		*ret_int_ptr = (int32)wme->apsd_auto_trigger;
		break;

	case IOV_SVAL(IOV_WME_AUTO_TRIGGER):
		wme->apsd_auto_trigger = bool_val;
		break;

	case IOV_SVAL(IOV_SEND_NULLDATA): {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif

		if (!bsscfg->BSS) {
			WL_ERROR(("wl%d.%d: %s: not an Infra STA\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), name));
			err = BCME_ERROR;
			break;
		}
		if (!bsscfg->associated) {
			WL_ERROR(("wl%d.%d: %s: not associated\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), name));
			err = BCME_NOTASSOCIATED;
			break;
		}

		/* transmit */
		if (!wlc_sendapsdtrigger(wlc, bsscfg)) {
			WL_ERROR(("wl%d: %s: unable to send null frame to %s\n",
			          wlc->pub->unit, name, bcm_ether_ntoa(&bsscfg->BSSID, eabuf)));
			err = BCME_NORESOURCE;
		}
		break;
	}
#endif /* STA */

	case IOV_GVAL(IOV_WME_DP):
		*ret_int_ptr = (int32)wlc->wme_dp;
		break;

	case IOV_SVAL(IOV_WME_DP):
		wlc->wme_dp = (uint8)int_val;
		wlc->wme_dp_precmap = wlc_convert_acbitmap_to_precbitmap(wlc->wme_dp);
		break;

#if defined(WLCNT)
	case IOV_GVAL(IOV_WME_COUNTERS):
		bcopy(wlc->pub->_wme_cnt, arg, sizeof(wl_wme_cnt_t));
		break;
	case IOV_SVAL(IOV_WME_CLEAR_COUNTERS):
		/* Zero the counters but reinit the version and length */
		bzero(wlc->pub->_wme_cnt,  sizeof(wl_wme_cnt_t));
		WLCNTSET(wlc->pub->_wme_cnt->version, WL_WME_CNT_VERSION);
		WLCNTSET(wlc->pub->_wme_cnt->length, sizeof(wl_wme_cnt_t));
		break;
#endif
	case IOV_GVAL(IOV_WME_PREC_QUEUING):
		*ret_int_ptr = wlc->wme_prec_queuing ? 1 : 0;
		break;
	case IOV_SVAL(IOV_WME_PREC_QUEUING):
		wlc->wme_prec_queuing = bool_val;
		break;

#ifdef STA
	case IOV_GVAL(IOV_WME_QOSINFO):
		*ret_int_ptr = wme->apsd_sta_qosinfo;
		break;

	case IOV_SVAL(IOV_WME_QOSINFO):
		wme->apsd_sta_qosinfo = (uint8)int_val;
		break;
#endif /* STA */
#endif /* WME */

#ifdef MCAST_REGEN
	case IOV_GVAL(IOV_MCAST_REGEN_BSS_ENABLE):
		*ret_int_ptr = bsscfg->mcast_regen_enable;
		break;

	case IOV_SVAL(IOV_MCAST_REGEN_BSS_ENABLE):
		if (bool_val) {
			bsscfg->mcast_regen_enable = 1;
		} else {
			bsscfg->mcast_regen_enable = 0;
		}
		break;
#endif /* MCAST_REGEN */

	case IOV_GVAL(IOV_WLFEATUREFLAG):
		*ret_int_ptr = (int32)wlc->pub->wlfeatureflag;
		break;

	case IOV_SVAL(IOV_WLFEATUREFLAG):
		wlc->pub->wlfeatureflag = int_val;
		break;


#ifdef WET
	case IOV_GVAL(IOV_WET):
		*ret_int_ptr = (int32)wlc->wet;
		break;

	case IOV_SVAL(IOV_WET):
		if (!wlc->mac_spoof)
			wlc->wet = bool_val;
		else
			err = BCME_UNSUPPORTED;
		break;
#endif	/* WET */

#ifdef MAC_SPOOF
	case IOV_GVAL(IOV_MAC_SPOOF):
		*ret_int_ptr = (int32)wlc->mac_spoof;
		break;

	case IOV_SVAL(IOV_MAC_SPOOF):
		if (!wlc->wet && BSSCFG_STA(bsscfg) && !APSTA_ENAB(wlc->pub))
			wlc->mac_spoof = bool_val;
		else
			err = BCME_UNSUPPORTED;
		break;
#endif /* MAC_SPOOF */
#ifdef TOE
	case IOV_GVAL(IOV_TOE):
		*ret_int_ptr = (int32)wlc->pub->_toe;
		break;

	case IOV_SVAL(IOV_TOE):
		wlc->pub->_toe = bool_val;
		break;
#endif /* TOE */

#ifdef ARPOE
	case IOV_GVAL(IOV_ARPOE):
		*ret_int_ptr = (int32)wlc->pub->_arpoe;
		break;

	case IOV_SVAL(IOV_ARPOE):
		if (ARPOE_SUPPORT(wlc->pub))
			wlc->pub->_arpoe = bool_val;
		else
			err = BCME_UNSUPPORTED;
		break;
#endif /* ARPOE */

#ifdef PLC
	case IOV_GVAL(IOV_PLC): {
		wl_plc_params_t *plc = (wl_plc_params_t *)params;

		if (plc->cmd == PLC_CMD_FAILOVER)
			plc->plc_failover = wlc->pub->_plc;
#ifdef PLC_WET
		else if (plc->cmd == PLC_CMD_NODE_LIST) {
			wl_plc_node_list(wlc->wl, (wl_plc_nodelist_t *)arg);
			break;
		} else if ((plc->cmd == PLC_CMD_MAC_COST) || (plc->cmd == PLC_CMD_LINK_COST))
			plc->cost = wl_plc_node_get_cost(wlc->wl, plc->node_ea.octet,
			                                 plc->cmd);
#endif /* PLC_WET */
		bcopy(plc, arg, sizeof(*plc));
		break;
	}

	case IOV_SVAL(IOV_PLC): {
		wl_plc_params_t *plc = (wl_plc_params_t *)params;

		if (plc->cmd == PLC_CMD_FAILOVER)
			wlc->pub->_plc = plc->plc_failover;
#ifdef PLC_WET
		else if ((plc->cmd == PLC_CMD_MAC_COST) || (plc->cmd == PLC_CMD_LINK_COST))
			wl_plc_node_set_cost(wlc->wl, plc->node_ea.octet,
			                     plc->cmd, plc->cost);
#endif /* PLC_WET */
		break;
	}

#ifdef PLC_WET
	case IOV_GVAL(IOV_PLC_PATH):
		*ret_int_ptr = (int32)wlc->pub->plc_path;
		break;

	case IOV_SVAL(IOV_PLC_PATH):
		wlc->pub->plc_path = bool_val;
		break;
#endif /* PLC_WET */
#endif /* PLC */

	case IOV_GVAL(IOV_EAP_RESTRICT):
		*ret_int_ptr = (int32)bsscfg->eap_restrict;
		break;

	case IOV_SVAL(IOV_EAP_RESTRICT):
		bsscfg->eap_restrict = bool_val;
		break;
#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_EIRP):
		*ret_int_ptr = wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP;
		break;
#endif /* defined(BCMDBG) || defined(WLTEST) */

	case IOV_GVAL(IOV_CUR_ETHERADDR):
		bcopy(&bsscfg->cur_etheraddr, arg, ETHER_ADDR_LEN);
		break;

	case IOV_SVAL(IOV_CUR_ETHERADDR):

		err = wlc_validate_mac(wlc, bsscfg, arg);
		break;

	case IOV_GVAL(IOV_PERM_ETHERADDR):
		bcopy(&wlc->perm_etheraddr, arg, ETHER_ADDR_LEN);
		break;
#ifdef BCMDBG
	case IOV_GVAL(IOV_RAND):
		wlc_getrand(wlc, arg, sizeof(uint16));
		break;
#endif /* BCMDBG */
	case IOV_GVAL(IOV_MPC):
		*ret_int_ptr = (int32)wlc->mpc;
		break;

	case IOV_SVAL(IOV_MPC):
		if (bool_val && !wlc_mpccap(wlc)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		wlc->mpc = bool_val;
#ifdef STA
		wlc_radio_mpc_upd(wlc);

#endif /* STA */
		break;

	case IOV_GVAL(IOV_WATCHDOG_DISABLE):
		*ret_int_ptr = (int32)wlc->watchdog_disable;
		break;

	case IOV_SVAL(IOV_WATCHDOG_DISABLE):
		wlc->watchdog_disable = bool_val;
		break;

#ifdef STA
	case IOV_GVAL(IOV_MPC_DUR):
		*ret_int_ptr = wlc_get_mpc_dur(wlc);
		break;

	case IOV_SVAL(IOV_MPC_DUR):
		wlc->mpc_dur = 0;
		wlc->mpc_laston_ts = OSL_SYSUPTIME(); /* need to do this when mpc radio is off */
		break;
#endif /* STA */

	case IOV_GVAL(IOV_BCMERRORSTR):
#ifdef BCMSPACE
		strncpy((char *)arg, bcmerrorstr(wlc->pub->bcmerror), BCME_STRLEN);
#else
		snprintf((char *)arg, BCME_STRLEN, "error %d", wlc->pub->bcmerror);
#endif
		break;

	case IOV_GVAL(IOV_CLMVER):
		bcm_binit(&b, arg, len);
		wlc_dump_clmver(wlc->cmi, &b);
		break;

	case IOV_GVAL(IOV_CLM_DATA_VER):
		bcm_binit(&b, arg, len);
		wlc_dump_clm_data_ver(&b);
		break;

	case IOV_GVAL(IOV_BCMERROR):
		*ret_int_ptr = (int32) wlc->pub->bcmerror;
		break;

#ifdef WLCNT
	case IOV_GVAL(IOV_COUNTERS):
		if (WLC_UPDATE_STATS(wlc)) {
			wlc_statsupd(wlc);
		}
		bcopy(wlc->pub->_cnt, arg, sizeof(wl_cnt_t));
		break;

#endif /* WLCNT */

#if defined(DELTASTATS)
	case IOV_GVAL(IOV_DELTA_STATS_INTERVAL):
		*ret_int_ptr = wlc->delta_stats->interval;
		break;

	case IOV_SVAL(IOV_DELTA_STATS_INTERVAL):
		wlc->delta_stats->interval = int_val;
		if (wlc->delta_stats->interval > 0) {
			/* delta stats enabled */
			wlc->delta_stats->seconds = 0;
			wlc->delta_stats->current_index = 0;
		}
		break;

	case IOV_GVAL(IOV_DELTA_STATS):
		err = wlc_get_delta_stats(wlc, arg);
		break;
#endif /* DELTASTATS */


#if defined(MACOSX)
	case IOV_GVAL(IOV_TXFIFO_SZ): {
		wl_txfifo_sz_t *ts = (wl_txfifo_sz_t *)params;
		uint block_size = 0;

		if ((uint16)int_val != WL_TXFIFO_SZ_MAGIC) {
			WL_ERROR(("txfifo_sz: wl version don't match the driver\n"));
			err = BCME_VERSION;
			break;
		}

		err = wlc_bmac_xmtfifo_sz_get(wlc->hw, ts->fifo, &block_size);
		if (err)
			break;

		ts->size = (uint16)block_size;

		bcopy(ts, arg, sizeof(*ts));
		break;
	}

	case IOV_SVAL(IOV_TXFIFO_SZ): {
		wl_txfifo_sz_t *ts = (wl_txfifo_sz_t *)params;

		if ((uint16)int_val != WL_TXFIFO_SZ_MAGIC) {
			WL_ERROR(("txfifo_sz: wl version don't match the driver\n"));
			err = BCME_VERSION;
			break;
		}

		err = wlc_xmtfifo_sz_set(wlc, ts->fifo, ts->size);
		break;
	}
#endif 
	case IOV_SVAL(IOV_COUNTRY_LIST_EXTENDED):
		wlc->country_list_extended = bool_val;
		break;

	case IOV_GVAL(IOV_COUNTRY_LIST_EXTENDED):
		*ret_int_ptr = (int32)wlc->country_list_extended;
		break;

#ifdef WLCNT
	case IOV_SVAL(IOV_RESET_D11CNTS): {
		if (WLC_UPDATE_STATS(wlc)) {
			wl_cnt_t *cnt = wlc->pub->_cnt;
			wlc_statsupd(wlc);
			cnt->d11cnt_txrts_off += cnt->txrts;
			cnt->d11cnt_rxcrc_off += cnt->rxcrc;
			cnt->d11cnt_txnocts_off += cnt->txnocts;

			cnt->txframe = cnt->txfrag = cnt->txmulti = cnt->txfail =
			cnt->txerror = cnt->txserr = cnt->txretry = cnt->txretrie = cnt->rxdup =
			cnt->txbcnfrm =  cnt->txrts = cnt->txnocts =
			cnt->txnoack = cnt->rxframe = cnt->rxerror = cnt->rxfrag = cnt->rxmulti =
			cnt->rxbeaconmbss = cnt->rxcrc = cnt->txfrmsnt = cnt->rxundec =
			cnt->rxundec_mcst = 0;
			if (wlcif) {
				bzero(&(wlcif->_cnt), sizeof(wl_if_stats_t));
				WLCNTSET(wlcif->_cnt.version, WL_IF_STATS_T_VERSION);
				WLCNTSET(wlcif->_cnt.length, sizeof(wl_if_stats_t));
			} else if (bsscfg->wlcif) {
				bzero(&(bsscfg->wlcif->_cnt), sizeof(wl_if_stats_t));
				WLCNTSET(bsscfg->wlcif->_cnt.version, WL_IF_STATS_T_VERSION);
				WLCNTSET(bsscfg->wlcif->_cnt.length, sizeof(wl_if_stats_t));
			}
		}
		break;
	}
	case IOV_GVAL(IOV_RESET_CNTS):
		if (WLC_UPDATE_STATS(wlc)) {
			wl_cnt_t *cnt = wlc->pub->_cnt;

			if (wlc->clk)
				wlc_statsupd(wlc);

			/* Reset all counters and restore version/size */
			bzero(cnt,  sizeof(wl_cnt_t));
			WLCNTSET(cnt->version, WL_CNT_T_VERSION);
			WLCNTSET(cnt->length, sizeof(cnt));
			if (wlcif) {
				bzero(&(wlcif->_cnt), sizeof(wl_if_stats_t));
				WLCNTSET(wlcif->_cnt.version, WL_IF_STATS_T_VERSION);
				WLCNTSET(wlcif->_cnt.length, sizeof(wl_if_stats_t));
			} else if (bsscfg->wlcif) {
				bzero(&(bsscfg->wlcif->_cnt), sizeof(wl_if_stats_t));
				WLCNTSET(bsscfg->wlcif->_cnt.version, WL_IF_STATS_T_VERSION);
				WLCNTSET(bsscfg->wlcif->_cnt.length, sizeof(wl_if_stats_t));
			}
			*ret_int_ptr = 0;
		}
		break;
#endif /* WLCNT */

	case IOV_SVAL(IOV_SCANABORT):
		wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
		break;

	case IOV_GVAL(IOV_IBSS_ALLOWED):
		*ret_int_ptr = (int32)wlc->ibss_allowed;
		break;

	case IOV_SVAL(IOV_IBSS_ALLOWED):
		wlc->ibss_allowed = bool_val;
		break;

	case IOV_GVAL(IOV_MCAST_LIST):
		/* Check input buffer length */
		maclist = (struct maclist *)arg;
		if (len < (int)(sizeof(uint32) + bsscfg->nmulticast * ETHER_ADDR_LEN)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		*ret_int_ptr = bsscfg->nmulticast;

		/* Copy each address */
		bcopy((void *)(bsscfg->multicast), (void *)(maclist->ea),
			(bsscfg->nmulticast * ETHER_ADDR_LEN));

		break;

	case IOV_SVAL(IOV_MCAST_LIST):
		/* Store number of addresses in list */
		maclist = (struct maclist *)arg;
		bcopy(&maclist->count, &int_val, sizeof(uint32));
		if (int_val > MAXMULTILIST) {
			err = BCME_RANGE;
			break;
		}
		if (len < (int)(sizeof(uint32) + int_val * ETHER_ADDR_LEN)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		bsscfg->nmulticast = int_val;

		/* Copy each address */
		bcopy((void *)(maclist->ea), (void *)(bsscfg->multicast),
			(bsscfg->nmulticast * ETHER_ADDR_LEN));

		break;
#ifdef WLATF
	case IOV_GVAL(IOV_ATF):
		*ret_int_ptr = wlc->atf;
		break;

	case IOV_SVAL(IOV_ATF):
		wlc->atf = bool_val;
		break;

#endif /* WLATF */
	/* Chanspecs with current driver settings */
	case IOV_GVAL(IOV_CHANSPECS_DEFSET): {
		char abbrev[1] = ""; /* No country abbrev should be take as input. */
		wl_uint32_list_t *list = (wl_uint32_list_t *)arg;

		list->count = 0;

		switch (wlc_ht_get_mimo_band_bwcap(wlc->hti)) {
			case WLC_N_BW_20ALL:
				/* 2G 20MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_20, TRUE, abbrev);

				/* 5G 20MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_20, FALSE, abbrev);
				break;
			case WLC_N_BW_40ALL:
				/* 2G 20MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_20, TRUE, abbrev);
				/* 2G 40MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_40, TRUE, abbrev);

				/* 5G 20MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_20, FALSE, abbrev);
				/* 5G 40MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_40, FALSE, abbrev);
				break;
			case WLC_N_BW_20IN2G_40IN5G:
				/* 2G 20MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_20, TRUE, abbrev);

				/* 5G 20MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_20, FALSE, abbrev);
				/* 5G 40MHz */
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_40, FALSE, abbrev);
				break;
			default:
				WL_ERROR(("mimo_bw_cap has invalid value set.\n"));
				break;
		}
		ASSERT(list->count < WL_NUMCHANSPECS);
	}
	break;

	case IOV_GVAL(IOV_CHANSPECS): {
		chanspec_t chanspec = *((chanspec_t *)params);
		char abbrev[WLC_CNTRY_BUF_SZ];
		wl_uint32_list_t *list_arg = (wl_uint32_list_t *)arg;
		wl_uint32_list_t *list = NULL;
		uint16 max_num_chanspecs = 0, alloc_len = 0;
		if (chanspec == 0) {
			max_num_chanspecs = (WL_NUMCHANSPECS);
		} else {
			if (CHSPEC_IS2G(chanspec)) {
				if (CHSPEC_IS20(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_2G_20;
				if (CHSPEC_IS40(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_2G_40;
			}
			if (CHSPEC_IS5G(chanspec)) {
				if (CHSPEC_IS20(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_5G_20;
				if (CHSPEC_IS40(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_5G_40;
				if (CHSPEC_IS80(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_5G_80;
				if (CHSPEC_IS8080(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_5G_8080;
				if (CHSPEC_IS160(chanspec))
					max_num_chanspecs += WL_NUMCHANSPECS_5G_160;
			}

		}
		alloc_len = ((1 + max_num_chanspecs) * sizeof(uint32));
		/* if caller's buf may not be long enough, malloc a temp buf */
		if (len < alloc_len) {
			list = (wl_uint32_list_t *)MALLOC(wlc->osh, alloc_len);
			if (list == NULL) {
				err = BCME_NOMEM;
				break;
			}
		} else {
			list = list_arg;
		}
		bzero(abbrev, WLC_CNTRY_BUF_SZ);
		strncpy(abbrev, ((char*)params + sizeof(chanspec_t)), WLC_CNTRY_BUF_SZ - 1);

		if (!bcmp(wlc_channel_ccode(wlc->cmi), abbrev, WLC_CNTRY_BUF_SZ -1) &&
			(wlc->band->bandunit == CHSPEC_WLCBANDUNIT(chanspec))) {
			/* If requested abrev is the current one, within the same band,
			 * avoid country lookup and return chanspecs of current country/regrev
			 */
			bzero(abbrev, WLC_CNTRY_BUF_SZ);
		}

		list->count = 0;
		if (CHSPEC_IS2G(chanspec) || (chanspec == 0)) {
			if (CHSPEC_IS20(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_20, TRUE, abbrev);
			if (CHSPEC_IS40(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_40, TRUE, abbrev);
		}
		if (CHSPEC_IS5G(chanspec) || (chanspec == 0)) {
			if (CHSPEC_IS20(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_20, FALSE, abbrev);
			if (CHSPEC_IS40(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_40, FALSE, abbrev);
			if (CHSPEC_IS80(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_80, FALSE, abbrev);
			if (CHSPEC_IS8080(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_8080, FALSE, abbrev);
			if (CHSPEC_IS160(chanspec) || (chanspec == 0))
				wlc_get_valid_chanspecs(wlc->cmi, list,
				                        WL_CHANSPEC_BW_160, FALSE, abbrev);
		}
		ASSERT(list->count <= max_num_chanspecs);
		/* if not reusing the caller's buffer */
		if (list != list_arg) {
			/* now copy if we can fr internal buf to caller's or return error */
			if (list->count <= (len - sizeof(uint32))/sizeof(uint32)) {
				bcopy(list, list_arg, (list->count + 1) * sizeof(uint32));
			}
			else {
				/* caller buf too short - free mem and return error */
				err = BCME_BUFTOOSHORT;
			}
			MFREE(wlc->osh, list, alloc_len);
		}
		break;
	}

	case IOV_GVAL(IOV_CHANSPEC):
		if (bsscfg->associated)
			*ret_int_ptr = (int32)current_bss->chanspec;
		else {
			if (APSTA_ENAB(wlc->pub) || PSTA_ENAB(wlc->pub)) {
				wlc_bsscfg_t *ap_cfg;
				int idx;

				FOREACH_BSS(wlc, idx, ap_cfg) {
					if (ap_cfg->associated)
						break;
				}
				if (ap_cfg)
					*ret_int_ptr = (int32)ap_cfg->current_bss->chanspec;
				else
					*ret_int_ptr = (int32)wlc->default_bss->chanspec;
			}
			else
				*ret_int_ptr = (int32)wlc->default_bss->chanspec;
		}
		break;

	case IOV_SVAL(IOV_CHANSPEC): {
		chanspec_t chspec = (chanspec_t)int_val;

		if (wf_chspec_malformed(chspec)) {
			WL_ERROR(("wl%d: malformed chanspec 0x%04x\n",
			          wlc->pub->unit, chspec));
			err = BCME_BADCHAN;
			break;
		}

		if (!wlc_valid_chanspec_db(wlc->cmi, chspec)) {
			WL_ERROR(("wl%d: invalid chanspec 0x%04x\n",
			          wlc->pub->unit, chspec));
			err = BCME_BADCHAN;
			break;
		}

		if (!wlc->pub->up && IS_MBAND_UNLOCKED(wlc)) {
			if (wlc->band->bandunit != CHSPEC_WLCBANDUNIT(chspec))
				wlc->bandinit_pending = TRUE;
			else
				wlc->bandinit_pending = FALSE;
		}

		/* want to migrate to use bsscfg->chanspec as the configured chanspec
		 * and wlc->chanspec as the current chanspec.
		 * wlc->default_bss->chanspec would have no meaning
		 */
		wlc->default_bss->chanspec = chspec;

		if (AP_ENAB(wlc->pub)) {
			wlc->ap->chanspec_selected = chspec;
		}

#ifdef WLMCHAN
		/* check for wlcif pointer, if valid it means we refer correct bsscfg */
		if (MCHAN_ENAB(wlc->pub) && (wlcif != NULL))
			wlc_mchan_config_go_chanspec(wlc->mchan, bsscfg, chspec);
#endif /* WLMCHAN */

		/* wlc_BSSinit() will sanitize the rateset before using it.. */
		if (wlc->pub->up && !wlc->pub->associated &&
		    (WLC_BAND_PI_RADIO_CHANSPEC != chspec)) {
			wlc_set_home_chanspec(wlc, chspec);
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, chspec);
			wlc_enable_mac(wlc);
		}
		break;
	}

#if defined(WLTEST) || defined(WLPKTENG)
	case IOV_SVAL(IOV_PKTENG): {
		wl_pkteng_t pkteng;
		void *pkt = NULL;
		uint32 pkteng_flags;

#ifdef WLOFFLD
		if (WLOFFLD_ENAB(wlc->pub) && wlc->pub->up)
			wlc_ol_down(wlc->ol);
#endif
		if (D11REV_LT(wlc->pub->corerev, 11))
			return BCME_UNSUPPORTED;

		bcopy(params, &pkteng, sizeof(wl_pkteng_t));
		pkteng_flags = pkteng.flags  & WL_PKTENG_PER_MASK;

		if (pkteng.length + TXOFF > WL_PKTENG_MAXPKTSZ)
			return BCME_BADLEN;

		/* Prepare the packet for Tx */
		if ((pkteng_flags == WL_PKTENG_PER_TX_START) ||
			(pkteng_flags == WL_PKTENG_PER_TX_WITH_ACK_START)) {
			struct ether_addr *sa;

			/* Don't allow pkteng to start if associated */
			if (wlc->pub->associated)
				return BCME_ASSOCIATED;
			if (!wlc->band->rspec_override)
				return BCME_ERROR;
#ifdef WL_BEAMFORMING
			if (TXBF_ENAB(wlc->pub)) {
				wlc_txbf_pkteng_tx_start(wlc->txbf, wlc->band->hwrs_scb);
			}
#endif /*  WL_BEAMFORMING */

			sa = (ETHER_ISNULLADDR(&pkteng.src)) ?
				&wlc->pub->cur_etheraddr : &pkteng.src;
			/* pkt will be freed in wlc_bmac_pkteng() */
			pkt = wlc_tx_testframe(wlc, &pkteng.dest, sa,
				0, pkteng.length);
			if (pkt == NULL)
				return BCME_ERROR;

#ifdef WLC_HIGH_ONLY
			/* For the returning txstatus */
			wlc_rpctx_pktenq(wlc->rpctx, TX_DATA_FIFO, pkt);
			/* For PKTENG, we have to increase the pktpend here for the
			 * returning txstatus.
			 */
			TXPKTPENDINC(wlc, TX_DATA_FIFO, 1);
#endif /* WLC_HIGH_ONLY */

			/* Unmute the channel for pkteng if quiet */
			if (wlc_quiet_chanspec(wlc->cmi, wlc->chanspec))
				wlc_mute(wlc, OFF, 0);
		}

		err = wlc_bmac_pkteng(wlc->hw, &pkteng, pkt);

		if ((err != BCME_OK) || (pkteng_flags == WL_PKTENG_PER_TX_STOP)) {
			/* restore Mute State after pkteng is done */
			if (wlc_quiet_chanspec(wlc->cmi, wlc->chanspec))
				wlc_mute(wlc, ON, 0);
#ifdef WL_BEAMFORMING
			if (TXBF_ENAB(wlc->pub)) {
				wlc_txbf_pkteng_tx_stop(wlc->txbf, wlc->band->hwrs_scb);
			}
#endif /*  WL_BEAMFORMING */
		}
		break;
	}

	case IOV_GVAL(IOV_PKTENG_MAXLEN): {
		*ret_int_ptr = wlc->pkteng_maxlen;
		break;
	}
#endif 
#if defined(WLTEST)
	case IOV_GVAL(IOV_BOARDFLAGS): {
		*ret_int_ptr = wlc->pub->boardflags;
		break;
	}

	case IOV_SVAL(IOV_BOARDFLAGS): {
		wlc->pub->boardflags = int_val;

		/* This iovar needs to be done at both host and dongle */
		err = wlc_iovar_op(wlc, "bmac_bf", arg, len, arg, len, IOV_SET, wlcif);
		break;
	}

	case IOV_GVAL(IOV_BOARDFLAGS2): {
		*ret_int_ptr = wlc->pub->boardflags2;
		break;
	}

	case IOV_SVAL(IOV_BOARDFLAGS2): {
		wlc->pub->boardflags2 = int_val;

		/* This iovar needs to be done at both host and dongle */
		err = wlc_iovar_op(wlc, "bmac_bf2", arg, len, arg, len, IOV_SET, wlcif);
		break;
	}

	case IOV_GVAL(IOV_ANTGAIN): {
		uint8	*ag = (uint8*)arg;
		uint j;

		ag[0] = 0;
		ag[1] = 0;

		for (j = 0; j < NBANDS(wlc); j++) {
			/* Use band 1 for single band 11a */
			if (IS_SINGLEBAND_5G(wlc->deviceid))
				j = BAND_5G_INDEX;

			/* Compute Antenna Gain */
			ag[j] = wlc->bandstate[j]->antgain >> 2;
		}
		break;
	}

	case IOV_SVAL(IOV_ANTGAIN): {
		uint8	*ag = (uint8*)params;
		uint j;

		for (j = 0; j < NBANDS(wlc); j++) {
			/* Use band 1 for single band 11a */
			if (IS_SINGLEBAND_5G(wlc->deviceid))
				j = BAND_5G_INDEX;

			wlc_pi_band_update(wlc, j);

			/* Compute Antenna Gain */
			wlc->band->antgain = BAND_5G(wlc->band->bandtype) ? ag[1] : ag[0];
			wlc->band->antgain = wlc_antgain_calc(wlc->band->antgain);
		}
		break;
	}

	case IOV_GVAL(IOV_NVRAM_GET): {
		const char *nv_str;

		nv_str = getvar(wlc->pub->vars, params);

		if (nv_str != NULL) {
			size_t nv_len = strlen(nv_str);
			if ((size_t)len < nv_len + 1)
				err = BCME_BUFTOOSHORT;
			else {
				char *dst = (char *)arg;
				strncpy(dst, nv_str, nv_len);
				dst[nv_len] = '\0';
			}
		} else {
			err = BCME_NOTFOUND;
		}
		break;
	}

	case IOV_GVAL(IOV_NVRAM_DUMP):
		bcm_binit(&b, (char*)arg, len);
		err = wlc_nvram_dump(wlc, &b);
		break;
#endif 

#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && defined(WLTEST)
#ifdef BCMNVRAMW
	case IOV_SVAL(IOV_CISWRITE):
	{
		cis_rw_t cis;
		uint16 *tbuf;

		ASSERT(len >= (int)sizeof(cis));
		bcopy(arg, &cis, sizeof(cis));
		len -= sizeof(cis);
		arg = (uint8 *)arg + sizeof(cis);

		/* Basic validity checks.  For now retain 2-byte mode, insist on byteoff 0 */
		if (cis.source || (cis.byteoff & 1) || (cis.nbytes & 1) ||
			cis.byteoff) {
			err = BCME_BADARG;
			break;
		}

		/* Use a temp buffer to ensure alignment */
		if ((tbuf = (uint16*)MALLOC(wlc->osh, cis.nbytes)) == NULL) {
			err = BCME_NOMEM;
			break;
		}
		bcopy(arg, tbuf, cis.nbytes);
		ltoh16_buf(tbuf, cis.nbytes);

		err = wlc_bmac_ciswrite(wlc->hw, &cis, tbuf, len);

		MFREE(wlc->osh, tbuf, cis.nbytes);
		break;
	}
#endif /* BCMNVRAMW */

	case IOV_GVAL(IOV_CISDUMP):
	{
#if defined(WLC_HIGH_ONLY) && defined(USBAP) && !defined(WLTEST)
		/* To save usbap dongle memory, only support this command with WLTEST defined */
		err = BCME_UNSUPPORTED;
#else
		cis_rw_t cis;
		uint16 *tbuf = NULL;

		/* Extract parameters (to allow length spec) */
		if ((p_len < sizeof(cis)) || (len < (int)sizeof(cis))) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy(params, &cis, sizeof(cis));

		/* Some basic consistency checks */
		if (cis.source || cis.byteoff || (cis.nbytes & 1) || (cis.nbytes > SROM_MAX)) {
			err = BCME_BADARG;
			break;
		}

		/* Use a temp buffer to ensure alignment */
		if ((tbuf = (uint16*)MALLOC(wlc->osh, SROM_MAX)) == NULL) {
			err = BCME_NOMEM;
			break;
		}
		memset(tbuf, 0, SROM_MAX);

		/* Adjust length to available bytes */
		len -= sizeof(cis);

		/* Read the CIS */
		err = wlc_bmac_cisdump(wlc->hw, &cis, tbuf, len);

		/* Prepare return buffer */
		if (!err) {
			bcopy(&cis, arg, sizeof(cis));
			htol16_buf(tbuf, cis.nbytes);
			bcopy(tbuf, (char*)arg + sizeof(cis), cis.nbytes);
		}

		MFREE(wlc->osh, tbuf, SROM_MAX);
#endif /* WLC_HIGH_ONLY && USBAP && !WLTEST */
		break;
	}
#endif 

	case IOV_GVAL(IOV_LIFETIME):
	{
		wl_lifetime_t *lifetime = (wl_lifetime_t *)arg;

		if (int_val < AC_COUNT) {
			lifetime->ac = int_val;
			lifetime->lifetime = wlc->lifetime[int_val] / 1000; /* us -> ms */
		}
		else
			err = BCME_BADARG;

		break;
	}

	case IOV_SVAL(IOV_LIFETIME):
	{
		wl_lifetime_t lifetime;

		bcopy(arg, &lifetime, sizeof(wl_lifetime_t));
		if (lifetime.ac < AC_COUNT && lifetime.lifetime <= WL_LIFETIME_MAX) {
			wlc->lifetime[lifetime.ac] = lifetime.lifetime * 1000; /* ms ->us */

			wlc_rfaware_lifetime_set(wlc, wlc->rfaware_lifetime);
		} else
			err = BCME_BADARG;

		break;
	}
#ifdef RWL_DONGLE
	case IOV_GVAL(IOV_RWLDONGLE_DATA):
		/* Wait for serial protocol layer to set the packet_status bit
		  *  if bit set then copy the data in arg
		  *  reset the packet_status bit
		  * if not set send 0 to server application
		  */
		if (g_rwl_dongle_data.packet_status) {
			if (g_rwl_dongle_data.packet_buf != NULL) {
				bcopy(g_rwl_dongle_data.packet_buf, (char*)arg,
				 g_rwl_dongle_data.packet_len);
				MFREE(NULL, g_rwl_dongle_data.packet_buf,
				 g_rwl_dongle_data.packet_len);
			}
			g_rwl_dongle_data.packet_status = 0;
		} else {
			uint status = 0;
			bcopy(&status, arg, sizeof(status));
		}
		break;
	case IOV_SVAL(IOV_RWLDONGLE_DATA):
		/* Transmit the server response to client
		 * Code jumps to hnd_cons.c at this point
		 */
		remote_uart_tx(arg);
		break;
	case IOV_GVAL(IOV_DONGLE_FLAG):
		{
			*ret_int_ptr = g_rwl_dongle_flag;
			break;
		}
	case IOV_SVAL(IOV_DONGLE_FLAG):
		{
			g_rwl_dongle_flag = int_val;
			break;
		}
#endif /* RWL_DONGLE */

#ifdef WIFI_ACT_FRAME
	case IOV_SVAL(IOV_ACTION_FRAME):
		if (wlc->pub->up) {
			/* Send frame to remote client/server */
			err = wlc_send_action_frame(wlc, bsscfg, NULL, arg);
		} else {
			err = BCME_NOTUP;
		}
		break;
	case IOV_SVAL(IOV_AF): {
#ifdef STA
		bool af_continue;
#endif /* STA */
		bool do_event = TRUE;

		wl_af_params_t *af = NULL;

		af = MALLOC(wlc->osh, WL_WIFI_AF_PARAMS_SIZE);
		if (!af) {
			err = BCME_NOMEM;
			break;
		}
		bcopy(arg, af, len);
		do {
			if (BSSCFG_AP(bsscfg)) {
				if (!wlc->pub->up) {
					err = BCME_NOTUP;
					break;
				}
				/* Send frame to remote client/server */
				err = wlc_send_action_frame(wlc, bsscfg, &bsscfg->BSSID,
					&af->action_frame);
				do_event = FALSE;
				break;
			}

#ifdef STA
			af_continue = FALSE;

			/* Piggyback on current scan channel if the user
			 * does not care about the channel
			 */
			if (SCAN_IN_PROGRESS(wlc->scan) && (af->channel == 0 ||
				af->channel == wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC))) {
				af_continue = TRUE;
			}

			if (WLFMC_ENAB(wlc->pub) && af->channel == 0) {
				af_continue = TRUE;
			}

			if (!af_continue) {
				if (ACT_FRAME_IN_PROGRESS(wlc->scan)) {
					err = BCME_BUSY;
					break;
				}
				err = wlc_send_action_frame_off_channel(wlc, bsscfg,
					af->channel, af->dwell_time, &af->BSSID, &af->action_frame);
				do_event = FALSE;
				break;
			}

			if (!wlc->pub->up) {
				err = BCME_NOTUP;
				break;
			}

			/* If current channel is a 'quiet' channel then reject */
			if (BSS_QUIET_STATE(wlc->quiet, bsscfg) & SILENCE) {
				err = BCME_NOTREADY;
				break;
			}

			/* Send frame to remote client/server */
			err = wlc_send_action_frame(wlc, bsscfg, &af->BSSID, &af->action_frame);

			if (!err && af->dwell_time) {
				/* apply new dwell time to the channel */
				wlc_scan_timer_update(wlc->scan, af->dwell_time);
			}

			do_event = FALSE;
#endif /* STA */
		} while (FALSE);

		if (do_event) {
			/* Send back the Tx status to the host */
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_ACTION_FRAME_COMPLETE,
				NULL, WLC_E_STATUS_NO_ACK, 0, 0,
				&af->action_frame.packetId, sizeof(af->action_frame.packetId));
		}
		MFREE(wlc->osh, af, WL_WIFI_AF_PARAMS_SIZE);
		break;
	}
#endif /* WIFI_ACT_FRAME */

#ifdef STA
#ifdef BCMDBG
	case IOV_GVAL(IOV_APSTA_DBG):
		*ret_int_ptr = wl_apsta_dbg;
		break;

	case IOV_SVAL(IOV_APSTA_DBG):
		wl_apsta_dbg = int_val;
		break;
#endif /* BCMDBG */

	case IOV_GVAL(IOV_IBSS_JOIN_ONLY):
		*ret_int_ptr = (int32)wlc->IBSS_join_only;
		break;
	case IOV_SVAL(IOV_IBSS_JOIN_ONLY):
		wlc->IBSS_join_only = bool_val;
		break;

	case IOV_GVAL(IOV_BCN_TIMEOUT):
		*ret_int_ptr = (int32)roam->bcn_timeout;
		break;

	case IOV_SVAL(IOV_BCN_TIMEOUT):
		roam->bcn_timeout = (uint)int_val;
		break;

#endif /* STA */


	case IOV_GVAL(IOV_DUMP):
#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST) || \
	defined(BCMDBG_AMPDU) || defined(MCHAN_MINIDUMP) || defined(BCM_DNGDMP) || \
	defined(BCMDBG_PHYDUMP)
		if (wl_msg_level & WL_LOG_VAL || WL_CHANLOG_ON())
#if defined(BCMTSTAMPEDLOGS)
			bcmdumptslog((char*)arg, len);
#else
			bcmdumplog((char*)arg, len);
#endif
		else
			err = wlc_iovar_dump(wlc, (const char *)params, p_len, (char*)arg, len);
#elif defined(WLTINYDUMP)
	#if defined(TDLS_TESTBED)
	if (bcmp(params, "tdls", sizeof("tdls") - 1)) {
	#endif /* TDLS_TESTBED */
		bcm_binit(&b, arg, len);
		err = wlc_tinydump(wlc, &b);
	#if defined(TDLS_TESTBED)
		}
	else
		err = wlc_iovar_dump(wlc, (const char *)params, p_len, (char*)arg, len);
	#endif /* TDLS_TESTBED */
#else
		strcpy(arg, "N/A\n");
#endif 
		break;

	case IOV_GVAL(IOV_VER):
		bcm_binit(&b, arg, len);
		wl_dump_ver(wlc->wl, &b);

		break;

	case IOV_GVAL(IOV_ANTENNAS):
		*ret_int_ptr = wlc_get_antennas(wlc);
		break;

#ifdef STA
	case IOV_GVAL(IOV_SCANRESULTS_MINRSSI):
		bcopy(&wlc->scanresults_minrssi, arg, sizeof(wlc->scanresults_minrssi));
		break;

	case IOV_SVAL(IOV_SCANRESULTS_MINRSSI):
		wlc->scanresults_minrssi = int_val;
		break;

	case IOV_SVAL(IOV_ESCAN): {
		uint32 escanver;
		uint16 action;
		uint16 escan_sync_id;
		char *inbuf = (char *) arg;

		bcopy(inbuf, &escanver, sizeof(escanver));
		if (escanver != ESCAN_REQ_VERSION) {
			WL_ERROR(("%s: bad version %d\n", name, escanver));
			err = BCME_VERSION;
			break;
		}
		bcopy(inbuf + OFFSETOF(wl_escan_params_t, action), &action, sizeof(action));
		bcopy(inbuf + OFFSETOF(wl_escan_params_t, sync_id), &escan_sync_id,
		      sizeof(escan_sync_id));

		if (action == WL_SCAN_ACTION_START) {
			/* nothing to do here */
		} else if (action == WL_SCAN_ACTION_ABORT) {
			if (ESCAN_IN_PROGRESS(wlc->scan))
				wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
			break;
		} else {
			err = BCME_BADOPTION;
			break;
		}

		wlc->escan_sync_id = escan_sync_id;

		/* start the scan state machine */
		err = wlc_custom_scan(wlc, inbuf + OFFSETOF(wl_escan_params_t, params),
		                      len - OFFSETOF(wl_escan_params_t, params),
		                      0, WLC_ACTION_ESCAN, bsscfg);
		break;
	}

	case IOV_SVAL(IOV_ISCAN): {
		uint32 iscanver;
		uint16 action;
		uint16 scan_duration;
		char *inbuf = (char *) arg;

		bcopy(inbuf, &iscanver, sizeof(iscanver));
		if (iscanver != ISCAN_REQ_VERSION) {
			WL_ERROR(("%s: bad version %d\n", name, iscanver));
			err = BCME_VERSION;
			break;
		}
		bcopy(inbuf + OFFSETOF(wl_iscan_params_t, action), &action, sizeof(action));
		bcopy(inbuf + OFFSETOF(wl_iscan_params_t, scan_duration), &scan_duration,
		      sizeof(scan_duration));

		/* malloc the iscan_ignore_list iff required. Safe because
		 * the pointer gets zeroed when wlc is initialized,
		 * and the buffer gets freed at the same time as wlc is freed.
		 */
		if (!wlc->iscan_ignore_list) {
			wlc->iscan_ignore_list = (iscan_ignore_t *)
			    MALLOC(wlc->osh, WLC_ISCAN_IGNORE_LIST_SZ);
			if (!wlc->iscan_ignore_list) {
				err = BCME_NOMEM;
				break;
			}
		}
		if (action == WL_SCAN_ACTION_START) {
			wlc->iscan_chanspec_last = 0;
			wlc->iscan_ignore_count = 0;
		} else if (action == WL_SCAN_ACTION_CONTINUE) {
			/* If we still have results from previous scan that have not been
			 * retrieved, sendup an event so the host can retrieve.
			 */
			if ((wlc->iscan_result_last != 0) &&
			    (wlc->iscan_result_last < wlc->custom_scan_results->count)) {
				wlc_mac_event(wlc, WLC_E_SCAN_COMPLETE, NULL, WLC_E_STATUS_PARTIAL,
				              0, 0, 0, 0);
				break;
			}
			wlc->scan->iscan_cont = TRUE;
		} else  {
			err = BCME_BADOPTION;
			break;
		}

		/* kick off the scan state machine */
		inbuf += OFFSETOF(wl_iscan_params_t, params);
		wlc->iscan_ignore_last = wlc->iscan_ignore_count;
		wlc->iscan_result_last = 0;
		err = wlc_custom_scan(wlc, inbuf, len - OFFSETOF(wl_iscan_params_t, params),
		                      wlc->iscan_chanspec_last, WLC_ACTION_ISCAN,
		                      bsscfg);

		wlc->scan->iscan_cont = FALSE;
		if (err == BCME_OK && scan_duration)
			/* specified value must allow time for at least 2 channels! */
			wl_add_timer(wlc->wl, wlc->iscan_timer, scan_duration, 0);

		break;
	}

	case IOV_GVAL(IOV_ISCANRESULTS):
	{
		wl_iscan_results_t *resp = (wl_iscan_results_t *)arg;
		int buflen;

		/* BCME_BUFTOOSHORT is used only to indicate results were partial. */
		if (len < (int)WL_ISCAN_RESULTS_FIXED_SIZE) {
			err = BCME_BADARG;
			break;
		}

		/* params has in/out data, so shift it to be the start of the resp buf */
		bcopy(params, resp, WL_ISCAN_RESULTS_FIXED_SIZE);

		buflen = (int)resp->results.buflen;

		if (buflen < (int)WL_ISCAN_RESULTS_FIXED_SIZE) {
			err = BCME_BADARG;
			break;
		}

		if (buflen > len)
			buflen = len;

		buflen -= (int)WL_ISCAN_RESULTS_FIXED_SIZE;

		err = wlc_scan_results(wlc, &resp->results, &buflen,
		                       wlc->custom_iscan_results_state);
		if (err == BCME_BUFTOOSHORT)
			resp->results.buflen = buflen;
		else if (ISCAN_ABORTED(wlc))
			/* query failure: dump this round of ignore elements */
			wlc->iscan_ignore_count = wlc->iscan_ignore_last;

		/* iscan pass is complete, but we were not able to return all the
		 * results, return status as partial.
		 */
		if (wlc->custom_iscan_results_state == WL_SCAN_RESULTS_SUCCESS) {
			if (wlc->iscan_result_last != 0)
				resp->status = WL_SCAN_RESULTS_PARTIAL;
			/* if we were not able to return any results because the
			 * output buffer is too small to hold even 1 result, return
			 * status as partial.
			 */
			else if (err == BCME_BUFTOOSHORT)
				resp->status = WL_SCAN_RESULTS_PARTIAL;
		} else
			resp->status = wlc->custom_iscan_results_state;

		/* Force partial status to complete if the ignore list is full and
		 * all current items are delivered (avoid possible infinite loop).
		 */
		if ((resp->status == WL_SCAN_RESULTS_PARTIAL) &&
		    (wlc->iscan_ignore_count == WLC_ISCAN_IGNORE_MAX)) {
			if ((wlc->iscan_result_last != 0) &&
			    (wlc->iscan_result_last < wlc->custom_scan_results->count)) {
				/* In this case we're ok (still more in current scan
				 * list to return) so leave it alone (invite iscan_c).
				 */
			} else {
				resp->status = WL_SCAN_RESULTS_SUCCESS;
			}
		}

		/* response struct will have the actual status code */
		/* free results if host has retrieved all of them */
		if ((resp->status == WL_SCAN_RESULTS_SUCCESS) ||
			(resp->status == WL_SCAN_RESULTS_ABORTED))
			wlc_bss_list_free(wlc, wlc->custom_scan_results);

		err = BCME_OK;
		break;
	}
#endif /* STA */


#ifdef NLO
	case IOV_SVAL(IOV_NLO):
		bsscfg->nlo = bool_val;
		break;
#endif /* NLO */

#if defined(MACOSX)
	case IOV_GVAL(IOV_SENDUP_MGMT):
		*ret_int_ptr = (int32)bsscfg->sendup_mgmt;
		break;

	case IOV_SVAL(IOV_SENDUP_MGMT):
		bsscfg->sendup_mgmt = bool_val;
		break;
#endif

#if defined(WME_PER_AC_TUNING) && defined(WME_PER_AC_TX_PARAMS)
	case IOV_GVAL(IOV_WME_TX_PARAMS):
		if (WME_PER_AC_TX_PARAMS_ENAB(wlc->pub)) {
			int ac;
			wme_tx_params_t *prms;

			prms = (wme_tx_params_t *)arg;

			for (ac = 0; ac < AC_COUNT; ac++) {
				prms[ac].max_rate = wlc->wme_max_rate[ac];
				prms[ac].short_retry = WLC_WME_RETRY_SHORT_GET(wlc, ac);
				prms[ac].short_fallback = WLC_WME_RETRY_SFB_GET(wlc, ac);
				prms[ac].long_retry = WLC_WME_RETRY_LONG_GET(wlc, ac);
				prms[ac].long_fallback = WLC_WME_RETRY_LFB_GET(wlc, ac);
			}
		}
		break;

	case IOV_SVAL(IOV_WME_TX_PARAMS):
		if (WME_PER_AC_TX_PARAMS_ENAB(wlc->pub)) {
			int ac;
			wme_tx_params_t *prms;

			prms = (wme_tx_params_t *)arg;

			wlc->LRL = prms[AC_BE].long_retry;
			wlc->SRL = prms[AC_BE].short_retry;
			wlc_bmac_retrylimit_upd(wlc->hw, wlc->SRL, wlc->LRL);

			for (ac = 0; ac < AC_COUNT; ac++) {
				wlc->wme_max_rate[ac] = prms[ac].max_rate;
				WLC_WME_RETRY_SHORT_SET(wlc, ac, prms[ac].short_retry);
				WLC_WME_RETRY_SFB_SET(wlc, ac, prms[ac].short_fallback);
				WLC_WME_RETRY_LONG_SET(wlc, ac, prms[ac].long_retry);
				WLC_WME_RETRY_LFB_SET(wlc, ac, prms[ac].long_fallback);
			}

			wlc_wme_retries_write(wlc);
		}
		break;

#endif /* WME_PER_AC_TUNING && PER_AC_TX_PARAMS */


	case IOV_SVAL(IOV_IBSS_COALESCE_ALLOWED):
	       wlc->ibss_coalesce_allowed = bool_val;
	       break;

	case IOV_GVAL(IOV_IBSS_COALESCE_ALLOWED):
	       *ret_int_ptr = (int32)wlc->ibss_coalesce_allowed;
	       break;

	case IOV_SVAL(IOV_MSGLEVEL2): {
		struct wl_msglevel2 *msglevel = (struct wl_msglevel2 *)arg;
		bcopy(&msglevel->low, &wl_msg_level, sizeof(uint32));
		bcopy(&msglevel->high, &wl_msg_level2, sizeof(uint32));
		break;
	}

	case IOV_GVAL(IOV_MSGLEVEL2): {
		struct wl_msglevel2 *msglevel = (struct wl_msglevel2 *)arg;
		msglevel->low = wl_msg_level;
		msglevel->high = wl_msg_level2;
		break;
	}

#ifdef STA
	case IOV_GVAL(IOV_PSPOLL_PRD):
		*ret_int_ptr = pm->pspoll_prd;
		break;

	case IOV_SVAL(IOV_PSPOLL_PRD):
		pm->pspoll_prd = (uint8)int_val;
		wlc_pspoll_timer_upd(bsscfg, TRUE);
		break;

	case IOV_GVAL(IOV_BCN_LI_BCN):
		*ret_int_ptr = wlc->bcn_li_bcn;
		break;

	case IOV_SVAL(IOV_BCN_LI_BCN):
		wlc->bcn_li_bcn = (uint8)int_val;
	        if (wlc->pub->up)
			wlc_bcn_li_upd(wlc);
		break;

	case IOV_GVAL(IOV_BCN_LI_DTIM):
		*ret_int_ptr = wlc->bcn_li_dtim;
		break;

	case IOV_SVAL(IOV_BCN_LI_DTIM):
		wlc->bcn_li_dtim = (uint8)int_val;
	        if (wlc->pub->up)
			wlc_bcn_li_upd(wlc);
		break;

	case IOV_GVAL(IOV_ASSOC_LISTEN):
		*ret_int_ptr = as->listen;
		break;

	case IOV_SVAL(IOV_ASSOC_LISTEN):
		as->listen = (uint16)int_val;
		break;

#ifdef WL_ASSOC_RECREATE
	case IOV_SVAL(IOV_ASSOC_RECREATE): {
		int i;
		wlc_bsscfg_t *bc;

		if (wlc->pub->_assoc_recreate == bool_val)
			break;

		/* if we are turning the feature on, nothing more to do */
		if (bool_val) {
			wlc->pub->_assoc_recreate = bool_val;
			break;
		}

		/* if we are turning the feature off clean up assoc recreate state */

		/* if we are in the process of an association recreation, abort it */
		if (as->type == AS_RECREATE)
			wlc_assoc_abort(bsscfg);
		FOREACH_BSS(wlc, i, bc) {
			/* if we are down with a STA bsscfg enabled due to the preserve
			 * flag, disable the bsscfg. Without the preserve flag we would
			 * normally disable the bsscfg as we put the driver into the down
			 * state.
			 */
			if (!wlc->pub->up && BSSCFG_STA(bc) && (bc->flags & WLC_BSSCFG_PRESERVE))
				wlc_bsscfg_disable(wlc, bc);

			/* clean up assoc recreate preserve flag */
			bc->flags &= ~WLC_BSSCFG_PRESERVE;
		}

		wlc->pub->_assoc_recreate = bool_val;
		break;
	}

	case IOV_GVAL(IOV_ASSOC_RECREATE):
		*ret_int_ptr = wlc->pub->_assoc_recreate;
		break;

	case IOV_GVAL(IOV_ASSOC_PRESERVED):
	        *ret_int_ptr = as->preserved;
		break;

	case IOV_SVAL(IOV_PRESERVE_ASSOC):
		if (!ASSOC_RECREATE_ENAB(wlc->pub) && bool_val) {
			err = BCME_BADOPTION;
			break;
		}

		if (bool_val)
			bsscfg->flags |= WLC_BSSCFG_PRESERVE;
		else
			bsscfg->flags &= ~WLC_BSSCFG_PRESERVE;
		break;

	case IOV_GVAL(IOV_PRESERVE_ASSOC):
		*ret_int_ptr = ((bsscfg->flags & WLC_BSSCFG_PRESERVE) != 0);
		break;

	case IOV_SVAL(IOV_ASSOC_VERIFY_TIMEOUT):
		as->verify_timeout = (uint)int_val;
		break;

	case IOV_GVAL(IOV_ASSOC_VERIFY_TIMEOUT):
		*ret_int_ptr = (int)as->verify_timeout;
		break;

	case IOV_SVAL(IOV_RECREATE_BI_TIMEOUT):
		as->recreate_bi_timeout = (uint)int_val;
		break;

	case IOV_GVAL(IOV_RECREATE_BI_TIMEOUT):
		*ret_int_ptr = (int)as->recreate_bi_timeout;
		break;
#endif /* WL_ASSOC_RECREATE */

	case IOV_GVAL(IOV_INFRA_CONFIGURATION):
		*ret_int_ptr = wlc->default_bss->infra;
		break;

	case IOV_SVAL(IOV_INFRA_CONFIGURATION):
		if (int_val > DOT11_BSSTYPE_ANY) {
			err = BCME_RANGE;
			break;
		}
		if (AP_ENAB(wlc->pub) &&
		    int_val != DOT11_BSSTYPE_INFRASTRUCTURE) {
			err = BCME_NOTSTA;
			break;
		}
		wlc->default_bss->infra = (uint)int_val;
		break;
#endif	/* STA */

#if defined(BCMDBG)
	case IOV_GVAL(IOV_RATE_HISTO):{
		int min_len = sizeof(wl_mac_ratehisto_res_t);
		ratespec_t most_used_rate_spec, highest_used_ratespec;
		wl_mac_ratehisto_res_t *rate_histo;

		if (len < min_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		/* Legacy rates immediately followed by mcs rates */
		rate_histo = (wl_mac_ratehisto_res_t *)arg;
		bzero(arg, sizeof(wl_mac_ratehisto_res_t));
		wlc_get_rate_histo_bsscfg(bsscfg, rate_histo, &most_used_rate_spec,
			&highest_used_ratespec);
		}
		break;
#endif 

	case IOV_GVAL(IOV_DOWN_OVERRIDE):
		*ret_int_ptr = (int32)wlc->down_override;
		break;

	case IOV_SVAL(IOV_DOWN_OVERRIDE):
		wlc->down_override = bool_val;
		break;


	case IOV_GVAL(IOV_ALLMULTI):
		*ret_int_ptr = (int32)bsscfg->allmulti;
		break;

	case IOV_SVAL(IOV_ALLMULTI):
		bsscfg->allmulti = bool_val;
		break;

	case IOV_GVAL(IOV_LEGACY_PROBE):
		*ret_int_ptr = (int32)wlc->legacy_probe;
		break;

	case IOV_SVAL(IOV_LEGACY_PROBE):
		wlc->legacy_probe = bool_val;
		break;

#ifdef AP
	case IOV_SVAL(IOV_WOWL_PKT): {
	        /* Send a Wake packet to a ea */
	        /* The format of this iovar params is
		 *   pkt_len
		 *   dst
		 *   Type of pkt - WL_WOWL_MAGIC or WL_WOWL_NET
		 *   for net pkt, wl_wowl_pattern_t
		 *   for magic pkt, dst ea, sta ea
		 */
		struct type_len_ea {
			uint16 pktlen;
			struct ether_addr dst;
			uint16 type;
		} tlea;

		wl_wowl_pattern_t wl_pattern;
		void *pkt;
		uint loc = 0;
		struct ether_header *eh;

		bcopy((uint8*)arg, (uint8 *) &tlea, sizeof(struct type_len_ea));
		loc += sizeof(struct type_len_ea);

		if (tlea.type  != WL_WOWL_NET &&
		    tlea.type  != WL_WOWL_MAGIC &&
			tlea.type  != WL_WOWL_EAPID) {
			err = BCME_BADARG;
			break;
		}

		if (tlea.type == WL_WOWL_NET) {
			uint8 *pattern;
			uint8 *buf;
			uint8 *wakeup_reason;

			bcopy(((uint8 *)arg + loc),
				(uint8 *) &wl_pattern, sizeof(wl_wowl_pattern_t));
			if (tlea.pktlen <
					(wl_pattern.offset + wl_pattern.patternsize
						+ wl_pattern.reasonsize)) {
				err = BCME_RANGE;
				break;
			}

			if ((pkt = PKTGET(wlc->osh, tlea.pktlen +
			                  sizeof(struct ether_header) + TXOFF,
			                  TRUE)) == NULL) {
				err = BCME_NOMEM;
				break;
			}

			PKTPULL(wlc->osh, pkt, TXOFF);

			eh = (struct ether_header *)PKTDATA(wlc->osh, pkt);
			buf = (uint8 *)eh;

			/* Go to end to find pattern */
			pattern = ((uint8*)arg + loc + wl_pattern.patternoffset);
			bcopy(pattern, &buf[wl_pattern.offset], wl_pattern.patternsize);
	                if (wl_pattern.reasonsize) {
				wakeup_reason = ((uint8*)pattern + wl_pattern.patternsize);
				bcopy(wakeup_reason,
					&buf[wl_pattern.offset + wl_pattern.patternsize],
					wl_pattern.reasonsize);
			}
			bcopy((char *)&wlc->pub->cur_etheraddr, (char *)eh->ether_shost,
			      ETHER_ADDR_LEN);
			if (wl_pattern.offset >= ETHER_ADDR_LEN)
				bcopy((char *)&tlea.dst, (char *)eh->ether_dhost, ETHER_ADDR_LEN);

			wl_msg_level |= WL_PRPKT_VAL;
			wlc_sendpkt(wlc, pkt, NULL);
			wl_msg_level &= ~WL_PRPKT_VAL;
		} else if (tlea.type == WL_WOWL_MAGIC) {
			struct ether_addr ea;
			uint8 *buf, ptr, extended_magic_pattern[6];
			bool extended_magic = FALSE;
			int i, j;

			if (tlea.pktlen < MAGIC_PKT_MINLEN) {
				err = BCME_RANGE;
				break;
			}

			bcopy(((uint8 *)arg + loc), (char *)&ea, ETHER_ADDR_LEN);

			if (wlc_iovar_op(wlc, "wowl_ext_magic", NULL, 0,
				extended_magic_pattern, sizeof(extended_magic_pattern),
				IOV_GET, NULL) == BCME_OK)
				extended_magic = TRUE;

			if ((pkt = PKTGET(wlc->osh, tlea.pktlen +
				(extended_magic ? sizeof(extended_magic_pattern) : 0) +
			                  sizeof(struct ether_header) + TXOFF,
			                  TRUE)) == NULL) {
				err = BCME_NOMEM;
				break;
			}
			PKTPULL(wlc->osh, pkt, TXOFF);
			eh = (struct ether_header *)PKTDATA(wlc->osh, pkt);
			bcopy((char *)&wlc->pub->cur_etheraddr, (char *)eh->ether_shost,
			      ETHER_ADDR_LEN);
			bcopy((char *)&tlea.dst, (char*)eh->ether_dhost, ETHER_ADDR_LEN);
			eh->ether_type = hton16(ETHER_TYPE_MIN);
			buf = ((uint8 *)eh + sizeof(struct ether_header));
			for (i = 0; i < 6; i++)
				buf[i] = 0xff;
			ptr = 6;
			for (j = 0; j < 16; j++) {
				bcopy(&ea, buf + ptr, ETHER_ADDR_LEN);
				ptr += 6;
			}
			if (extended_magic)
				bcopy(extended_magic_pattern,
					buf + ptr,
					sizeof(extended_magic_pattern));
			wl_msg_level |= WL_PRPKT_VAL;
			wlc_sendpkt(wlc, pkt, NULL);
			wl_msg_level &= ~WL_PRPKT_VAL;
		} else if (tlea.type == WL_WOWL_EAPID) {
			uint8 id_hdr[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01};
			uint8 id_len = *((uint8 *)arg + loc);
			uint16 body_len = 5 + id_len;
			uint8 *buf;

			if ((pkt = PKTGET(wlc->osh, sizeof(id_hdr) + id_len +
			                  sizeof(struct ether_header) + TXOFF,
			                  TRUE)) == NULL) {
				err = BCME_NOMEM;
				break;
			}
			PKTPULL(wlc->osh, pkt, TXOFF);

			eh = (struct ether_header *)PKTDATA(wlc->osh, pkt);
			bcopy((char *)&wlc->pub->cur_etheraddr, (char *)eh->ether_shost,
			      ETHER_ADDR_LEN);
			bcopy((char *)&tlea.dst, (char*)eh->ether_dhost, ETHER_ADDR_LEN);
			eh->ether_type = hton16(ETHER_TYPE_802_1X);

			*((uint16*)&id_hdr[2]) = hton16(body_len);
			*((uint16*)&id_hdr[6]) = hton16(body_len);

			buf = (uint8 *)eh + sizeof(struct ether_header);
			bcopy(id_hdr, buf, sizeof(id_hdr));
			buf += sizeof(id_hdr);
			bcopy((uint8*)arg + loc + 1, buf, id_len);

			wl_msg_level |= WL_PRPKT_VAL;
			wlc_sendpkt(wlc, pkt, NULL);
			wl_msg_level &= ~WL_PRPKT_VAL;
		} else
			err = BCME_UNSUPPORTED;
	}
		break;
#endif /* AP */

#ifdef STA
	case IOV_SVAL(IOV_ROAM_OFF):
		roam->off = bool_val;
		break;

	case IOV_GVAL(IOV_ROAM_OFF):
		*ret_int_ptr = (int32)roam->off;
		break;

	case IOV_GVAL(IOV_ASSOCROAM):
		*ret_int_ptr = (int32)roam->assocroam;
		break;

	case IOV_SVAL(IOV_ASSOCROAM):
		roam->assocroam = bool_val;
		break;
	case IOV_SVAL(IOV_ASSOCROAM_START):
		wlc_assoc_roam(bsscfg);
		break;
	case IOV_GVAL(IOV_FULLROAM_PERIOD):
		int_val = roam->fullscan_period;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_FULLROAM_PERIOD):
		roam->fullscan_period = int_val;
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;

	case IOV_GVAL(IOV_ROAM_PERIOD):
		int_val = roam->partialscan_period;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_ROAM_PERIOD):
		roam->partialscan_period = int_val;
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;

	case IOV_GVAL(IOV_ROAM_NFSCAN):
		*ret_int_ptr = (int32)bsscfg->roam->nfullscans;
		break;

	case IOV_SVAL(IOV_ROAM_NFSCAN):
		if (int_val > 0) {
			bsscfg->roam->nfullscans = (uint8)int_val;
			wlc_roam_prof_update_default(wlc, bsscfg);
		} else
			err = BCME_BADARG;
		break;

	case IOV_GVAL(IOV_TXFAIL_ROAMTHRESH):
		int_val = roam->minrate_txfail_thresh;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_TXFAIL_ROAMTHRESH):
		roam->minrate_txfail_thresh = int_val;
		break;

	/* Start roam scans if we are sending too often at the min TX rate */
	/* Setting to zero disables this feature */
	case IOV_GVAL(IOV_TXMIN_ROAMTHRESH):
		*ret_int_ptr = roam->minrate_txpass_thresh;
		break;

	case IOV_SVAL(IOV_TXMIN_ROAMTHRESH):
	        roam->minrate_txpass_thresh = (uint8)int_val;
	        /* this must be set to turn off the trigger */
	        roam->txpass_thresh = (uint8)int_val;
		break;

	/* Auto-detect the AP density within the environment. '0' disables all auto-detection.
	 * Setting to AP_ENV_INDETERMINATE turns on auto-detection
	 */
	case IOV_GVAL(IOV_AP_ENV_DETECT):
	        *ret_int_ptr = (int32)roam->ap_environment;
		break;

	case IOV_SVAL(IOV_AP_ENV_DETECT):
	        roam->ap_environment = (uint8)int_val;
		break;

	/* change roaming behavior if we detect that the STA is moving */
	case IOV_GVAL(IOV_MOTION_RSSI_DELTA):
	        *ret_int_ptr = roam->motion_rssi_delta;
	        break;

	case IOV_SVAL(IOV_MOTION_RSSI_DELTA):
	        roam->motion_rssi_delta = (uint8)int_val;
	        break;

	/* Piggyback roam scans on periodic upper-layer scans, e.g. the 63sec WinXP WZC scan */
	case IOV_GVAL(IOV_SCAN_PIGGYBACK):
	        *ret_int_ptr = (int32)roam->piggyback_enab;
		break;

	case IOV_SVAL(IOV_SCAN_PIGGYBACK):
	        roam->piggyback_enab = bool_val;
		break;

	case IOV_GVAL(IOV_ROAM_RSSI_CANCEL_HYSTERESIS):
		*ret_int_ptr = wlc->roam_rssi_cancel_hysteresis;
		break;

	case IOV_SVAL(IOV_ROAM_RSSI_CANCEL_HYSTERESIS):
		wlc->roam_rssi_cancel_hysteresis = (uint8)int_val;
		break;

#if defined(WL_PM2_RCV_DUR_LIMIT)
	case IOV_GVAL(IOV_PM2_RCV_DUR):
		*ret_int_ptr = pm->pm2_rcv_percent;
		break;

	case IOV_SVAL(IOV_PM2_RCV_DUR):
		if (int_val != 0) {
			if (int_val >= WLC_PM2_RCV_DUR_MIN &&
			    int_val <= WLC_PM2_RCV_DUR_MAX) {
				pm->pm2_rcv_percent = int_val;
				pm->pm2_rcv_time = current_bss->beacon_period *
				    pm->pm2_rcv_percent/100;
			} else {
				err = BCME_RANGE;
			}
		}
		else {
			/* Setting pm2_rcv_dur = 0 disables the Receive Throttle feature */
			pm->pm2_rcv_percent = 0;
			wlc_pm2_rcv_reset(bsscfg);
		}
		break;

#endif /* WL_PM2_RCV_DUR_LIMIT */

	case IOV_GVAL(IOV_PM2_SLEEP_RET):
		*ret_int_ptr = (uint32)pm->pm2_sleep_ret_time;
		break;

	case IOV_SVAL(IOV_PM2_SLEEP_RET):
		if ((int_val < WLC_PM2_TICK_MS) || (int_val > WLC_PM2_MAX_MS)) {
			WL_ERROR(("%s: should be in range %d-%d\n",
				name, WLC_PM2_TICK_MS, WLC_PM2_MAX_MS));
			err = BCME_RANGE;
			break;
		}
		pm->pm2_sleep_ret_time = (uint)int_val;
		pm->pm2_sleep_ret_time_left = pm->pm2_sleep_ret_time;
		break;

	case IOV_GVAL(IOV_PM2_SLEEP_RET_EXT): {
		wl_pm2_sleep_ret_ext_t *sleep_ret_ext = (wl_pm2_sleep_ret_ext_t *)arg;
		bzero(sleep_ret_ext, sizeof(wl_pm2_sleep_ret_ext_t));
		sleep_ret_ext->logic = pm->dfrts_logic;
		if (sleep_ret_ext->logic != WL_DFRTS_LOGIC_OFF) {
			sleep_ret_ext->low_ms = (uint16) pm->pm2_sleep_ret_time;
			sleep_ret_ext->high_ms = pm->dfrts_high_ms;
			sleep_ret_ext->rx_pkts_threshold = pm->dfrts_rx_pkts_threshold;
			sleep_ret_ext->tx_pkts_threshold = pm->dfrts_tx_pkts_threshold;
			sleep_ret_ext->txrx_pkts_threshold = pm->dfrts_txrx_pkts_threshold;
			sleep_ret_ext->rx_bytes_threshold = pm->dfrts_rx_bytes_threshold;
			sleep_ret_ext->tx_bytes_threshold = pm->dfrts_tx_bytes_threshold;
			sleep_ret_ext->txrx_bytes_threshold = pm->dfrts_txrx_bytes_threshold;
		}
		break;
	}
	case IOV_SVAL(IOV_PM2_SLEEP_RET_EXT): {
		wl_pm2_sleep_ret_ext_t *sleep_ret_ext = (wl_pm2_sleep_ret_ext_t *)arg;
		pm->dfrts_logic = (uint8)sleep_ret_ext->logic;
		if (sleep_ret_ext->logic == WL_DFRTS_LOGIC_OFF) {
			/* Disable this feature */
			pm->pm2_sleep_ret_time = PM2_SLEEP_RET_MS_DEFAULT;
		} else {
			pm->pm2_sleep_ret_time = (uint)sleep_ret_ext->low_ms;
			pm->dfrts_high_ms = (uint16)sleep_ret_ext->high_ms;
			pm->dfrts_rx_pkts_threshold = (uint16)sleep_ret_ext->rx_pkts_threshold;
			pm->dfrts_tx_pkts_threshold = (uint16)sleep_ret_ext->tx_pkts_threshold;
			pm->dfrts_txrx_pkts_threshold = (uint16)sleep_ret_ext->txrx_pkts_threshold;
			pm->dfrts_rx_bytes_threshold = (uint32)sleep_ret_ext->rx_bytes_threshold;
			pm->dfrts_tx_bytes_threshold = (uint32)sleep_ret_ext->tx_bytes_threshold;
			pm->dfrts_txrx_bytes_threshold =
				(uint32)sleep_ret_ext->txrx_bytes_threshold;
		}
		wlc_dfrts_reset_counters(bsscfg);
		break;
	}
	case IOV_GVAL(IOV_PM2_REFRESH_BADIV):
		*ret_int_ptr = pm->pm2_refresh_badiv;
		break;

	case IOV_SVAL(IOV_PM2_REFRESH_BADIV):
		pm->pm2_refresh_badiv = bool_val;
		break;

	case IOV_GVAL(IOV_NOLINKUP):
		*ret_int_ptr = wlc->nolinkup;
		break;

	case IOV_SVAL(IOV_NOLINKUP):
		wlc->nolinkup = bool_val;
		break;
#endif /* STA */

	case IOV_GVAL(IOV_BANDUNIT):
		*ret_int_ptr = wlc->band->bandunit;
		break;



#if defined(STA) && defined(ADV_PS_POLL)
	case IOV_GVAL(IOV_ADV_PS_POLL):
		*ret_int_ptr = (int32)pm->adv_ps_poll;
		break;

	case IOV_SVAL(IOV_ADV_PS_POLL):
		pm->adv_ps_poll = bool_val;
		break;
#endif

#if defined(BCMDBG) && defined(MBSS)
	case IOV_GVAL(IOV_SRCHMEM): {
		uint reg = (uint)int_val;

		if (reg > 15) {
			err = BCME_BADADDR;
			break;
		}

		wlc_bmac_copyfrom_objmem(wlc->hw, reg * SHM_MBSS_SSIDSE_BLKSZ,
			arg, SHM_MBSS_SSIDSE_BLKSZ, OBJADDR_SRCHM_SEL);
		break;
	}
	case IOV_SVAL(IOV_SRCHMEM): {
		uint reg = (uint)int_val;
		int *parg = (int *)arg;

		/* srchmem set params:
		 *	uint32		register num
		 *	uint32		ssid len
		 *	int8[32]	ssid name
		 */

		if (reg > 15) {
			err = BCME_BADADDR;
			break;
		}

		wlc_bmac_copyto_objmem(wlc->hw, reg * SHM_MBSS_SSIDSE_BLKSZ,
			(parg + 1), SHM_MBSS_SSIDSE_BLKSZ, OBJADDR_SRCHM_SEL);
		break;
	}
#endif	/* BCMDBG && MBSS */

#ifdef WLC_HIGH_ONLY
	case IOV_SVAL(IOV_BMAC_REBOOT):
		wlc_bmac_dngl_reboot(wlc->rpc);
		break;

	case IOV_GVAL(IOV_RPC_AGG):
		*ret_int_ptr = (int32)wlc->rpc_agg;
		break;

	case IOV_SVAL(IOV_RPC_AGG):
		wlc->rpc_agg = int_val;

		/* high 16 bits are for host->dongle agg, use it directly */

		wlc_bmac_dngl_rpc_agg(wlc->rpc, wlc->rpc_agg & 0xffff);
		break;

	case IOV_GVAL(IOV_RPC_MSGLEVEL):
		*ret_int_ptr = (int32)wlc->rpc_msglevel;
		break;

	case IOV_SVAL(IOV_RPC_MSGLEVEL):
		wlc->rpc_msglevel = int_val;

		bcm_rpc_msglevel_set(wlc->rpc, (uint16)(wlc->rpc_msglevel >> 16), TRUE);
		wlc_bmac_dngl_rpc_msglevel(wlc->rpc, (uint16)(wlc->rpc_msglevel & 0xffff));
		break;

	case IOV_GVAL(IOV_RPC_DNGL_TXQ_WM):
		wlc_bmac_dngl_rpc_txq_wm_get(wlc->rpc, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_RPC_DNGL_TXQ_WM):
		wlc_bmac_dngl_rpc_txq_wm_set(wlc->rpc, (uint32)int_val);
		break;

	case IOV_GVAL(IOV_RPC_DNGL_AGG_LIMIT):
		wlc_bmac_dngl_rpc_agg_limit_get(wlc->rpc, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_RPC_DNGL_AGG_LIMIT):
		wlc_bmac_dngl_rpc_agg_limit_set(wlc->rpc, (uint32)int_val);
		break;

	case IOV_GVAL(IOV_RPC_HOST_AGG_LIMIT): {
		rpc_tp_info_t *rpcb = bcm_rpc_tp_get(wlc->rpc);
		uint8 sf;
		uint16 bytes;

		bcm_rpc_tp_agg_limit_get(rpcb, &sf, &bytes);
		*ret_int_ptr = (uint32)((sf << 16) + bytes);
		break;

	}

	case IOV_SVAL(IOV_RPC_HOST_AGG_LIMIT): {
		rpc_tp_info_t *rpcb = bcm_rpc_tp_get(wlc->rpc);
		bcm_rpc_tp_agg_limit_set(rpcb, (int_val >> 16)&0xFFFF, (int_val & 0xFFFF));
		break;
	}

	case IOV_GVAL(IOV_BMAC_DNGL_SUSPEND_ENABLE):
		bcm_rpc_dngl_suspend_enable_get(wlc->rpc, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_BMAC_DNGL_SUSPEND_ENABLE):
		bcm_rpc_dngl_suspend_enable_set(wlc->rpc, (uint32)int_val);
		break;
#endif	/* WLC_HIGH_ONLY */

#ifdef STA
	case IOV_GVAL(IOV_WAKE_EVENT):
		*ret_int_ptr = wlc->wake_event_timeout ? 1 : 0;
		break;

	case IOV_SVAL(IOV_WAKE_EVENT): {
		uint16 timeout = (uint16) int_val;

		if (timeout < 1 || timeout > 120) {
			err = BCME_BADARG;
			break;
		}
		wlc->wake_event_timeout = timeout;
		wl_del_timer(wlc->wl, wlc->wake_event_timer);
		wl_add_timer(wlc->wl, wlc->wake_event_timer, timeout * 1000, FALSE);
		break;
		}
#endif /* STA */

	case IOV_SVAL(IOV_NAV_RESET_WAR_DISABLE):
		wlc_nav_reset_war(wlc, bool_val);
		break;

	case IOV_GVAL(IOV_NAV_RESET_WAR_DISABLE):
		*ret_int_ptr = wlc->nav_reset_war_disable;
		break;

	case IOV_GVAL(IOV_RFAWARE_LIFETIME):
		*ret_int_ptr = (uint32)wlc->rfaware_lifetime;
		break;

	case IOV_SVAL(IOV_RFAWARE_LIFETIME):
		err = wlc_rfaware_lifetime_set(wlc, (uint16)int_val);
		break;


	case IOV_GVAL(IOV_ASSERT_TYPE):
		*ret_int_ptr = g_assert_type;

		break;

	case IOV_SVAL(IOV_ASSERT_TYPE):
		g_assert_type = (uint32)int_val;
#ifdef WLC_HIGH_ONLY
		wlc_bmac_assert_type_set(wlc->hw, g_assert_type);
#endif
		break;

#ifdef PHYCAL_CACHING
	case IOV_GVAL(IOV_PHYCAL_CACHE):
	        *ret_int_ptr = (int32)wlc_bmac_get_phycal_cache_flag(wlc->hw);
		break;

	case IOV_SVAL(IOV_PHYCAL_CACHE):
	        if (!bool_val) {
			if (wlc_bmac_get_phycal_cache_flag(wlc->hw)) {
				wlc_phy_cal_cache_deinit(pi);
				wlc_bmac_set_phycal_cache_flag(wlc->hw, FALSE);
				wlc_iovar_setint(wlc, "phy_percal", PHY_PERICAL_MPHASE);
			}
			break;
		}
		/* enable caching, break association, compute TX/RX IQ Cal and RSSI cal values */
		else {
			if (wlc_bmac_get_phycal_cache_flag(wlc->hw))
				break;

			if (!(WLCISNPHY(wlc->band) && NREV_GE(wlc->band->phyrev, 3)) &&
			    !WLCISHTPHY(wlc->band)) {
				WL_ERROR(("wl%d: %s: %s: Caching only supported "
					"on nphy rev 3 or newer and htphy\n",
					wlc->pub->unit, __FUNCTION__, name));
				err = BCME_UNSUPPORTED;
			}
			else if (wlc_phy_cal_cache_init(pi) == BCME_OK)
				err = wlc_cache_cals(wlc);
			else
				err = BCME_NOMEM;
			break;
		}

	case IOV_GVAL(IOV_CACHE_CHANSWEEP):
	        if (wlc_bmac_get_phycal_cache_flag(wlc->hw))
			*ret_int_ptr = wlc_cachedcal_sweep(wlc);
		else {
			*ret_int_ptr = BCME_UNSUPPORTED;
			WL_ERROR(("wl%d: %s: %s: PhyCal caching not enabled, "
			          "can't scan\n", wlc->pub->unit, __FUNCTION__, name));
		}
		break;

	case IOV_SVAL(IOV_CACHE_CHANSWEEP):
	        if (wlc_bmac_get_phycal_cache_flag(wlc->hw)) {
			if (int_val > 0 && int_val < MAXCHANNEL)
				err = wlc_cachedcal_tune(wlc, (uint16) int_val);
			else if (!wf_chspec_malformed((uint16)int_val))
				err = wlc_cachedcal_tune(wlc,
				                         (uint16) int_val & WL_CHANSPEC_CHAN_MASK);
			else
				err = BCME_BADCHAN;
		} else {
			err = BCME_UNSUPPORTED;
			WL_ERROR(("wl%d: %s: %s: PhyCal caching not enabled, "
			          "can't scan\n", wlc->pub->unit, __FUNCTION__, name));
		}
		break;

#endif /* PHYCAL_CACHING */

	case IOV_GVAL(IOV_PM2_RADIO_SHUTOFF_DLY):
		*ret_int_ptr = wlc->pm2_radio_shutoff_dly;
		break;

	case IOV_SVAL(IOV_PM2_RADIO_SHUTOFF_DLY): {
		uint16 dly = (uint16) int_val;

		if (dly > 10) {
			err = BCME_BADARG;
			break;
		}
		wlc->pm2_radio_shutoff_dly = dly;
		break;
	}

#ifdef SMF_STATS
	case IOV_GVAL(IOV_SMF_STATS): {
		int idx = int_val;
		if (len < (int)sizeof(wl_smf_stats_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		err = wlc_bsscfg_get_smfs(bsscfg, idx, arg, len);
		break;
	}
	case IOV_SVAL(IOV_SMF_STATS):
		err = wlc_bsscfg_clear_smfs(wlc, bsscfg);
		break;

	case IOV_GVAL(IOV_SMF_STATS_ENABLE):
		*ret_int_ptr = bsscfg->smfs_info->enable;
		break;

	case IOV_SVAL(IOV_SMF_STATS_ENABLE):
		bsscfg->smfs_info->enable = int_val;
		break;

#endif /* SMF_STATS */
#if defined(WLTEST)
	case IOV_GVAL(IOV_MANF_INFO):
		err = wlc_manf_info_get(wlc, (char *)arg, len, wlcif);
		break;
#endif 
	case IOV_GVAL(IOV_POOL):
		if (POOL_ENAB(wlc->pub->pktpool))
			*ret_int_ptr = pktpool_len(wlc->pub->pktpool);
		else
			*ret_int_ptr = 0;
		break;
	case IOV_SVAL(IOV_POOL):
		if (!POOL_ENAB(wlc->pub->pktpool)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (pktpool_setmaxlen(wlc->pub->pktpool, (uint16)int_val) != int_val) {
			err = BCME_BADLEN;
			break;
		}

#ifdef DMATXRC
		if (DMATXRC_ENAB(wlc->pub) && PHDR_ENAB(wlc)) {
			if (pktpool_setmaxlen(PHDR_POOL(wlc), (uint16)int_val) != int_val) {
				err = BCME_BADLEN;
				break;
			}
			wlc->phdr_len = int_val;
		}
#endif
		break;

	case IOV_GVAL(IOV_CURR_RATESET): {
		wl_rateset_args_t *ret_rs = (wl_rateset_args_t *)arg;
		wlc_rateset_t *rs;
#ifdef WL11AC
		uint8 bw;
#endif /* WL11AC */

		if (wlc->pub->associated) {
			rs = &bsscfg->current_bss->rateset;
#ifdef WL11AC
			bw = WLC_GET_BW_FROM_CHSPEC(bsscfg->current_bss->chanspec);
#endif /* WL11AC */
		} else {
			rs = &wlc->default_bss->rateset;
#ifdef WL11AC
			bw = BW_20MHZ;
#endif /* WL11AC */
		}

		if (len < (int)(rs->count + sizeof(rs->count))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* Copy legacy rateset and mcsset */
		ret_rs->count = rs->count;
		bcopy(&rs->rates, &ret_rs->rates, rs->count);

		if (N_ENAB(wlc->pub))
			bcopy(rs->mcs, ret_rs->mcs, MCSSET_LEN);
#ifdef WL11AC
		if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype)) {
			uint8 nss, mcs_code;
			for (nss = 1; nss <= wlc->stf->txstreams; nss++) {
				mcs_code = VHT_MCS_MAP_GET_MCS_PER_SS(nss, rs->vht_mcsmap);
				if (wlc->pub->associated) {
					uint8 vht_ratemask = 0;
					if AP_ENAB(wlc->pub) {
						vht_ratemask = BAND_5G(wlc->band->bandtype) ?
							WLC_VHT_FEATURES_RATES_5G(wlc->pub) :
							WLC_VHT_FEATURES_RATES_2G(wlc->pub);
					} else {
						/* Look for scb corresponding to the bss */
						struct scb *scb = wlc_scbfind(wlc,
							bsscfg, &bsscfg->current_bss->BSSID);
						if (scb) {
							vht_ratemask =
								wlc_vht_get_scb_ratemask(
									wlc->vhti, scb);
							vht_ratemask &=
								BAND_5G(wlc->band->bandtype) ?
								~(WL_VHT_FEATURES_RATES_2G) :
								~(WL_VHT_FEATURES_RATES_5G);
						} else {
							vht_ratemask =
								BAND_5G(wlc->band->bandtype) ?
								WLC_VHT_FEATURES_RATES_5G(wlc->pub)
								:
								WLC_VHT_FEATURES_RATES_2G(wlc->pub);
						}
					}

					ret_rs->vht_mcs[nss - 1] =
						wlc_get_valid_vht_mcsmap(mcs_code, bw,
						((wlc->stf->ldpc_tx == AUTO)||
						(wlc->stf->ldpc_tx == ON)),
						nss, vht_ratemask);
				} else {
					ret_rs->vht_mcs[nss - 1] =
						VHT_MCS_CODE_TO_MCS_MAP(mcs_code);
				}
			}
		}
#endif /* WL11AC */
		break;
	}

	case IOV_GVAL(IOV_RATESET): {
		wlc_rateset_t rs;
		wl_rateset_args_t *ret_rs = (wl_rateset_args_t *)arg;

		bzero(&rs, sizeof(wlc_rateset_t));
		wlc_default_rateset(wlc, (wlc_rateset_t*)&rs);

		if (len < (int)(rs.count + sizeof(rs.count))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* Copy legacy rateset */
		ret_rs->count = rs.count;
		bcopy(&rs.rates, &ret_rs->rates, rs.count);

		/* Copy mcs rateset */
		if (N_ENAB(wlc->pub))
			bcopy(rs.mcs, ret_rs->mcs, MCSSET_LEN);
		/* copy vht mcs set */
		if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype)) {
			uint8 nss, mcs_code;
			for (nss = 1; nss <= wlc->stf->txstreams; nss++) {
				mcs_code = VHT_MCS_MAP_GET_MCS_PER_SS(nss, rs.vht_mcsmap);
				ret_rs->vht_mcs[nss - 1] = VHT_MCS_CODE_TO_MCS_MAP(mcs_code);
			}
		}
		break;
	}

	case IOV_SVAL(IOV_RATESET): {
		wlc_rateset_t rs;
		wl_rateset_args_t *in_rs = (wl_rateset_args_t *)arg;

		if (len < (int)(in_rs->count + sizeof(in_rs->count))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		bzero(&rs, sizeof(wlc_rateset_t));

		/* Copy legacy rateset */
		rs.count = in_rs->count;
		bcopy(&in_rs->rates, &rs.rates, rs.count);

		/* Copy mcs rateset */
		if (N_ENAB(wlc->pub))
			bcopy(in_rs->mcs, rs.mcs, MCSSET_LEN);
#if defined(WL11AC)
		if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype)) {
			/* Convert user level vht_mcs rate masks to driver level rateset map. */
			int nss;

			for (nss = 1; nss <= VHT_CAP_MCS_MAP_NSS_MAX; ++nss) {

				VHT_MCS_MAP_SET_MCS_PER_SS(nss,
					VHT_MCS_MAP_TO_MCS_CODE(in_rs->vht_mcs[nss-1]),
					rs.vht_mcsmap);
			}
		}
#endif /* WL11AC */
		err = wlc_set_rateset(wlc, &rs);

		break;
	}

#if defined(BCM_DCS) && defined(STA)
	case IOV_SVAL(IOV_BRCM_DCS_REQ): {

		chanspec_t chspec = (chanspec_t) int_val;

		if (SCAN_IN_PROGRESS(wlc->scan)) {
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
			err = BCME_BUSY;
			break;
		}

		if (wf_chspec_malformed(chspec) || !wlc_valid_chanspec(wlc->cmi, chspec)) {
			err = BCME_BADCHAN;
			break;
		}

		if (BSSCFG_STA(bsscfg) && bsscfg->associated) {
			wl_bcmdcs_data_t bcmdcs_data;
			bcmdcs_data.reason = BCM_DCS_IOVAR;
			bcmdcs_data.chspec = chspec;
			err = wlc_send_action_brcm_dcs(wlc, &bcmdcs_data,
				wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID));
		}

		break;
	}
#endif /* BCM_DCS && STA */


#if defined(PROP_TXSTATUS)
	case IOV_GVAL(IOV_TLV_SIGNAL):
		*ret_int_ptr = wlc->wlfc_flags;
		break;
	case IOV_SVAL(IOV_TLV_SIGNAL):
		wlc->wlfc_flags = int_val;
		if (!HOST_PROPTXSTATUS_ACTIVATED(wlc))
			wl_reset_credittohost(wlc->wl);
		break;
#endif

	case IOV_GVAL(IOV_PM_DUR):
		*ret_int_ptr = wlc_get_accum_pmdur(wlc);
		break;

	/* iovar versions of WLC_SCB_AUTHORIZE et al */
	case IOV_SVAL(IOV_AUTHOPS): {
		authops_t *pp = params;

		switch (pp->code) {
			/* these only get an ea pointer */
			case WLC_SCB_AUTHORIZE:
			case WLC_SCB_DEAUTHORIZE:
				err = wlc_ioctl(wlc, pp->code, &pp->ioctl_args.ea,
					sizeof(pp->ioctl_args.ea), wlcif);
				break;
			default:
			/* the rest get an scb_val_t pointer */
				err = wlc_ioctl(wlc, pp->code, &pp->ioctl_args,
					sizeof(pp->ioctl_args), wlcif);
		}
		break;
	}
#ifdef DNGL_WD_KEEP_ALIVE
	case IOV_GVAL(IOV_DNGL_WD_KEEP_ALIVE):
		*ret_int_ptr = (int)wlc->dngl_wd_keep_alive;
		break;
	case IOV_SVAL(IOV_DNGL_WD_KEEP_ALIVE):
			wlc->dngl_wd_keep_alive = bool_val;
			delay = bool_val ? TIMER_INTERVAL_DNGL_WATCHDOG : 0;
			wlc_bmac_dngl_wd_keep_alive(wlc->hw, delay);
		break;
	case IOV_SVAL(IOV_DNGL_WD_FORCE_TRIGGER):
			wlc_bmac_dngl_wd_keep_alive(wlc->hw, int_val);
		break;
#endif
	case IOV_SVAL(IOV_IS_WPS_ENROLLEE):
		if (BSSCFG_STA(bsscfg))
			bsscfg->is_WPS_enrollee = bool_val;
		else
			err = BCME_NOTSTA;
		break;

	case IOV_GVAL(IOV_IS_WPS_ENROLLEE):
		if (BSSCFG_STA(bsscfg))
			*ret_int_ptr = (int32)bsscfg->is_WPS_enrollee;
		else
			err = BCME_NOTSTA;
		break;


#ifdef BCMDBG
	case IOV_SVAL(IOV_TSF):
		wlc_tsf_set(wlc, int_val, int_val2);
		break;

	case IOV_GVAL(IOV_TSF): {
		uint32 tsf_l, tsf_h;
		wlc_read_tsf(wlc, &tsf_l, &tsf_h);
		((uint32*)arg)[0] = tsf_l;
		((uint32*)arg)[1] = tsf_h;
		break;
	}

	case IOV_SVAL(IOV_TSF_ADJUST):
		wlc_tsf_adjust(wlc, int_val);
		break;
#endif /* BCMDBG */

	case IOV_GVAL(IOV_WLIF_BSSCFG_IDX):
		*ret_int_ptr = WLC_BSSCFG_IDX(bsscfg);
		break;

	case IOV_GVAL(IOV_SSID): {
		wlc_ssid_t ssid;
		int ssid_copy_len;

		/* copy the bsscfg's SSID to the on-stack return structure */
		ssid.SSID_len = bsscfg->SSID_len;
		if (ssid.SSID_len)
			bcopy(bsscfg->SSID, ssid.SSID, ssid.SSID_len);

		ssid_copy_len = (int)(sizeof(ssid.SSID_len) + ssid.SSID_len);
		if (len < ssid_copy_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		bcopy(&ssid, arg, ssid_copy_len);
		break;
	}

	case IOV_SVAL(IOV_SSID): {
		int32 ssidlen;
		uchar* ssid_ptr;
#if defined(BCMDBG) || defined(WLMSG_OID)
		char ssidbuf[SSID_FMT_BUF_LEN];
#endif
		/* input buffer is:
		 * uint32 ssid_len      (local int_val)
		 * char   ssid[]	(local arg + 4)
		 */

		/* get the SSID length from the wlc_ssid_t struct starting just past the
		 * bsscfg index
		 */
		ssidlen = int_val;

		if (ssidlen < 0 || ssidlen > DOT11_MAX_SSID_LEN) {
			err = BCME_BADSSIDLEN;
			break;
		}

		if (len < (int)(sizeof(int32) + ssidlen)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		ssid_ptr = (uchar*)arg + sizeof(int32);

		WL_OID(("wl: wlc_iovar_op: setting SSID of bsscfg %d to \"%s\"\n",
		 WLC_BSSCFG_IDX(bsscfg),
			(wlc_format_ssid(ssidbuf, ssid_ptr, ssidlen), ssidbuf)));

		/* can only change SSID if the config is down */
		if (bsscfg->up) {
			err = BCME_NOTDOWN;
			break;
		}
		wlc_bsscfg_SSID_set(bsscfg, ssid_ptr, ssidlen);

		break;
	}


#ifdef STA
	case IOV_SVAL(IOV_JOIN): {
		wlc_ssid_t *ssid = (wlc_ssid_t *)arg;
		wl_join_scan_params_t *scan_params = NULL;
		wl_join_assoc_params_t *assoc_params = NULL;
		int assoc_params_len = 0;

		scan_params = &((wl_extjoin_params_t *)arg)->scan;

		if ((uint)len >= WL_EXTJOIN_PARAMS_FIXED_SIZE) {
			assoc_params = &((wl_extjoin_params_t *)arg)->assoc;
			assoc_params_len = len - OFFSETOF(wl_extjoin_params_t, assoc);
			err = wlc_assoc_chanspec_sanitize(wlc,
			        (chanspec_list_t *)&assoc_params->chanspec_num,
			        len - OFFSETOF(wl_extjoin_params_t, assoc.chanspec_num));
			if (err != BCME_OK)
				break;
#ifdef WLRCC
			if (WLRCC_ENAB(wlc->pub) && (bsscfg->roam->rcc_mode != RCC_MODE_FORCE))
				rcc_update_channel_list(bsscfg->roam, ssid, assoc_params);
#endif
		}
		WL_APSTA_UPDN(("wl%d: \"join\" -> wlc_join()\n", wlc->pub->unit));
		err = wlc_join(wlc, bsscfg, ssid->SSID, load32_ua((uint8 *)&ssid->SSID_len),
		         scan_params,
		         assoc_params, assoc_params_len);
		break;
	}
#endif /* STA */


	case IOV_GVAL(IOV_RPT_HITXRATE):
		*ret_int_ptr = wlc->rpt_hitxrate;
		break;

	case IOV_SVAL(IOV_RPT_HITXRATE):
		wlc->rpt_hitxrate = bool_val;
		break;

#ifdef WLRXOV
	case IOV_SVAL(IOV_RXOV):
		wlc->pub->_rxov = bool_val;
		if (WLRXOV_ENAB(wlc->pub) && (wlc->rxov_timer == NULL)) {
			wlc->rxov_timer = wl_init_timer(wlc->wl, wlc_rxov_timer, wlc, "rxov");
			ASSERT(wlc->rxov_timer);
		}

		if (WLRXOV_ENAB(wlc->pub) && wlc->rxov_timer) {
			/* wlc_write_shm(wlc, M_RXOV_HIWAT, RXOV_MPDU_HIWAT); */
			wlc_bmac_set_defmacintmask(wlc->hw, MI_RXOV, MI_RXOV);
		} else
			wlc_bmac_set_defmacintmask(wlc->hw, MI_RXOV, ~MI_RXOV);
		break;
	case IOV_GVAL(IOV_RXOV):
		*ret_int_ptr = wlc->pub->_rxov;
		WL_TRACE(("rxov hiwat: %d\n", wlc_read_shm(wlc, M_RXOV_HIWAT)));
		break;
#endif /* WLRXOV */

#ifdef BCMDBG
#ifdef STA
	case IOV_SVAL(IOV_RPMT): {
		uint32 rpmt_1_prd = load32_ua((uint8 *)arg);
		uint32 rpmt_0_prd = load32_ua((uint8 *)arg + sizeof(uint32));
		bool enab = rpmt_1_prd != 0 && rpmt_0_prd != 0;

		if (enab) {
			if (bsscfg->rpmt_timer != NULL) {
				err = BCME_BUSY;
				break;
			}
			if ((bsscfg->rpmt_timer = wlc_hrt_alloc_timeout(wlc->hrti)) == NULL) {
				WL_ERROR(("wl%d.%d: failed to alloc Rapid PM Transition timeout\n",
				          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
				err = BCME_NORESOURCE;
				break;
			}
			bsscfg->rpmt_1_prd = rpmt_1_prd;
			bsscfg->rpmt_0_prd = rpmt_0_prd;
			bsscfg->rpmt_n_st = 1;
			wlc_rpmt_timer_cb(bsscfg);
		}
		else {
			if (bsscfg->rpmt_timer == NULL) {
				err = BCME_ERROR;
				break;
			}
			wlc_hrt_del_timeout(bsscfg->rpmt_timer);
			wlc_hrt_free_timeout(bsscfg->rpmt_timer);
			bsscfg->rpmt_timer = NULL;
		}
		break;
	}
#endif /* STA */
#endif /* BCMDBG */

#ifdef BPRESET
	case IOV_SVAL(IOV_BPRESET_ENAB):
	        /* configure value for _bpreset */
	        wlc->pub->_bpreset = (int_val != 0);
	        break;
	case IOV_GVAL(IOV_BPRESET_ENAB):
	        *ret_int_ptr = (int)wlc->pub->_bpreset;
	        break;
#ifdef BCMDBG
	case IOV_SVAL(IOV_BACKPLANE_RESET):
		wlc_full_reset(wlc->hw, (uint32)int_val);
		break;
#endif /* BCMDBG */
#endif /* BPRESET */

#ifdef PKTQ_LOG
	case IOV_GVAL(IOV_PKTQ_STATS):	{
		struct bcmstrbuf b;
		int32  avail_len;
		uint8 params_version;
		wl_iov_pktq_log_t*  iov_pktq_log_p  =  (wl_iov_pktq_log_t*) arg;
		wl_iov_mac_full_params_t iov_full_params;

		/* keep a local copy of the input parameter block */
		iov_full_params.params = *(wl_iov_mac_params_t*) params;

		params_version = (iov_full_params.params.num_addrs >> 8) & 0xFF;
		iov_full_params.params.num_addrs &= ~(0xFF << 8);

		if (params_version == 4) {
			/* this means the "extra_params" is there */
			iov_full_params.extra_params =
			          ((wl_iov_mac_full_params_t*) params)->extra_params;
		}

		/* clear return parameter block */
		bzero(iov_pktq_log_p, sizeof(*iov_pktq_log_p));

		iov_pktq_log_p->version = 0x05;

		/* Copy back the input parameter block */
		iov_pktq_log_p->params = iov_full_params.params;

		/* Setup character string return block */
		avail_len = len - OFFSETOF(wl_iov_pktq_log_t, pktq_log.v05.headings);
		avail_len = (avail_len > 0 ? avail_len : 0);
		bcm_binit(&b, (char*)(& iov_pktq_log_p->pktq_log.v05.headings[0]), avail_len);

		err = wlc_pktq_stats(&iov_full_params, params_version, &b, iov_pktq_log_p,
		                     wlc, bsscfg);
		break;
	}
#endif /* PKTQ_LOG */

#ifdef STA
#ifdef WL_EXCESS_PMWAKE
	case IOV_GVAL(IOV_EXCESS_PM_PERIOD):
		*ret_int_ptr = (int32)wlc->excess_pm_period;
		break;

	case IOV_SVAL(IOV_EXCESS_PM_PERIOD):
		wlc->excess_pm_period = (int16)int_val;
		wlc_reset_epm_dur(wlc);
		break;

	case IOV_GVAL(IOV_EXCESS_PM_PERCENT):
		*ret_int_ptr = (int32)wlc->excess_pm_percent;
		break;

	case IOV_SVAL(IOV_EXCESS_PM_PERCENT):
		if (int_val > 100) {
			err = BCME_RANGE;
			break;
		}
		wlc->excess_pm_percent = (int16)int_val;
		break;
#endif /* WL_EXCESS_PMWAKE */
#endif /* STA */
	case IOV_GVAL(IOV_MEMPOOL):  {
		wl_mempool_stats_t *stats;

		stats = (wl_mempool_stats_t *)arg;

		/* Calculate the max # of statistics that can fit in buffer. */
		stats->num = (len - OFFSETOF(wl_mempool_stats_t, s)) / sizeof(bcm_mp_stats_t);

		err = bcm_mpm_stats(wlc->mem_pool_mgr, stats->s, &stats->num);
		break;
	}

#ifdef BCMOVLHW
	case IOV_GVAL(IOV_OVERLAY): {
		if (si_arm_ovl_vaildaddr(wlc->pub->sih, (void *)int_val)) {
			if (len > 512) len = 512;
			memcpy(arg, (void *)int_val, len);
		}
		else
			err = BCME_ERROR;
		break;
	}
	case IOV_SVAL(IOV_OVERLAY): {
		uint32 p[3];
		memcpy(p, arg, 3*sizeof(uint32));
		err = si_arm_ovl_remap(wlc->pub->sih, (void *)p[0], (void *)p[1], p[2]);
		break;
	}
#endif /* BCMOVLHW */

#ifndef WLC_HIGH_ONLY
	case IOV_GVAL(IOV_FABID):
		*ret_int_ptr = wlc->fabid;
		break;
#endif

#ifdef SRHWVSDB
	case IOV_SVAL(IOV_FORCE_VSDB):
		if (SRHWVSDB_ENAB(wlc->pub)) {
			wlc_srvsdb_force_set(wlc, (uint8)int_val);
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_FORCE_VSDB):
		if (SRHWVSDB_ENAB(wlc->pub)) {
			*ret_int_ptr = wlc->srvsdb_info->iovar_force_vsdb;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_SVAL(IOV_FORCE_VSDB_CHANS):
		if (SRHWVSDB_ENAB(wlc->pub)) {
			wlc_srvsdb_info_t *srvsdb = wlc->srvsdb_info;

			/* Reset SRVSDB engine so that previous saves are flushed out */
			wlc_srvsdb_reset_engine(wlc);

			memcpy(srvsdb->vsdb_chans, arg, sizeof(srvsdb->vsdb_chans));

			if (srvsdb->vsdb_chans[0] && srvsdb->vsdb_chans[1]) {
				/* Set vsdb chans */
				wlc_phy_force_vsdb_chans(pi, srvsdb->vsdb_chans, 1);
			} else {
				/* Reset vsdb chans */
				wlc_phy_force_vsdb_chans(pi, srvsdb->vsdb_chans, 0);
			}
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_FORCE_VSDB_CHANS):
		if (SRHWVSDB_ENAB(wlc->pub)) {
			memcpy(arg, wlc->srvsdb_info->vsdb_chans,
				sizeof(wlc->srvsdb_info->vsdb_chans));
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
#endif /* SRHWVSDB */

#if defined(SAVERESTORE) && defined(WLTEST)
	case IOV_GVAL(IOV_SR_ENABLE):
		if (SR_ENAB() && !sr_cap(wlc->pub->sih)) {
			*ret_int_ptr = 0;
			break;
		}

		*ret_int_ptr = sr_isenab(wlc->pub->sih);
		break;
	case IOV_SVAL(IOV_SR_ENABLE):
		if (SR_ENAB() && !sr_cap(wlc->pub->sih))
			break;

		if ((CHIPID(wlc->pub->sih->chip) == BCM4345_CHIP_ID) ||
			(CHIPID(wlc->pub->sih->chip) == BCM4335_CHIP_ID)) {
			sr_engine_enable_post_dnld(wlc->pub->sih, bool_val);
		} else {
			sr_engine_enable(wlc->pub->sih, IOV_SET, bool_val);
			si_update_masks(wlc->pub->sih);
		}
		break;
#endif 

	case IOV_GVAL(IOV_BRCM_IE):
		*ret_int_ptr = (int32)wlc->brcm_ie;
		break;

	case IOV_SVAL(IOV_BRCM_IE):
		wlc->brcm_ie = bool_val;
		break;
#ifdef MACOSX
	case IOV_GVAL(IOV_HEALTH_STATUS):
		*ret_int_ptr = (int32)wlc->pub->health;
		break;

	case IOV_SVAL(IOV_HEALTH_STATUS):
		wlc->pub->health = 0;
#ifdef BCMDBG
		/* allow override for testing */
		wlc->pub->health = (uint32)int_val;
#endif /* BCMDBG */
		break;
#endif /* MACOSX */
	case IOV_GVAL(IOV_SAR_ENABLE):
		*ret_int_ptr = (int)wlc_channel_sarenable_get(wlc->cmi);
		break;

	case IOV_SVAL(IOV_SAR_ENABLE):
		wlc_channel_sarenable_set(wlc->cmi, bool_val);
#ifdef WL_SAR_SIMPLE_CONTROL
		wlc_phy_dynamic_sarctrl_set(WLC_PI(wlc), bool_val);
#endif /* WL_SAR_SIMPLE_CONTROL */

		break;
#ifdef WL_SARLIMIT
	case IOV_GVAL(IOV_SAR_LIMIT):
	{
		sar_limit_t *sar = (sar_limit_t *)arg;
		if (len < (int)sizeof(sar_limit_t))
			return BCME_BUFTOOSHORT;

		err = wlc_channel_sarlimit_get(wlc->cmi, sar);
		break;
	}

	case IOV_SVAL(IOV_SAR_LIMIT):
	{
		sar_limit_t *sar = (sar_limit_t *)arg;
		if (len < (int)sizeof(sar_limit_t))
			return BCME_BUFTOOSHORT;

		err = wlc_channel_sarlimit_set(wlc->cmi, sar);
		break;
	}
#endif /* WL_SARLIMIT */
#ifdef WLNDOE
	case IOV_GVAL(IOV_NDOE):
		*ret_int_ptr = (int32)wlc->pub->_ndoe;
		break;

	case IOV_SVAL(IOV_NDOE):
		wlc->pub->_ndoe = bool_val;
		break;
#endif /* WLNDOE */

#ifdef STA
	case IOV_GVAL(IOV_ROAM_CONF_AGGRESSIVE):
		if (BSSCFG_AP(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		*ret_int_ptr = ((-roam->roam_trigger_aggr) & 0xFFFF) << 16;
		*ret_int_ptr |= (roam->roam_delta_aggr & 0xFFFF);
		break;

	case IOV_SVAL(IOV_ROAM_CONF_AGGRESSIVE): {
		int16 roam_trigger;
		uint16 roam_delta;
		roam_trigger = -(int16)(int_val >> 16) & 0xFFFF;
		roam_delta = (uint16)(int_val & 0xFFFF);

		if (BSSCFG_AP(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		if ((roam_trigger > 0) || (roam_trigger < -100) || (roam_delta > 100)) {
			err = BCME_BADOPTION;
			break;
		}

		roam->roam_trigger_aggr = roam_trigger;
		roam->roam_delta_aggr =  roam_delta;
		wlc_roam_prof_update_default(wlc, bsscfg);
		break;
	}

	case IOV_GVAL(IOV_ROAM_MULTI_AP_ENV):
		if (BSSCFG_AP(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}
		*ret_int_ptr = (int32)roam->multi_ap;
		break;

	case IOV_SVAL(IOV_ROAM_MULTI_AP_ENV):
		if (BSSCFG_AP(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		/* Only Set option is allowed */
		if (!bool_val) {
			err = BCME_BADOPTION;
			break;
		}

		if (!roam->multi_ap) {
			roam->multi_ap = TRUE;
			wlc_roam_prof_update_default(wlc, bsscfg);
		}
		err = BCME_OK;
		break;

	case IOV_SVAL(IOV_ROAM_CI_DELTA):
		/* 8-bit ci_delta uses MSB for enable/disable */
		bsscfg->roam->ci_delta &= ROAM_CACHEINVALIDATE_DELTA_MAX;
		bsscfg->roam->ci_delta |= (uint8)int_val & (ROAM_CACHEINVALIDATE_DELTA_MAX - 1);
		break;

	case IOV_GVAL(IOV_ROAM_CI_DELTA):
		/* 8-bit ci_delta uses MSB for enable/disable */
		*ret_int_ptr = (int32)
			(bsscfg->roam->ci_delta & (ROAM_CACHEINVALIDATE_DELTA_MAX - 1));
		break;

	case IOV_GVAL(IOV_SPLIT_ROAMSCAN):
		*ret_int_ptr = (int32)roam->split_roam_scan;
		break;

	case IOV_SVAL(IOV_SPLIT_ROAMSCAN):
		if (BSSCFG_AP(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		roam->split_roam_scan = bool_val;

		if (!bool_val) {
			/* Reset split roam state */
			roam->split_roam_phase = 0;
		}
		break;
#endif /* STA */
	case IOV_GVAL(IOV_ANTDIV_BCNLOSS):
		err = BCME_UNSUPPORTED;
		break;
	case IOV_SVAL(IOV_ANTDIV_BCNLOSS):
		err = BCME_UNSUPPORTED;
		break;
#if defined(PKTC) || defined(PKTC_DONGLE)
	case IOV_GVAL(IOV_PKTC):
		*ret_int_ptr = (int32)wlc->pub->_pktc;
		break;

	case IOV_SVAL(IOV_PKTC):
		wlc->pub->_pktc = bool_val;
		break;

	case IOV_GVAL(IOV_PKTCBND):
		*ret_int_ptr = wlc->pub->tunables->pktcbnd;
		break;

	case IOV_SVAL(IOV_PKTCBND):
		wlc->pub->tunables->pktcbnd = MAX(int_val, wlc->pub->tunables->rxbnd);
		break;
#endif /* PKTC || PKTC_DONGLE */
#if defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND)
	case IOV_GVAL(IOV_TXDELAY_PARAMS): {
		txdelay_params_t *params = (txdelay_params_t *)arg;
		params->ratio = wlc->pub->txdelay_params.ratio;
		params->period = wlc->pub->txdelay_params.period;
		params->cnt = wlc->pub->txdelay_params.cnt;
		params->tune = wlc->pub->txdelay_params.tune;
		break;
	}
	case IOV_SVAL(IOV_TXDELAY_PARAMS): {
		txdelay_params_t *params = (txdelay_params_t *)arg;
		wlc->pub->txdelay_params.ratio = params->ratio;
		wlc->pub->txdelay_params.period = params->period;
		wlc->pub->txdelay_params.cnt = params->cnt;
		wlc->pub->txdelay_params.tune = params->tune;
		break;
	}
#endif /* defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND) */
#if defined(PROP_TXSTATUS)
	case IOV_GVAL(IOV_COMPSTAT):
		*ret_int_ptr = wlc->comp_stat_enab;
		break;

	case IOV_SVAL(IOV_COMPSTAT):
		if (int_val)
			wlc->comp_stat_enab = TRUE;
		else
			wlc->comp_stat_enab = FALSE;
		break;
#endif

	case IOV_GVAL(IOV_ROAM_CHN_CACHE_LIMIT):
		*ret_int_ptr = (int32) bsscfg->roam->roam_chn_cache_limit;
		break;

	case IOV_SVAL(IOV_ROAM_CHN_CACHE_LIMIT):
		bsscfg->roam->roam_chn_cache_limit = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_ROAM_CHANNELS_IN_CACHE): {
		wl_uint32_list_t *list = (wl_uint32_list_t *)arg;
		uint i;

		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		if (len < (int)(sizeof(uint32) * (ROAM_CACHELIST_SIZE + 1))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		list->count = 0;
		if (bsscfg->roam->cache_valid) {
			for (i = 0; i < bsscfg->roam->cache_numentries; i++) {
				list->element[list->count++] =
					bsscfg->roam->cached_ap[i].chanspec;
			}
		}
		break;
	}

	case IOV_GVAL(IOV_ROAM_CHANNELS_IN_HOTLIST): {
		wl_uint32_list_t *list = (wl_uint32_list_t *)arg;
		uint i;

		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		if (len < (int)(sizeof(uint32) * (WL_NUMCHANSPECS + 1))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		list->count = 0;
		for (i = 0; i < sizeof(chanvec_t); i++) {
			uint8 chn_list = bsscfg->roam->roam_chn_cache.vec[i];
			uint32 ch_idx = i << 3;
			while (chn_list) {
				if (chn_list & 1) {
					list->element[list->count++] =
						CH20MHZ_CHSPEC(ch_idx);
				}
				ch_idx++;
				chn_list >>= 1;
			}
		}
		break;
	}

	case IOV_GVAL(IOV_ROAMSCAN_PARMS):
	{
		wl_scan_params_t *psrc = bsscfg->roam_scan_params;
		wl_scan_params_t *pdst = arg;
		int reqlen = WL_MAX_ROAMSCAN_DATSZ;

		if (!psrc) {
			err = BCME_NOTREADY;
			break;
		}
		if (len < reqlen) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* finally: grab the data we were asked for */
		bcopy(psrc, pdst, reqlen);

		break;
	}
	case IOV_SVAL(IOV_ROAMSCAN_PARMS):
	{
		wl_scan_params_t *psrc = (wl_scan_params_t *)params;
		uint reqlen = WL_MAX_ROAMSCAN_DATSZ;

		/* No check for bad parameters */
		if (p_len > reqlen) {
			err = BCME_BUFTOOLONG;
			break;
		}

		/* first time: allocate memory */
		if (!bsscfg->roam_scan_params) {
			if ((bsscfg->roam_scan_params = (wl_scan_params_t *)
			MALLOCZ(wlc->osh, WL_MAX_ROAMSCAN_DATSZ)) == NULL) {
				err = BCME_NOMEM;
				break;
			}
		}

		/* Too easy: finally */
		bcopy(psrc, bsscfg->roam_scan_params, reqlen);

		break;

	}
	case IOV_GVAL(IOV_PASSIVE_ON_RESTRICTED_MODE):
		*ret_int_ptr = (int32)wlc->passive_on_restricted;
		break;

	case IOV_SVAL(IOV_PASSIVE_ON_RESTRICTED_MODE):
		wlc->passive_on_restricted = (uint8) int_val;
		break;

#ifdef EVENT_LOG_COMPILE
	case IOV_SVAL(IOV_EVENT_LOG_SET_INIT):
	{
		uint8 set = ((wl_el_set_params_t *) arg)->set;
		int size = ((wl_el_set_params_t *) arg)->size;
		*ret_int_ptr = event_log_set_init(wlc->hw->sih, set, size);
		break;
	}
	case IOV_SVAL(IOV_EVENT_LOG_SET_EXPAND):
	{
		uint8 set = ((wl_el_set_params_t *) arg)->set;
		int size = ((wl_el_set_params_t *) arg)->size;
		*ret_int_ptr = event_log_set_expand(wlc->hw->sih, set, size);
		break;
	}
	case IOV_SVAL(IOV_EVENT_LOG_SET_SHRINK):
	{
		uint8 set = ((wl_el_set_params_t *) arg)->set;
		int size = ((wl_el_set_params_t *) arg)->size;
		*ret_int_ptr = event_log_set_shrink(wlc->hw->sih, set, size);
		break;
	}

	case IOV_SVAL(IOV_EVENT_LOG_TAG_CONTROL):
	{
		uint16 tag = ((wl_el_tag_params_t *) arg)->tag;
		uint8 set = ((wl_el_tag_params_t *) arg)->set;
		uint8 flags = ((wl_el_tag_params_t *) arg)->flags;
		*ret_int_ptr = event_log_tag_start(tag, set, flags);
		break;
	}
#ifdef LOGTRACE
	case IOV_GVAL(IOV_EVENT_LOG_TAG_CONTROL):
	{
		uint8 set = ((wl_el_set_params_t *) params)->set;
		err = event_log_get(set, len, arg);
		break;
	}
#endif /* LOGTRACE */
#endif /* EVENT_LOG_COMPILE */

	case IOV_SVAL(IOV_NAS_NOTIFY_EVENT): {
		scb_val_t *val_tmp = (scb_val_t *)arg;
		if (len < (int)sizeof(scb_val_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (val_tmp->val == DOT11_RC_MIC_FAILURE)
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_PRUNE, &val_tmp->ea, 0,
				WLC_E_PRUNE_ENCR_MISMATCH, 0, 0, 0);
		break;
	}
#ifdef STA
	case IOV_GVAL(IOV_PS_RESEND_MODE):
		*ret_int_ptr = (uint8) pm->ps_resend_mode;
		break;
	case IOV_SVAL(IOV_PS_RESEND_MODE):
		if (int_val >= PS_RESEND_MODE_MAX)
			err = BCME_RANGE;
		else
			pm->ps_resend_mode = (uint8) int_val;
		break;

	/* IOV_SLEEP_BETWEEN_PS_RESEND is an Olympic aliased subset of IOV_PS_RESEND_MODE */
	case IOV_GVAL(IOV_SLEEP_BETWEEN_PS_RESEND):
		if (pm->ps_resend_mode == PS_RESEND_MODE_BCN_NO_SLEEP)
			*ret_int_ptr = 0;
		else if (pm->ps_resend_mode == PS_RESEND_MODE_BCN_SLEEP)
			*ret_int_ptr = 1;
		else if (pm->ps_resend_mode == PS_RESEND_MODE_WDOG_ONLY)
			*ret_int_ptr = 2;
		else
			*ret_int_ptr = pm->ps_resend_mode;
		break;
	case IOV_SVAL(IOV_SLEEP_BETWEEN_PS_RESEND):
		if (int_val == 0)
			pm->ps_resend_mode = PS_RESEND_MODE_BCN_NO_SLEEP;
		else if (int_val == 1)
			pm->ps_resend_mode = PS_RESEND_MODE_BCN_SLEEP;
		else
			/* To set other values, use IOV_PS_RESEND_MODE instead */
			err = BCME_RANGE;
		break;

	case IOV_GVAL(IOV_PM2_BCN_SLEEP_RET): {
		*ret_int_ptr = pm->pm2_bcn_sleep_ret_time;
		break;
	}
	case IOV_SVAL(IOV_PM2_BCN_SLEEP_RET): {
		pm->pm2_bcn_sleep_ret_time = (uint)int_val;
		break;
	}

	case IOV_GVAL(IOV_PM2_MD_SLEEP_EXT): {
		*ret_int_ptr = (int32)pm->pm2_md_sleep_ext;
		break;
	}
	case IOV_SVAL(IOV_PM2_MD_SLEEP_EXT): {
		pm->pm2_md_sleep_ext = (int8)int_val;
		break;
	}

	case IOV_GVAL(IOV_EARLY_BCN_THRESH):
		*ret_int_ptr = wlc->ebd->thresh;
		break;

	case IOV_SVAL(IOV_EARLY_BCN_THRESH):
		wlc->ebd->thresh = int_val;
		if (int_val == 0)
			memset(wlc->ebd, 0, sizeof(wlc_early_bcn_detect_t));
		break;
#ifdef WL_EXCESS_PMWAKE
	case IOV_GVAL(IOV_PFN_ROAM_ALERT_THRESH):
	{
		wl_pfn_roam_thresh_t *thresh = arg;
		if ((uint)len < sizeof(wl_pfn_roam_thresh_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		thresh->pfn_alert_thresh = wlc->excess_pmwake->pfn_alert_thresh;
		thresh->roam_alert_thresh = wlc->excess_pmwake->roam_alert_thresh;
		break;
	}
	case IOV_SVAL(IOV_PFN_ROAM_ALERT_THRESH):
	{
		wl_pfn_roam_thresh_t *thresh = (wl_pfn_roam_thresh_t *)params;

		if (WLPFN_ENAB(wlc->pub) || thresh->pfn_alert_thresh == 0) {
			wlc->excess_pmwake->pfn_alert_thresh = thresh->pfn_alert_thresh;
			wlc->excess_pmwake->pfn_alert_enable = !!thresh->pfn_alert_thresh;
			wlc->excess_pmwake->pfn_alert_thresh_ts = wlc_get_pfn_ms(wlc);
		}
		else
			return BCME_ERROR;

		wlc->excess_pmwake->roam_alert_thresh = thresh->roam_alert_thresh;
		wlc->excess_pmwake->roam_alert_enable = !!thresh->roam_alert_thresh;
		wlc->excess_pmwake->roam_alert_thresh_ts = wlc_get_roam_ms(wlc);
		break;
	}
	case IOV_GVAL(IOV_CONST_AWAKE_THRESH):
		*ret_int_ptr = wlc->excess_pmwake->ca_thresh;
		break;
	case IOV_SVAL(IOV_CONST_AWAKE_THRESH):
	{
		wlc_excess_pm_wake_info_t *epm = wlc->excess_pmwake;
		epm->ca_thresh = int_val;
		epm->ca_pkts_allowance = epm->ca_thresh;
		epm->ca_alert_pmdur = 0;
		epm->ca_alert_pmdur_valid = 0;
		wlc_reset_epm_ca(wlc);
	}
	break;

#endif /* WL_EXCESS_PMWAKE */
#endif /* STA */

#ifdef WLPM_BCNRX
	case IOV_SVAL(IOV_PM_BCNRX): {
		if (!BSSCFG_STA(bsscfg)) {
					err = BCME_EPERM;
					break;
			}

#if defined(AP) && (defined(RADIO_PWRSAVE) || defined(RXCHAIN_PWRSAVE))
			if (RADIO_PWRSAVE_ENAB(wlc->ap) || RXCHAIN_PWRSAVE_ENAB(wlc->ap)) {
					WL_ERROR(("wl%d: Not allowed, AP PS active\n",
						wlc->pub->unit));

					/* modes are enabled */
					if (PM_BCNRX_ENAB(wlc->pub))
							wlc_pm_bcnrx_disable(wlc);
					err = BCME_EPERM;
					break;
			}
#endif /* AP && (RADIO_PWRSAVE || RXCHAIN_PWRSAVE) */

			if (bool_val && !wlc_pm_bcnrx_allowed(wlc)) {
				/* Only trigger ucode if rxchain has not changed */
					err = wlc_pm_bcnrx_set(wlc, FALSE);
			} else
					err = wlc_pm_bcnrx_set(wlc, bool_val);

			if (!err)
					wlc->pub->_pm_bcnrx = bool_val;
		}
		break;

	case IOV_GVAL(IOV_PM_BCNRX):
		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_EPERM;
			break;
		}

		*ret_int_ptr = (int32)(PM_BCNRX_ENAB(wlc->pub) ? TRUE : FALSE);
		break;
#endif /* WLPM_BCNRX */

	case IOV_GVAL(IOV_BSS_PEER_INFO): {
		bss_peer_list_info_t *peer_list_info = (bss_peer_list_info_t  *)arg;
		struct scb *scb;
		struct scb_iter scbiter;
		bss_peer_info_param_t	*in_param = (bss_peer_info_param_t	*)params;
		struct ether_addr ea;
		uint32 cnt = 0;

		if ((load16_ua(&in_param->version)) != BSS_PEER_INFO_PARAM_CUR_VER) {
			err = BCME_VERSION;
			break;
		}

		bcopy(&in_param->ea, &ea, sizeof(struct ether_addr));
		store16_ua((uint8 *)&peer_list_info->version, BSS_PEER_LIST_INFO_CUR_VER);
		store16_ua((uint8 *)&peer_list_info->bss_peer_info_len, sizeof(bss_peer_info_t));

		/* If it is STA, return the current AP's info alone */
		if (BSSCFG_STA(bsscfg) && bsscfg->BSS && ETHER_ISNULLDEST(&ea)) {
			bcopy(&bsscfg->BSSID, &ea, ETHER_ADDR_LEN);
		}

		/* Get only the peer info specified by the mac address */
		if (!ETHER_ISNULLDEST(&ea)) {
			if ((scb = wlc_scbfind(wlc, bsscfg, &ea)) == NULL) {
				err = BCME_BADADDR;
				break;
			}

			if (BSSCFG_AP(bsscfg) && !SCB_ASSOCIATED(scb)) {
				err = BCME_BADADDR;
				break;
			}

			store32_ua((uint8 *)&peer_list_info->count, 1);
			wlc_copy_peer_info(bsscfg, scb, peer_list_info->peer_info);
			break;
		}

		len -= BSS_PEER_LIST_INFO_FIXED_LEN;
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (BSSCFG_AP(bsscfg) && !SCB_ASSOCIATED(scb)) {
				continue;
			}

			cnt++;
			if (len < (int)sizeof(bss_peer_info_t)) {
				/* return the actual no of peers with error */
				err = BCME_BUFTOOSHORT;
				continue;
			}

			wlc_copy_peer_info(bsscfg, scb, &peer_list_info->peer_info[cnt - 1]);
			len -= sizeof(bss_peer_info_t);
		}
		store32_ua((uint8 *)&peer_list_info->count, cnt);
		break;
	}
#ifdef SR_DEBUG
	case IOV_GVAL(IOV_DUMP_PMU):
		si_dump_pmu(wlc->pub->sih, arg);
		break;
	case IOV_GVAL(IOV_PMU_KEEP_ON):
		*ret_int_ptr = si_pmu_keep_on_get(wlc->pub->sih);
		break;
	case IOV_SVAL(IOV_PMU_KEEP_ON):
		si_pmu_keep_on(wlc->pub->sih, int_val);
		/* Calculating fast power up delay again as min_res_mask is changed now */
		wlc->hw->fastpwrup_dly = si_clkctl_fast_pwrup_delay(wlc->hw->sih);
		W_REG(wlc->osh, &wlc->regs->u.d11regs.scc_fastpwrup_dly, wlc->hw->fastpwrup_dly);
		break;
	case IOV_GVAL(IOV_POWER_ISLAND):
		*ret_int_ptr = si_power_island_get(wlc->pub->sih);
		break;
	case IOV_SVAL(IOV_POWER_ISLAND):
		*ret_int_ptr = si_power_island_set(wlc->pub->sih, int_val);
		if (*ret_int_ptr == 0) {
			err = BCME_UNSUPPORTED;
		}
		break;
#endif /*  SR_DEBUG */
#if defined(WLTEST) || defined(WLPKTENG)
	case IOV_GVAL(IOV_LONGPKT):
		*ret_int_ptr = ((wlc->pkteng_maxlen == WL_PKTENG_MAXPKTSZ) ? 1 : 0);
		break;

	case IOV_SVAL(IOV_LONGPKT):
		if ((int_val == 0) && (wlc->pkteng_maxlen == WL_PKTENG_MAXPKTSZ)) {
			/* restore 'wl rtsthresh' */
			wlc->RTSThresh = wlc->longpkt_rtsthresh;

			/* restore 'wl shmem 0x20' */
			wlc_write_shm(wlc, (uint16)0x20, wlc->longpkt_shm);

			/* restore pkteng_maxlen */
			wlc->pkteng_maxlen = PKTBUFSZ - TXOFF;
		} else if ((int_val == 1) && (wlc->pkteng_maxlen == (PKTBUFSZ - TXOFF))) {
			/* save current values */
			wlc->longpkt_rtsthresh = wlc->RTSThresh;
			wlc->longpkt_shm = wlc_read_shm(wlc, (uint16)0x20);

			/* 'wl rtsthresh 20000' */
			wlc->RTSThresh = (uint16) 20000;

			/* 'wl shmem 0x20 20000' */
			wlc_write_shm(wlc, (uint16)0x20, (uint16)20000);

			/* increase pkteng_maxlen */
			wlc->pkteng_maxlen = WL_PKTENG_MAXPKTSZ;
		}
		break;
#endif 
#ifdef ATE_BUILD
	case IOV_SVAL(IOV_GPAIO): {
		wlc_phy_gpaio(wlc->hw->band->pi, int_val);
		break;
	}
#endif
#ifdef WLRSDB
#if defined(BCMDBG)
	case IOV_GVAL(IOV_IOC):

		switch (int_val) {
			case WLC_UP:
			case WLC_DOWN:
				return BCME_UNSUPPORTED;
		}

		err = wlc_ioctl(wlc, int_val, (void*)((int8*)params + sizeof(int32)),
			p_len - sizeof(int32), wlcif);
		memmove(arg, (void*)((int8*)params + sizeof(int32)), p_len - sizeof(int32));
		break;
	case IOV_SVAL(IOV_IOC):

		switch (int_val) {
			case WLC_UP:
			case WLC_DOWN:
				return BCME_UNSUPPORTED;
		}

		err = wlc_ioctl(wlc, int_val, (void*)((int8*)params + sizeof(int32)),
			p_len - sizeof(int32), wlcif);
		break;
#endif 
#endif /* WLRSDB */
#ifdef STA
	case IOV_SVAL(IOV_PM_BCMC_WAIT):
		if (D11REV_LT(wlc->pub->corerev, 40))
			return BCME_UNSUPPORTED;
		wlc->pm_bcmc_wait = (uint16)int_val;

		/* If the core is not up, the shmem will be updated in wlc_up */
		if (wlc->pub->hw_up) {
			wlc_write_shm(wlc, M_BCMC_TIMEOUT, (uint16)int_val);
		}
		break;
	case IOV_GVAL(IOV_PM_BCMC_WAIT):
		if (D11REV_LT(wlc->pub->corerev, 40))
			return BCME_UNSUPPORTED;
		*ret_int_ptr = wlc->pm_bcmc_wait;
		break;
#ifdef LPAS
	case IOV_SVAL(IOV_LPAS):
		wlc->lpas = (uint32)int_val;
		if (wlc->lpas) {
			/* for LPAS mode increase the threshold to 8 seconds */
			bsscfg->roam->max_roam_time_thresh = 8000;
			/* In LPAS mode sleep time is up to 2 seconds. Since
			 * wdog must be triggered every sec, it cannot be fed from
			 * tbtt in LPAS mode.
			 */
			wlc->pub->align_wd_tbtt = FALSE;
		} else {
			bsscfg->roam->max_roam_time_thresh = wlc->pub->tunables->maxroamthresh;
			wlc->pub->align_wd_tbtt = TRUE;
		}
		wlc_watchdog_upd(bsscfg, PS_ALLOWED(bsscfg));
		break;
	case IOV_GVAL(IOV_LPAS):
		*ret_int_ptr = wlc->lpas;
		break;
#endif /* LPAS */
#endif /* STA */
#ifdef OPPORTUNISTIC_ROAM

	case IOV_SVAL(IOV_OPPORTUNISTIC_ROAM_OFF):
		roam->oppr_roam_off = bool_val;
		break;

	case IOV_GVAL(IOV_OPPORTUNISTIC_ROAM_OFF):
		*ret_int_ptr = (int32)roam->oppr_roam_off;
		break;
#endif
	case IOV_GVAL(IOV_TCPACK_FAST_TX):
		*ret_int_ptr = (int32)wlc->tcpack_fast_tx;
		break;

	case IOV_SVAL(IOV_TCPACK_FAST_TX):
		wlc->tcpack_fast_tx = bool_val;
		break;

#ifdef STA
	case IOV_GVAL(IOV_ROAM_PROF):
	{
		uint32 band_id;
		wlcband_t *band;
		wl_roam_prof_band_t *rp = (wl_roam_prof_band_t *)arg;

		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		if (!BAND_2G(int_val) && !BAND_5G(int_val)) {
			err = BCME_BADBAND;
			break;
		}
		band_id = BAND_2G(int_val) ? BAND_2G_INDEX : BAND_5G_INDEX;

		band = wlc->bandstate[band_id];
		if (!band || !band->roam_prof) {
			err = BCME_UNSUPPORTED;
			break;
		}

		rp->band = int_val;
		rp->ver = WL_MAX_ROAM_PROF_VER;
		rp->len = WL_MAX_ROAM_PROF_BRACKETS * sizeof(wl_roam_prof_t);
		memcpy(&rp->roam_prof[0], &band->roam_prof[0],
			WL_MAX_ROAM_PROF_BRACKETS * sizeof(wl_roam_prof_t));
	}	break;

	case IOV_SVAL(IOV_ROAM_PROF):
	{
		int i, np;
		uint32 band_id;
		wlcband_t *band;
		wl_roam_prof_band_t *rp = (wl_roam_prof_band_t *)arg;

		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		if (!BAND_2G(int_val) && !BAND_5G(int_val)) {
			err = BCME_BADBAND;
			break;
		}
		band_id = BAND_2G(int_val) ? BAND_2G_INDEX : BAND_5G_INDEX;

		band = wlc->bandstate[band_id];
		if (!band || !band->roam_prof) {
			err = BCME_UNSUPPORTED;
			break;
		}

		/* Sanity check on size */
		if (rp->ver != WL_MAX_ROAM_PROF_VER) {
			err = BCME_VERSION;
			break;
		}

		if ((rp->len < sizeof(wl_roam_prof_t)) ||
		    (rp->len > WL_MAX_ROAM_PROF_BRACKETS * sizeof(wl_roam_prof_t)) ||
		    (rp->len % sizeof(wl_roam_prof_t))) {
			err = BCME_BADLEN;
			break;
		}

		np = rp->len / sizeof(wl_roam_prof_t);
		for (i = 0; i < np; i++) {
			/* Sanity check */
			if ((rp->roam_prof[i].roam_trigger <=
				rp->roam_prof[i].rssi_lower) ||
			    (rp->roam_prof[i].nfscan == 0) ||
			    (rp->roam_prof[i].fullscan_period == 0) ||
			    (rp->roam_prof[i].init_scan_period &&
				(rp->roam_prof[i].backoff_multiplier == 0)) ||
			    (rp->roam_prof[i].max_scan_period <
				rp->roam_prof[i].init_scan_period))
				err = BCME_BADARG;

			if (i > 0) {
				if ((rp->roam_prof[i].roam_trigger >=
					rp->roam_prof[i-1].roam_trigger) ||
				    (rp->roam_prof[i].rssi_lower >=
					rp->roam_prof[i-1].rssi_lower) ||
				    (rp->roam_prof[i].roam_trigger <
					rp->roam_prof[i-1].rssi_lower))
					err = BCME_BADARG;
			}

			/* Clear default profile flag, and switch to multiple profile mode */
			rp->roam_prof[i].roam_flags &= ~WL_ROAM_PROF_DEFAULT;

			if (i == (np - 1)) {
				/* Last profile can't be crazy */
				rp->roam_prof[i].roam_flags &= ~ WL_ROAM_PROF_LAZY;

				/* Last rssi_lower must be set to min. */
				rp->roam_prof[i].rssi_lower = WLC_RSSI_MINVAL_INT8;
			}
		}

		/* Reset roma profile - it will be adjusted automatically later */
		if (err == BCME_OK) {
			memset(&(band->roam_prof[0]), 0,
				WL_MAX_ROAM_PROF_BRACKETS * sizeof(wl_roam_prof_t));
			memcpy(&band->roam_prof[0], &rp->roam_prof[0], rp->len);
			wlc_roam_prof_update(wlc, bsscfg, TRUE);
		}
	}	break;
#endif /* STA */

	case IOV_GVAL(IOV_WD_ON_BCN):
		*ret_int_ptr = wlc->watchdog_on_bcn;
		break;

	case IOV_SVAL(IOV_WD_ON_BCN):
		if (bool_val)
			 wlc->watchdog_on_bcn = TRUE;
		else
			 wlc->watchdog_on_bcn = FALSE;
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	goto exit;	/* avoid unused label warning */

exit:
	return err;
}

static void
BCMATTACHFN(wlc_mfbr_retrylimit_upd)(wlc_info_t *wlc)
{
	int ac;

	wlc->LRL = RETRY_LONG_DEF_AQM;

	wlc_bmac_retrylimit_upd(wlc->hw, wlc->SRL, wlc->LRL);

	for (ac = 0; ac < AC_COUNT; ac++)
		WLC_WME_RETRY_LONG_SET(wlc, ac, wlc->LRL);

	wlc_wme_retries_write(wlc);
}

static int
wlc_iovar_rangecheck(wlc_info_t *wlc, uint32 val, const bcm_iovar_t *vi)
{
	int err = 0;
	uint32 min_val = 0;
	uint32 max_val = 0;

	/* Only ranged integers are checked */
	switch (vi->type) {
	case IOVT_INT32:
		max_val |= 0x7fffffff;
		/* fall through */
	case IOVT_INT16:
		max_val |= 0x00007fff;
		/* fall through */
	case IOVT_INT8:
		max_val |= 0x0000007f;
		min_val = ~max_val;
		if (vi->flags & IOVF_NTRL)
			min_val = 1;
		else if (vi->flags & IOVF_WHL)
			min_val = 0;
		/* Signed values are checked against max_val and min_val */
		if ((int32)val < (int32)min_val || (int32)val > (int32)max_val)
			err = BCME_RANGE;
		break;

	case IOVT_UINT32:
		max_val |= 0xffffffff;
		/* fall through */
	case IOVT_UINT16:
		max_val |= 0x0000ffff;
		/* fall through */
	case IOVT_UINT8:
		max_val |= 0x000000ff;
		if (vi->flags & IOVF_NTRL)
			min_val = 1;
		if ((val < min_val) || (val > max_val))
			err = BCME_RANGE;
		break;
	}

	return err;
}

/** format a bsscfg indexed iovar buffer */
int
wlc_bssiovar_mkbuf(char *iovar, int bssidx, void *param,
	int paramlen, void *bufptr, int buflen, int *perr)
{
	const char *prefix = "bsscfg:";
	int8* p;
	uint prefixlen;
	uint namelen;
	uint iolen;

	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/* check for overflow */
	if (buflen < 0 || iolen > (uint)buflen) {
		*perr = BCME_BUFTOOSHORT;
		return 0;
	}

	p = (int8*)bufptr;

	/* copy prefix, no null */
	bcopy(prefix, p, prefixlen);
	p += prefixlen;

	/* copy iovar name including null */
	bcopy(iovar, p, namelen);
	p += namelen;

	/* bss config index as first param */
	bcopy(&bssidx, p, sizeof(int32));
	p += sizeof(int32);

	/* parameter buffer follows */
	if (paramlen)
		bcopy(param, p, paramlen);

	*perr = 0;
	return iolen;
}
/* Convert a wlc_bss_info_t into an external wl_bss_info_t
 * Destination pointer does not need to be aligned.
 */
int
wlc_bss2wl_bss(wlc_info_t *wlc, wlc_bss_info_t *bi, wl_bss_info_t *to_bi, int to_bi_len,
	bool need_ies)
{
	wl_bss_info_t wl_bi;
	uint bi_length;
	int ie_length;
	uint16 ie_offset;
	uint8 *ie;

	/* user buffer should at least hold fixed portion of bss info */
	if (to_bi_len < (int) sizeof(wl_bss_info_t)) {
		WL_INFORM(("%s: to_bi_len %d too small\n", __FUNCTION__, to_bi_len));
		return BCME_BUFTOOSHORT;
	}

	/* init ie length, point to IEs */
	if (bi->bcn_prb) {
		ie_length = bi->bcn_prb_len - DOT11_BCN_PRB_LEN;
		ie_offset = sizeof(wl_bss_info_t);
		if (ie_length > 0)
			ie = (uint8 *)bi->bcn_prb + DOT11_BCN_PRB_LEN;
		else {
			ie_length = 0;
			ie_offset = 0;
			ie = NULL;
		}
	}
	/* no IEs */
	else {
		ie_length = 0;
		ie_offset = 0;
		ie = NULL;
	}

	/* check for user buffer again to see if it can take IEs */
	if (ie_length) {
		/* adjust bss info length to hold variable length IEs as well */
		bi_length = sizeof(wl_bss_info_t) + ROUNDUP(ie_length, 4);
		/* check again for user buffer length */
		if (to_bi_len < (int) bi_length) {
			if (need_ies) {
				WL_INFORM(("%s: to_bi_len %d too small for ies\n",
					__FUNCTION__, to_bi_len));
				return BCME_BUFTOOSHORT;
			}
			bi_length = sizeof(wl_bss_info_t); /* just the fixed bss info */
			ie_length = 0;
			ie_offset = 0;
			ie = NULL;
		}
	}
	else
		bi_length = sizeof(wl_bss_info_t);

	bzero(&wl_bi, sizeof(wl_bss_info_t));

	/* constant values for wl_bss_info */
	wl_bi.version		= WL_BSS_INFO_VERSION;
	wl_bi.length		= bi_length;

	/* simple copies from wlc_bss_info */
	wl_bi.beacon_period	= bi->beacon_period;
	wl_bi.capability	= bi->capability;
	wl_bi.chanspec		= bi->chanspec;
	wl_bi.atim_window	= bi->atim_window;
	wl_bi.dtim_period	= bi->dtim_period;
	wl_bi.RSSI		= bi->RSSI;
	wl_bi.SNR		= bi->SNR;
	wl_bi.phy_noise		= bi->phy_noise;

	bcopy(bi->BSSID.octet, wl_bi.BSSID.octet, ETHER_ADDR_LEN);

	wl_bi.SSID_len	= bi->SSID_len;
	bcopy(bi->SSID, wl_bi.SSID, wl_bi.SSID_len);

	if (bi->flags & WLC_BSS_BEACON)
		wl_bi.flags |= WL_BSS_FLAGS_FROM_BEACON;
	if (bi->flags & WLC_BSS_CACHE)
		wl_bi.flags |= WL_BSS_FLAGS_FROM_CACHE;
	if (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)
		wl_bi.flags |= WL_BSS_FLAGS_RSSI_ONCHANNEL;
	if (bi->flags2 & WLC_BSS_RSSI_INVALID)
		wl_bi.flags |= WL_BSS_FLAGS_RSSI_INVALID;
	if (bi->flags2 & WLC_BSS_RSSI_INACCURATE)
		wl_bi.flags |= WL_BSS_FLAGS_RSSI_INACCURATE;
	if (bi->flags2 & WLC_BSS_HS20)
		wl_bi.flags |= WL_BSS_FLAGS_HS20;

	/* limit rates to destination rate array size */
	wl_bi.rateset.count = MIN(bi->rateset.count, sizeof(wl_bi.rateset.rates));
	bcopy(bi->rateset.rates, wl_bi.rateset.rates, wl_bi.rateset.count);

	/* Some 802.11N related capabilities */
	if (bi->flags & WLC_BSS_HT) {
		wl_bi.n_cap = TRUE;
		if (bi->flags & WLC_BSS_40MHZ)
			wl_bi.nbss_cap |= HT_CAP_40MHZ;
		if (bi->flags & WLC_BSS_SGI_20)
			wl_bi.nbss_cap |= HT_CAP_SHORT_GI_20;
		if (bi->flags & WLC_BSS_SGI_40)
			wl_bi.nbss_cap |= HT_CAP_SHORT_GI_40;
		/* Copy Basic MCS set */
		bcopy(&bi->rateset.mcs[0], &wl_bi.basic_mcs[0], MCSSET_LEN);
		wl_bi.ctl_ch = wf_chspec_ctlchan(bi->chanspec);
	}

#ifdef WL11AC
	if (bi->flags2 & WLC_BSS_VHT) {
		wl_bi.vht_cap = TRUE;
		if ((bi->flags2 & WLC_BSS_SGI_80)) {
			wl_bi.nbss_cap |= VHT_BI_SGI_80MHZ;
		}
		wl_bi.vht_rxmcsmap = bi->vht_rxmcsmap;
		wl_bi.vht_txmcsmap = bi->vht_txmcsmap;
	}
#endif /* WL11AC */
	/* ie length */
	wl_bi.ie_length = ie_length;
	wl_bi.ie_offset = ie_offset;

	/* copy fixed portion of the bss info structure to user buffer */
	bcopy(&wl_bi, to_bi, sizeof(wl_bss_info_t));

	/* append beacon/probe response IEs */
	if (ie_length)
		bcopy(ie, (uint8 *)to_bi + sizeof(wl_bss_info_t), ie_length);
	return 0;
}

#if ((defined(BCMDBG) || defined(WLMSG_PRPKT)) && defined(STA)) || defined(BCMDBG)
static const struct wlc_id_name_entry dot11_ie_names[] =
{
	{DOT11_MNG_SSID_ID, "SSID"},
	{DOT11_MNG_RATES_ID, "Rates"},
	{DOT11_MNG_FH_PARMS_ID, "FH Parms"},
	{DOT11_MNG_DS_PARMS_ID, "DS Parms"},
	{DOT11_MNG_CF_PARMS_ID, "CF Parms"},
	{DOT11_MNG_TIM_ID, "TIM"},
	{DOT11_MNG_IBSS_PARMS_ID, "IBSS Parms"},
	{DOT11_MNG_COUNTRY_ID, "Country"},
	{DOT11_MNG_HOPPING_PARMS_ID, "Hopping Parms"},
	{DOT11_MNG_HOPPING_TABLE_ID, "Hopping Table"},
	{DOT11_MNG_REQUEST_ID, "Request"},
	{DOT11_MNG_QBSS_LOAD_ID, "QBSS LOAD"},
	{DOT11_MNG_CHALLENGE_ID, "Challenge"},
	{DOT11_MNG_PWR_CONSTRAINT_ID, "Pwr Constraint"},
	{DOT11_MNG_PWR_CAP_ID, "Pwr Capability"},
	{DOT11_MNG_TPC_REQUEST_ID, "TPC Request"},
	{DOT11_MNG_TPC_REPORT_ID, "TPC Report"},
	{DOT11_MNG_SUPP_CHANNELS_ID, "Supported Channels"},
	{DOT11_MNG_CHANNEL_SWITCH_ID, "Channel Switch"},
	{DOT11_MNG_MEASURE_REQUEST_ID, "Measure Request"},
	{DOT11_MNG_MEASURE_REPORT_ID, "Measure Report"},
	{DOT11_MNG_QUIET_ID, "Quiet"},
	{DOT11_MNG_IBSS_DFS_ID, "IBSS DFS"},
	{DOT11_MNG_ERP_ID, "ERP Info"},
	{DOT11_MNG_TS_DELAY_ID, "TS Delay"},
	{DOT11_MNG_HT_CAP, "HT Capability"},
	{DOT11_MNG_NONERP_ID, "Legacy ERP Info"},
	{DOT11_MNG_RSN_ID, "RSN"},
	{DOT11_MNG_EXT_RATES_ID, "Ext Rates"},
	{DOT11_MNG_HT_ADD, "HT Additional"},
	{DOT11_MNG_EXT_CHANNEL_OFFSET, "Ext Channel Offset"},
	{DOT11_MNG_WAPI_ID, "WAPI"},
	{DOT11_MNG_VS_ID, "Vendor Specific"},
	{DOT11_MNG_VHT_CAP_ID, "VHT Capability"},
	{DOT11_MNG_VHT_OPERATION_ID, "VHT Op"},
	{DOT11_MNG_OPER_MODE_NOTIF_ID, "HT/VHT Operating Mode Notification"},
	{0, NULL}
};
#endif 

#if defined(BCMDBG)
static int
wlc_dump_ratestuff(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint i;
	uint r;

# if defined(WL11AC)
	wlc_dump_vht_mcsmap("\nhw_vhtmap ", wlc->band->hw_rateset.vht_mcsmap, b);
# endif /* WL11AC */

#ifdef WL11N
	wlc_dump_mcsset("\nhw_mcsset ", &wlc->band->hw_rateset.mcs[0], b);
#endif
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "basic_rate: ");
	for (i = 0; i < sizeof(wlc->band->basic_rate); i++)
		if ((r = wlc->band->basic_rate[i]))
			bcm_bprintf(b, "%d%s->%d%s ",
				(i / 2), (i % 2)?".5":"",
				(r / 2), (r % 2)?".5":"");
	bcm_bprintf(b, "\n");

	return 0;
}

static int
wlc_dump_wlc(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	wlcband_t *band;
	uint fifo_size[NFIFO];
	char perm[32], cur[32];
	char chn[8];
	uint16 maj_rev, min_rev, date, ucodetime;
	uint16 dbgst;
	uint i;
	wlc_bsscfg_t *cfg;
	wlc_txq_info_t *qi;
	wlc_if_t *wlcif;
	wlc_tunables_t *tunables;
	char chanbuf[CHANSPEC_STR_LEN];

	/* read ucode revision info */
	maj_rev = wlc->ucode_rev >> NBITS(uint16);
	min_rev = wlc->ucode_rev & 0xffff;

	/* skip accessing registers if the clock is off */
	if (!wlc->clk) {
		date = ucodetime = dbgst = 0;
	} else {
		date = wlc_read_shm(wlc, M_UCODE_DATE);
		ucodetime = wlc_read_shm(wlc, M_UCODE_TIME);
		/* read ucode debug status */
		dbgst = wlc_read_shm(wlc, M_UCODE_DBGST);
	}

	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "wl%d: wlc %p wl %p msglevel 0x%x clk %d up %d hw_off %d now %d\n",
	            wlc->pub->unit, wlc, wlc->wl, wl_msg_level, wlc->clk,
	            wlc->pub->up, wlc->pub->hw_off, wlc->pub->now);

	bcm_bprintf(b, "ucode %d.%d %02d/%02d/%02d %02d:%02d:%02d dbgst 0x%x\n",
	            maj_rev, min_rev,
	            (date >> 8) & 0xf, date & 0xff, (date >> 12) & 0xf,
	            (ucodetime >> 11) & 0x1f, (ucodetime >> 5) & 0x3f, ucodetime & 0x1f, dbgst);

	bcm_bprintf(b, "capabilities: ");
	wlc_cap_bcmstrbuf(wlc, b);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "promisc %d monitor %d piomode %d gmode %d\n",
	            wlc->pub->promisc, wlc->monitor, PIO_ENAB(wlc->pub), wlc->band->gmode);

	bcm_bprintf(b, "ap %d apsta %d wet %d wme %d mac_spoof %d per-ac maxrate %d\n",
	            AP_ENAB(wlc->pub), APSTA_ENAB(wlc->pub), wlc->wet, wlc->pub->_wme,
	            wlc->mac_spoof, WME_PER_AC_MAXRATE_ENAB(wlc->pub));

	bcm_bprintf(b, "vendor 0x%x device 0x%x nbands %d regs %p\n",
	            wlc->vendorid, wlc->deviceid, NBANDS(wlc), wlc->regs);

	bcm_bprintf(b, "chip %s chiprev %d corerev %d maccap 0x%x\n",
	            bcm_chipname(wlc->pub->sih->chip, chn, 8), wlc->pub->sih->chiprev,
	            wlc->pub->corerev, wlc->machwcap);

	bcm_bprintf(b, "boardvendor 0x%x boardtype 0x%x boardrev %s "
	            "boardflags 0x%x boardflags2 0x%x sromrev %d\n",
	            wlc->pub->sih->boardvendor, wlc->pub->sih->boardtype,
	            bcm_brev_str(wlc->pub->boardrev, cur), wlc->pub->boardflags,
	            wlc->pub->boardflags2, wlc->pub->sromrev);
	if (wlc->pub->boardrev == BOARDREV_PROMOTED)
		bcm_bprintf(b, " (actually 0x%02x)", BOARDREV_PROMOTABLE);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "perm_etheraddr %s cur_etheraddr %s\n",
	            bcm_ether_ntoa(&wlc->perm_etheraddr, (char*)perm),
	            bcm_ether_ntoa(&wlc->pub->cur_etheraddr, (char*)cur));

	bcm_bprintf(b, "swdecrypt %d\n",
	            wlc->pub->swdecrypt);

	bcm_bprintf(b, "\nuserfragthresh %d, fragthresh %d/%d/%d/%d RTSThresh %d \n",
	            wlc->usr_fragthresh, wlc->fragthresh[0], wlc->fragthresh[1],
	            wlc->fragthresh[2], wlc->fragthresh[3], wlc->RTSThresh);

	bcm_bprintf(b, "\nSRL %d LRL %d SFBL %d LFBL %d\n",
	            wlc->SRL, wlc->LRL, wlc->SFBL, wlc->LFBL);

	bcm_bprintf(b, "shortslot %d shortslot_ovrrd %d ignore_bcns %d\n",
	            wlc->shortslot, wlc->shortslot_override, wlc->ignore_bcns);

	bcm_bprintf(b, "\nblock_datafifo 0x%x tx_suspended %d\n",
	            wlc->block_datafifo, wlc->tx_suspended);

	bcm_bprintf(b, "bandunit %d bandlocked %d \n", wlc->band->bandunit, wlc->bandlocked);
	bcm_bprintf(b, "radio_disabled 0x%x down_override %d\n", wlc->pub->radio_disabled,
	            wlc->down_override);
#ifdef WL11AC
	bcm_bprintf(b, "vht_features  0x%x\n", wlc->pub->vht_features);
#endif

#ifdef STA
	bcm_bprintf(b, "mpc %d, mpc_scan %d mpc_join %d mpc_oidscan %d mpc_oidjoin %d"
	            " mpc_oidnettype %d mpc_out %d\n",
	            wlc->mpc, wlc->mpc_scan, wlc->mpc_join, wlc->mpc_oidscan, wlc->mpc_oidjoin,
	            wlc->mpc_oidnettype, wlc->mpc_out);
#endif

	bcm_bprintf(b, "5G band: ratespec_override 0x%x mratespec_override 0x%x\n",
	            wlc->bandstate[BAND_5G_INDEX]->rspec_override,
	            wlc->bandstate[BAND_5G_INDEX]->mrspec_override);

	bcm_bprintf(b, "2G band: ratespec_override 0x%x mratespec_override 0x%x\n",
	            wlc->bandstate[BAND_2G_INDEX]->rspec_override,
	            wlc->bandstate[BAND_2G_INDEX]->mrspec_override);
	bcm_bprintf(b, "\n");

	FOREACH_BSS(wlc, i, cfg) {
		bcm_bprintf(b, "PLCPHdr_ovrrd %d\n",
			cfg->PLCPHdr_override);
	}

	bcm_bprintf(b, "CCK_power_boost %d \n", (wlc->pub->boardflags & BFL_CCKHIPWR) ? 1 : 0);

	bcm_bprintf(b, "mhf 0x%02x mhf2 0x%02x mhf3 0x%02x mhf4 0x%02x mhf5 0x%02x\n",
		wlc_bmac_mhf_get(wlc->hw, MHF1, WLC_BAND_AUTO),
		wlc_bmac_mhf_get(wlc->hw, MHF2, WLC_BAND_AUTO),
		wlc_bmac_mhf_get(wlc->hw, MHF3, WLC_BAND_AUTO),
		wlc_bmac_mhf_get(wlc->hw, MHF4, WLC_BAND_AUTO),
		wlc_bmac_mhf_get(wlc->hw, MHF5, WLC_BAND_AUTO));

	bcm_bprintf(b, "swdecrypt %d\n", wlc->pub->swdecrypt);

#ifdef STA
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "STAY_AWAKE() %d wake %d PMpending %d\n",
	            STAY_AWAKE(wlc), wlc->wake, wlc->PMpending);
	FOREACH_AS_STA(wlc, i, cfg) {
		bcm_bprintf(b, "join_pref->band %d\n", cfg->join_pref->band);

		bcm_bprintf(b, "assoc->state %d assoc->type %d assoc->flags 0x%x assocroam %d\n",
		            cfg->assoc->state, cfg->assoc->type, cfg->assoc->flags,
		            cfg->roam->assocroam);

		bcm_bprintf(b, "wsec_portopen %d WME_PM_blocked %d\n",
		            cfg->wsec_portopen, cfg->pm->WME_PM_blocked);

		bcm_bprintf(b, "PM mode %d PMenabled %d PM_override %d "
		            "PS_ALLOWED() %d PSpoll %d\n",
		            cfg->pm->PM, cfg->pm->PMenabled, cfg->pm->PM_override,
		            PS_ALLOWED(cfg), cfg->pm->PSpoll);
		bcm_bprintf(b, "WLC_PORTOPEN() %d dtim_programmed %d PMpending %d "
		            "priorPMstate %d\n",
		            WLC_PORTOPEN(cfg), cfg->dtim_programmed,
		            cfg->pm->PMpending, cfg->pm->priorPMstate);

		bcm_bprintf(b, "bcns_lost %d time_since_bcn %d\n",
		            cfg->roam->bcns_lost, cfg->roam->time_since_bcn);

		bcm_bprintf(b, "BSSID %s BSS %d\n",
		            bcm_ether_ntoa(&cfg->BSSID, (char*)cur),
		            cfg->BSS);
		bcm_bprintf(b, "reprate %dkbps\n",
		            RSPEC2KBPS(wlc_get_rspec_history(cfg)));

		bcm_bprintf(b, "\n");
	}
#endif /* STA */

	bcm_bprintf(b, "associated %d stas_associated %d aps_associated %d\n",
	            wlc->pub->associated, wlc->stas_associated, wlc->aps_associated);

	FOREACH_BSS(wlc, i, cfg) {
		if (!BSSCFG_AP(cfg))
			continue;
	        bcm_bprintf(b, "BSSID %s\n", bcm_ether_ntoa(&cfg->BSSID, (char*)perm));
	}
	bcm_bprintf(b, "AID 0x%x\n", wlc->AID);

	if (wlc->pub->up)
		bcm_bprintf(b, "chanspec %s\n",
			wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf));
	else
		bcm_bprintf(b, "chan N/A ");

	bcm_bprintf(b, "country \"%s\"\n", wlc_channel_country_abbrev(wlc->cmi));

	bcm_bprintf(b, "counter %d\n", wlc->counter & 0xfff);

	bcm_bprintf(b, "\n");

	for (qi = wlc->tx_queues; qi != NULL; qi = qi->next) {
		bcm_bprintf(b, "txqinfo %p len %d stopped 0x%x\n",
		            qi, pktq_len(&qi->q), qi->stopped);
		bcm_bprintf(b, "associated wlcifs:");

		for (wlcif = wlc->wlcif_list;  wlcif != NULL; wlcif = wlcif->next) {
			char ifname[32];

			if (wlcif->qi != qi)
				continue;

			strncpy(ifname, wl_ifname(wlc->wl, wlcif->wlif), sizeof(ifname));
			ifname[sizeof(ifname) - 1] = '\0';

			bcm_bprintf(b, " \"%s\" 0x%p", ifname, wlcif);
		}
		bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "malloc_failed %d\n", MALLOC_FAILED(wlc->osh));

#ifdef STA
	bcm_bprintf(b, "freqtrack: wideband %d override %d attempts %d duration %d\n",
	            wlc->freqtrack,
	            wlc->freqtrack_override,
	            wlc->freqtrack_attempts,
	            (wlc->freqtrack_attempts > 0) ? (wlc->pub->now - wlc->freqtrack_starttime) : 0);
#endif /* STA */

	bcm_bprintf(b, "txpktpend AC_BK %d AC_BE %d AC_VI %d AC_VO %d BCMC %d fifo5 %d "
	            "total_tx_pkt %d\n",
	            TXPKTPENDGET(wlc, TX_AC_BK_FIFO), TXPKTPENDGET(wlc, TX_AC_BE_FIFO),
	            TXPKTPENDGET(wlc, TX_AC_VI_FIFO), TXPKTPENDGET(wlc, TX_AC_VO_FIFO),
	            TXPKTPENDGET(wlc, TX_BCMC_FIFO), TXPKTPENDGET(wlc, TX_ATIM_FIFO),
	            wlc_txpktcnt(wlc));

#if !defined(NEW_TXQ)
	bcm_bprintf(b, "pkt_callback_reg_fail %d tx_prec_map %x\n",
		WLCNTVAL(wlc->pub->_cnt->pkt_callback_reg_fail), wlc->tx_prec_map);
#else
	bcm_bprintf(b, "pkt_callback_reg_fail %d tx_stopped_map %x\n",
		WLCNTVAL(wlc->pub->_cnt->pkt_callback_reg_fail),
		txq_stopped_map(wlc->active_queue->low_txq));
#endif

	for (i = 0; i < NFIFO; i++) {
		if (wlc_bmac_xmtfifo_sz_get(wlc->hw, i, &fifo_size[i]))
			fifo_size[i] = 0;
	}
	bcm_bprintf(b, "xmtfifo_sz(in unit of 256B)");
	bcm_bprintf(b, "AC_BK %d AC_BE %d AC_VI %d AC_VO %d 5th %d 6th %d\n",
	            fifo_size[TX_AC_BK_FIFO],
	            fifo_size[TX_AC_BE_FIFO],
	            fifo_size[TX_AC_VI_FIFO],
	            fifo_size[TX_AC_VO_FIFO],
	            fifo_size[4],
	            fifo_size[5]);

	band = wlc->bandstate[IS_SINGLEBAND_5G(wlc->deviceid) ? BAND_5G_INDEX : BAND_2G_INDEX];
	bcm_bprintf(b, "(ROAMThreshold, ROAMDelta) (2.4G) default: %d, %d::current: %d, %d\n",
	            band->roam_trigger_def, band->roam_delta_def,
	            band->roam_trigger, band->roam_delta);

	if (NBANDS(wlc) > 1) {
		band = wlc->bandstate[BAND_5G_INDEX];
		bcm_bprintf(b, "(ROAMThreshold, ROAMDelta) (5G) default: %d, %d::current: %d, %d\n",
		            band->roam_trigger_def, band->roam_delta_def,
		            band->roam_trigger, band->roam_delta);
	}

	if (wlc->stf->throttle_state) {
		bcm_bprintf(b, "state:%d duty cycle:%d rxchain: %x txchain: %x\n",
		            wlc->stf->throttle_state,
		            wlc->stf->tx_duty_cycle_pwr,
		            wlc->stf->rxchain,
		            wlc->stf->txchain);
	}

	tunables = wlc->pub->tunables;
	bcm_bprintf(b, "tunables:\n");
	bcm_bprintf(b, "\tntxd = %d, nrxd = %d, rxbufsz = %d, nrxbufpost = %d, maxscb = %d\n",
	            tunables->ntxd, tunables->nrxd, tunables->rxbufsz,
	            tunables->nrxbufpost, tunables->maxscb);
	bcm_bprintf(b, "\tampdunummpdu2streams = %d, ampdunummpdu3streams = %d\n",
	            tunables->ampdunummpdu2streams, tunables->ampdunummpdu3streams);
	bcm_bprintf(b, "\tmaxpktcb = %d,  maxucodebss = %d, maxucodebss4 = %d\n",
	            tunables->maxpktcb,
	            tunables->maxucodebss, tunables->maxucodebss4);
	bcm_bprintf(b, "\tmaxbss = %d, datahiwat = %d, ampdudatahiwat = %d, maxubss = %d\n",
	            tunables->maxbss, tunables->datahiwat, tunables->ampdudatahiwat,
	            tunables->maxubss);
	bcm_bprintf(b, "\trxbnd = %d, txsbnd = %d\n", tunables->rxbnd, tunables->txsbnd);
#ifdef WLC_HIGH_ONLY
	bcm_bprintf(b, "\trpctxbufpost = %d\n", tunables->rpctxbufpost);
#endif
#ifdef WLC_LOW_ONLY
	bcm_bprintf(b, "\tdngl_mem_restrict_rxdma = %d\n", tunables->dngl_mem_restrict_rxdma);
#endif
	bcm_bprintf(b, "\tampdu_pktq_size = %d, ampdu_pktq_fav_size = %d\n",
		tunables->ampdu_pktq_size, tunables->ampdu_pktq_fav_size);

#ifdef PROP_TXSTATUS
	bcm_bprintf(b, "\twlfcfifocreditac0 = %d, wlfcfifocreditac1 = %d, wlfcfifocreditac2 = %d, "
	            "wlfcfifocreditac3 = %d\n",
	            tunables->wlfcfifocreditac0, tunables->wlfcfifocreditac1,
	            tunables->wlfcfifocreditac2, tunables->wlfcfifocreditac3);
	bcm_bprintf(b, "\twlfcfifocreditbcmc = %d, wlfcfifocreditother = %d\n",
	            tunables->wlfcfifocreditbcmc, tunables->wlfcfifocreditother);
#endif

	bcm_bprintf(b, "-------- hwrs scbs --------\n");
	for (i = 0; i < MAXBANDS; i ++) {
		if (wlc->bandstate[i] != NULL &&
		    wlc->bandstate[i]->hwrs_scb != NULL)
			wlc_scb_dump_scb(wlc, NULL, wlc->bandstate[i]->hwrs_scb, b, -1);
	}

	return 0;
}

/** Print out the AC Params in an IE */
static void
wlc_dump_edcf_acp(wlc_info_t *wlc, struct bcmstrbuf *b, edcf_acparam_t *acp_ie, const char *desc)
{
	int ac;

	bcm_bprintf(b, "\nEDCF params for %s\n", desc);

	for (ac = 0; ac < AC_COUNT; ac++, acp_ie++) {
		bcm_bprintf(b,
		               "%s: ACI 0x%02x ECW 0x%02x "
		               "(aci %d acm %d aifsn %d ecwmin %d ecwmax %d txop 0x%x)\n",
		               aci_names[ac],
		               acp_ie->ACI, acp_ie->ECW,
		               (acp_ie->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT,
		               (acp_ie->ACI & EDCF_ACM_MASK) ? 1 : 0,
		               acp_ie->ACI & EDCF_AIFSN_MASK,
		               acp_ie->ECW & EDCF_ECWMIN_MASK,
		               (acp_ie->ECW & EDCF_ECWMAX_MASK) >> EDCF_ECWMAX_SHIFT,
		               ltoh16(acp_ie->TXOP));
	}

	bcm_bprintf(b, "\n");
}

/** Print out the AC Params in use by ucode */
static void
wlc_dump_wme_shm(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int ac;

	bcm_bprintf(b, "\nEDCF params in shared memory\n");

	for (ac = 0; ac < AC_COUNT; ac++) {
		shm_acparams_t shm_acp;
		uint16 *element;
		int j;

		element = (uint16 *)&shm_acp;

		/* fill in the ac param element from the shm locations */
		for (j = 0; j < (int)sizeof(shm_acparams_t); j += 2) {
			uint offset = M_EDCF_QINFO + wme_shmemacindex(ac) * M_EDCF_QLEN + j;
			*element++ = wlc_read_shm(wlc, offset);
		}

		bcm_bprintf(b, "%s: txop 0x%x cwmin 0x%x cwmax 0x%x cwcur 0x%x\n"
		               "       aifs 0x%x bslots 0x%x reggap 0x%x status 0x%x\n",
		               aci_names[ac],
		               shm_acp.txop, shm_acp.cwmin, shm_acp.cwmax, shm_acp.cwcur,
		               shm_acp.aifs, shm_acp.bslots, shm_acp.reggap, shm_acp.status);
	}

	bcm_bprintf(b, "\n");
}

static int
wlc_dump_wme(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	d11regs_t *regs = wlc->regs;
	uint32 cwcur, cwmin, cwmax;
	uint8 qi;
	osl_t *osh;
	int idx;
	wlc_bsscfg_t *bsscfg;

	bcm_bprintf(b, "up %d EDCF %d WME %d dp 0x%x\n",
	            wlc->pub->up, EDCF_ENAB(wlc->pub), WME_ENAB(wlc->pub), wlc->wme_dp);

	FOREACH_BSS(wlc, idx, bsscfg) {
		wlc_wme_t *wme = bsscfg->wme;

		qi = wme->wme_param_ie.qosinfo;

		bcm_bprintf(b, "\ncfg %d WME %d apsd %d count %d admctl 0x%x\n",
		            WLC_BSSCFG_IDX(bsscfg), BSS_WME_ENAB(wlc, bsscfg),
		            (qi & WME_QI_AP_APSD_MASK) >> WME_QI_AP_APSD_SHIFT,
		            (qi & WME_QI_AP_COUNT_MASK) >> WME_QI_AP_COUNT_SHIFT,
		            wme->wme_admctl);

		wlc_dump_edcf_acp(wlc, b, wme->wme_param_ie.acparam,
		                BSSCFG_AP(bsscfg) ? "AP" : "STA");

		if (BSSCFG_AP(bsscfg))
			wlc_dump_edcf_acp(wlc, b, bsscfg->wme->wme_param_ie_ad->acparam,
			                  "BCN/PRBRSP");
	}

	if (!EDCF_ENAB(wlc->pub))
		return BCME_OK;

	if (!wlc->pub->up)
		return BCME_OK;

	wlc_dump_wme_shm(wlc, b);

	osh = wlc->osh;

	/* read current cwcur, cwmin, cwmax */
	wlc_bmac_copyfrom_objmem(wlc->hw, S_DOT11_CWMIN << 2, &cwmin,
		sizeof(cwmin), OBJADDR_SCR_SEL);
	wlc_bmac_copyfrom_objmem(wlc->hw, S_DOT11_CWMAX << 2, &cwmax,
		sizeof(cwmax), OBJADDR_SCR_SEL);
	wlc_bmac_copyfrom_objmem(wlc->hw, S_DOT11_CWCUR << 2, &cwcur,
		sizeof(cwcur), OBJADDR_SCR_SEL);

	bcm_bprintf(b, "xfifordy 0x%x psm_b0 0x%x txf_cur_idx 0x%x\n",
		R_REG(osh, &regs->u.d11regs.xmtfifordy), R_REG(osh, &regs->psm_base_0),
		wlc_read_shm(wlc, M_TXF_CUR_INDEX));
	bcm_bprintf(b, "cwcur 0x%x cwmin 0x%x cwmax 0x%x\n", cwcur, cwmin, cwmax);

	bcm_bprintf(b, "\n");

	return BCME_OK;
}

#endif 

#ifdef BCMDBG
static void
wlc_print_txs_status_raw(uint16 s)
{
	printf("Raw\n [15:12]  %d  frame attempts\n",
	       (s & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT);
	printf(" [11:8]  %d  rts attempts\n",
	       (s & TX_STATUS_RTS_RTX_MASK) >> TX_STATUS_RTS_RTX_SHIFT);
	printf("    [7]  %d  PM\n", ((s & TX_STATUS_PMINDCTD) ? 1 : 0));
	printf("    [6]  %d  intermediate status\n", ((s & TX_STATUS_INTERMEDIATE) ? 1 : 0));
	printf("    [5]  %d  AMPDU\n", (s & TX_STATUS_AMPDU) ? 1 : 0);
	printf("  [4:2]  %d  Suppress Reason\n",
	   ((s & TX_STATUS_SUPR_MASK) >> TX_STATUS_SUPR_SHIFT));
	printf("    [1]  %d  acked\n", ((s & TX_STATUS_ACK_RCV) ? 1 : 0));
}

static void
wlc_print_txs_status_aqm_raw(tx_status_t* txs)
{
	uint16 status_bits = txs->status.raw_bits;

	printf("Raw\n[0]    %d Valid\n", ((status_bits & TX_STATUS_VALID) != 0));
	printf("[2]    %d IM\n", ((status_bits & TX_STATUS40_INTERMEDIATE) != 0));
	printf("[3]    %d PM\n", ((status_bits & TX_STATUS40_PMINDCTD) != 0));
	printf("[7-4]  %d Suppr\n",
		((status_bits & TX_STATUS40_SUPR) >> TX_STATUS40_SUPR_SHIFT));
	printf("[14:8] %d Ncons\n", ((status_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT));
	printf("[15]   %d Acked\n", (status_bits & TX_STATUS40_ACK_RCV) != 0);
}
#endif /* only needed for dbg */

void
wlc_print_txstatus(wlc_info_t* wlc, tx_status_t* txs)
{
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_PRHDRS) || \
	defined(WLMSG_PRPKT)
	uint16 s = txs->status.raw_bits;
	static const char *supr_reason[] = {
		"None", "PMQ Entry", "Flush request",
		"Previous frag failure", "Channel mismatch",
		"Lifetime Expiry", "Underflow", "AB NACK or TX SUPR"
	};

	printf("\ntxpkt (MPDU) Complete\n");

	printf("FrameID: 0x%04x   ", txs->frameid);
	printf("Seq: 0x%04x   ", txs->sequence);
	printf("TxStatus: 0x%04x", s);
	printf("\n");

	printf("ACK %d IM %d PM %d Suppr %d (%s)\n",
	       txs->status.was_acked, txs->status.is_intermediate,
	       txs->status.pm_indicated, txs->status.suppr_ind,
	       (txs->status.suppr_ind < ARRAYSIZE(supr_reason) ?
	        supr_reason[txs->status.suppr_ind] : "Unkn supr"));
	printf("CNT(rts_tx)=%d CNT(frag_tx_cnt)=%d CNT(cts_rx_cnt)=%d\n",
		txs->status.rts_tx_cnt, txs->status.frag_tx_cnt,
		txs->status.cts_rx_cnt);

	printf("DequeueTime: 0x%08x ", txs->dequeuetime);
	printf("LastTxTime: 0x%08x ", txs->lasttxtime);
	printf("PHYTxErr:   0x%04x ", txs->phyerr);
	printf("RxAckRSSI: 0x%04x ", (txs->ackphyrxsh & PRXS1_JSSI_MASK) >> PRXS1_JSSI_SHIFT);
	printf("RxAckSQ: 0x%04x", (txs->ackphyrxsh & PRXS1_SQ_MASK) >> PRXS1_SQ_SHIFT);
	printf("\n");

#ifdef BCMDBG
	if (txs->phyerr) {
		wlc_dump_phytxerr(wlc, txs->phyerr);
	}

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		wlc_print_txs_status_raw(txs->status.raw_bits);
	} else {
		wlc_print_txs_status_aqm_raw(txs);
	}
#endif
#endif	/* BCMDBG || BCMDBG_ERR || WLMSG_PRHDRS || WLMSG_PRPKT */
}

#if defined(BCMDBG)
#define	PRVAL(name)	bcm_bprintf(b, "%s %d ", #name, WLCNTVAL(wlc->pub->_cnt->name))
#define	PRNL()		bcm_bprintf(b, "\n")

#define	PRREG(name)	bcm_bprintf(b, #name " 0x%x ", R_REG(wlc->osh, &regs->name))
#define PRREG_INDEX(name, reg) bcm_bprintf(b, #name " 0x%x ", R_REG(wlc->osh, &reg))

static int
wlc_dump_dma(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int i;

	if (!wlc->clk)
		return BCME_NOCLK;

	for (i = 0; i < NFIFO; i++) {
		PRREG_INDEX(intstatus, wlc->regs->intctrlregs[i].intstatus);
		PRREG_INDEX(intmask, wlc->regs->intctrlregs[i].intmask);
		bcm_bprintf(b, "\n");
		if (!PIO_ENAB(wlc->pub)) {
			hnddma_t *di = WLC_HW_DI(wlc, i);
			bcm_bprintf(b, "DMA %d: ", i);
			if (di != NULL) {
				dma_dumptx(di, b, TRUE);
				if ((i == RX_FIFO) ||
				    (D11REV_IS(wlc->pub->corerev, 4) && (i == RX_TXSTATUS_FIFO))) {
					dma_dumprx(di, b, TRUE);
					PRVAL(rxuflo[i]);
				} else
					PRVAL(txuflo);
				PRNL();
			}
		}
		bcm_bprintf(b, "\n");
	}

	PRVAL(dmada); PRVAL(dmade); PRVAL(rxoflo); PRVAL(dmape);
	bcm_bprintf(b, "\n");

	return 0;
}

static int
wlc_dump_mac(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	d11regs_t *regs;
	char ea[ETHER_ADDR_STR_LEN];

	regs = wlc->regs;

	if (!wlc->clk)
		return BCME_NOCLK;

	PRREG(maccontrol); PRREG(maccommand); PRREG(macintstatus); PRREG(macintmask);
	bcm_bprintf(b, "\n");
	PRREG(chnstatus);
	PRREG(psmdebug); PRREG(phydebug);
	bcm_bprintf(b, "\n");

	PRREG(psm_gpio_in); PRREG(psm_gpio_out); PRREG(psm_gpio_oe);
	bcm_bprintf(b, "\n\n");

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		uint16 match_data[6];
		char *p;
		int i;

		for (i = 0; i < ETHER_ADDR_LEN; i++) {
			W_REG(wlc->osh, &regs->u_rcv.d11regs.rcm_ctl, i);
			match_data[i] = R_REG(wlc->osh, &regs->u_rcv.d11regs.rcm_mat_data);
			match_data[i] = htol16(match_data[i]);
		}

		PRREG(biststatus);
		bcm_bprintf(b, "\n");

		/* read mac addr words from match_data[0, 2] */
		p = (char*)&match_data[0];
		bcm_bprintf(b, "MAC addr %s ", bcm_ether_ntoa((struct ether_addr*)p, ea));

		/* read mac addr words from match_data[3, 5] */
		p = (char*)&match_data[3];
		bcm_bprintf(b, "BSSID %s\n\n", bcm_ether_ntoa((struct ether_addr*)p, ea));
	} else {
		bcm_bprintf(b, "toe_capable %d toe_bypass %d ToECTL 0x%x\n",
		            wlc->toe_capable, wlc->toe_bypass,
		            R_REG(wlc->osh, &regs->u.d11acregs.ToECTL));
		/* ifsstat has the same addr as in pre-ac. it's not defined in d11acregs yet */
		bcm_bprintf(b, "aqm_rdy 0x%x framecnt 0x%x ifsstat 0x%x\n",
		            R_REG(wlc->osh, &regs->u.d11acregs.AQMFifoReady),
		            R_REG(wlc->osh, &regs->u.d11acregs.XmtFifoFrameCnt),
		            R_REG(wlc->osh, &regs->u.d11regs.ifsstat));
	}

	return 0;
}

static int
wlc_dump_pio(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int i;

	if (!wlc->clk)
		return BCME_NOCLK;

	if (!PIO_ENAB(wlc->pub))
		return 0;

	for (i = 0; i < NFIFO; i++) {
		pio_t *pio = WLC_HW_PIO(wlc, i);
		bcm_bprintf(b, "PIO %d: ", i);
		if (pio != NULL)
			wlc_pio_dump(pio, b);
		bcm_bprintf(b, "\n");
	}

	return 0;
}

/** dump beacon (from read_txe_ram in d11procs.tcl) */
static void
wlc_dump_bcntpl(wlc_info_t *wlc, struct bcmstrbuf *b, int offset, int len)
{
	d11regs_t *regs;
	osl_t *osh;
	uint16 val;
	uint i;

	regs = wlc->regs;
	osh = wlc->osh;

	len = (len + 3) & ~3;

	bcm_bprintf(b, "tpl: offset %d len %d\n", offset, len);

	while ((val = R_REG(osh, &regs->u.d11regs.xmttplateptr)) & 3)
		printf("read_txe_ram: polling xmttplateptr 0x%x\n", val);

	for (i = 0; i < (uint)len; i += 4) {
		W_REG(osh, &regs->u.d11regs.xmttplateptr, (offset + i) | 2);
		while ((val = R_REG(osh, &regs->u.d11regs.xmttplateptr)) & 3)
			printf("read_txe_ram: polling xmttplateptr 0x%x\n", val);
		bcm_bprintf(b, "%04X: %04X%04X\n", i,
		            R_REG(osh, &regs->u.d11regs.xmttplatedatahi),
		            R_REG(osh, &regs->u.d11regs.xmttplatedatalo));
	}
}

static int
wlc_dump_bcntpls(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint16 len;
	uint shm_bcn0_tpl_base, shm_bcn1_tpl_base;

	/* if multiband, band must be set */
	if (IS_MBAND_UNLOCKED(wlc)) {
		WL_ERROR(("wl%d: %s: band must be locked\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOTBANDLOCKED;
	}

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		shm_bcn0_tpl_base = D11AC_T_BCN0_TPL_BASE;
		shm_bcn1_tpl_base = D11AC_T_BCN1_TPL_BASE;
	} else {
		shm_bcn0_tpl_base = D11_T_BCN0_TPL_BASE;
		shm_bcn1_tpl_base = D11_T_BCN1_TPL_BASE;
	}

	len = wlc_read_shm(wlc, M_BCN0_FRM_BYTESZ);
	bcm_bprintf(b, "bcn 0: len %u\n", len);
	wlc_dump_bcntpl(wlc, b, shm_bcn0_tpl_base, len);
	len = wlc_read_shm(wlc, M_BCN1_FRM_BYTESZ);
	bcm_bprintf(b, "bcn 1: len %u\n", len);
	wlc_dump_bcntpl(wlc, b, shm_bcn1_tpl_base, len);

	return 0;
}
#endif	

#if defined(BCMDBG)
int
wlc_dump_bss_info(const char *name, wlc_bss_info_t *bi, struct bcmstrbuf *b)
{
	char bssid[DOT11_MAX_SSID_LEN];
	char ssidbuf[SSID_FMT_BUF_LEN];
	char chanbuf[CHANSPEC_STR_LEN];

	wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);

	bcm_bprintf(b, "%s: %s BSSID %s\n", name, (bi->infra == 1?"Infra":(bi->infra ==
		0?"IBSS":"auto")),
		bcm_ether_ntoa(&bi->BSSID, bssid));
	bcm_bprintf(b, "SSID len %d \"%s\" ISBRCM %d ISHT %d ISVHT %d\n",
		bi->SSID_len, ssidbuf, ((bi->flags & WLC_BSS_BRCM) != 0),
		((bi->flags & WLC_BSS_HT) != 0), ((bi->flags2 & WLC_BSS_VHT) != 0));
	bcm_bprintf(b, "channel %d chanspec %s beacon %d dtim %d atim %d capability 0x%04x"
		" flags 0x%02x RSSI %d\n", wf_chspec_ctlchan(bi->chanspec),
		wf_chspec_ntoa_ex(bi->chanspec, chanbuf), bi->beacon_period, bi->dtim_period,
		bi->atim_window, bi->capability, bi->flags, bi->RSSI);
	wlc_dump_rateset("rateset", &bi->rateset, b);
	if (bi->rateset.htphy_membership)
		bcm_bprintf(b, "\nmembership %d(b)",
			(bi->rateset.htphy_membership & RATE_MASK));
	if (bi->flags & WLC_BSS_HT) {
		wlc_dump_mcsset("\nmcs", &bi->rateset.mcs[0], b);
	}

# if defined(WL11AC)
	if (bi->flags2 & WLC_BSS_VHT) {
		wlc_dump_vht_mcsmap("\nvht", bi->rateset.vht_mcsmap, b);
		bcm_bprintf(b, "(rx %04x tx %04x)\n", bi->vht_rxmcsmap, bi->vht_txmcsmap);
	}
#endif /* WL11AC */

	bcm_bprintf(b, "\n");

	return 0;
}

/** print aggregate (since driver load time) driver and macstat counter values */
static int
wlc_dump_stats(wlc_info_t *wlc, struct bcmstrbuf * b)
{
#ifdef WLCNT
	int i;
	wl_cnt_t *cnt = wlc->pub->_cnt;

	if (WLC_UPDATE_STATS(wlc)) {
		wlc_statsupd(wlc);
	}

	/* summary stat counter line */
	PRVAL(txframe); PRVAL(txbyte); PRVAL(txretrans); PRVAL(txerror);
	PRVAL(rxframe); PRVAL(rxbyte); PRVAL(rxerror); PRNL();

	PRVAL(txprshort); PRVAL(txdmawar); PRVAL(txnobuf); PRVAL(txnoassoc);
	PRVAL(txchit); PRVAL(txcmiss); PRVAL(txchanrej); PRVAL(txexptime); PRNL();

	PRVAL(reset); PRVAL(txserr); PRVAL(txphyerr); PRVAL(txphycrs);
	PRVAL(txfail); PRVAL(psmwds); PRVAL(rfdisable); PRNL();

	bcm_bprintf(b, "d11_txfrag %d d11_txmulti %d d11_txretry %d d11_txretrie %d\n",
		cnt->txfrag, cnt->txmulti, cnt->txretry, cnt->txretrie);

	bcm_bprintf(b, "d11_txrts %d d11_txnocts %d d11_txnoack %d d11_txfrmsnt %d\n",
		cnt->txrts, cnt->txnocts, cnt->txnoack, cnt->txfrmsnt);

	PRVAL(rxcrc); PRVAL(rxnobuf); PRVAL(rxnondata); PRVAL(rxbadds);
	PRVAL(rxbadcm); PRVAL(rxdup); PRVAL(rxfragerr); PRNL();

	PRVAL(rxrunt); PRVAL(rxgiant); PRVAL(rxnoscb); PRVAL(rxbadproto);
	PRVAL(rxbadsrcmac); PRNL();

	bcm_bprintf(b, "d11_rxfrag %d d11_rxmulti %d d11_rxundec %d d11_rxundec_mcst %d\n",
		cnt->rxfrag, cnt->rxmulti, cnt->rxundec, cnt->rxundec_mcst);

	PRVAL(rxctl); PRVAL(rxbadda); PRVAL(rxfilter); PRNL();

	bcm_bprintf(b, "rxuflo: ");
	for (i = 0; i < NFIFO; i++)
		bcm_bprintf(b, "%d ", cnt->rxuflo[i]);
	bcm_bprintf(b, "\n");
	PRVAL(txallfrm); PRVAL(txrtsfrm); PRVAL(txctsfrm); PRVAL(txackfrm); PRNL();
	PRVAL(txdnlfrm); PRVAL(txbcnfrm); PRVAL(txtplunfl); PRVAL(txphyerr); PRNL();
	bcm_bprintf(b, "txfunfl: ");
	for (i = 0; i < NFIFO; i++)
		bcm_bprintf(b, "%d ", cnt->txfunfl[i]);
	bcm_bprintf(b, "\n");

	/* WPA2 counters */
	PRVAL(tkipmicfaill); PRVAL(tkipicverr); PRVAL(tkipcntrmsr); PRNL();
	PRVAL(tkipreplay); PRVAL(ccmpfmterr); PRVAL(ccmpreplay); PRNL();
	PRVAL(ccmpundec); PRVAL(fourwayfail); PRVAL(wepundec); PRNL();
	PRVAL(wepicverr); PRVAL(decsuccess); PRVAL(rxundec); PRNL();

	PRVAL(tkipmicfaill_mcst); PRVAL(tkipicverr_mcst); PRVAL(tkipcntrmsr_mcst); PRNL();
	PRVAL(tkipreplay_mcst); PRVAL(ccmpfmterr_mcst); PRVAL(ccmpreplay_mcst); PRNL();
	PRVAL(ccmpundec_mcst); PRVAL(fourwayfail_mcst); PRVAL(wepundec_mcst); PRNL();
	PRVAL(wepicverr_mcst); PRVAL(decsuccess_mcst); PRVAL(rxundec_mcst); PRNL();

	PRVAL(rxfrmtoolong); PRVAL(rxfrmtooshrt); PRVAL(rxinvmachdr); PRVAL(rxbadfcs); PRNL();
	PRVAL(rxbadplcp); PRVAL(rxcrsglitch); PRVAL(rxstrt); PRVAL(rxdfrmucastmbss); PRNL();
	PRVAL(rxmfrmucastmbss); PRVAL(rxcfrmucast); PRVAL(rxrtsucast); PRVAL(rxctsucast); PRNL();
	PRVAL(rxackucast); PRVAL(rxdfrmocast); PRVAL(rxmfrmocast); PRVAL(rxcfrmocast); PRNL();
	PRVAL(rxrtsocast); PRVAL(rxctsocast); PRVAL(rxdfrmmcast); PRVAL(rxmfrmmcast); PRNL();
	PRVAL(rxcfrmmcast); PRVAL(rxbeaconmbss); PRVAL(rxdfrmucastobss); PRVAL(rxbeaconobss);
		PRNL();
	PRVAL(rxrsptmout); PRVAL(bcntxcancl); PRVAL(rxf0ovfl); PRVAL(rxf1ovfl); PRNL();
	PRVAL(rxf2ovfl); PRVAL(txsfovfl); PRVAL(pmqovfl); PRNL();
	PRVAL(rxcgprqfrm); PRVAL(rxcgprsqovfl); PRVAL(txcgprsfail); PRVAL(txcgprssuc); PRNL();
	PRVAL(prs_timeout); PRVAL(rxnack); PRVAL(frmscons); PRVAL(txnack); PRNL();
	PRVAL(txphyerror); PRVAL(txchanrej); PRNL();
	PRVAL(pktengrxducast); PRVAL(pktengrxdmcast); PRNL();
	PRVAL(txmpdu_sgi); PRVAL(rxmpdu_sgi); PRVAL(txmpdu_stbc); PRVAL(rxmpdu_stbc); PRNL();
	PRVAL(dma_hang); PRVAL(reinit); PRVAL(cso_normal); PRVAL(cso_passthrough); PRNL();


#endif /* WLCNT */
	return 0;
}
#endif 

void
wlc_dump_ucode_fatal(wlc_info_t *wlc)
{
	uint32 phydebug, psm_brc, psmdebug;
	uint16 psm_brc_1;
	d11regs_t *regs;
	uint32 loop;

	regs = wlc->regs;

	WL_ERROR(("ucode revision %d.%d \n",
		wlc_read_shm(wlc, UCODE_MAJOR_BOM), wlc_read_shm(wlc, UCODE_MINOR_BOM)));
	WL_ERROR(("wl%d: PSM microcode watchdog fired at %d (seconds)\n",
		wlc->pub->unit, wlc->pub->now));

	psmdebug = R_REG(wlc->osh, &regs->psmdebug);
	phydebug = R_REG(wlc->osh, &regs->phydebug);
	psm_brc = R_REG(wlc->osh, &regs->psm_brc);
	psm_brc_1 = R_REG(wlc->osh, &regs->psm_brc_1);
	BCM_REFERENCE(psm_brc_1);

	wlc_mac_event(wlc, WLC_E_PSM_WATCHDOG, NULL, psmdebug, phydebug, psm_brc, NULL, 0);
	/* dump regs */
	WL_ERROR(("wl%d: psmdebug 0x%08x, phydebug 0x%08x, psm_brc 0x%08x psm_brc_1 0x%04x\n",
		wlc->pub->unit, psmdebug, phydebug, psm_brc, psm_brc_1));

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		uint16 wepctl, pcmctl, pcmstat;
		wepctl = R_REG(wlc->osh, &regs->u.d11regs.wepctl);
		pcmctl = R_REG(wlc->osh, &regs->u.d11regs.pcmctl);
		pcmstat = R_REG(wlc->osh, &regs->u.d11regs.pcmstat);
		BCM_REFERENCE(wepctl);
		BCM_REFERENCE(pcmctl);
		BCM_REFERENCE(pcmstat);
		WL_ERROR(("     wepctl 0x%04x, pcmctl 0x%04x, pcmstat 0x%04x\n",
			wepctl, pcmctl, pcmstat));
	} else {
		/* new definitions for corerev >= 40 */
		uint16 wepctl, wepstat, wep_hdrloc, wep_psdulen;
		wepctl = R_REG(wlc->osh, &regs->u.d11regs.wepctl);
		wepstat = R_REG(wlc->osh, &regs->u.d11regs.wepivloc);
		wep_hdrloc = R_REG(wlc->osh, &regs->u.d11regs.wepivkey);
		wep_psdulen = R_REG(wlc->osh, &regs->u.d11regs.wepwkey);
		BCM_REFERENCE(wepctl);
		BCM_REFERENCE(wepstat);
		BCM_REFERENCE(wep_hdrloc);
		BCM_REFERENCE(wep_psdulen);
		WL_ERROR(("     wepctl 0x%04x wepstat 0x%04x wep_hdrloc 0x%04x wep_psdulen %d\n",
			wepctl, wepstat, wep_hdrloc, wep_psdulen));
#ifdef BCMDBG
	{
		uint16 rxcnt, daggctl, bytesleft, shoffs, daggstat, dagglen;
		rxcnt = R_REG(wlc->osh, &regs->rxe_rx_cntr);
		daggctl = R_REG(wlc->osh, &regs->u.d11acregs.DAGG_CTL2);
		bytesleft = R_REG(wlc->osh, &regs->u.d11acregs.DAGG_BYTESLEFT);
		shoffs = R_REG(wlc->osh, &regs->u.d11acregs.DAGG_SH_OFFSET);
		daggstat = R_REG(wlc->osh, &regs->u.d11acregs.DAGG_STAT);
		dagglen = R_REG(wlc->osh, &regs->u.d11acregs.DAGG_LEN);
		WL_ERROR(("     RXE_RXCNT: 0x%04x   DAGG: ctl: 0x%04x, bleft: 0x%04x\n"
				  "     offs: 0x%04x, stat: 0x%04x, len: 0x%04x\n",
				  rxcnt, daggctl, bytesleft, shoffs, daggstat, dagglen));
	}
#endif /* BCMDBG */
	}

	if (phydebug > 0) {
		WL_ERROR(("phydebug\n"));
		for (loop = 0; loop < 64; loop++) {
			phydebug = R_REG(wlc->osh, &regs->phydebug);
			WL_ERROR(("0x%04x\n", phydebug));
		}
	}
	WL_ERROR(("PC :\n"));
	for (loop = 0; loop < 64; loop++) {
		psmdebug = R_REG(wlc->osh, &regs->psmdebug);
		WL_ERROR(("0x%04x\n", psmdebug));
	}
#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(wlc->pub))
		wlc_dump_aggfifo(wlc, NULL);
#endif
}


#ifdef WLCNT
static void
wlc_ctrupd_cache(uint16 cur_stat, uint16 *macstat_snapshot, uint32 *macstat)
{
	uint16 v;
	uint16 delta;

	v = ltoh16(cur_stat);
	delta = (uint16)(v - *macstat_snapshot);

	if (delta != 0) {
		*macstat += delta;
		*macstat_snapshot = v;
	}
}
#endif /* WLCNT */

#define MACSTATUPD(name) \
	wlc_ctrupd_cache(macstats.name, &wlc->core->macstat_snapshot->name, &wlc->pub->_cnt->name)

void
wlc_statsupd(wlc_info_t *wlc)
{
	int i;
#ifdef BCMDBG
	uint16 delta;
	uint16 rxf0ovfl;
	uint16 txfunfl[NFIFO];
#endif /* BCMDBG */
#ifdef WLCNT
	macstat_t macstats;
#endif

	/* if driver or core are down, makes no sense to update stats */
	if (!wlc->pub->up || !wlc->clk)
		return;

#ifdef BCMDBG
	/* save last rx fifo 0 overflow count */
	rxf0ovfl = wlc->core->macstat_snapshot->rxf0ovfl;

	/* save last tx fifo  underflow count */
	for (i = 0; i < NFIFO; i++)
		txfunfl[i] = wlc->core->macstat_snapshot->txfunfl[i];
#endif /* BCMDBG */

#ifdef WLCNT
	/* Read mac stats from contiguous shared memory */
	wlc_bmac_copyfrom_shm(wlc->hw, M_UCODE_MACSTAT, &macstats, sizeof(macstat_t));

	/* update mac stats */
	MACSTATUPD(txallfrm);
	MACSTATUPD(txrtsfrm);
	MACSTATUPD(txctsfrm);
	MACSTATUPD(txackfrm);
	MACSTATUPD(txdnlfrm);
	MACSTATUPD(txbcnfrm);
	for (i = 0; i < NFIFO; i++)
		MACSTATUPD(txfunfl[i]);
	MACSTATUPD(txtplunfl);
	MACSTATUPD(txphyerr);
	MACSTATUPD(rxfrmtoolong);
	MACSTATUPD(rxfrmtooshrt);
	MACSTATUPD(rxinvmachdr);
	MACSTATUPD(rxbadfcs);
	MACSTATUPD(rxbadplcp);
	MACSTATUPD(rxcrsglitch);
	MACSTATUPD(rxstrt);
	MACSTATUPD(rxdfrmucastmbss);
	MACSTATUPD(rxmfrmucastmbss);
	MACSTATUPD(rxcfrmucast);
	MACSTATUPD(rxrtsucast);
	MACSTATUPD(rxctsucast);
	MACSTATUPD(rxackucast);
	MACSTATUPD(rxdfrmocast);
	MACSTATUPD(rxmfrmocast);
	MACSTATUPD(rxcfrmocast);
	MACSTATUPD(rxrtsocast);
	MACSTATUPD(rxctsocast);
	MACSTATUPD(rxdfrmmcast);
	MACSTATUPD(rxmfrmmcast);
	MACSTATUPD(rxcfrmmcast);
	MACSTATUPD(rxbeaconmbss);
	MACSTATUPD(rxdfrmucastobss);
	MACSTATUPD(rxbeaconobss);
	MACSTATUPD(rxrsptmout);
	MACSTATUPD(bcntxcancl);
	MACSTATUPD(rxf0ovfl);
	MACSTATUPD(rxf1ovfl);
	MACSTATUPD(rxf2ovfl);
	MACSTATUPD(txsfovfl);
	MACSTATUPD(pmqovfl);
	MACSTATUPD(rxcgprqfrm);
	MACSTATUPD(rxcgprsqovfl);
	MACSTATUPD(txcgprsfail);
	MACSTATUPD(txcgprssuc);
	MACSTATUPD(prs_timeout);
	MACSTATUPD(rxnack);
	MACSTATUPD(frmscons);
	MACSTATUPD(txnack);
	MACSTATUPD(pktengrxducast);
	MACSTATUPD(pktengrxdmcast);
#endif /* WLCNT */

#ifdef BCMDBG
	/* check for rx fifo 0 overflow */
	delta = (uint16)(wlc->core->macstat_snapshot->rxf0ovfl - rxf0ovfl);
	if (delta)
		WL_ERROR(("wl%d: %u rx fifo 0 overflows!\n", wlc->pub->unit, delta));

	/* check for tx fifo underflows */
	for (i = 0; i < NFIFO; i++) {
		delta = (uint16)(wlc->core->macstat_snapshot->txfunfl[i] - txfunfl[i]);
		if (delta)
			WL_ERROR(("wl%d: %u tx fifo %d underflows!\n", wlc->pub->unit, delta, i));
	}
#endif /* BCMDBG */

	/* dot11 counter update */

	WLCNTSET(wlc->pub->_cnt->txrts,
	         (wlc->pub->_cnt->rxctsucast - wlc->pub->_cnt->d11cnt_txrts_off));
	WLCNTSET(wlc->pub->_cnt->rxcrc,
	         (wlc->pub->_cnt->rxbadfcs - wlc->pub->_cnt->d11cnt_rxcrc_off));
	WLCNTSET(wlc->pub->_cnt->txnocts,
	         ((wlc->pub->_cnt->txrtsfrm - wlc->pub->_cnt->rxctsucast) -
	          wlc->pub->_cnt->d11cnt_txnocts_off));

	if (!PIO_ENAB(wlc->pub)) {
		/* merge counters from dma module */
		for (i = 0; i < NFIFO; i++) {
			hnddma_t *di = WLC_HW_DI(wlc, i);
			if (di != NULL) {
				WLCNTADD(wlc->pub->_cnt->txnobuf, di->txnobuf);
				WLCNTADD(wlc->pub->_cnt->rxnobuf, di->rxnobuf);
				WLCNTADD(wlc->pub->_cnt->rxgiant, di->rxgiants);
				dma_counterreset(di);
			}
		}
	}

	/*
	 * Aggregate transmit and receive errors that probably resulted
	 * in the loss of a frame are computed on the fly.
	 */
	WLCNTSET(wlc->pub->_cnt->txerror,  wlc->pub->_cnt->txnobuf + wlc->pub->_cnt->txnoassoc +
	       wlc->pub->_cnt->txuflo + wlc->pub->_cnt->txrunt + wlc->pub->_cnt->dmade +
	       wlc->pub->_cnt->dmada + wlc->pub->_cnt->dmape);
	WLCNTSET(wlc->pub->_cnt->rxerror, wlc->pub->_cnt->rxoflo + wlc->pub->_cnt->rxnobuf +
	       wlc->pub->_cnt->rxfragerr + wlc->pub->_cnt->rxrunt + wlc->pub->_cnt->rxgiant +
	       wlc->pub->_cnt->rxnoscb + wlc->pub->_cnt->rxbadsrcmac);
	for (i = 0; i < NFIFO; i++)
		WLCNTADD(wlc->pub->_cnt->rxerror, wlc->pub->_cnt->rxuflo[i]);
}

#if defined(DELTASTATS)
static void
wlc_delta_stats_update(wlc_info_t *wlc)
{
	wl_delta_stats_t *stats;
#ifdef WLCNT
	wl_cnt_t *cnt = wlc->pub->_cnt;
#endif

	/* current stats becomes previous stats */
	wlc->delta_stats->current_index =
		(wlc->delta_stats->current_index + 1) % DELTA_STATS_NUM_INTERVAL;

	/* update current stats */
	stats = &wlc->delta_stats->stats[wlc->delta_stats->current_index];

	/* NOTE: wlc_statsupd() must be invoked to update the stats if there are
	 * any stats which depend on macstat_t
	 */

	/* stats not dependent on macstat_t */
	WLCNTSET(stats->txframe,   WLCNTVAL(cnt->txframe));
	WLCNTSET(stats->txbyte,    WLCNTVAL(cnt->txbyte));
	WLCNTSET(stats->txretrans, WLCNTVAL(cnt->txretrans));
	WLCNTSET(stats->txfail,    WLCNTVAL(cnt->txfail));
	WLCNTSET(stats->rxframe,   WLCNTVAL(cnt->rxframe));
	WLCNTSET(stats->rxbyte,    WLCNTVAL(cnt->rxbyte));
	WLCNTSET(stats->rx1mbps,   WLCNTVAL(cnt->rx1mbps));
	WLCNTSET(stats->rx2mbps,   WLCNTVAL(cnt->rx2mbps));
	WLCNTSET(stats->rx5mbps5,  WLCNTVAL(cnt->rx5mbps5));
	WLCNTSET(stats->rx6mbps,   WLCNTVAL(cnt->rx6mbps));
	WLCNTSET(stats->rx9mbps,   WLCNTVAL(cnt->rx9mbps));
	WLCNTSET(stats->rx11mbps,  WLCNTVAL(cnt->rx11mbps));
	WLCNTSET(stats->rx12mbps,  WLCNTVAL(cnt->rx12mbps));
	WLCNTSET(stats->rx18mbps,  WLCNTVAL(cnt->rx18mbps));
	WLCNTSET(stats->rx24mbps,  WLCNTVAL(cnt->rx24mbps));
	WLCNTSET(stats->rx36mbps,  WLCNTVAL(cnt->rx36mbps));
	WLCNTSET(stats->rx48mbps,  WLCNTVAL(cnt->rx48mbps));
	WLCNTSET(stats->rx54mbps,  WLCNTVAL(cnt->rx54mbps));

	WLCNTSET(stats->rxbadplcp,	WLCNTVAL(cnt->rxbadplcp));
	WLCNTSET(stats->rxcrsglitch,	WLCNTVAL(cnt->rxcrsglitch));
	WLCNTSET(stats->bphy_rxcrsglitch,  WLCNTVAL(cnt->bphy_rxcrsglitch));
	WLCNTSET(stats->bphy_badplcp,	WLCNTVAL(cnt->bphy_badplcp));
}

static int
wlc_get_delta_stats(wlc_info_t *wlc, wl_delta_stats_t *stats)
{
	int err = BCME_OK;

	/* return zeros if disabled or interval has not yet elapsed */
	if (wlc->delta_stats->interval == 0 ||
		wlc->delta_stats->seconds <= wlc->delta_stats->interval) {
		/* zero the entire structure */
		bzero(stats, sizeof(wl_delta_stats_t));
		err = BCME_ERROR;
	} else {
		/* indexes to current and previous stats */
#ifdef WLCNT
		uint32 curr_index = wlc->delta_stats->current_index;
		uint32 prev_index = (curr_index - 1) % DELTA_STATS_NUM_INTERVAL;

		/* pointers to current and previous stats */
		wl_delta_stats_t *curr = &wlc->delta_stats->stats[curr_index];
		wl_delta_stats_t *prev = &wlc->delta_stats->stats[prev_index];
#endif

		/* calculate delta statistics */
		WLCNTSET(stats->txframe,   curr->txframe   - prev->txframe);
		WLCNTSET(stats->txbyte,    curr->txbyte    - prev->txbyte);
		WLCNTSET(stats->txretrans, curr->txretrans - prev->txretrans);
		WLCNTSET(stats->txfail,    curr->txfail    - prev->txfail);
		WLCNTSET(stats->rxframe,   curr->rxframe   - prev->rxframe);
		WLCNTSET(stats->rxbyte,    curr->rxbyte    - prev->rxbyte);
		WLCNTSET(stats->rx1mbps,   curr->rx1mbps   - prev->rx1mbps);
		WLCNTSET(stats->rx2mbps,   curr->rx2mbps   - prev->rx2mbps);
		WLCNTSET(stats->rx5mbps5,  curr->rx5mbps5  - prev->rx5mbps5);
		WLCNTSET(stats->rx6mbps,   curr->rx6mbps   - prev->rx6mbps);
		WLCNTSET(stats->rx9mbps,   curr->rx9mbps   - prev->rx9mbps);
		WLCNTSET(stats->rx11mbps,  curr->rx11mbps  - prev->rx11mbps);
		WLCNTSET(stats->rx12mbps,  curr->rx12mbps  - prev->rx12mbps);
		WLCNTSET(stats->rx18mbps,  curr->rx18mbps  - prev->rx18mbps);
		WLCNTSET(stats->rx24mbps,  curr->rx24mbps  - prev->rx24mbps);
		WLCNTSET(stats->rx36mbps,  curr->rx36mbps  - prev->rx36mbps);
		WLCNTSET(stats->rx48mbps,  curr->rx48mbps  - prev->rx48mbps);
		WLCNTSET(stats->rx54mbps,  curr->rx54mbps  - prev->rx54mbps);

		WLCNTSET(stats->rxbadplcp,	curr->rxbadplcp  - prev->rxbadplcp);
		WLCNTSET(stats->rxcrsglitch,	curr->rxcrsglitch  - prev->rxcrsglitch);
		WLCNTSET(stats->bphy_rxcrsglitch, curr->bphy_rxcrsglitch  - prev->bphy_rxcrsglitch);
		WLCNTSET(stats->bphy_badplcp,	curr->bphy_badplcp  - prev->bphy_badplcp);
	}

	/* set version and length of structure */
	WLCNTSET(stats->version, WL_DELTA_STATS_T_VERSION);
	WLCNTSET(stats->length, sizeof(wl_delta_stats_t));

	return err;
}
#endif /* DELTASTATS */

uint
wlc_ctrupd(wlc_info_t *wlc, uint ucode_offset, uint offset)
{
#ifdef WLCNT
	uint16 v, delta;
	uint32 *total;
	uint16 *per_core;

	total = (uint32*)((char*)(&wlc->pub->_cnt->txallfrm) + offset);
	per_core = (uint16*)((char*)wlc->core->macstat_snapshot + ucode_offset);

	v = wlc_bmac_read_shm(wlc->hw, M_UCODE_MACSTAT + ucode_offset);
	delta = (uint16)(v - *per_core);

	if (delta != 0) {
		*total += delta;
		*per_core = v;
	}
	return (delta);
#else
	return (0);
#endif /* WLCNT */
}

static const uint16 BCMATTACHDATA(wlc_devid)[] = {
	BCM4306_D11G_ID, BCM4306_D11G_ID2, BCM4303_D11B_ID, BCM4306_D11A_ID, BCM4306_D11DUAL_ID,
	BCM4318_D11G_ID, BCM4318_D11DUAL_ID, BCM4318_D11A_ID,
	BCM4311_D11G_ID, BCM4311_D11A_ID, BCM4311_D11DUAL_ID,
	BCM4328_D11DUAL_ID, BCM4328_D11G_ID, BCM4328_D11A_ID,
	BCM4325_D11DUAL_ID, BCM4325_D11G_ID, BCM4325_D11A_ID,
	BCM4321_D11N_ID, BCM4321_D11N2G_ID, BCM4321_D11N5G_ID,
	BCM4322_D11N_ID, BCM4322_D11N2G_ID, BCM4322_D11N5G_ID,
	BCM43221_D11N2G_ID,
	BCM43231_D11N2G_ID,
	BCM43222_D11N_ID, BCM43222_D11N2G_ID, BCM43222_D11N5G_ID,
	BCM43224_D11N_ID,
	BCM43225_D11N2G_ID,
	BCM43421_D11N_ID,
	BCM43224_D11N_ID_VEN1,
	BCM43226_D11N_ID,
	BCM6362_D11N_ID, BCM6362_D11N2G_ID, BCM6362_D11N5G_ID,
	BCM4329_D11N2G_ID,
	BCM4315_D11DUAL_ID, BCM4315_D11G_ID, BCM4315_D11A_ID,
	BCM4319_D11N_ID, BCM4319_D11N2G_ID, BCM4319_D11N5G_ID,
	BCM4716_CHIP_ID,
	BCM4748_CHIP_ID,
	BCM4313_D11N2G_ID,
	BCM4336_D11N_ID,
	BCM4330_D11N_ID,
	BCM43236_D11N_ID, BCM43236_D11N2G_ID, BCM43236_D11N5G_ID,
	BCM4331_D11N_ID, BCM4331_D11N2G_ID, BCM4331_D11N5G_ID,
	BCM43131_D11N2G_ID,
	BCM43227_D11N2G_ID,
	BCM43228_D11N_ID, BCM43228_D11N5G_ID,
	BCM43217_D11N2G_ID,
	BCM43237_D11N_ID, BCM43237_D11N5G_ID,
	BCM43362_D11N_ID,
	BCM4334_D11N_ID, BCM4334_D11N2G_ID, BCM4334_D11N5G_ID, BCM4314_D11N2G_ID,
	BCM43142_D11N2G_ID,
	BCM43143_D11N2G_ID,
	BCM4324_D11N_ID,
	BCM43242_D11N_ID,
	BCM4360_D11AC_ID, BCM4360_D11AC2G_ID, BCM4360_D11AC5G_ID,
	BCM4335_D11AC_ID, BCM4335_D11AC2G_ID, BCM4335_D11AC5G_ID,
	BCM4345_D11AC_ID, BCM4345_D11AC2G_ID, BCM4345_D11AC5G_ID,
	BCM43602_D11AC_ID, BCM43602_D11AC2G_ID, BCM43602_D11AC5G_ID,
	BCM4352_D11AC_ID, BCM4352_D11AC2G_ID, BCM4352_D11AC5G_ID,
	BCM4350_D11AC_ID, BCM4350_D11AC2G_ID, BCM4350_D11AC5G_ID,
	BCM43556_D11AC_ID, BCM43556_D11AC2G_ID, BCM43556_D11AC5G_ID,
	BCM43558_D11AC_ID, BCM43558_D11AC2G_ID, BCM43558_D11AC5G_ID,
	BCM43341_D11N_ID, BCM43341_D11N2G_ID, BCM43341_D11N5G_ID,
	BCM43566_D11AC_ID, BCM43566_D11AC2G_ID, BCM43566_D11AC5G_ID,
	BCM43568_D11AC_ID, BCM43568_D11AC2G_ID, BCM43568_D11AC5G_ID,
	BCM43569_D11AC_ID, BCM43569_D11AC2G_ID, BCM43569_D11AC5G_ID,
	BCM4354_D11AC_ID, BCM4354_D11AC2G_ID, BCM4354_D11AC5G_ID,
	BCM4356_D11AC_ID, BCM4356_D11AC2G_ID, BCM4356_D11AC5G_ID,
	/* Chips with blank-OTP */
	/* BCMHOSTVARS is enabled only if WLTEST is enabled or BCMEXTNVM is enabled */
#if defined(BCMHOSTVARS)
	BCM4322_CHIP_ID, BCM43222_CHIP_ID,
	BCM43111_CHIP_ID,
	BCM43112_CHIP_ID,
	BCM43420_CHIP_ID,
	BCM43224_CHIP_ID,
	BCM43421_CHIP_ID,
	BCM4313_CHIP_ID,
	BCM4314_CHIP_ID,
	BCM43142_CHIP_ID,
	BCM4331_CHIP_ID,
	BCM43431_CHIP_ID,
	BCM43227_CHIP_ID,
	BCM43217_CHIP_ID,
	BCM43228_CHIP_ID, BCM43428_CHIP_ID,
	BCM43602_CHIP_ID,
	BCM43569_CHIP_ID,
	BCM43570_CHIP_ID
#endif 
};

bool
BCMATTACHFN(wlc_chipmatch)(uint16 vendor, uint16 device)
{
	uint i;
	if (vendor != VENDOR_BROADCOM) {
		WL_ERROR(("%s: unknown vendor id %04x\n", __FUNCTION__, vendor));
		return (FALSE);
	}

	for (i = 0; i < sizeof(wlc_devid)/sizeof(wlc_devid[0]); i++) {
		if (device == wlc_devid[i])
			return TRUE;
	}

	WL_ERROR(("%s: unknown device id %04x\n", __FUNCTION__, device));
	return (FALSE);
}

uint16
wlc_rate_shm_offset(wlc_info_t *wlc, uint8 rate)
{
	return (wlc_bmac_rate_shm_offset(wlc->hw, rate));
}

/** Callback for device removed */
#if defined(WLC_HIGH_ONLY)
void
wlc_device_removed(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;

	wlc->device_present = FALSE;
}
#endif 

/**
 * If a lifetime is configured, calculate the expiry time for the packet and update the pkttag.
 */
void
wlc_lifetime_set(wlc_info_t *wlc, void *sdu, uint32 lifetime)
{
	WLPKTTAG(sdu)->flags |= WLF_EXPTIME;
	WLPKTTAG(sdu)->u.exptime = R_REG(wlc->osh, &wlc->regs->tsf_timerlow) + lifetime;
}

/**
 * Attempts to queue a packet onto a multiple-precedence queue, if necessary evicting a lower
 * precedence packet from the queue.
 *
 * 'prec' is the precedence number that has already been mapped from the packet priority.
 *
 * Returns TRUE if packet consumed (queued), FALSE if not.
 */
bool BCMFASTPATH
wlc_prec_enq(wlc_info_t *wlc, struct pktq *q, void *pkt, int prec)
{
	return wlc_prec_enq_head(wlc, q, pkt, prec, FALSE);
}

bool BCMFASTPATH
wlc_prec_enq_head(wlc_info_t *wlc, struct pktq *q, void *pkt, int prec, bool head)
{
	void *p;
	void *dqp = NULL;
	int eprec = -1;		/* precedence to evict from */
#ifdef PKTQ_LOG
	pktq_counters_t* prec_cnt = 0;

	if (q->pktqlog) {
		prec_cnt = q->pktqlog->_prec_cnt[prec];

		/* check for auto enabled logging */
		if (prec_cnt == NULL && (q->pktqlog->_prec_log & PKTQ_LOG_AUTO)) {
			prec_cnt = wlc_txq_prec_log_enable(wlc, q, (uint32)prec);
		}
	}
	WLCNTCONDINCR(prec_cnt, prec_cnt->requested);
#endif

	/* Determine precedence from which to evict packet, if any */
	if (pktq_pfull(q, prec))
		eprec = prec;
	else if (pktq_full(q)) {
		p = pktq_peek_tail(q, &eprec);
		if (p == NULL)
			WL_ERROR(("%s: null ptr", __FUNCTION__));
		ASSERT(p != NULL);
		BCM_REFERENCE(p);
		if (eprec > prec) {
#ifdef PKTQ_LOG
		WLCNTCONDINCR(prec_cnt, prec_cnt->full_dropped);
#endif
			return FALSE;
		}
	}

	/* Evict if needed */
	if (eprec >= 0) {
		bool discard_oldest;
#if defined(WLCNT) || defined(WLCNTSCB)
		struct scb *scb;
#endif /* WLCNT || WLCNTSCB */

		/* Detect queueing to unconfigured precedence */
		ASSERT(!pktq_pempty(q, eprec));

		discard_oldest = AC_BITMAP_TST(wlc->wme_dp_precmap, eprec);

		/* Refuse newer packet unless configured to discard oldest */
		if (eprec == prec && !discard_oldest) {
#ifdef PKTQ_LOG
			WLCNTCONDINCR(prec_cnt, prec_cnt->dropped);
#endif
			return FALSE;
		}
		/* Evict packet according to discard policy */
		dqp = discard_oldest ? pktq_pdeq(q, eprec) : pktq_pdeq_tail(q, eprec);
		ASSERT(dqp != NULL);

		/* Increment wme stats */
		if (WME_ENAB(wlc->pub)) {
			WLCNTINCR(wlc->pub->_wme_cnt->tx_failed[WME_PRIO2AC(PKTPRIO(dqp))].packets);
			WLCNTADD(wlc->pub->_wme_cnt->tx_failed[WME_PRIO2AC(PKTPRIO(dqp))].bytes,
			         pkttotlen(wlc->osh, dqp));
		}

#if defined(WLCNT) || defined(WLCNTSCB)
		scb = WLPKTTAGSCBGET(dqp);
#endif /* WLCNT || WLCNTSCB */
#ifdef PKTQ_LOG
		if (eprec == prec) {
			WLCNTCONDINCR(prec_cnt, prec_cnt->selfsaved);
		}
		else {
			WLCNTCONDINCR(prec_cnt, prec_cnt->saved);
			WLCNTCONDINCR(prec_cnt, prec_cnt->sacrificed);
		}
#endif
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		WLCIFCNTINCR(scb, txnobuf);
		WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_failures);
	}

	/* Enqueue */
#ifdef PKTQ_LOG
	WLCNTCONDINCR(prec_cnt, prec_cnt->stored);

	if (prec_cnt)
	{
		uint32 qlen = pktq_pavail(q, prec);
		uint32* max_avail = &prec_cnt->max_avail;
		uint32* max_used = &prec_cnt->max_used;

		if (qlen < *max_avail) {
			*max_avail = qlen;
		}
		qlen = pktq_plen(q, prec);
		if (qlen > *max_used) {
			*max_used = qlen;
		}
	}
#endif /* PKTQ_LOG */

	if (head)
		p = pktq_penq_head(q, prec, pkt);
	else
		p = pktq_penq(q, prec, pkt);
	if (p == NULL)
		WL_ERROR(("%s: null ptr2", __FUNCTION__));
	ASSERT(p != NULL);

	PKTDBG_TRACE(wlc->osh, p, PKTLIST_PRECQ);

	/* Free dequeued packet */
	if (dqp != NULL) {
		PKTFREE(wlc->osh, dqp, TRUE);
	}

	return TRUE;
}

int
wlc_prio2prec(wlc_info_t *wlc, int prio)
{
	ASSERT(prio >= 0 && prio < ARRAYSIZE(wlc_prio2prec_map));
	if (EDCF_ENAB(wlc->pub)) {
		return wlc_prio2prec_map[prio];
	} else {
		return _WLC_PREC_BE; /* default to BE */
	}
}

/** Sends an 80211 packet with radio parameters specified */
bool
wlc_send80211_specified(wlc_info_t *wlc, void *sdu, ratespec_t rspec, struct wlc_if *wlcif)
{
	bool short_preamble = FALSE;
	wlc_bsscfg_t *bsscfg;
	struct scb *scb;
	chanspec_t chanspec;

	/* figure out the bsscfg for this packet */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	ASSERT(wlcif->qi != NULL);

	chanspec = wlc_get_home_chanspec(bsscfg);

	/* grab the hwrs_scb and validate the rate override
	 * based on the home channel band for this wlcif
	 */
	if (CHSPEC_IS2G(chanspec)) {
		scb = wlc->bandstate[BAND_2G_INDEX]->hwrs_scb;
	} else {
		scb = wlc->bandstate[BAND_5G_INDEX]->hwrs_scb;
		/* make sure the rate override does not specify a DSSS/CCK rate */
		if (IS_CCK(rspec)) {
			rspec = 0; /* clear the invalid rate override */
			WL_ERROR(("wl%d: %s: rate override with DSSS rate "
			          "incompatible with 5GHz band\n",
			          wlc->pub->unit, __FUNCTION__));
		}
	}

	/* validate the rspec for the correct BW */
	if ((rspec & RSPEC_OVERRIDE_MODE) &&
	    CHSPEC_IS20(wlc->chanspec) &&
	    !RSPEC_IS20MHZ(rspec)) {
		rspec = 0; /* clear the invalid rate override */
		WL_ERROR(("wl%d: %s: rate override with bw > 20MHz but only 20MHz channel\n",
		          wlc->pub->unit, __FUNCTION__));
	} else if ((rspec & RSPEC_OVERRIDE_MODE) &&
	           CHSPEC_IS40(wlc->chanspec) &&
	           !(RSPEC_IS20MHZ(rspec) || RSPEC_IS40MHZ(rspec))) {
		rspec = 0; /* clear the invalid rate override */
		WL_ERROR(("wl%d: %s: rate override with bw > 40MHz but only 40MHz channel\n",
		          wlc->pub->unit, __FUNCTION__));
	}

	/* if the rspec is for 2,5.5,11 rate, and specifies a mode override,
	 * check for short preamble
	 */
	if ((rspec & RSPEC_OVERRIDE_MODE) &&
	    RSPEC_ISLEGACY(rspec) &&
	    ((rspec & RATE_MASK) == WLC_RATE_2M ||
	     (rspec & RATE_MASK) == WLC_RATE_5M5 ||
	     (rspec & RATE_MASK) == WLC_RATE_11M))
		short_preamble = ((rspec & RSPEC_SHORT_PREAMBLE) != 0);

	return wlc_queue_80211_frag(wlc, sdu, wlc->active_queue, scb,
		(scb ? scb->bsscfg : NULL), short_preamble, NULL, rspec);
}

#ifdef PLC_WET
void
wlc_plc_wet_node_add(wlc_info_t *wlc, uint32 node_type, uint8 *eaddr, uint8 *waddr)
{
	struct wl_plc_node *node;
	struct scb *scb;
	struct ether_addr ea;
	wlc_bsscfg_t *bsscfg;

	if (!PLC_ENAB(wlc->pub))
		return;

	if ((node_type != NODE_TYPE_WIFI_ONLY) && (node_type != NODE_TYPE_WIFI_PLC)) {
		WL_ERROR(("wl%d: node type should be either wifi only or wifi plc\n",
		          wlc->pub->unit));
		return;
	}

	if (wl_plc_node_add(wlc->wl, node_type, eaddr, waddr, &node) == BCME_OK) {
		/* While adding WET tunnel end point do the scb find using
		 * WET mac address.
		 */
		bcopy(waddr, &ea, ETHER_ADDR_LEN);
		bsscfg = wlc_bsscfg_find_by_bssid(wlc, &ea);
		ASSERT(bsscfg != NULL);
		scb = wlc_scbfind(wlc, bsscfg, &ea);
		ASSERT(scb != NULL);
		wl_plc_node_set_scb(wlc->wl, node, scb);
		wl_plc_node_set_bsscfg(wlc->wl, node, SCB_BSSCFG(scb));
	}

	return;
}

void
wlc_plc_node_add(wlc_info_t *wlc, uint32 node_type, struct scb *scb)
{
	struct wl_plc_node *node;

	if (!PLC_ENAB(wlc->pub))
		return;

	if ((node_type != NODE_TYPE_WIFI_ONLY) && (node_type != NODE_TYPE_WIFI_PLC)) {
		WL_ERROR(("wl%d: node type should be either wifi only or wifi plc\n",
		          wlc->pub->unit));
		return;
	}

	if (wl_plc_node_add(wlc->wl, node_type, scb->ea.octet, NULL, &node) == BCME_OK) {
		wl_plc_node_set_scb(wlc->wl, node, scb);
		wl_plc_node_set_bsscfg(wlc->wl, node, SCB_BSSCFG(scb));
	}

	return;
}
#endif /* PLC_WET */

#ifdef WMF
bool
is_igmp(struct ether_header *eh)
{
	uint8 *iph = (uint8 *)eh + ETHER_HDR_LEN;

	if ((ntoh16(eh->ether_type) == ETHER_TYPE_IP) && (IP_VER(iph) == IP_VER_4) &&
		(IPV4_PROT(iph) == IP_PROT_IGMP))
		return TRUE;
	else
		return FALSE;
}

#ifdef PLC_WET
int32 BCMFASTPATH
wlc_wmf_proc(wlc_bsscfg_t *cfg, void *sdu, struct ether_header *eh, void *handle, bool frombss)
{
	/* Do the WMF processing for multicast packets */
	if (!WMF_ENAB(cfg) || !(ETHER_ISMULTI(eh->ether_dhost) ||
		(cfg->wmf_ucast_igmp && (is_igmp(eh)))))
		return BCME_OK;

	switch (wlc_wmf_packets_handle(cfg, handle, sdu, frombss)) {
	case WMF_DROP:
		/* Packet DROP decision by WMF. Toss it */
		PKTFREE(cfg->wlc->osh, sdu, !frombss);
	case WMF_TAKEN:
		/* The packet is taken by WMF return */
		break;
	default:
		/* Continue the transmit/receive path */
		return BCME_OK;
	}

	return BCME_ERROR;
}
#endif /* PLC_WET */
#endif /* WMF */

#if defined(PKTC) || defined(PKTC_TX_DONGLE)
static void BCMFASTPATH
wlc_pktc_sdu_prep_copy(wlc_info_t *wlc, struct scb *scb, void *p, void *n, uint32 lifetime)
{
	struct dot3_mac_llc_snap_header *dot3lsh;

	ASSERT(n != NULL);

	WLPKTTAGSCBSET(n, scb);
	WLPKTTAGBSSCFGSET(n, WLPKTTAGBSSCFGGET(p));
#ifdef DMATXRC
	WLPKTTAG(n)->flags = WLPKTTAG(p)->flags & ~WLF_PHDR;
#else
	WLPKTTAG(n)->flags = WLPKTTAG(p)->flags;
#endif
	WLPKTTAG(n)->flags2 = WLPKTTAG(p)->flags2;
	WLPKTTAG(n)->flags3 = WLPKTTAG(p)->flags3;
	PKTSETPRIO(n, PKTPRIO(p));

	/* Init llc snap hdr and fixup length */
	/* Original is ethernet hdr (14)
	 * Convert to 802.3 hdr (22 bytes) so only need to
	 * another 8 bytes (DOT11_LLC_SNAP_HDR_LEN)
	 */
	PKTPUSH(wlc->osh, n, DOT11_LLC_SNAP_HDR_LEN);
	dot3lsh = (struct dot3_mac_llc_snap_header *)PKTDATA(wlc->osh, n);
	*dot3lsh = *(struct dot3_mac_llc_snap_header *)PKTDATA(wlc->osh, p);

	/* Set packet exptime */
	if (lifetime != 0)
		wlc_lifetime_set(wlc, n, lifetime);
}

void BCMFASTPATH
wlc_pktc_sdu_prep(wlc_info_t *wlc, struct scb *scb, void *p, void *n, uint32 lifetime)
{
	struct dot3_mac_llc_snap_header *dot3lsh;

	wlc_pktc_sdu_prep_copy(wlc, scb, p, n, lifetime);
	dot3lsh = (struct dot3_mac_llc_snap_header *)PKTDATA(wlc->osh, n);
	dot3lsh->length = HTON16(pkttotlen(wlc->osh, n) - ETHER_HDR_LEN);
}

#ifdef DMATXRC
/** Prepend phdr and move p's 802.3 header/flags/attributes to phdr */
void* BCMFASTPATH
wlc_pktc_sdu_prep_phdr(wlc_info_t *wlc, struct scb *scb, void *p, uint32 lifetime)
{
	void *phdr = wlc_phdr_get(wlc);

	if (phdr == NULL)
		return NULL;

	ASSERT(!(WLPKTTAG(p)->flags & WLF_PHDR));

	/*
	 * wlc_pktc_sdu_prep_copy() assumes pkt has ether header
	 * so make room for it.
	 */
	PKTPUSH(wlc->osh, phdr, ETHER_HDR_LEN);

	/* Move 802.3 header/flags/etc. from p to phdr */
	wlc_pktc_sdu_prep_copy(wlc, scb, p, phdr, lifetime);
	PKTPULL(wlc->osh, p, sizeof(struct dot3_mac_llc_snap_header));

	WLPKTTAG(phdr)->flags |= WLF_PHDR;
	if (PROP_TXSTATUS_ENAB(wlc->pub))
		WLFC_PKTAG_INFO_MOVE(p, phdr);

	if (PKTLINK(p)) {
		PKTSETLINK(phdr, PKTLINK(p));
		PKTSETLINK(p, NULL);
		PKTSETCHAINED(wlc->osh, phdr);
		PKTCLRCHAINED(wlc->osh, p);
	}

	PKTSETNEXT(wlc->osh, phdr, p);
	ASSERT(PKTNEXT(wlc->osh, p) == NULL);

	return phdr;
}
#endif /* DMATXRC */
#endif /* PKTC */

/* Move flags from one packet to another */
#define WLPKTTAG_FLAGS_COPY_COMMON(pkt_from, pkt_to) \
	do { \
		WLPKTTAG(pkt_to)->flags = WLPKTTAG(pkt_from)->flags & \
			(WLF_APSD | WLF_PSDONTQ | WLF_AMSDU | WLF_AMPDU_MPDU | \
			 WLF_BSS_DOWN | \
			 WLF_USERTS | WLF_RATE_AUTO | WLF_8021X | WLF_EXPTIME | \
		         WLF_DATA | WLF_TXHDR | WLF_PROPTX_PROCESSED); \
		WLPKTTAG(pkt_to)->flags2 = WLPKTTAG(pkt_from)->flags2 & \
			(WLF2_PCB1_MASK | WLF2_PCB3_MASK); \
		WLPKTTAG(pkt_to)->flags3 = WLPKTTAG(pkt_from)->flags3 & \
			(WLF3_NO_PMCHANGE | WLF3_HW_CSUM | WLF3_DATA_WOWL_PKT | WLF3_FAVORED); \
	} while (0)

#define WLPKTTAG_FLAGS_COPY(pkt_from, pkt_to) \
	do { \
		WLPKTTAG_FLAGS_COPY_COMMON(pkt_from, pkt_to); \
	} while (0)

#define WLPKTTAG_FLAGS_MOVE(pkt_from, pkt_to) \
	do { \
		WLPKTTAG_FLAGS_COPY(pkt_from, pkt_to); \
	} while (0)

/* Move callback functions from a packet to another
 * CAUTION: This is destructive operation for pkt_from
 */
void
wlc_pkttag_info_move(wlc_pub_t *pub, void *pkt_from, void *pkt_to)
{
	/* Make sure not moving to same packet! */
	ASSERT(pkt_from != pkt_to);
	WLPKTTAG(pkt_to)->callbackidx = WLPKTTAG(pkt_from)->callbackidx;
	WLPKTTAG(pkt_from)->callbackidx = 0;
	WLPKTTAG(pkt_to)->_bsscfgidx = WLPKTTAG(pkt_from)->_bsscfgidx;
	WLPKTTAG(pkt_to)->_scb = WLPKTTAG(pkt_from)->_scb;
	WLPKTTAG(pkt_to)->rspec = WLPKTTAG(pkt_from)->rspec;
	WLPKTTAG(pkt_to)->seq = WLPKTTAG(pkt_from)->seq;
	WLPKTTAG(pkt_to)->u.exptime = WLPKTTAG(pkt_from)->u.exptime;
	WLFC_PKTAG_INFO_MOVE(pkt_from, pkt_to);
	WLPKTTAG_FLAGS_MOVE(pkt_from, pkt_to);
	WLPKTTAG(pkt_to)->shared.packetid = WLPKTTAG(pkt_from)->shared.packetid;
}

/* queue packet suppressed due to scheduled absence periods to the end
 * of the BSS's suppression queue or the STA's PS queue.
 */
bool
wlc_pkt_abs_supr_enq(wlc_info_t *wlc, struct scb *scb, void *pkt)
{
	bool free_pkt = FALSE;
	wlc_bsscfg_t *cfg;
	wlc_pkttag_t *pkttag;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	ASSERT(scb != NULL);
	ASSERT(pkt != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	if (!cfg->enable) {
		WL_ERROR(("%s: called with cfg %p disabled\n", __FUNCTION__, cfg));
		return TRUE; /* free packets */
	}

	ASSERT(BSS_TX_SUPR(cfg));

#ifdef WLP2P
	/* Basically do not put packets in abs_q (PSQ) if NOA or OpPS */
	/* is already cancelled */
	if (BSS_P2P_ENAB(wlc, SCB_BSSCFG(scb)) &&
	    !wlc_p2p_noa_valid(wlc->p2p, cfg) &&
	    !wlc_p2p_ops_valid(wlc->p2p, cfg)) {
		WL_INFORM(("wl%d: free supr'd frame outside absence period "
			"due to rx processing lag???\n", wlc->pub->unit));
		return TRUE;
	}
#endif /* WLP2P */

	pkttag = WLPKTTAG(pkt);

	if (pkttag->flags3 & WLF3_TXQ_SHORT_LIFETIME) {
		WL_P2P(("%s: drop short-lived pkt %p...\n", __FUNCTION__, pkt));
		free_pkt = TRUE;
	}
	else if (BSSCFG_AP(cfg) &&
	         SCB_PS(scb) && !SCB_ISMULTI(scb) &&
	         SCB_ASSOCIATED(scb) && SCB_P2P(scb)) {
		if (!(pkttag->flags & WLF_PSDONTQ)) {
			free_pkt = wlc_apps_scb_supr_enq(wlc, scb, pkt);
			WL_P2P(("wl%d.%d: %s: requeue packet %p for %s %s\n",
			        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkt,
			        bcm_ether_ntoa(&scb->ea, eabuf),
			        free_pkt?"unsuccessfully":"successfully"));
		}
		/* toss driver generated NULL frame */
		else {
			WL_P2P(("wl%d.%d: %s: drop NULL packet %p for %s\n",
			        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkt,
			        bcm_ether_ntoa(&scb->ea, eabuf)));
			free_pkt = TRUE;
		}
	}
	else if (BSSCFG_STA(cfg)) {
		d11txh_t *txh = (d11txh_t *)PKTDATA(wlc->osh, pkt);
		uint8 *plcp = (uint8 *)(txh + ((pkttag->flags & WLF_TXHDR) != 0 ? 1 : 0));
		struct dot11_management_header *h =
		        (struct dot11_management_header *)(plcp + D11_PHY_HDR_LEN);
		uint16 fc = ltoh16(h->fc);
		bool psp = (fc & FC_KIND_MASK) == FC_PS_POLL;

		if (!psp) {
			free_pkt = wlc_bsscfg_tx_supr_enq(wlc, cfg, pkt);
			WL_P2P(("wl%d.%d: %s: requeue packet %p %s\n",
			        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkt,
			        free_pkt?"unsuccessfully":"successfully"));
		}
		/* toss driver generated PSPoll frame */
		else {
			WL_P2P(("wl%d.%d: %s: drop PSPoll packet %p for %s\n",
			        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkt,
			        bcm_ether_ntoa(&scb->ea, eabuf)));
			free_pkt = TRUE;
		}
	}
	else {
		free_pkt = wlc_bsscfg_tx_supr_enq(wlc, cfg, pkt);
		WL_P2P(("wl%d.%d: %s: requeue packet %p %s\n",
		        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkt,
		        free_pkt?"unsuccessfully":"successfully"));
	}

	return free_pkt;
}


/** get original etype for converted ether frame or 8023 frame */
uint16
wlc_sdu_etype(wlc_info_t *wlc, void *sdu)
{
	uint16 etype;
	struct ether_header *eh;

	eh = (struct ether_header*) PKTDATA(wlc->osh, sdu);
	if (WLPKTTAG(sdu)->flags & WLF_NON8023) {
		/* ether and llc/snap are in one continuous buffer */
		etype = *(uint16 *)((uint8*)eh + ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN - 2);
		ASSERT(ntoh16(etype) > ETHER_MAX_DATA);
	} else {
		etype = eh->ether_type;
	}

	return etype;
}

/** get data/body pointer from 8023 frame(original or converted) */
uint8 *
wlc_sdu_data(wlc_info_t *wlc, void *sdu)
{
	uint body_offset;
	uint hdrlen;
	osl_t *osh = wlc->osh;

	BCM_REFERENCE(osh);

	body_offset = ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN;

	/* handle chained hdr buffer and data buffer */
	hdrlen = PKTLEN(osh, sdu);
	if (body_offset >= hdrlen)
		return ((uint8*)PKTDATA(osh, PKTNEXT(osh, sdu)) + (body_offset - hdrlen));
	else
		return ((uint8*)PKTDATA(osh, sdu) + body_offset);
}

/** Convert Ethernet to 802.3 per 802.1H (use bridge-tunnel if type in SST) */
void BCMFASTPATH
wlc_ether_8023hdr(wlc_info_t *wlc, osl_t *osh, struct ether_header *eh, void *p)
{
	struct ether_header *neh;
	struct dot11_llc_snap_header *lsh;
	uint16 plen, ether_type;

	ether_type = ntoh16(eh->ether_type);
	neh = (struct ether_header *)PKTPUSH(osh, p, DOT11_LLC_SNAP_HDR_LEN);

	/* 802.3 MAC header */
	eacopy((char*)eh->ether_dhost, (char*)neh->ether_dhost);
	eacopy((char*)eh->ether_shost, (char*)neh->ether_shost);
	plen = (uint16)pkttotlen(osh, p) - ETHER_HDR_LEN;
	neh->ether_type = hton16(plen);

	/* 802.2 LLC header */
	lsh = (struct dot11_llc_snap_header *)&neh[1];
	lsh->dsap = 0xaa;
	lsh->ssap = 0xaa;
	lsh->ctl = 0x03;

	/* 802.2 SNAP header Use RFC1042 or bridge-tunnel if type in SST per 802.1H */
	lsh->oui[0] = 0x00;
	lsh->oui[1] = 0x00;
	if (sstlookup(wlc, ether_type))
		lsh->oui[2] = 0xf8;
	else
		lsh->oui[2] = 0x00;
	lsh->type = hton16(ether_type);
}

static uint16
wlc_compute_airtime(wlc_info_t *wlc, ratespec_t rspec, uint length)
{
	uint16 usec = 0;
	uint mac_rate = RSPEC2RATE(rspec);
	uint nsyms;

	if (RSPEC_ISHT(rspec) || RSPEC_ISVHT(rspec)) {
		/* not supported yet */
		ASSERT(0);
	} else if (IS_OFDM(rspec)) {
		/* nsyms = Ceiling(Nbits / (Nbits/sym))
		 *
		 * Nbits = length * 8
		 * Nbits/sym = Mbps * 4 = mac_rate * 2
		 */
		nsyms = CEIL((length * 8), (mac_rate * 2));

		/* usec = symbols * usec/symbol */
		usec = (uint16) (nsyms * APHY_SYMBOL_TIME);
		return (usec);
	} else {
		switch (mac_rate) {
		case WLC_RATE_1M:
			usec = length << 3;
			break;
		case WLC_RATE_2M:
			usec = length << 2;
			break;
		case WLC_RATE_5M5:
			usec = (length << 4)/11;
			break;
		case WLC_RATE_11M:
			usec = (length << 3)/11;
			break;
		default:
			WL_ERROR(("wl%d: %s: unsupported rspec 0x%x\n",
				wlc->pub->unit, __FUNCTION__, rspec));
			ASSERT(!"Bad phy_rate");
			break;
		}
	}

	return (usec);
}

void
wlc_compute_plcp(wlc_info_t *wlc, ratespec_t rspec, uint length, uint16 fc, uint8 *plcp)
{
	WL_TRACE(("wl%d: wlc_compute_plcp: rate 0x%x, length %d\n", wlc->pub->unit, rspec, length));

	if (RSPEC_ISVHT(rspec)) {
		wlc_compute_vht_plcp(rspec, length, fc, plcp);
	} else if (RSPEC_ISHT(rspec)) {
		wlc_compute_mimo_plcp(rspec, length, plcp);
	} else if (IS_OFDM(rspec)) {
		wlc_compute_ofdm_plcp(rspec, length, plcp);
	} else {
		wlc_compute_cck_plcp(rspec, length, plcp);
	}
	return;
}

static uint32
wlc_vht_siga1_get_partial_aid(void)
{
	/* 9.17a Partial AID in VHT PPDUs
	The TXVECTOR parameter PARTIAL_AID(#1509) is set as follows:
	In a VHT PPDU that carries group addressed MPDUs, the TXVECTOR parameter PARTIAL_AID(#1509)
	is set to 0. In a VHT PPDU sent by an AP(#1512) that carries MPDUs addressed to a single
	non-AP STA, the TXVECTOR parameter PARTIAL_AID(#1509) is set to:
	PARTIAL_AID[0:8] = (AID(0:8) + ((BSSID[0:3] . BSSID[4:7]) << 5)) mod 29 (9-1)
	Where A[b:c] indicates the bits in positions from b to c of the binary representation of A;
	. is a bitwise exclusive OR operation; << 5 indicates a 5 positions bit shift operation
	towards MSB; mod X indicates the Xmodulo operation; AID is the AID of the recipient STA.
	BSSID is the BSSID the STA is associated with. In DLS or TDLS transmission, the AID for the
	peer STA is obtained from DLS Setup Request and Response frame or TDLS Setup Request and
	Response frame. In a VHT PPDU that carries MPDUs addressed to an AP(#1278), the TXVECTOR
	parameter PARTIAL_AID(#1509) is set to the lower 9 bits of the BSSID. In a VHT PPDU
	addressed to an IBSS peer STA, the TXVECTOR parameter PARTIAL_AID(#1509) is set to 0.
	TODO PW Need to follow above in comments to generate partial AID -- ok for quickturn tho
	*/
	return 0;
}
static vht_group_id_t
wlc_compute_vht_groupid(uint16 fc)
{
	/* In a SU VHT PPDU, if the PPDU carries MPDU(s)
	addressed to an AP the Group ID(Ed) field is set to 0, otherwise
	it is set to 63.
	For a MU-MIMO PPDU the Group ID is set as in 22.3.12.3
	(Group ID)(#972)
	*/
	bool toAP = ((fc & (FC_TODS | FC_FROMDS)) == FC_TODS);
	vht_group_id_t gid = VHT_SIGA1_GID_MAX_GID;

	if (toAP) {
		gid = VHT_SIGA1_GID_TO_AP;
	} else {
		gid = VHT_SIGA1_GID_NOT_TO_AP;
	}
	ASSERT(gid <= VHT_SIGA1_GID_MAX_GID);
	ASSERT(gid >= 0);
	return gid;
}

/** Rate: 802.11 rate code, length: PSDU length in octets */
static void
wlc_compute_vht_plcp(ratespec_t rspec, uint length, uint16 fc, uint8 *plcp)
{
	/* compute VHT plcp - 6 B */
	uint32 plcp0 = 0, plcp1 = 0;
	vht_group_id_t gid = wlc_compute_vht_groupid(fc);

	/* set up const fields -- bit 23 and bit 2 */
	plcp0 = VHT_SIGA1_CONST_MASK;

	/* L-STF L-LTF LSIG VHT-SIG-A VHTSTF VHTLTF VHTLTF      VHTSIG-B Data */
	/* VHT Sig A1 3 Bytes */
	/* bit 0..1 bandwidth */
	/* Set to 0 for 20 MHz,
	        1 for 40 MHz, 2 for 80 MHz, 3 for
	        160 MHz and 80+80 MHz
	*/
	switch (RSPEC_BW(rspec)) {
		case RSPEC_BW_20MHZ:
		/* set to zero eg no-op */
			plcp0 |= VHT_SIGA1_20MHZ_VAL;
			break;
		case RSPEC_BW_40MHZ:
			plcp0 |= VHT_SIGA1_40MHZ_VAL;
			break;
		case RSPEC_BW_80MHZ:
			plcp0 |= VHT_SIGA1_80MHZ_VAL;
			break;
		case RSPEC_BW_160MHZ:
			plcp0 |= VHT_SIGA1_160MHZ_VAL;
			break;
		default:
			ASSERT(0);
	}

	/* bit 3 stb coding */
	if (RSPEC_ISSTBC(rspec))
		plcp0 |= VHT_SIGA1_STBC;

	/* (in SU) bit4-9 if pkt addressed to AP = 0, else 63 */
	plcp0 |= (gid << VHT_SIGA1_GID_SHIFT);

	/* b10-b13 NSTS */
	/* for SU b10-b12 set to num STS-1 (fr 0-7) */
	plcp0 |= ((wlc_ratespec_nsts(rspec)-1) << VHT_SIGA1_NSTS_SHIFT);

	/* b13-b21 partial AID: Set to value of TXVECTOR param PARTIAL_AID */
	plcp0 |= (wlc_vht_siga1_get_partial_aid() << VHT_SIGA1_PARTIAL_AID_SHIFT);


	/* b22 TXOP 1 if txop PS permitted, 0 otherwise */
	/* Indicates whether or not AP
		supports VHT TXOP PS Mode
		for STAs in BSS when included
		in Beacon/Probe Response
		frames.
		Indicates whether or not STA is
		in VHT TXOP PS mode when
		included in(#1513) Association/
		Reassociation Requests(#1501)
	*/

	/* put plcp0 in plcp */
	plcp[0] = (plcp0 & 0xff);
	plcp[1] = ((plcp0 >> 8) & 0xff);
	plcp[2] = ((plcp0 >> 16) & 0xff);

	/* VHT Sig A2 3 Bytes */

	/* B0-B1 Short GI:
	 * B0 is Short GI
	 * B2 disambiguates length, but HW sets this bit
	 */
	if (RSPEC_ISSGI(rspec)) {
		plcp1 |= VHT_SIGA2_GI_SHORT;
	}

	/* B2-B3 Coding 2 B3:(#1304)
	For SU, B2 is set to 0 for BCC, 1 for LDPC
	For MU, if the NSTS field for user 1 is non-zero, then B2
	indicates the coding used for user 1; set to 0 for BCC and
	1 for LDPC. If the NSTS field for user 1 is set to 0, then
	this field is reserved and set to 1.
	B3:
	Set to 1 if LDPC PPDU encoding process (or at least one
	LPDC user's PPDU encoding process) results in an extra
	OFDM symbol (or symbols) as described in 22.3.11.5.2
	(LDPC coding)(#853) and 22.3.11.5.3 (Encoding process
	for MU transmissions)(#1304). Set to 0 otherwise.
	*/
	if (RSPEC_ISLDPC(rspec))
		plcp1 |= VHT_SIGA2_CODING_LDPC;

	/* B4-B7 MCS 4 For SU:
	MCS index
	For MU:
	If the NSTS field for user 2 is non-zero, then B4 indicates
	coding for user 2: set to 0 for BCC, 1 for LDPC. If NSTS
	for user 2 is set to 0, then B4 is reserved and set to 1.
	If the NSTS field for user 3 is non-zero, then B5 indicates
	coding for user 3: set to 0 for BCC, 1 for LDPC. If NSTS
	for user 3 is set to 0, then B5 is reserved and set to 1.
	If the NSTS field for user 4 is non-zero, then B6 indicates
	coding for user 4: set to 0 for BCC, 1 for LDPC. If NSTS
	for user 4 is set to 0, then B6(#612) is reserved and set to
	1. B7 is reserved and set to 1
	*/
	plcp1 |= ((rspec & RSPEC_VHT_MCS_MASK) << VHT_SIGA2_MCS_SHIFT);

	/* B8 Beamformed 1 For SU:
	Set to 1 if a Beamforming steering matrix is applied to the
	waveform in an SU transmission as described in
	19.3.11.11.2 (Spatial mapping)(#1631), set to 0 otherwise.
	For MU:
	Reserved and set to 1
	*/
	if (RSPEC_ISTXBF(rspec) && RSPEC_ISVHT(rspec))
		plcp1 |= VHT_SIGA2_BEAMFORM_ENABLE;

	/* B9 Reserved 1 Reserved and set to 1 */
	plcp1 |= VHT_SIGA2_B9_RESERVED;


	plcp[3] = (plcp1 & 0xff);
	plcp[4] = ((plcp1 >> 8) & 0xff);
	plcp[5] = ((plcp1 >> 16) & 0xff);
}

/** Transmit related. Length: PSDU length in octets */
static void
wlc_compute_mimo_plcp(ratespec_t rspec, uint length, uint8 *plcp)
{
	uint8 plcp3;
	uint8 mcs = (uint8)(rspec & RSPEC_RATE_MASK);
	ASSERT(RSPEC_ISHT(rspec));
	plcp[0] = mcs;
	if (RSPEC_IS40MHZ(rspec) || (mcs == 32))
		plcp[0] |= MIMO_PLCP_40MHZ;
	WLC_SET_MIMO_PLCP_LEN(plcp, length);

	plcp3 = 0x7;	/* set smoothing, not sounding ppdu & reserved */

	/* support STBC expansion of Nss = 1 (MCS 0-7) to Nsts = 2 */
	if (RSPEC_ISSTBC(rspec)) {
		/* STC = Nsts - Nss */
		plcp3 |= 1 << PLCP3_STC_SHIFT;
	}

	if (RSPEC_ISLDPC(rspec)) {
		plcp3 |= PLCP3_LDPC;
	}

	if (RSPEC_ISSGI(rspec)) {
		plcp3 |= PLCP3_SGI;
	}

	plcp[3] = plcp3;

	plcp[4] = 0;	/* number of extension spatial streams bit 0 & 1 */
	plcp[5] = 0;
}

/** Only called with a 'basic' or 'a/g' phy rate spec. length: PSDU length in octets */
static void
wlc_compute_ofdm_plcp(ratespec_t rspec, uint32 length, uint8 *plcp)
{
	uint8 rate_signal;
	uint32 tmp = 0;
	int rate;

	ASSERT(IS_OFDM(rspec));

	/* extract the 500Kbps rate for rate_info lookup */
	rate = (rspec & RSPEC_RATE_MASK);

	/* encode rate per 802.11a-1999 sec 17.3.4.1, with lsb transmitted first */
	rate_signal = rate_info[rate] & RATE_MASK;
	ASSERT(rate_signal != 0);

	bzero(plcp, D11_PHY_HDR_LEN);
	D11A_PHY_HDR_SRATE((ofdm_phy_hdr_t *)plcp, rate_signal);

	tmp = (length & 0xfff) << 5;
	plcp[2] |= (tmp >> 16) & 0xff;
	plcp[1] |= (tmp >> 8) & 0xff;
	plcp[0] |= tmp & 0xff;

	return;
}

/**
 * Compute PLCP, but only requires actual rate and length of pkt. Rate is given in the driver
 * standard multiple of 500 kbps. le is set for 11 Mbps rate if necessary. Broken out for PRQ.
 */
void
wlc_cck_plcp_set(int rate_500, uint length, uint8 *plcp)
{
	uint16 usec = 0;
	uint8 le = 0;

	switch (rate_500) {
	case 2:
		usec = length << 3;
		break;
	case 4:
		usec = length << 2;
		break;
	case 11:
		usec = (length << 4)/11;
		if ((length << 4) - (usec * 11) > 0)
			usec++;
		break;
	case 22:
		usec = (length << 3)/11;
		if ((length << 3) - (usec * 11) > 0) {
			usec++;
			if ((usec * 11) - (length << 3) >= 8)
				le = D11B_PLCP_SIGNAL_LE;
		}
		break;

	default:
		WL_ERROR(("%s: unsupported rate %d\n", __FUNCTION__, rate_500));
		ASSERT(!"invalid rate");
		break;
	}
	/* PLCP signal byte */
	plcp[0] = rate_500 * 5; /* r (500kbps) * 5 == r (100kbps) */
	/* PLCP service byte */
	plcp[1] = (uint8)(le | D11B_PLCP_SIGNAL_LOCKED);
	/* PLCP length uint16, little endian */
	plcp[2] = usec & 0xff;
	plcp[3] = (usec >> 8) & 0xff;
	/* PLCP CRC16 */
	plcp[4] = 0;
	plcp[5] = 0;
}

/** Rate: 802.11 rate code, length: PSDU length in octets */
static void
wlc_compute_cck_plcp(ratespec_t rspec, uint length, uint8 *plcp)
{
	int rate;

	ASSERT(IS_CCK(rspec));

	/* extract the 500Kbps rate for rate_info lookup */
	rate = (rspec & RSPEC_RATE_MASK);

	wlc_cck_plcp_set(rate, length, plcp);
}

#if defined(MBSS) || defined(WLTXMONITOR)

/** convert rate in OFDM PLCP to ratespec */
ratespec_t
ofdm_plcp_to_rspec(uint8 rate)
{
	ratespec_t rspec = 0;


	switch (rate & 0x0F) {
	case 0x0B: /* 6Mbps */
		rspec = 6*2;
		break;
	case 0x0F: /* 9Mbps */
		rspec = 9*2;
		break;
	case 0x0A: /* 12 Mbps */
		rspec = 12*2;
		break;
	case 0x0E: /* 18 Mbps */
		rspec = 18*2;
		break;
	case 0x09: /* 24 Mbps */
		rspec = 24*2;
		break;
	case 0x0D: /* 36 Mbps */
		rspec = 36*2;
		break;
	case 0x08: /* 48 Mbps */
		rspec = 48*2;
		break;
	case 0x0C: /* 54 Mbps */
		rspec = 54*2;
		break;
	default:
		/* Not a valid OFDM rate */
		ASSERT(0);
		break;
	}

	return rspec;
}
#endif /* MBSS || WLTXMONITOR */

/**
 * Calculate the 802.11 MAC header DUR field for MPDU
 * DUR for a single frame = 1 SIFS + 1 ACK
 * DUR for a frame with following frags = 3 SIFS + 2 ACK + next frag time
 *
 * rate			MPDU rate in unit of 500kbps
 * next_frag_len	next MPDU length in bytes
 * preamble_type	use short/GF or long/MM PLCP header
 */
uint16
wlc_compute_frame_dur(wlc_info_t *wlc, ratespec_t rate, uint8 preamble_type, uint next_frag_len)
{
	uint16 dur, sifs;

	sifs = SIFS(wlc->band);

	dur = sifs;
	dur += (uint16)wlc_calc_ack_time(wlc, rate, preamble_type);

	if (next_frag_len) {
		/* Double the current DUR to get 2 SIFS + 2 ACKs */
		dur *= 2;
		/* add another SIFS and the frag time */
		dur += sifs;
		dur += (uint16)wlc_calc_frame_time(wlc, rate, preamble_type, next_frag_len);
	}
	return (dur);
}

/**
 * Calculate the 802.11 MAC header DUR field for an RTS or CTS frame
 * DUR for normal RTS/CTS w/ frame = 3 SIFS + 1 CTS + next frame time + 1 ACK
 * DUR for CTS-TO-SELF w/ frame    = 2 SIFS         + next frame time + 1 ACK
 *
 * cts			cts-to-self or rts/cts
 * rts_rate		rts or cts rate in unit of 500kbps
 * rate			next MPDU rate in unit of 500kbps
 * frame_len		next MPDU frame length in bytes
 */
uint16
wlc_compute_rtscts_dur(wlc_info_t *wlc, bool cts_only, ratespec_t rts_rate, ratespec_t frame_rate,
                       uint8 rts_preamble_type, uint8 frame_preamble_type, uint frame_len, bool ba)
{
	uint16 dur, sifs;

	sifs = SIFS(wlc->band);

	if (!cts_only) {	/* RTS/CTS */
		dur = 3 * sifs;
		dur += (uint16)wlc_calc_cts_time(wlc, rts_rate, rts_preamble_type);
	} else {	/* CTS-TO-SELF */
		dur = 2 * sifs;
	}

	dur += (uint16)wlc_calc_frame_time(wlc, frame_rate, frame_preamble_type, frame_len);
#ifdef WL11N
	if (ba)
		dur += (uint16)wlc_calc_ba_time(wlc->hti, frame_rate, WLC_SHORT_PREAMBLE);
	else
#endif
		dur += (uint16)wlc_calc_ack_time(wlc, frame_rate, frame_preamble_type);
	return (dur);
}

/** transmit related */
static bool
wlc_phy_rspec_check(wlc_info_t *wlc, uint16 bw, ratespec_t rspec)
{
	if (RSPEC_ISVHT(rspec)) {
		uint8 mcs;
		uint Nss;
		uint Nsts;

		mcs = rspec & WL_RSPEC_VHT_MCS_MASK;
		Nss = (rspec & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;

		BCM_REFERENCE(mcs);
		ASSERT(mcs <= 9);

		/* VHT STBC expansion always is Nsts = Nss*2, so STBC expansion == Nss */
		if (RSPEC_ISSTBC(rspec)) {
			Nsts = 2*Nss;
		} else {
			Nsts = Nss;
		}

		/* we only support up to 3x3 */
		BCM_REFERENCE(Nsts);
		ASSERT(Nsts + RSPEC_TXEXP(rspec) <= 3);
	} else if (RSPEC_ISHT(rspec)) {
		uint mcs = rspec & RSPEC_RATE_MASK;

		ASSERT(mcs <= 32);

		if (mcs == 32) {
			ASSERT(RSPEC_ISSTBC(rspec) == FALSE);
			ASSERT(RSPEC_TXEXP(rspec) == 0);
			ASSERT(bw == PHY_TXC1_BW_40MHZ_DUP);
		} else {
			uint Nss = 1 + (mcs / 8);
			uint Nsts;

			/* BRCM HT chips only support STBC expansion by 2*Nss */
			if (RSPEC_ISSTBC(rspec)) {
				Nsts = 2*Nss;
			} else {
				Nsts = Nss;
			}

			/* we only support up to 3x3 */
			BCM_REFERENCE(Nsts);
			ASSERT(Nsts + RSPEC_TXEXP(rspec) <= 3);
		}
	} else if (IS_OFDM(rspec)) {
		ASSERT(RSPEC_ISSTBC(rspec) == FALSE);
	} else {
		ASSERT(IS_CCK(rspec));

		ASSERT((bw == PHY_TXC1_BW_20MHZ) || (bw == PHY_TXC1_BW_20MHZ_UP));
		ASSERT(RSPEC_ISSTBC(rspec) == FALSE);
#if defined(WL_BEAMFORMING)
		if (TXBF_ENAB(wlc->pub) && PHYCORENUM(wlc->stf->txstreams) > 1) {
			ASSERT(RSPEC_ISTXBF(rspec) == FALSE);
		}
#endif
	}

	return TRUE;
} /* wlc_phy_rspec_check */

uint16 wlc_phytxctl1_calc(wlc_info_t *wlc, ratespec_t rspec, chanspec_t chanspec)
{
	uint16 phyctl1 = 0;
	uint16 bw;
	wlcband_t *band = wlc->bandstate[CHSPEC_BANDUNIT(chanspec)];

	if (WLCISLPPHY(band) || WLCISLCNPHY(band)) {
		bw = PHY_TXC1_BW_20MHZ;
	} else {
		if (RSPEC_IS20MHZ(rspec)) {
			bw = PHY_TXC1_BW_20MHZ;
			if (CHSPEC_IS40(chanspec) &&
				CHSPEC_SB_UPPER(WLC_BAND_PI_RADIO_CHANSPEC)) {
				bw = PHY_TXC1_BW_20MHZ_UP;
			}
		} else  {
			ASSERT(RSPEC_IS40MHZ(rspec));

			bw = PHY_TXC1_BW_40MHZ;
			if (((rspec & RSPEC_RATE_MASK) == 32) || IS_OFDM(rspec)) {
				bw = PHY_TXC1_BW_40MHZ_DUP;
			}

		}
		wlc_phy_rspec_check(wlc, bw, rspec);
	}

	/* Handle HTPHY */
	if (WLCISHTPHY(band)) {
		uint16 spatial_map = wlc_stf_spatial_expansion_get(wlc, rspec);
		/* retrieve power offset on per packet basis */
		uint8 pwr = wlc_stf_get_pwrperrate(wlc, rspec, spatial_map);
		phyctl1 = (bw | (pwr << PHY_TXC1_HTTXPWR_OFFSET_SHIFT));
		/* spatial mapper table lookup on per packet basis */
		phyctl1 |= (spatial_map << PHY_TXC1_HTSPARTIAL_MAP_SHIFT);
		goto exit;
	}

	/* Handle the other mcs capable phys (NPHY, SSLPN, SSN, LP, LCN at this point) */
	if (RSPEC_ISHT(rspec)) {
		uint mcs = rspec & RSPEC_RATE_MASK;
		uint16 stf;

		/* Determine the PHY_TXC1_MODE value based on MCS and STBC or TX expansion.
		 * PHY_TXC1_MODE values cover 1 and 2 chains only and are:
		 * (Nss = 2, NTx = 2) 		=> SDM (Spatial Division Multiplexing)
		 * (Nss = 1, NTx = 2) 		=> CDD (Cyclic Delay Diversity)
		 * (Nss = 1, +1 STBC, NTx = 2) 	=> STBC (Space Time Block Coding)
		 * (Nss = 1, NTx = 1) 		=> SISO (Single In Single Out)
		 *
		 * MCS 0-7 || MCS == 32 	=> Nss = 1 (Number of spatial streams)
		 * MCS 8-15 			=> Nss = 2
		 */
		if (mcs > 7 && mcs <= 15) {
			/* must be SDM, only 2x2 devices */
			stf = PHY_TXC1_MODE_SDM;
		} else if (RSPEC_TXEXP(rspec)) {
			/*
			 * Tx Expansion: number of tx chains (NTx) beyond the minimum required for
			 * the space-time-streams, to increase robustness. Expansion is non-STBC.
			 */
			stf = PHY_TXC1_MODE_CDD;
		} else if (RSPEC_ISSTBC(rspec)) {
			/* expansion is STBC */
			stf = PHY_TXC1_MODE_STBC;
		} else {
			/* no expansion */
			stf = PHY_TXC1_MODE_SISO;
		}

		phyctl1 = bw | (stf << PHY_TXC1_MODE_SHIFT);

		/* set the upper byte of phyctl1 */
		phyctl1 |= (mcs_table[mcs].tx_phy_ctl3 << 8);
	} else if (IS_CCK(rspec) && !WLCISLPPHY(band) &&
		!WLCISSSLPNPHY(band) && !WLCISLCNPHY(band)) {
		/* In CCK mode LPPHY overloads OFDM Modulation bits with CCK Data Rate */

		phyctl1 = (bw | PHY_TXC1_MODE_SISO);
	} else {	/* legacy OFDM/CCK */
		int16 phycfg;
		uint16 stf;
		uint rate = (rspec & RSPEC_RATE_MASK);

		/* get the phyctl byte from rate phycfg table */
		if ((phycfg = wlc_rate_legacy_phyctl(rate)) == -1) {
			WL_ERROR(("%s: wrong legacy OFDM/CCK rate\n", __FUNCTION__));
			ASSERT(0);
			phycfg = 0;
		}

		if (RSPEC_TXEXP(rspec)) {
			/* CDD expansion for OFDM */
			stf = PHY_TXC1_MODE_CDD;
		} else {
			stf = PHY_TXC1_MODE_SISO;
		}

		/* set the upper byte of phyctl1 */
		phyctl1 = (bw | (phycfg << 8) | (stf << PHY_TXC1_MODE_SHIFT));
	}
exit:
#ifdef BCMDBG
	/* phy clock must support 40Mhz if tx descriptor uses it */
	if ((phyctl1 & PHY_TXC1_BW_MASK) >= PHY_TXC1_BW_40MHZ) {
		ASSERT(CHSPEC_IS40(wlc->chanspec));
#ifdef WLC_LOW
		ASSERT(wlc->chanspec == phy_utils_get_chanspec((phy_info_t *)WLC_PI(wlc)));
#endif
	}
#endif /* BCMDBG */
	return phyctl1;
}

ratespec_t
wlc_rspec_to_rts_rspec(wlc_bsscfg_t *cfg, ratespec_t rspec, bool use_rspec)
{
	wlc_info_t *wlc = cfg->wlc;
	bool g_prot = wlc->band->gmode && WLC_PROT_G_CFG_G(wlc->prot_g, cfg);
	return wlc_rspec_to_rts_rspec_ex(cfg->wlc, rspec, use_rspec, g_prot);
}

ratespec_t
wlc_rspec_to_rts_rspec_ex(wlc_info_t *wlc, ratespec_t rspec, bool use_rspec, bool g_prot)
{
	ratespec_t rts_rspec = 0;

	if (use_rspec) {
		/* use frame rate as rts rate */
		rts_rspec = rspec;

	} else if (g_prot && !IS_CCK(rspec)) {
		/* Use 11Mbps as the g protection RTS target rate and fallback.
		 * Use the WLC_BASIC_RATE() lookup to find the best basic rate under the
		 * target in case 11 Mbps is not Basic.
		 * 6 and 9 Mbps are not usually selected by rate selection, but even
		 * if the OFDM rate we are protecting is 6 or 9 Mbps, 11 is more robust.
		 */
		rts_rspec = WLC_BASIC_RATE(wlc, WLC_RATE_11M);
	} else {
		/* calculate RTS rate and fallback rate based on the frame rate
		 * RTS must be sent at a basic rate since it is a
		 * control frame, sec 9.6 of 802.11 spec
		 */
		rts_rspec = WLC_BASIC_RATE(wlc, rspec);
	}

#ifdef WL11N
	if (WLC_PHY_11N_CAP(wlc->band)) {
		/* set rts txbw to correct side band */
		rts_rspec &= ~RSPEC_BW_MASK;

		/* if rspec/rspec_fallback is 40MHz, then send RTS on both 20MHz channel
		 * (DUP), otherwise send RTS on control channel
		 */
		if (RSPEC_IS40MHZ(rspec) && !IS_CCK(rts_rspec))
			rts_rspec |= RSPEC_BW_40MHZ;
		else
			rts_rspec |= RSPEC_BW_20MHZ;

		/* pick siso/cdd as default for ofdm */
		if (IS_OFDM(rts_rspec)) {
			rts_rspec &= ~RSPEC_TXEXP_MASK;
			if (wlc->stf->ss_opmode == PHY_TXC1_MODE_CDD)
				rts_rspec |= (1 << RSPEC_TXEXP_SHIFT);
		}
	}
#else
	/* set rts txbw to control channel BW if 11 n feature is not turned on */
	rts_rspec &= ~RSPEC_BW_MASK;
	rts_rspec |= RSPEC_BW_20MHZ;
#endif /* WL11N */

	return rts_rspec;
}

/** called by e.g. 'NIC + offloads' functionality */
uint16
wlc_acphy_txctl0_calc_ex(wlc_info_t *wlc, ratespec_t rspec, uint8 preamble_type)
{
	return wlc_acphy_txctl0_calc(wlc, rspec, preamble_type);
}

static uint16
wlc_acphy_txctl0_calc(wlc_info_t *wlc, ratespec_t rspec, uint8 preamble_type)
{
	uint16 phyctl;
	uint16 bw;

	/* PhyTxControlWord_0 */
	phyctl = PHY_TXC_FRAMETYPE(rspec);
	if ((preamble_type == WLC_SHORT_PREAMBLE) ||
	    (preamble_type == WLC_GF_PREAMBLE)) {
		ASSERT((preamble_type == WLC_GF_PREAMBLE) || !RSPEC_ISHT(rspec));
		phyctl |= D11AC_PHY_TXC_SHORT_PREAMBLE;
		WLCNTINCR(wlc->pub->_cnt->txprshort);
	}

	phyctl |= wlc_stf_d11hdrs_phyctl_txant(wlc, rspec);

	/* bit 14/15 - 00/01/10/11 => 20/40/80/160 */
	switch (RSPEC_BW(rspec)) {
		case RSPEC_BW_20MHZ:
			bw = D11AC_PHY_TXC_BW_20MHZ;
		break;
		case RSPEC_BW_40MHZ:
			bw = D11AC_PHY_TXC_BW_40MHZ;
		break;
		case RSPEC_BW_80MHZ:
			bw = D11AC_PHY_TXC_BW_80MHZ;
		break;
		case RSPEC_BW_160MHZ:
			bw = D11AC_PHY_TXC_BW_160MHZ;
		break;
		default:
			ASSERT(0);
			bw = D11AC_PHY_TXC_BW_20MHZ;
		break;
	}
	phyctl |= bw;

	phyctl |= (D11AC_PHY_TXC_NON_SOUNDING);


	return phyctl;
}

#ifdef WL_LPC
#define PWR_SATURATE_POS 0x1F
#define PWR_SATURATE_NEG 0xE0
#endif

/** called by e.g. 'NIC + offloads' functionality */
uint16 wlc_acphy_txctl1_calc_ex(wlc_info_t *wlc, ratespec_t rspec, int8 lpc_offset, uint8 txpwr)
{
	return wlc_acphy_txctl1_calc(wlc, rspec, lpc_offset, txpwr);
}

static uint16
wlc_acphy_txctl1_calc(wlc_info_t *wlc, ratespec_t rspec, int8 lpc_offset, uint8 txpwr)
{
	uint16 phyctl = 0;
	chanspec_t cspec = wlc->chanspec;
	uint16 sb = ((cspec & WL_CHANSPEC_CTL_SB_MASK) >> WL_CHANSPEC_CTL_SB_SHIFT);

	/* Primary Subband Location: b 16-18
		LLL ==> 000
		LLU ==> 001
		...
		UUU ==> 111
	*/
	phyctl = sb >> ((RSPEC_BW(rspec) >> RSPEC_BW_SHIFT) - 1);

#ifdef WL_LPC
	/* Include the LPC offset also in the power offset */
	if (LPC_ENAB(wlc)) {
		int8 rate_offset = txpwr, tot_offset;

		/* Addition of two S4.1 words */
		rate_offset <<= 2;
		rate_offset >>= 2; /* Sign Extend */

		lpc_offset <<= 2;
		lpc_offset >>= 2; /* Sign Extend */

		tot_offset = rate_offset + lpc_offset;

		if (tot_offset < (int8)PWR_SATURATE_NEG)
			tot_offset = PWR_SATURATE_NEG;
		else if (tot_offset > PWR_SATURATE_POS)
			tot_offset = PWR_SATURATE_POS;

		txpwr = tot_offset;
	}
#endif /* WL_LPC */

	phyctl |= (txpwr << PHY_TXC1_HTTXPWR_OFFSET_SHIFT);

	return phyctl;
}

/** called by e.g. 'NIC + offloads' functionality */
uint16 wlc_acphy_txctl2_calc_ex(wlc_info_t *wlc, ratespec_t rspec)
{
	return wlc_acphy_txctl2_calc(wlc, rspec);
}

static uint16
wlc_acphy_txctl2_calc(wlc_info_t *wlc, ratespec_t rspec)
{
	uint16 phyctl = 0;
	uint nss = wlc_ratespec_nss(rspec);
	uint mcs;
	uint8 rate;
	const wlc_rateset_t* cur_rates = NULL;
	uint16 rindex;

	/* mcs+nss: bit 32 through 37 */
	if (RSPEC_ISHT(rspec)) {
		/* for 11n: B[32:37] for mcs[0:32] */
		mcs = rspec & RSPEC_RATE_MASK;
		phyctl = (uint16)mcs;
	} else if (RSPEC_ISVHT(rspec)) {
		/* for 11ac: B[32:35] for mcs[0-9], B[36:37] for n_ss[1-4]
			(0 = 1ss, 1 = 2ss, 2 = 3ss, 3 = 4ss)
		*/
		mcs = rspec & RSPEC_VHT_MCS_MASK;
		ASSERT(mcs <= 9);
		phyctl = (uint16)mcs;
		ASSERT(nss <= 8);
		phyctl |= ((nss-1) << D11AC_PHY_TXC_11AC_NSS_SHIFT);
	} else {
		rate = RSPEC2RATE(rspec);

		if (IS_OFDM(rspec)) {
			cur_rates = &ofdm_rates;
		} else {
			/* for 11b: B[32:33] represents phyrate
				(0 = 1mbps, 1 = 2mbps, 2 = 5.5mbps, 3 = 11mbps)
			*/
			cur_rates = &cck_rates;
		}
		for (rindex = 0; rindex < cur_rates->count; rindex++) {
			if ((cur_rates->rates[rindex] & RATE_MASK) == rate) {
				break;
			}
		}
		ASSERT(rindex < cur_rates->count);
		phyctl = rindex;
	}

	/* STBC */
	if (RSPEC_ISSTBC(rspec)) {
		/* set b38 */
		phyctl |= D11AC_PHY_TXC_STBC;
	}
	/* b39 - b 47 all 0 */

	return phyctl;
}

uint16
wlc_mgmt_ctl_d11hdrs(wlc_info_t *wlc, void *pkt, struct scb *scb,
	uint queue, ratespec_t rspec_override)
{
	return wlc_d11hdrs(wlc, pkt, scb, FALSE, 0, 0, TX_ATIM_FIFO,
	                   0, NULL, rspec_override, NULL);
}

void
wlc_tx_suspend(wlc_info_t *wlc)
{
	wlc->tx_suspended = TRUE;

	wlc_bmac_tx_fifo_suspend(wlc->hw, TX_DATA_FIFO);
}

bool
wlc_tx_suspended(wlc_info_t *wlc)
{
	return wlc_bmac_tx_fifo_suspended(wlc->hw, TX_DATA_FIFO);
}

void
wlc_tx_resume(wlc_info_t *wlc)
{
	bool silence = FALSE;
	int idx;
	wlc_bsscfg_t *cfg;

	wlc->tx_suspended = FALSE;

	FOREACH_AS_STA(wlc, idx, cfg) {
		if (BSS_QUIET_STATE(wlc->quiet, cfg) & SILENCE) {
			/* do not unsuspend when in quiet period */
			silence = TRUE;
			break;
		}
	}
	if (!silence && !wlc_quiet_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC)) {
		wlc_bmac_tx_fifo_resume(wlc->hw, TX_DATA_FIFO);
	}
}

#ifdef STA
static void
wlc_run_watchdog(wlc_info_t *wlc)
{
	uint32 cur, delta, wdtimer_delay;

	ASSERT(WLC_WATCHDOG_TBTT(wlc));
	wdtimer_delay = wlc_watchdog_backup_bi(wlc);

	if (wlc->WDarmed) {
		wl_del_timer(wlc->wl, wlc->wdtimer);
		wlc->WDarmed = FALSE;
	}

	cur = OSL_SYSUPTIME();
	delta = cur > wlc->WDlast ? cur - wlc->WDlast :
	        (uint32)~0 - wlc->WDlast + cur + 1;
	if (delta >= TIMER_INTERVAL_WATCHDOG) {
		if (wlc->watchdog_on_bcn) {
			/* Timeout on tbtt + 10ms delay before scheduling wd */
			wdtimer_delay = wlc_pretbtt_calc(wlc->cfg)/1000 + 10;
			wlc->wd_waitfor_bcn = TRUE;
		} else {
			wlc_watchdog((void *)wlc);
		}

		/* Possible to have missed multiple TIMER_INTERVAL_WATCHDOG? */
		do {
			wlc->WDlast += TIMER_INTERVAL_WATCHDOG;
			delta -= TIMER_INTERVAL_WATCHDOG;
		} while (delta > TIMER_INTERVAL_WATCHDOG);
	}

	wl_add_timer(wlc->wl, wlc->wdtimer, wdtimer_delay, TRUE);
	wlc->WDarmed = TRUE;
}
#endif /* STA */

void
wlc_bss_tbtt(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	(void)wlc;

	WL_APSTA_BCN(("wl%d: wlc_tbtt: wlc_bss_tbtt()\n", wlc->pub->unit));

#ifdef STA
	if (BSSCFG_STA(cfg) && cfg->BSS) {
		/* run watchdog here if the watchdog timer is not armed */
		if ((cfg == wlc->cfg) && WLC_WATCHDOG_TBTT(wlc))
			wlc_run_watchdog(wlc);

#ifdef BCMDBG
		if (cfg->associated)
			cfg->roam->tbtt_since_bcn++;
#endif
	}
#endif /* STA */

#ifdef AP
	if (BSSCFG_AP(cfg)) {
		wlc_vht_bss_tbtt(wlc->vhti, cfg);
		if (BSS_WL11H_ENAB(wlc, cfg) ||
		    FALSE) {
			wlc_11h_tbtt(wlc->m11h, cfg);
		}
#ifdef WLWNM_AP
		if (WLWNM_ENAB(wlc->pub))
			wlc_wnm_tbtt(wlc, cfg);
#endif /* WLWNM_AP */
	}
#endif /* AP */
}

/* MI_TBTT and MI_DTIM_TBTT interrupt handler.
 *
 * The interrupt is generated when tsf_cfpstart register is programmed and
 * tsf_timerlow = tsf_cfpstart + pre_tbtt.
 *
 * - In APSTA mode when the AP is brought up the tsf timers including tsf_cfpstart
 * register are allocated to the AP otherwise when there is no AP running they are
 * allocated to the primary STA (primary bsscfg).
 * - When P2P is enabled SHM TSFs are allocated for each STA BSS including the
 * primary STA (primary bsscfg).
 */
void
wlc_tbtt(wlc_info_t *wlc, d11regs_t *regs)
{
	WLCNTINCR(wlc->pub->_cnt->tbtt);

	WL_TRACE(("wl%d: wlc_tbtt: TBTT indication\n", wlc->pub->unit));

#ifdef DEBUG_TBTT
	if (WL_INFORM_ON()) {
		wlc->prev_TBTT = R_REG(wlc->osh, &regs->tsf_timerlow);
	}
#endif /* DEBUG_TBTT */

	if (AP_ACTIVE(wlc)) {
		wlc_ap_tbtt(wlc);
		return;
	}
	wlc_sta_tbtt(wlc);
}

static void
wlc_sta_tbtt(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (!BSSCFG_STA(cfg))
		return;

	WL_APSTA_BCN(("wl%d: wlc_tbtt: wlc_sta_tbtt()\n", wlc->pub->unit));
	WL_RTDC2(wlc, "wlc_tbtt: PMe=%u", cfg->pm->PMenabled, 0);


	if (!cfg->BSS) {
		/* DirFrmQ is now valid...defer setting until end of ATIM window */
		wlc->qvalid |= MCMD_DIRFRMQVAL;
	}

#ifdef WLMCNX
	/* p2p ucode has per BSS pretbtt */
	if (MCNX_ENAB(wlc->pub) && cfg->BSS)
		return;
#endif
	wlc_bss_tbtt(cfg);
}

static void
wlc_ap_tbtt(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

	ASSERT(AP_ACTIVE(wlc));

	WL_APSTA_BCN(("wl%d: wlc_tbtt: wlc_ap_tbtt()\n", wlc->pub->unit));

	/* increment scb tbtt counts, used for PS aging */
	wlc_apps_tbtt_update(wlc);


#ifdef WLMCNX
	/* p2p ucode has per BSS pretbtt */
	if (MCNX_ENAB(wlc->pub))
		return;
#endif
	FOREACH_UP_AP(wlc, idx, cfg) {
		wlc_bss_tbtt(cfg);
	}
}

/* This function needs to be called if the user of gptimer requires the device to
 * stay awake while gptimer is running.
 * An exception to this rule would be if the user of gptimer knows that the device
 * will always be awake when it is using gptimer.
 */
void
wlc_gptimer_wake_upd(wlc_info_t *wlc, mbool requester, bool set)
{
	mbool old_req = wlc->gptimer_stay_awake_req;

	if (set) {
		mboolset(wlc->gptimer_stay_awake_req, requester);
		/* old request was cleared, we just set a new request.
		 * need to update wake ctrl
		 */
		if (old_req == 0) {
			wlc_set_wake_ctrl(wlc);
		}
	}
	else {
		mboolclr(wlc->gptimer_stay_awake_req, requester);
		/* if old request was set and now it is not set,
		 * need to update wake ctrl
		 */
		if (old_req && (wlc->gptimer_stay_awake_req == 0)) {
			wlc_set_wake_ctrl(wlc);
		}
	}
}

/* This function needs to be called if the user requires the device to
 * stay awake.
 */
void
wlc_user_wake_upd(wlc_info_t *wlc, mbool requester, bool set)
{
	mbool old_req = wlc->user_wake_req;

	if (set) {
		mboolset(wlc->user_wake_req, requester);
		/* old request was cleared, we just set a new request.
		 * need to update wake ctrl
		 */
		if (old_req == 0) {
			wlc_set_wake_ctrl(wlc);
		}
	}
	else {
		mboolclr(wlc->user_wake_req, requester);
		/* if old request was set and now it is not set,
		 * need to update wake ctrl
		 */
		if (old_req && (wlc->user_wake_req == 0)) {
			wlc_set_wake_ctrl(wlc);
		}
	}
}

static void
wlc_nav_reset_war(wlc_info_t *wlc, bool disable)
{
	if (disable) {
		wlc->nav_reset_war_disable = TRUE;
		wlc_mhf(wlc, MHF2, MHF2_BTCANTMODE, MHF2_BTCANTMODE, WLC_BAND_ALL);
	} else {
		wlc->nav_reset_war_disable = FALSE;
		wlc_mhf(wlc, MHF2, MHF2_BTCANTMODE, 0, WLC_BAND_ALL);
	}
}

static void*
wlc_15420war(wlc_info_t *wlc, uint queue)
{
	hnddma_t *di;
	void *p;

	ASSERT(queue < NFIFO);

	if (PIO_ENAB(wlc->pub))
		return (NULL);

	if ((D11REV_IS(wlc->pub->corerev, 4)) || (D11REV_GT(wlc->pub->corerev, 6)))
		return (NULL);

	di = WLC_HW_DI(wlc, queue);
	ASSERT(di != NULL);

	/* get next packet, ignoring XmtStatus.Curr */
	p = dma_getnexttxp(di, HNDDMA_RANGE_ALL);

	/* sw block tx dma */
	dma_txblock(di);

	/* if tx ring is now empty, reset and re-init the tx dma channel */
	if (dma_txactive(di) == 0) {
		WLCNTINCR(wlc->pub->_cnt->txdmawar);
		if (!dma_txreset(di))
			WL_ERROR(("wl%d: %s: dma_txreset[%d]: cannot stop dma\n",
			          wlc->pub->unit, __FUNCTION__, queue));
		dma_txinit(di);
	}

	return (p);
}

void
wlc_war16165(wlc_info_t *wlc, bool tx)
{
	if (tx) {
		/* the post-increment is used in STAY_AWAKE macro */
		if (wlc->txpend16165war++ == 0)
			wlc_set_ps_ctrl(wlc->cfg);
	} else {
		wlc->txpend16165war--;
		if (wlc->txpend16165war == 0)
			wlc_set_ps_ctrl(wlc->cfg);
	}
}

/** Return driver capability string where the "capability" means *both* the sw and hw exist */
static char*
BCMRAMFN(wlc_cap)(wlc_info_t *wlc, char *buf, uint bufsize)
{
	struct bcmstrbuf b;
	ASSERT(bufsize >= WLC_IOCTL_SMLEN);

	bcm_binit(&b, buf, bufsize);

	wlc_cap_bcmstrbuf(wlc, &b);

	return (buf);
}

static void
BCMRAMFN(wlc_cap_bcmstrbuf)(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	/* pure software-only components */

/* Need AP defined here to print out capability only if enabled */
#ifdef AP
	bcm_bprintf(b, "ap ");
#endif

#ifdef STA
	bcm_bprintf(b, "sta ");
#endif /* STA */
#ifdef WET
	bcm_bprintf(b, "wet ");
#endif /* WET */
#ifdef WET_TUNNEL
	bcm_bprintf(b, "wet_tunnel ");
#endif /* WET_TUNNEL */
#ifdef MAC_SPOOF
	bcm_bprintf(b, "mac_spoof ");
#endif /* MAC_SPOOF */
#ifdef TOE
	bcm_bprintf(b, "toe ");
#endif /* TOE */
#ifdef WLLED
	bcm_bprintf(b, "led ");
#endif /* WLLED */
#ifdef WME
	bcm_bprintf(b, "wme ");
#endif /* WME */
#ifdef WLPIO
	bcm_bprintf(b, "pio ");
#endif /* PIO */
#ifdef WL11D
	bcm_bprintf(b, "802.11d ");
#endif /* WL11D */
#ifdef WL11H
	bcm_bprintf(b, "802.11h ");
#endif /* WL11H */
#ifdef WLRM
	bcm_bprintf(b, "rm ");
#endif /* WLRM */
#ifdef WLCQ
	bcm_bprintf(b, "cqa ");
#endif /* WLCQ */
#ifdef BCMRECLAIM
	bcm_bprintf(b, "reclm ");
#endif /* BCMRECLAIM */
#ifdef WLCAC
	bcm_bprintf(b, "cac ");
#endif /* WLCAC */
#ifdef MBSS
	if (MBSS_SUPPORT(wlc->pub)) {
		/* Special case for 4313 to only advertise mbss4 */
		if (D11REV_ISMBSS4(wlc->pub->corerev) || D11REV_IS(wlc->pub->corerev, 24))
			bcm_bprintf(b, "mbss4 ");
		else if (D11REV_ISMBSS16(wlc->pub->corerev) && wlc_bmac_ucodembss_hwcap(wlc->hw))
			bcm_bprintf(b, "mbss%d ", wlc->pub->tunables->maxucodebss);
	}
#endif /* MBSS */

	/* combo software+hardware components */

	if (NBANDS(wlc) > 1)
		bcm_bprintf(b, "dualband ");
#ifdef WLAMPDU
	if (wlc_ampdu_tx_cap(wlc->ampdu_tx) || wlc_ampdu_rx_cap(wlc->ampdu_rx))
		bcm_bprintf(b, "ampdu ");
	if (wlc_ampdu_tx_cap(wlc->ampdu_tx))
		bcm_bprintf(b, "ampdu_tx ");
	if (wlc_ampdu_rx_cap(wlc->ampdu_rx))
		bcm_bprintf(b, "ampdu_rx ");
#endif
#ifdef WLAMSDU
	if (wlc_amsdurx_cap(wlc->ami))
		bcm_bprintf(b, "amsdurx ");

	if (wlc_amsdutx_cap(wlc->ami))
		bcm_bprintf(b, "amsdutx ");
#endif
#if defined(WLTDLS)
	if (TDLS_SUPPORT(wlc->pub) && wlc_tdls_cap(wlc->tdls))
		bcm_bprintf(b, "tdls ");
#endif /* defined(WLTDLS) */
#ifdef WOWL
	if (wlc_wowl_cap(wlc))
		bcm_bprintf(b, "wowl ");
#endif
#ifdef WMF
	bcm_bprintf(b, "wmf ");
#endif
#ifdef RXCHAIN_PWRSAVE
	bcm_bprintf(b, "rxchain_pwrsave ");
#endif
#ifdef RADIO_PWRSAVE
	bcm_bprintf(b, "radio_pwrsave ");
#endif
#if defined(WLP2P) && !defined(WLP2P_DISABLED)
	if (wlc_p2p_cap(wlc->p2p))
		bcm_bprintf(b, "p2p ");
#endif
#ifdef BCM_DCS
	bcm_bprintf(b, "bcm_dcs ");
#endif
#if defined(SRHWVSDB)
	if (SRHWVSDB_ENAB(wlc->pub))
		bcm_bprintf(b, "srvsdb ");
#endif
#if defined(PROP_TXSTATUS)
	if (PROP_TXSTATUS_ENAB(wlc->pub))
		bcm_bprintf(b, "proptxstatus ");
#endif
#if defined(WLMCHAN)
	if (MCHAN_ENAB(wlc->pub))
		bcm_bprintf(b, "mchan ");
#if defined(WLSRVSDB)
	if (SRHWVSDB_ENAB(wlc->pub))
		bcm_bprintf(b, "srvsdb ");
#endif
#endif	/* WLMCHAN */
#ifdef PSTA
	bcm_bprintf(b, "psta psr ");
#endif /* PSTA */
#ifdef WDS
	bcm_bprintf(b, "wds ");
#endif
#ifdef DWDS
	bcm_bprintf(b, "dwds ");
#endif
#ifdef WLCSO
	if (wlc->toe_capable)
		bcm_bprintf(b, "cso ");
#endif
#ifdef WLTSO
	if (wlc->toe_capable)
		bcm_bprintf(b, "tso ");
#endif
#ifdef TRAFFIC_MGMT
	bcm_bprintf(b, "traffic-mgmt ");
	#ifdef TRAFFIC_MGMT_DWM
		bcm_bprintf(b, "traffic-mgmt-dwm ");
	#endif
#endif /* TRAFFIC_MGMT */
#ifdef P2PO
	bcm_bprintf(b, "p2po ");
#endif
#ifdef ANQPO
	bcm_bprintf(b, "anqpo ");
#endif
#ifdef WL11AC
	if (WLC_VHT_PROP_RATES_CAP_PHY(wlc))
		bcm_bprintf(b, "vht-prop-rates ");
#endif
#ifdef STA
	bcm_bprintf(b, "dfrts ");
#ifdef LPAS
	bcm_bprintf(b, "lpas ");
#endif /* LPAS */
#endif /* STA */
#ifdef WLTXPWR_CACHE
	bcm_bprintf(b, "txpwrcache ");
#endif
#if defined(WL11N) || defined(WL11AC)
	if (WLC_STBC_CAP_PHY(wlc)) {
		bcm_bprintf(b, "stbc-tx ");
		bcm_bprintf(b, "stbc-rx-1ss ");
	}
#endif
#ifdef PSPRETEND
	bcm_bprintf(b, "pspretend ");
#endif
#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub))
		bcm_bprintf(b, "rsdb ");
#endif
#ifdef WLPROBRESP_MAC_FILTER
	bcm_bprintf(b, "probresp_mac_filter ");
#endif
#if defined(MFP)
	bcm_bprintf(b, "mfp ");
#endif /* MFP */
#if defined(WLWNM)
	bcm_bprintf(b, "wnm ");
	bcm_bprintf(b, "bsstrans ");
#endif
}

#ifdef PROP_TXSTATUS
	/* Return TRUE if the packet should be wlc_suppressed */
bool
wlc_suppress_sync_fsm(wlc_info_t *wlc, struct scb* sc, void* p, bool suppress)
{
	uint8 rc = FALSE;
	bool	amsdu_pkt = FALSE;
	uint32 *pwlhinfo;
	int     n_wlhdr, i;
#ifdef DMATXRC
	void   *phdr = NULL;
#endif /* DMATXRC */

	if (!HOST_PROPTXSTATUS_ACTIVATED(wlc))
		return FALSE;

	if ((WLPKTTAG(p)->flags & WLF_PROPTX_PROCESSED))
		return FALSE;

	n_wlhdr = 0;
	i = 0;
#ifdef DMATXRC
	if (DMATXRC_ENAB(wlc->pub) && (WLPKTTAG(p)->flags & WLF_PHDR)) {
		txrc_ctxt_t *rctxt;
		phdr = p;
		rctxt = TXRC_PKTTAIL(wlc->osh, phdr);
		ASSERT(TXRC_ISMARKER(rctxt));
		BCM_REFERENCE(rctxt);
#ifdef PKTC_TX_DONGLE
		if (PKTC_ENAB(wlc->pub) && TXRC_ISRECLAIMED(rctxt) && TXRC_ISWLHDR(rctxt)) {
			/* plus one includes wl_hdr_information from phdr */
			n_wlhdr = rctxt->n + 1;
			ASSERT(n_wlhdr);
		}
#endif
	}
#endif /* DMATXRC */

	if (n_wlhdr == 0)
		amsdu_pkt = WLPKTFLAG_AMSDU(WLPKTTAG(p));

	do {
#if defined(DMATXRC) && defined(PKTC_TX_DONGLE)
		if (DMATXRC_ENAB(wlc->pub) && PKTC_ENAB(wlc->pub) && n_wlhdr) {
			/* First wl_hdr_information is from phdr */
			pwlhinfo = (i == 0) ? &(WLPKTTAG(p)->wl_hdr_information) :
				&(TXRC_PKTTAIL(wlc->osh, phdr)->wlhdr[i - 1]);
		} else
#endif
			pwlhinfo = &(WLPKTTAG(p)->wl_hdr_information);

		if (SCB_PROPTXTSTATUS_SUPPR_STATE(sc)) {
			/* "suppress" state */
			if (!suppress &&
				WL_TXSTATUS_GET_GENERATION(*pwlhinfo) ==
				SCB_PROPTXTSTATUS_SUPPR_GEN(sc)) {
				/* now the host is in sync, since the generation is a match */
				SCB_PROPTXTSTATUS_SUPPR_SETSTATE(sc, 0);
				sc->first_sup_pkt = 0xffff;
			}
			else if (!WLFC_GET_REORDERSUPP(wlfc_query_mode(wlc->wl))&&
				(sc->first_sup_pkt == (*pwlhinfo & 0xff)))
			{
				SCB_PROPTXTSTATUS_SUPPR_SETGEN(sc,
					(SCB_PROPTXTSTATUS_SUPPR_GEN(sc) ^ 1));
			} else if (WLFC_GET_REORDERSUPP(wlfc_query_mode(wlc->wl)) &&
				((WL_TXSTATUS_GET_FLAGS(*pwlhinfo) & WLFC_PKTFLAG_PKTFROMHOST) &&
				(WL_TXSTATUS_GET_GENERATION(*pwlhinfo) ==
				SCB_PROPTXTSTATUS_SUPPR_GEN(sc)))) {
				SCB_PROPTXTSTATUS_SUPPR_SETGEN(sc,
					(SCB_PROPTXTSTATUS_SUPPR_GEN(sc) ^ 1));
			}
			else {
				rc = TRUE;
			}
		}
		else {
			/* "accept" state */
			if (suppress && (WL_TXSTATUS_GET_FLAGS(*pwlhinfo) &
				WLFC_PKTFLAG_PKTFROMHOST)) {
				/* A suppress occurred while in "accept" state;
				* switch state, update gen # ONLY when packet is from HOST
				*/
				SCB_PROPTXTSTATUS_SUPPR_SETSTATE(sc, 1);
				sc->first_sup_pkt = (*pwlhinfo & 0xff);
				SCB_PROPTXTSTATUS_SUPPR_SETGEN(sc,
					(SCB_PROPTXTSTATUS_SUPPR_GEN(sc) ^ 1));
			}
		}
		WL_TXSTATUS_SET_GENERATION(*pwlhinfo, SCB_PROPTXTSTATUS_SUPPR_GEN(sc));

		if (n_wlhdr)
			i++; /* Next */
	}  while ((amsdu_pkt && ((p = PKTNEXT(wlc->osh, p)) != NULL)) || (i < n_wlhdr));
	return rc;
}

void
wlc_process_wlhdr_txstatus(wlc_info_t *wlc, uint8 status_flag, void* p, bool hold)
{
	/* space for type(1), length(1) and value */
	uint8	results[TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TXSTATUS + WLFC_CTL_VALUE_LEN_SEQ];
	uint32  statusdata;
	uint32	wlhinfo;
	bool	amsdu_pkt, pushdata;
#ifdef DMATXRC
	void    *phdr = NULL;
#endif /* DMATXRC */
#if defined(DMATXRC) && defined(PKTC_TX_DONGLE)
	int     n_wlhdr = 0;
	int     i = 0;
#endif /* DMATXRC && PKTC_TX_DONGLE */
	uint16	seq;

#if defined(DMATXRC)
	if (DMATXRC_ENAB(wlc->pub) && (WLPKTTAG(p)->flags & WLF_PHDR)) {
		txrc_ctxt_t *rctxt;

		phdr = p;
		rctxt = TXRC_PKTTAIL(wlc->osh, phdr);
		ASSERT(TXRC_ISMARKER(rctxt));

		BCM_REFERENCE(rctxt);

#ifdef PKTC_TX_DONGLE
		if (PKTC_ENAB(wlc->pub) && TXRC_ISRECLAIMED(rctxt) && TXRC_ISWLHDR(rctxt)) {
			n_wlhdr = rctxt->n;
			ASSERT(n_wlhdr);
		}
#endif
	}
#endif /* DMATXRC */

	amsdu_pkt = WLPKTFLAG_AMSDU(WLPKTTAG(p));

	do {
		wlhinfo = WLPKTTAG(p)->wl_hdr_information;
		/* temporarily not return original seqenuce number for amsdu packet */
		seq = amsdu_pkt ? 0 : WLPKTTAG(p)->seq;

		statusdata = 0;
		pushdata = TRUE;

		/* send txstatus only if this packet came from the host */
		if (WL_TXSTATUS_GET_FLAGS(wlhinfo) & WLFC_PKTFLAG_PKTFROMHOST) {
			if (!(WLPKTTAG(p)->flags & WLF_PROPTX_PROCESSED)) {
				WLPKTTAG(p)->flags |= WLF_PROPTX_PROCESSED;
				if (!(WL_TXSTATUS_GET_FLAGS(wlhinfo) &
					WLFC_PKTFLAG_PKT_REQUESTED)) {
					/* if this packet was intended for AC FIFO and ac credit
					   has not been sent back, push a credit here
					*/
					if (!(WLPKTTAG(p)->flags & WLF_CREDITED)) {
						wlfc_push_credit_data(wlc->wl, p);
					}
				}

				while (pushdata) {
					results[TLV_TAG_OFF] = WLFC_CTL_TYPE_TXSTATUS;
					results[TLV_LEN_OFF] = WLFC_CTL_VALUE_LEN_TXSTATUS;
					WL_TXSTATUS_SET_PKTID(statusdata,
						WL_TXSTATUS_GET_PKTID(wlhinfo));
					WL_TXSTATUS_SET_FIFO(statusdata,
						WL_TXSTATUS_GET_FIFO(wlhinfo));
					WL_TXSTATUS_SET_FLAGS(statusdata, status_flag);
					WL_TXSTATUS_SET_GENERATION(statusdata,
						WL_TXSTATUS_GET_GENERATION(wlhinfo));
					memcpy(&results[TLV_BODY_OFF], &statusdata, sizeof(uint32));

					if (WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
						if ((status_flag == WLFC_CTL_PKTFLAG_D11SUPPRESS) ||
							(status_flag ==
							WLFC_CTL_PKTFLAG_WLSUPPRESS)) {
							if (WL_SEQ_GET_FROMDRV(seq)) {
								WL_SEQ_SET_FROMFW(seq, 1);
								WL_SEQ_SET_FROMDRV(seq, 0);
							}
						} else {
							seq = 0;
						}
						memcpy(&results[TLV_HDR_LEN +
							WLFC_CTL_VALUE_LEN_TXSTATUS],
							&seq,
							WLFC_CTL_VALUE_LEN_SEQ);
						results[TLV_LEN_OFF] += WLFC_CTL_VALUE_LEN_SEQ;
					}
#ifdef BCMPCIEDEV_ENABLED
					wlfc_push_pkt_txstatus(wlc->wl, p, status_flag);
#else
					wlfc_push_signal_data(wlc->wl, results,
						results[TLV_LEN_OFF] + TLV_HDR_LEN, hold);
#endif /* BCMPCIEDEV */

#if defined(DMATXRC) && defined(PKTC_TX_DONGLE)
					/* If pkt chain was reclaimed, we loop through wlhdr[]
					 * (if any) and with phdr only, we end outside loop since
					 * there's no next pkt
					 */
					if (DMATXRC_ENAB(wlc->pub) &&
						PKTC_ENAB(wlc->pub) && (i < n_wlhdr)) {
						wlhinfo = TXRC_PKTTAIL(wlc->osh, phdr)->wlhdr[i];
						seq = amsdu_pkt ?
							0 : TXRC_PKTTAIL(wlc->osh, phdr)->seq[i];
						i++;
						ASSERT(wlhinfo);
					} else
#endif
						break;
				}
				/* Dont clear, we use hdrinfo  even after posting status
				 * The new WLF_PROPTX_PROCESSED flag prevents duplicate posting
				 * WLPKTTAG(p)->wl_hdr_information = 0;
				 */
				if (status_flag == WLFC_CTL_PKTFLAG_D11SUPPRESS) {
					wlfc_sendup_ctl_info_now(wlc->wl);
				}
				WLFC_COUNTER_TXSTATUS_COUNT(wlc);
			}
		}
		else {
			WLFC_COUNTER_TXSTATUS_OTHER(wlc);
		}
	} while (amsdu_pkt && ((p = PKTNEXT(wlc->osh, p)) != NULL));

	return;
}

uint8
wlc_wlfc_d11_txstatus_interpret(void *status)
{
tx_status_macinfo_t *txstatus = (tx_status_macinfo_t *)status;
	switch (txstatus->suppr_ind) {
		case TX_STATUS_SUPR_PMQ :
		case TX_STATUS_SUPR_BADCH :
			return WLFC_CTL_PKTFLAG_D11SUPPRESS;
		case TX_STATUS_SUPR_NONE:
		default :
			return WLFC_CTL_PKTFLAG_DISCARD;
	}
}
#endif /* PROP_TXSTATUS */

void
wlc_recover_pkts(wlc_info_t *wlc, uint queue)
{
	void *p = NULL;
	volatile uint32 intstatus =
	        R_REG(wlc->osh, &wlc->regs->intctrlregs[queue].intstatus);

	if (intstatus) {
		if (!(intstatus & I_XI))
			WL_ERROR(("%s: failure 0x%x\t", __FUNCTION__, intstatus));

		while ((p = GETNEXTTXP(wlc, queue))) {
			WLC_TXFIFO_COMPLETE(wlc, queue, 1, WLPKTTIME(p));
			PKTFREE(wlc->osh, p, TRUE);
		}
		W_REG(wlc->osh, &wlc->regs->intctrlregs[queue].intstatus, intstatus);
	}
	while ((p = dma_rx(WLC_HW_DI(wlc, RX_FIFO))))
		PKTFREE(wlc->osh, p, FALSE);
}

#if defined(STA) && defined(ADV_PS_POLL)
void
wlc_adv_pspoll_upd(wlc_info_t *wlc, struct scb *scb, void *pkt, bool sent, uint fifo)
{
	wlc_bsscfg_t *cfg;
	wlc_pm_st_t *pm;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	pm = cfg->pm;

	/* If this was the last frame sent, send a pspoll to check any buffered
	 * frames at AP, before going back to sleep
	 */
	if (!BSSCFG_STA(cfg) || !cfg->BSS || !pm->adv_ps_poll)
		return;

	ASSERT(WLPKTTAG(pkt)->flags & WLF_TXHDR);

	if (!sent &&
	    pm->PM == PM_MAX) {
		struct dot11_management_header *h = (struct dot11_management_header *)
		        ((uint8 *)((d11txh_t *)PKTDATA(wlc->osh, pkt) + 1) + D11_PHY_HDR_LEN);
		uint16 fc = ltoh16(h->fc);
		bool qos = FC_SUBTYPE_ANY_QOS(FC_SUBTYPE(fc));

		if (!qos ||
		    !(fifo <= TX_AC_VO_FIFO &&
		      AC_BITMAP_TST(scb->apsd.ac_trig, WME_PRIO2AC(PKTPRIO(pkt))))) {
			pm->send_pspoll_after_tx = TRUE;
		}
	}
	else if (sent &&
	         !pm->PSpoll &&
	         TXPKTPENDTOT(wlc) == 0) {
		if (pm->send_pspoll_after_tx) {
			/* Re-using existing implementation of pspoll_prd */
			_wlc_pspoll_timer(cfg);
			pm->send_pspoll_after_tx = FALSE;
		}
	}
}
#endif /* STA && ADV_PS_POLL */

/* return pkt pointer and offset from beginning of pkt */
int BCMFASTPATH
wlc_pkt_get_vht_hdr(wlc_info_t* wlc, void* p, d11actxh_t **hdrPtrPtr)
{
	uint8 *pktHdr = (uint8*)PKTDATA(wlc->osh, p);
	int tsoHdrSize = 0;

	ASSERT(pktHdr != NULL);
	ASSERT(p != NULL);
	ASSERT(hdrPtrPtr != NULL);
	ASSERT(D11REV_GE(wlc->pub->corerev, 40));

#ifdef WLTOEHW
	tsoHdrSize = (wlc->toe_bypass ?
		0 : wlc_tso_hdr_length((d11ac_tso_t*)pktHdr));
#endif
	ASSERT(PKTLEN(wlc->osh, p) >= D11AC_TXH_SHORT_LEN + tsoHdrSize);
	*hdrPtrPtr = (d11actxh_t*)(pktHdr + tsoHdrSize);
	return tsoHdrSize;
}

struct dot11_header*
wlc_pkt_get_d11_hdr(wlc_info_t* wlc, void *p)
{
	struct dot11_header* h = NULL;
	ASSERT(wlc);
	ASSERT(p);
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t *)PKTDATA(wlc->osh, p);
		ASSERT(nonVHTHdr != NULL);
		h = (struct dot11_header*)((uint8*)(nonVHTHdr + 1) + D11_PHY_HDR_LEN);
	} else {
		int offset = 0;
		uint8 *pktHdr = (uint8*)PKTDATA(wlc->osh, p);
		d11actxh_t	*acHdrPtr = NULL;
#ifdef WLTOEHW
		offset = (wlc->toe_bypass ? 0 : wlc_tso_hdr_length((d11ac_tso_t*)pktHdr));
#endif
		acHdrPtr = (d11actxh_t*)(pktHdr + offset);
		if (acHdrPtr->PktInfo.MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
			offset += D11AC_TXH_SHORT_LEN;
		} else {
			offset += D11AC_TXH_LEN;
		}

		ASSERT(PKTLEN(wlc->osh, p) >= (offset + sizeof(struct dot11_header)));
		h = (struct dot11_header*)(pktHdr + offset);
	}
	return h;
}

void
wlc_pkt_set_ack(wlc_info_t* wlc, void* p, bool want_ack)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t *)PKTDATA(wlc->osh, p);
		if (!want_ack) {
			nonVHTHdr->MacTxControlLow &= ~TXC_IMMEDACK;
		} else {
			nonVHTHdr->MacTxControlLow |= TXC_IMMEDACK;
		}
	} else {
		d11actxh_t* vhtHdr = NULL;
		wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
		if (!want_ack) {
			vhtHdr->PktInfo.MacTxControlLow &= ~D11AC_TXC_IACK;
		} else {
			vhtHdr->PktInfo.MacTxControlLow |= D11AC_TXC_IACK;
		}
	}
}

void
wlc_pkt_set_core(wlc_info_t* wlc, void* p, uint8 core)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		/* not implemented */
		ASSERT(0);
	} else {
		d11actxh_t* vhtHdr = NULL;
		wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
		/* send on core #ant antenna #ant */
		vhtHdr->RateInfo[0].PhyTxControlWord_0 &= ~D11AC_PHY_TXC_CORE_MASK;
		vhtHdr->RateInfo[0].PhyTxControlWord_0 |=
			(1 << (core + D11AC_PHY_TXC_CORE_SHIFT));
	}
}

void
wlc_pkt_set_txpwr_offset(wlc_info_t* wlc, void* p, uint16 pwr_offset)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		/* not implemented */
		ASSERT(0);
	} else {
		d11actxh_t* vhtHdr = NULL;
		wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
		vhtHdr->RateInfo[0].PhyTxControlWord_1 &= ~D11AC_PHY_TXC_TXPWR_OFFSET_MASK;
		vhtHdr->RateInfo[0].PhyTxControlWord_1 |=
			(pwr_offset << D11AC_PHY_TXC_TXPWR_OFFSET_SHIFT);
	}
}

static ratespec_t
wlc_vht_get_rspec(wlc_info_t* wlc, void* p, struct scb* scb)
{
	return (scb->rateset.rates[0] & RATE_MASK);
}

bool
wlc_vht_is_40MHz(wlc_info_t* wlc, void* p, struct scb* scb)
{
	ratespec_t rspec = wlc_vht_get_rspec(wlc, p, scb);
	return (RSPEC_BW(rspec) == RSPEC_BW_40MHZ);
}

bool
wlc_vht_is_SGI(wlc_info_t* wlc, void* p, struct scb* scb)
{
	ratespec_t rspec = wlc_vht_get_rspec(wlc, p, scb);
	return RSPEC_ISSGI(rspec);
}

uint8
wlc_vht_get_mcs(wlc_info_t* wlc, void* p, struct scb* scb)
{
	ratespec_t rspec;

	ASSERT(wlc);
	ASSERT(p);
	ASSERT(scb);
	rspec = wlc_vht_get_rspec(wlc, p, scb);

	return (RSPEC_ISSGI(rspec) & RSPEC_VHT_MCS_MASK);
}

static void
wlc_txs_clear_supr(wlc_info_t *wlc, tx_status_macinfo_t *status)
{
	ASSERT(status);
	status->suppr_ind = TX_STATUS_SUPR_NONE;
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		status->raw_bits &= ~TX_STATUS_SUPR_MASK;
	} else {
		status->raw_bits &= ~TX_STATUS40_SUPR;
	}
}

uint16
wlc_txs_alias_to_old_fmt(wlc_info_t *wlc, tx_status_macinfo_t* status)
{
	ASSERT(status);

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		return status->raw_bits;
	} else {
		uint16 status_bits = 0;

		if (status->is_intermediate) {
			status_bits |= TX_STATUS_INTERMEDIATE;
		}
		if (status->pm_indicated) {
			status_bits |= TX_STATUS_PMINDCTD;
		}
		status_bits |= (status->suppr_ind << TX_STATUS_SUPR_SHIFT);
		if (status->was_acked) {
			status_bits |= TX_STATUS_ACK_RCV;
		}
		/* frag tx cnt */
		status_bits |= (TX_STATUS_FRM_RTX_MASK &
			(status->frag_tx_cnt << TX_STATUS_FRM_RTX_SHIFT));
		return status_bits;
	}
}

static INLINE bool
wlc_txs_reg_mpdu(wlc_info_t *wlc, tx_status_t *txs)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		return ((txs->status.raw_bits & TX_STATUS_AMPDU) == 0);
	} else {
		/* valid indications are lacking for rev >= 40 */
		return FALSE;
	}
}

static void
wlc_txs_dump_status_info(wlc_info_t *wlc, tx_status_t *txs, uint queue)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		WL_ERROR(("wl%d raw txstatus %04X %04X %04X %04X\n",
			wlc->pub->unit, txs->status.raw_bits,
			txs->frameid, txs->sequence, txs->phyerr));
	} else {
		WL_ERROR(("wl%d raw txstatus %04X %04X | %04X %04X | %08X %08X || "
			"%08X %08X | %08X\n", wlc->pub->unit,
			txs->status.raw_bits, txs->frameid, txs->sequence, txs->phyerr,
			txs->status.s3, txs->status.s4, txs->status.s5,
			txs->status.ack_map1, txs->status.ack_map2));
	}
	WL_ERROR(("pktpend: %d %d %d %d %d ap: %d\n",
		TXPKTPENDGET(wlc, TX_AC_BK_FIFO),
		TXPKTPENDGET(wlc, TX_AC_BE_FIFO),
		TXPKTPENDGET(wlc, TX_AC_VI_FIFO),
		TXPKTPENDGET(wlc, TX_AC_VO_FIFO),
		TXPKTPENDGET(wlc, TX_BCMC_FIFO),
		AP_ENAB(wlc->pub)));
	WL_ERROR(("frameid 0x%x queue %d\n", txs->frameid, queue));
}

/** process an individual tx_status_t */
/* WLC_HIGH_API */
bool BCMFASTPATH
wlc_dotxstatus(wlc_info_t *wlc, tx_status_t *txs, uint32 frm_tx2)
{
	void *p;
	uint queue;
	int err = 0;
	wlc_txh_info_t txh_info;
	struct scb *scb = NULL;
	bool update_rate, free_pdu;
	osl_t *osh;
	int tx_rts, tx_frame_count, tx_rts_count;
	uint totlen, supr_status;
	bool lastframe;
	struct dot11_header *h;
	uint16 fc;
	wlc_pkttag_t *pkttag;
	wlc_bsscfg_t *bsscfg = NULL;
	bool pkt_sent = FALSE;
	bool pkt_max_retries = FALSE;
#if defined(PSPRETEND) || defined(WLPKTDLYSTAT)
	bool ack_recd = FALSE;
#endif
#ifdef WLPKTDLYSTAT
	uint32 delay, now;
	scb_delay_stats_t *delay_stats;
#endif
#ifdef PKTQ_LOG
	uint32 prec_pktlen;
	pktq_counters_t *actq_cnt = 0;
#endif
#if defined(WLPKTDLYSTAT) || defined(WL_LPC)
	uint8 ac;
#endif /* WLPKTDLYSTAT || WL_LPC */
	bool from_host = TRUE;
	bool supr_pkt = FALSE;

	BCM_REFERENCE(from_host);
	BCM_REFERENCE(frm_tx2);

	if (WL_PRHDRS_ON()) {
		wlc_print_txstatus(wlc, txs);
	}


	/* discard intermediate indications for ucode with one legitimate case:
	 * e.g. if "useRTS" is set. ucode did a successful rts/cts exchange, but the subsequent
	 * tx of DATA failed. so it will start rts/cts from the beginning (resetting the rts
	 * transmission count)
	 */
	if ((txs->status.is_intermediate) &&
		wlc_txs_reg_mpdu(wlc, txs)) {
		WLCNTADD(wlc->pub->_cnt->txnoack, (txs->status.frag_tx_cnt));
		WL_TRACE(("%s: bail: txs status no ack\n", __FUNCTION__));
		return FALSE;
	}

#ifdef WLOFFLD
	if (((txs->frameid & TXFID_QUEUE_MASK) == TX_ATIM_FIFO) &&
		wlc_ol_arp_nd_enabled(wlc->ol)) {
			WL_ERROR(("frame id 0x%x ATIM_FIFO status!!!!!\n", txs->frameid));
			return FALSE;
	}
#endif
	osh = wlc->osh;
	queue = txs->frameid & TXFID_QUEUE_MASK;
	ASSERT(queue < NFIFO);
#ifdef WLC_HIGH_ONLY
	if (queue >= NFIFO) {
		p = NULL;
		WL_ERROR(("%s: bail: txs status FID->q invalid\n", __FUNCTION__));
		goto fatal;
	}
#else
	if (queue >= NFIFO || WLC_HW_DI(wlc, queue) == NULL) {
		p = NULL;
		WL_ERROR(("%s: bail: txs status FID->q %u invalid\n", __FUNCTION__, queue));
		ASSERT(p);
		goto fatal;
	}
#endif

	supr_status = txs->status.suppr_ind;

#ifdef WL_MULTIQUEUE
	/* Flushed packets have been re-enqueued, just toss the status */
	if (supr_status == TX_STATUS_SUPR_FLUSH) {
		WL_MQ(("MQ: %s: Flush status for txs->frameid 0x%x\n", __FUNCTION__,
		       txs->frameid));
		return FALSE;
	}
#endif /* WL_MULTIQUEUE */

	p = GETNEXTTXP(wlc, queue);
	WL_TRACE(("%s: pkt fr queue=%d p=%p\n", __FUNCTION__, queue, p));

	if (WLC_WAR16165(wlc))
		wlc_war16165(wlc, FALSE);

	if (p == NULL)
		p = wlc_15420war(wlc, queue);

#ifdef WL_MULTIQUEUE
	if (p == NULL)
		WL_MQ(("MQ: %s: txs->frameid 0x%x\n", __FUNCTION__, txs->frameid));
	else if (wlc->excursion_active || wlc->txfifo_detach_pending)
		WL_MQ(("MQ: %s: txs->frameid 0x%x\n", __FUNCTION__, txs->frameid));
#endif /* WL_MULTIQUEUE */

	if (p == NULL) {
		WL_ERROR(("%s: null ptr2\n", __FUNCTION__));
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			wlc_txs_dump_status_info(wlc, txs, queue);
		}
	}

	ASSERT(p != NULL);

	PKTDBG_TRACE(osh, p, PKTLIST_MI_TFS_RCVD);

	if (p == NULL) {
		WL_ERROR(("%s: bail: p == NULL\n", __FUNCTION__));
		goto fatal;
	}

	wlc_get_txh_info(wlc, p, &txh_info);


	if (AMPDU_HOST_ENAB(wlc->pub)) {
		if (txs->frameid != htol16(txh_info.TxFrameID)) {
			WL_ERROR(("wl%d: %s: txs->frameid 0x%x txh->TxFrameID 0x%x\n",
			          wlc->pub->unit, __FUNCTION__,
			          txs->frameid, htol16(txh_info.TxFrameID)));
			ASSERT(txs->frameid == htol16(txh_info.TxFrameID));
			goto fatal;
		}
	}

	pkttag = WLPKTTAG(p);

#if defined(WLPKTDLYSTAT) || defined(WL_LPC)
	ac = WME_PRIO2AC(PKTPRIO(p));
#endif

#ifdef PKTQ_LOG
	if (wlc->active_queue->q.pktqlog) {
		actq_cnt = wlc->active_queue->q.pktqlog->_prec_cnt[WLC_PRIO_TO_PREC(PKTPRIO(p))];
	}
#endif

#ifdef MACOSX
	/* If the packet tag pointer is null, we will crash shortly.
	 * Dump a panic message for more info in panic logs
	 */
	if (pkttag == NULL) {
		mbuf_t m = (mbuf_t)p;

		if (mbuf_type(m) == MBUF_TYPE_FREE) {
			osl_panic("wlc_dotxstatus: no pkttag, mbuf %p is a free mbuf status:0x%x\n",
				m, txs->status.raw_bits);
		} else if (!(mbuf_flags(m) & MBUF_PKTHDR)) {
			osl_panic("wlc_dotxstatus: no pkttag, mbuf %p is not a header mbuf "
				"status:0x%x\n", m, txs->status.raw_bits);
		} else {
			osl_panic("wlc_dotxstatus: mbuf %p does not have a wl pkttag status:0x%x\n",
				m, txs->status.raw_bits);
		}
	}
#endif /* MACOSX */

	h = (struct dot11_header *)(txh_info.d11HdrPtr);
	fc = ltoh16(h->fc);

#ifdef MBSS
	if (MBSS_ENAB(wlc->pub) && (queue == TX_ATIM_FIFO)) {
		/* Under MBSS, all ATIM packets are handled by mbss_dotxstatus */
		wlc_mbss_dotxstatus(wlc, txs, p, fc, pkttag, supr_status);
		WL_TRACE(("%s: bail after mbss dotxstatus\n", __FUNCTION__));
		return FALSE;
	}
#endif /* MBSS */

	/*  use the bsscfg index in the packet tag to find out which
	 *  bsscfg this packet belongs to
	 */
	bsscfg = wlc_bsscfg_find(wlc, WLPKTTAGBSSCFGGET(p), &err);

	if (ETHER_ISMULTI(txh_info.TxFrameRA)) {
		if (bsscfg != NULL) {
#ifdef MBSS
			if (MBSS_ENAB(wlc->pub) && (queue == TX_BCMC_FIFO))
				wlc_mbss_dotxstatus_mcmx(wlc, bsscfg, txs);
#endif /* MBSS */
			/* With pkt suppression, need to assign SCB for any frame */
			scb = WLC_BCMCSCB_GET(wlc, bsscfg);
			ASSERT(scb != NULL);
		}
	} else {
		/* For ucast frames, lookup the scb directly by the RA.
		 * The scb may not be found if we sent a frame with no scb, or if the
		 * scb was deleted while the frame was queued.
		 */
		if (bsscfg != NULL) {
			scb = wlc_scbfindband(wlc, bsscfg,
			                      (struct ether_addr*)txh_info.TxFrameRA,
			                      (wlc_txh_info_is5GHz(wlc, &txh_info)
			                       ?
			                       BAND_5G_INDEX : BAND_2G_INDEX));
		}

#ifdef PKTQ_LOG
		if (D11REV_GE(wlc->pub->corerev, 40)) {
#if defined(SCB_BS_DATA)
			wlc_bs_data_counters_t *bs_data_counters = NULL;
#endif /* SCB_BS_DATA */
			SCB_BS_DATA_CONDFIND(bs_data_counters, wlc, scb);

			WLCNTCONDADD(actq_cnt, actq_cnt->airtime, TX_STATUS40_TX_MEDIUM_DELAY(txs));
			SCB_BS_DATA_CONDADD(bs_data_counters, airtime,
				TX_STATUS40_TX_MEDIUM_DELAY(txs));
		}
#endif /* PKTQ_LOG */

		if (scb != NULL) {
			bsscfg = SCB_BSSCFG(scb);
			ASSERT(bsscfg != NULL);
		}
#ifdef STA
		else {
			/* no scb, but we may be able to look up a bsscfg by the BSSID if this
			 * was a frame sent by an infra STA to its AP
			 */
			bsscfg = wlc_bsscfg_find_by_bssid(wlc,
				(struct ether_addr *)txh_info.TxFrameRA);
		}
#endif /* STA */

#if defined(STA) && defined(WLOFFLD) && defined(UCODE_SEQ)
		if  ((D11REV_GE(wlc->pub->corerev, 42))	&&
			((txs->frameid & TXFID_QUEUE_MASK) == TX_AC_BK_FIFO) &&
			(supr_status == TX_STATUS_SUPR_PHASE1_KEY) && scb) {
			wlc_ol_update_seq_iv(wlc, TRUE, scb);
		}
#endif /* STA && WLOFFLD && UCODE_SEQ */
	}

#ifdef PROP_TXSTATUS
	from_host = WL_TXSTATUS_GET_FLAGS(pkttag->wl_hdr_information) & WLFC_PKTFLAG_PKTFROMHOST;
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		if (!(scb && SCB_ISMULTI(scb))) {
		if (scb && (WLFC_D11_STATUS_INTERPRET(txs) == WLFC_CTL_PKTFLAG_D11SUPPRESS)) {
			wlc_suppress_sync_fsm(wlc, scb, p, TRUE);
			wlc_process_wlhdr_txstatus(wlc, WLFC_CTL_PKTFLAG_WLSUPPRESS, p, FALSE);
#ifdef WLAMPDU
			if (WLPKTFLAG_AMPDU(pkttag))
				wlc_ampdu_ini_adjust(wlc->ampdu_tx, scb, p);
#endif

#ifdef WL_BSSCFG_TX_SUPR
			if (P2P_ABS_SUPR(wlc, supr_status))  {
				/* We have ABS_NACK for GO/GC which is _NOT_IN_ABS yet
				 * flush pkts and stop tx
				 */
				if (bsscfg && !BSS_TX_SUPR(bsscfg)) {
					if (wlc_p2p_noa_valid(wlc->p2p, bsscfg) ||
					    wlc_p2p_ops_valid(wlc->p2p, bsscfg))
						wlc_wlfc_interface_state_update(wlc,
						SCB_BSSCFG(scb), WLFC_CTL_TYPE_INTERFACE_CLOSE);
					wlc_wlfc_flush_pkts_to_host(wlc, SCB_BSSCFG(scb));
				}
			}
#endif /* WL_BSSCFG_TX_SUPR */
			if (supr_status == TX_STATUS_SUPR_PMQ) {
				/* We have PMQ Suppress from a SCB that is not in PowerSave yet
				 * Suppress TxFIFO first.
				 * MI_PMQ gets delayed sometimes, so we handle PS inline here
				 */
				if (!SCB_PS(scb)) {
					wlc_apps_process_ps_switch(wlc, &scb->ea, TRUE);
				}
			}
		} else
			wlc_process_wlhdr_txstatus(wlc, WLFC_D11_STATUS_INTERPRET(txs), p, FALSE);
		}
	}
#endif /* PROP_TXSTATUS */

	WLPKTTAGSCBSET(p, scb);

#ifdef WLCNT
	if (N_ENAB(wlc->pub)) {
		if (wlc_txh_get_isSGI(wlc, &txh_info))
			WLCNTINCR(wlc->pub->_cnt->txmpdu_sgi);
		if (wlc_txh_get_isSTBC(wlc, &txh_info))
			WLCNTINCR(wlc->pub->_cnt->txmpdu_stbc);
	}
#endif

#if defined(WLNAR)
	wlc_nar_dotxstatus(wlc->nar_handle, scb, p, txs);
#endif


#ifdef WL_BEAMFORMING
	if (scb && TXBF_ENAB(wlc->pub)) {
		if (txs->status.s5 & TX_STATUS40_IMPBF_MASK)
			wlc_txbf_imp_txstatus(wlc->txbf, scb, txs);
	}
#endif

#ifdef WLAMPDU
	if (WLPKTFLAG_AMPDU(pkttag)) {
		if (!wlc_txh_get_isAMPDU(wlc, &txh_info)) {
			WL_TRACE(("%s: regmpdu status\n", __FUNCTION__));

			wlc_ampdu_dotxstatus_regmpdu(wlc->ampdu_tx, scb, p, txs);
			/* continue with rest of regular mpdu processing */

#ifdef WL_LPC
			if (LPC_ENAB(wlc) && scb) {
				/* Update the LPC database */
				wlc_scb_lpc_update_pwr(wlc->wlpci, scb, txs->frameid,
					txh_info.PhyTxControlWord0, txh_info.PhyTxControlWord1);
				/* Reset the LPC vals in ratesel */
				wlc_scb_ratesel_reset_vals(wlc->wrsi, scb, ac);
			}
#endif /* WL_LPC */
		} else {
			bool ret_val;
#ifdef STA
			if (scb != NULL)
				wlc_adv_pspoll_upd(wlc, scb, p, TRUE, queue);
#endif /* STA */
			ret_val = wlc_ampdu_dotxstatus(wlc->ampdu_tx, scb, p, txs, &txh_info);
#ifdef WL_LPC
			if (LPC_ENAB(wlc) && scb) {
				/* Update the LPC database */
				wlc_scb_lpc_update_pwr(wlc->wlpci, scb, txs->frameid,
					txh_info.PhyTxControlWord0, txh_info.PhyTxControlWord1);
				/* Reset the LPC vals in ratesel */
				wlc_scb_ratesel_reset_vals(wlc->wrsi, scb, ac);
			}
#endif /* WL_LPC */
#ifdef STA
			if (bsscfg != NULL && BSSCFG_STA(bsscfg))
				wlc_pm2_ret_upd_last_wake_time(bsscfg, &txs->lasttxtime);
#endif /* STA */
			return ret_val;
		}
	}
#endif /* WLAMPDU */
	WL_TRACE(("%s: after if pkttag AMPDU\n", __FUNCTION__));

	tx_rts = wlc_txh_has_rts(wlc, &txh_info);
	tx_frame_count = txs->status.frag_tx_cnt;
	tx_rts_count = txs->status.rts_tx_cnt;

	/* count all retransmits */
	if (tx_frame_count) {
		WLCNTADD(wlc->pub->_cnt->txretrans, (tx_frame_count - 1));
		WLCNTSCB_COND_ADD(scb && from_host, scb->scb_stats.tx_pkts_retries,
			(tx_frame_count - 1));
		WLCNTSCB_COND_ADD(scb && !from_host, scb->scb_stats.tx_pkts_fw_retries,
			(tx_frame_count - 1));
	}
	if (tx_rts_count)
		WLCNTADD(wlc->pub->_cnt->txretrans, (tx_rts_count - 1));

#ifdef PKTQ_LOG
	WLCNTCONDADD(actq_cnt && tx_rts_count, actq_cnt->rtsfail, tx_rts_count - 1);
#endif

	lastframe = (fc & FC_MOREFRAG) == 0;

	totlen = pkttotlen(osh, p);
	BCM_REFERENCE(totlen);

	/* plan for success */
	update_rate = (pkttag->flags & WLF_RATE_AUTO) ? TRUE : FALSE;
	free_pdu = TRUE;

#ifdef PSPRETEND
	/* Whether we could be doing ps pretend in the case the
	 * tx status is NACK. PS pretend is not operating on multicast scb.
	 * "Conventional" PS mode (SCB_PS_PRETEND_NORMALPS) is excluded to keep the two
	 * mechanisms separated.
	 * This code handles tx status not processed by AMPDU. Normal PS pretend
	 * is (today) only supported on AMPDU traffic, but THRESHOLD PS pretend can be
	 * used for non AMPDU traffic, so this is checked.
	 * Finally, the flag for WLF_PSDONTQ is checked because this is used for probe
	 * packets. We should not operate ps pretend in this case.
	 */
	if (!txs->status.was_acked && scb && !SCB_ISMULTI(scb) &&
		SCB_PS_PRETEND_THRESHOLD(scb) && !(pkttag->flags & WLF_PSDONTQ)) {
		wlc_apps_scb_pspretend_on(wlc, scb, 0);
		supr_status |= TX_STATUS_SUPR_PMQ;
	}
#endif /* PSPRETEND */

	/* process txstatus info */
	if (txs->status.was_acked) {
		/* success... */
#if defined(PSPRETEND) || defined(WLPKTDLYSTAT)
		ack_recd = TRUE;
#endif
#if defined(WLAIBSS)
		if (AIBSS_ENAB(wlc->pub) && scb && bsscfg && !SCB_ISMULTI(scb) &&
			BSSCFG_IBSS(bsscfg)) {
			aibss_tx_monitor_t	*aibss_scb_tx_stats =
				SCB_CUBBY((scb), WLC_AIBSS_INFO_SCB_HDL(wlc->aibss_info));
				/* reset no-ack counters */
			aibss_scb_tx_stats->tx_noack_count = 0;
		}
#endif /* WLAIBSS */
		/* update counters */
		WLCNTINCR(wlc->pub->_cnt->txfrag);
		WLCNTADD(wlc->pub->_cnt->txnoack, (tx_frame_count - 1));
		WLCIFCNTINCR(scb, txfrag);
#ifdef PKTQ_LOG
		WLCNTCONDINCR(actq_cnt, actq_cnt->acked);
		WLCNTCONDADD(actq_cnt, actq_cnt->retry, (tx_frame_count - 1));
		prec_pktlen = pkttotlen(wlc->osh, p) - sizeof(d11txh_t) - DOT11_LLC_SNAP_HDR_LEN -
		              DOT11_MAC_HDR_LEN - DOT11_QOS_LEN;
		WLCNTCONDADD(actq_cnt, actq_cnt->throughput, prec_pktlen);
#endif
		if (tx_frame_count > 1) {
			/* counting number of retries for all data frames. */
			wlc->txretried += tx_frame_count - 1;
			/* counting the number of frames where a retry was necessary
			 * per each station.
			 */
			WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_pkts_retried);
		}
		if (lastframe) {
			WLCNTINCR(wlc->pub->_cnt->txfrmsnt);
			WLCIFCNTINCR(scb, txfrmsnt);
			if (wlc->txretried >= 1) {
				WLCNTINCR(wlc->pub->_cnt->txretry);
				WLCIFCNTINCR(scb, txretry);
			}
			if (wlc->txretried > 1) {
				WLCNTINCR(wlc->pub->_cnt->txretrie);
				WLCIFCNTINCR(scb, txretrie);
			}

			WLCNTSCB_COND_INCR(scb && from_host, scb->scb_stats.tx_pkts_total);
			WLCNTSCB_COND_INCR(scb && !from_host, scb->scb_stats.tx_pkts_fw_total);
		}
		if (scb) {
			scb->used = wlc->pub->now;
#if defined(STA) && defined(DBG_BCN_LOSS)
			scb->dbg_bcn.last_tx = wlc->pub->now;
#endif
		}

		pkt_sent = TRUE;

	} else if ((!txs->status.was_acked) &&
	           (WLPKTFLAG_AMSDU(pkttag) || WLPKTFLAG_RIFS(pkttag))) {
		update_rate = FALSE;

		/* update counters */
		WLCNTINCR(wlc->pub->_cnt->txfrag);
		WLCIFCNTINCR(scb, txfrag);
		if (lastframe) {
			WLCNTINCR(wlc->pub->_cnt->txfrmsnt);
			WLCIFCNTINCR(scb, txfrmsnt);

			WLCNTSCB_COND_INCR(scb && from_host, scb->scb_stats.tx_pkts_total);
			WLCNTSCB_COND_INCR(scb && !from_host, scb->scb_stats.tx_pkts_fw_total);
		}

		/* should not update scb->used for ap otherwise the ap probe will not work */
		if (scb && bsscfg && BSSCFG_STA(bsscfg)) {
			scb->used = wlc->pub->now;
#if defined(STA) && defined(DBG_BCN_LOSS)
			scb->dbg_bcn.last_tx = wlc->pub->now;
#endif
		}

		pkt_sent = TRUE;

	} else if (supr_status == TX_STATUS_SUPR_FRAG) {
		/* subsequent frag failure... */
		update_rate = FALSE;
	} else if (supr_status == TX_STATUS_SUPR_BADCH) {
		update_rate = FALSE;
		WLCNTINCR(wlc->pub->_cnt->txchanrej);
	} else if (supr_status == TX_STATUS_SUPR_EXPTIME) {
		WLCNTINCR(wlc->pub->_wme_cnt->tx_expired[(WME_ENAB(wlc->pub) ?
			WME_PRIO2AC(PKTPRIO(p)) : AC_BE)].packets);
		WLCNTINCR(wlc->pub->_cnt->txexptime);	/* generic lifetime expiration */

		/* Interference detected */
		if (wlc->rfaware_lifetime)
			wlc_exptime_start(wlc);
	} else if (supr_status == TX_STATUS_SUPR_PMQ) {
		update_rate = FALSE;

		if ((bsscfg != NULL && BSSCFG_AP(bsscfg)) || BSS_TDLS_ENAB(wlc, bsscfg)) {
#if defined(BCMDBG) || defined(WLMSG_PS)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */


			if (scb != NULL &&
			    (SCB_ISMULTI(scb) || SCB_ASSOCIATED(scb) ||
			     BSS_TDLS_ENAB(wlc, bsscfg))) {
				free_pdu = wlc_apps_suppr_frame_enq(wlc, p, txs, lastframe);
				supr_pkt = TRUE;
			}

			WL_PS(("wl%d: %s: suppress for PM frame 0x%x for %s %s\n",
			       wlc->pub->unit, __FUNCTION__, txs->frameid,
			       bcm_ether_ntoa((struct ether_addr *)txh_info.TxFrameRA, eabuf),
			       free_pdu?"tossed":"requeued"));
		}

		/* If the packet is not queued for later delivery, it will be freed
		 * and any packet callback needs to consider this as a un-ACKed packet.
		 * Change the txstatus to simple no-ack instead of suppressed.
		 */
		if (free_pdu)
			wlc_txs_clear_supr(wlc, &(txs->status));
#if defined(PSPRETEND)
	} else if (supr_status == TX_STATUS_SUPR_PPS) {
		update_rate = FALSE;
		free_pdu = TRUE;

		if (SCB_PS_PRETEND(scb) && !SCB_ISMULTI(scb)) {
			free_pdu = wlc_apps_suppr_frame_enq(wlc, p, txs, lastframe);
			supr_pkt = TRUE;
		}

		WL_PS(("wl%d.%d: %s: ps pretend %s suppress for PM frame 0x%x for "MACF" %s\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
			SCB_PS_PRETEND(scb) ? "ON":"OFF", txs->frameid,
			ETHERP_TO_MACF(txh_info.TxFrameRA), free_pdu?"tossed":"requeued"));

		/* If the packet is not queued for later delivery, it will be freed
		 * and any packet callback needs to consider this as a un-ACKed packet.
		 * Change the txstatus to simple no-ack instead of suppressed.
		 */
		if (free_pdu) {
			wlc_txs_clear_supr(wlc, &(txs->status));
		}
#endif /* PSPRETEND */
	} else if (supr_status == TX_STATUS_SUPR_FLUSH) {
		update_rate = FALSE;
		free_pdu = FALSE;

		ASSERT(0);
	} else if (supr_status == TX_STATUS_SUPR_UF) {
		update_rate = FALSE;
	}
	else if (P2P_ABS_SUPR(wlc, supr_status)) {
		update_rate = FALSE;
		if (bsscfg != NULL && BSS_TX_SUPR_ENAB(bsscfg)) {
#ifdef BCMDBG
			if (WL_P2P_ON()) {
#if defined(WLC_HIGH) && defined(WLP2P_UCODE)
				int bss = wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
				WL_P2P(("wl%d: suppress packet %p for absence, seqnum %d, "
				        "TSF 0x%x, state 0x%x\n",
				        wlc->pub->unit, p, ltoh16(h->seq) >> SEQNUM_SHIFT,
				        R_REG(wlc->osh, &wlc->regs->tsf_timerlow),
				        wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_ST(bss))));
#endif /* WLC_HIGH && WLP2P_UCODE */
				wlc_print_txdesc(wlc, (wlc_txd_t*)PKTDATA(osh, p));
				wlc_print_txstatus(wlc, txs);
			}
#endif /* BCMDBG */

			/* This is possible if we got a packet suppression
			 * before getting ABS interrupt
			 */
			if (!BSS_TX_SUPR(bsscfg) &&
#ifdef WLP2P
			    (wlc_p2p_noa_valid(wlc->p2p, bsscfg) ||
			     wlc_p2p_ops_valid(wlc->p2p, bsscfg)) &&
#endif
			    TRUE)
				wlc_bsscfg_tx_stop(bsscfg);

			/* requeue packet */
			if (scb != NULL) {
				bool requeue  = TRUE;

				if (BSSCFG_AP(bsscfg) &&
				    SCB_ASSOCIATED(scb) && SCB_P2P(scb))
					wlc_apps_scb_tx_block(wlc, scb, 1, TRUE);
#if defined(PROP_TXSTATUS)
				if (PROP_TXSTATUS_ENAB(wlc->pub)) {
					requeue = wlc_should_retry_suppressed_pkt(wlc,
					    p, supr_status);
					requeue |= SCB_ISMULTI(scb);
				}
#endif
				if (requeue) {
					if (BSS_TX_SUPR(bsscfg))
						free_pdu = wlc_pkt_abs_supr_enq(wlc, scb, p);
					else
						free_pdu = TRUE;
					supr_pkt = TRUE;
				}
			}
		}

		/* If the packet is not queued for later delivery, it will be freed
		 * and any packet callback needs to consider this as a un-ACKed packet.
		 * Change the txstatus to simple no-ack instead of suppressed.
		 */
		if (free_pdu)
			wlc_txs_clear_supr(wlc, &(txs->status));
	} else if (txs->phyerr) {
		update_rate = FALSE;
		WLCNTINCR(wlc->pub->_cnt->txphyerr);
		WL_ERROR(("wl%d: %s: tx phy error (0x%x)\n", wlc->pub->unit,
			__FUNCTION__, txs->phyerr));
#ifdef BCMDBG
		if (WL_ERROR_ON()) {
			prpkt("txpkt (MPDU)", osh, p);
			wlc_print_txdesc(wlc, (wlc_txd_t *)PKTDATA(osh, p));
			wlc_print_txstatus(wlc, txs);
		}
#endif /* BCMDBG */
	} else if (ETHER_ISMULTI(txh_info.TxFrameRA)) {
		/* mcast success */
		/* update counters */
		if (tx_frame_count) {
			WLCNTINCR(wlc->pub->_cnt->txfrag);
			WLCIFCNTINCR(scb, txfrag);
		}

		if (lastframe) {
			WLCNTINCR(wlc->pub->_cnt->txfrmsnt);
			WLCIFCNTINCR(scb, txfrmsnt);
			if (tx_frame_count) {
				WLCNTINCR(wlc->pub->_cnt->txmulti);
				WLCIFCNTINCR(scb, txmulti);
			}
		}
		pkt_sent = TRUE;
	}
#ifdef STA
	else if (bsscfg != NULL && BSSCFG_STA(bsscfg) && bsscfg->BSS && ETHER_ISMULTI(&h->a3)) {
		if (tx_frame_count)
			WLCNTINCR(wlc->pub->_cnt->txfrag);

		if (lastframe) {
			WLCNTINCR(wlc->pub->_cnt->txfrmsnt);
			WLCIFCNTINCR(scb, txfrmsnt);
			if (tx_frame_count) {
				WLCNTINCR(wlc->pub->_cnt->txmulti);
				WLCIFCNTINCR(scb, txmulti);
				WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_mcast_pkts);
			}
		}

		pkt_sent = TRUE;
	}
#endif /* STA */
	else if ((!tx_rts && tx_frame_count >= wlc->SRL) ||
	         (tx_rts && (tx_rts_count >= wlc->SRL || tx_frame_count >= wlc->LRL))) {
		WLCNTADD(wlc->pub->_cnt->txnoack, tx_frame_count);
#ifdef PKTQ_LOG
		WLCNTCONDADD(actq_cnt, actq_cnt->retry, tx_frame_count);
#endif
#if defined(WLAIBSS)
			if (AIBSS_ENAB(wlc->pub) && scb && bsscfg && !SCB_ISMULTI(scb) &&
				BSSCFG_IBSS(bsscfg) && (FC_TYPE(fc) == FC_TYPE_DATA)) {
				aibss_tx_monitor_t	*aibss_scb_tx_stats =
					SCB_CUBBY((scb), WLC_AIBSS_INFO_SCB_HDL(wlc->aibss_info));
				aibss_scb_tx_stats->tx_noack_count++;
				wlc_aibss_check_txfail(wlc->aibss_info, bsscfg, scb);
			}
#endif /* WLAIBSS */
		/* ucast failure */
		if (lastframe) {
			WLCNTINCR(wlc->pub->_cnt->txfail);
			WLCNTSCB_COND_INCR(scb && from_host,
				scb->scb_stats.tx_pkts_retry_exhausted);
			WLCNTSCB_COND_INCR(scb && !from_host,
				scb->scb_stats.tx_pkts_fw_retry_exhausted);
			WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_failures);
			WLCIFCNTINCR(scb, txfail);
#ifdef PKTQ_LOG
			WLCNTCONDINCR(actq_cnt, actq_cnt->retry_drop);
#endif
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_TXFAIL,
			                 (struct ether_addr *)txh_info.TxFrameRA,
			                 WLC_E_STATUS_TIMEOUT, 0, 0, 0, 0);
		}
#ifdef BCMDBG
		if ((!tx_rts && tx_frame_count != wlc->SRL) ||
		    (tx_rts && tx_rts_count != wlc->SRL &&
		               tx_frame_count != wlc->LRL)) {
			/* catch ucode retrying too much */
			WL_ERROR(("wl%d: %s: status indicates too many retries before "
			          "failure\n", wlc->pub->unit, __FUNCTION__));
			WL_ERROR(("tx_rts = %d tx_rts_count = %d tx_frame_count = %d ->"
				" srl=%d lrl=%d\n",
				tx_rts, tx_rts_count, tx_frame_count, wlc->SRL, wlc->LRL));
		}
#endif /* BCMDBG */
		pkt_max_retries = TRUE;
	} else {
		/* unexpected tx status */
		update_rate = FALSE;
		WLCNTINCR(wlc->pub->_cnt->txserr);
		WL_TRACE(("wl%d: %s: unexpected tx status returned \n",
			wlc->pub->unit, __FUNCTION__));
		WL_TRACE(("tx_rts = %d tx_rts_count = %d tx_frame_count = %d -> srl=%d lrl=%d\n",
			tx_rts, tx_rts_count, tx_frame_count, wlc->SRL, wlc->LRL));
	}

	/* Check if interference still there and print external log if needed */
	if (wlc->rfaware_lifetime && wlc->exptime_cnt &&
	    supr_status != TX_STATUS_SUPR_EXPTIME)
		wlc_exptime_check_end(wlc);

#ifdef WLAMSDU_TX
	if (AMSDU_TX_ENAB(wlc->pub) &&
	    scb && SCB_AMSDU(scb) && (FC_TYPE(fc) == FC_TYPE_DATA)) {
		wlc_amsdu_dotxstatus(wlc->ami, scb, p);
	}
#endif

	/* update rate state and scb used time */
	if (update_rate && scb) {
		uint16 sfbl, lfbl;
		uint8 mcs = MCS_INVALID;
		uint8 antselid = 0;
#ifdef WL11N
		uint8 frametype = ltoh16(txh_info.PhyTxControlWord0) & PHY_TXC_FT_MASK;
		if (frametype == FT_HT || frametype == FT_VHT) {
			mcs = wlc_txh_info_get_mcs(wlc, &txh_info);
		}
		if (WLANTSEL_ENAB(wlc))
			antselid = wlc_antsel_antsel2id(wlc->asi,
				ltoh16(txh_info.w.ABI_MimoAntSel));
#endif /* WL11N */

		ASSERT(!TX_STATUS_UNEXP(txs->status));

		if (queue < AC_COUNT) {
			sfbl = WLC_WME_RETRY_SFB_GET(wlc, wme_fifo2ac[queue]);
			lfbl = WLC_WME_RETRY_LFB_GET(wlc, wme_fifo2ac[queue]);
		} else {
			sfbl = wlc->SFBL;
			lfbl = wlc->LFBL;
		}
		wlc_scb_ratesel_upd_txstatus_normalack(wlc->wrsi, scb, txs, sfbl, lfbl,
			mcs, antselid, ltoh16(txh_info.MacTxControlHigh) & TXC_AMPDU_FBR ? 1 : 0);
#ifdef WL_LPC
		if (LPC_ENAB(wlc) && scb) {
			/* Update the LPC database */
			wlc_scb_lpc_update_pwr(wlc->wlpci, scb, txs->frameid,
				txh_info.PhyTxControlWord0, txh_info.PhyTxControlWord1);
			/* Reset the LPC vals in ratesel */
			wlc_scb_ratesel_reset_vals(wlc->wrsi, scb, ac);
		}
#endif /* WL_LPC */
	}
	WL_TRACE(("%s: calling txfifo_complete\n", __FUNCTION__));

	WLC_TXFIFO_COMPLETE(wlc, queue, 1, WLPKTTIME(p));

#ifdef PSPRETEND
	/* Provide tx status input to the PS Pretend state machine.
	 * The function may cause a tx fifo flush.
	 * As a result, this call needs to be done after
	 * the call to wlc_txfifo_complete() above.
	 */
	if (scb && (BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg)) && !(pkttag->flags & WLF_PSDONTQ)) {
		wlc_apps_process_pspretend_status(wlc, scb, ack_recd);
	}
#endif

#ifdef WLPKTDLYSTAT
	/* calculate latency and packet loss statistics */
	if (scb && free_pdu) {
		/* Get the current time */
		now = WLC_GET_CURR_TIME(wlc);
		/* Ignore wrap-around case */
		if (now > WLPKTTAG(p)->shared.enqtime) {
			uint tr = tx_frame_count - 1;
			if (tx_frame_count == 0 || tx_frame_count > RETRY_SHORT_DEF)
				tr = RETRY_SHORT_DEF-1;

			delay_stats = &scb->delay_stats[ac];
			delay = now - WLPKTTAG(p)->shared.enqtime;
			wlc_delay_stats_upd(delay_stats, delay, tr, ack_recd);
		}
	}
#endif /* WLPKTDLYSTAT */

#ifdef STA
	/* PM state change */
	/* - in mcast case we always have a valid bsscfg (unless the bsscfg is deleted).
	 * - in ucast case we may not have a valid bsscfg and scb could have been deleted before
	 *   the transmission finishes, in which case lookup the bsscfg from the BSSID
	 * - Infrastructure STAs should always find the bsscfg by RA BSSID match, unless the
	 *   bsscfg has been deleted (in which case we do not care about PM state update for it)
	 * - in case we located neither scb nor bsscfg we assume the PM states have
	 *   been taken care of when deleting the scb or the bsscfg, yes we hope;-)
	 */
	if (bsscfg != NULL) {
		wlc_update_pmstate(bsscfg, (uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))));

		if ((((fc & FC_KIND_MASK) == FC_DATA) ||
		     ((fc & FC_KIND_MASK) == FC_QOS_DATA)) && lastframe) {
			wlc_pm2_ret_upd_last_wake_time(bsscfg, &txs->lasttxtime);
		}
	}
#endif /* STA */


	/* update the bytes in/out counts */
	if (PIO_ENAB(wlc->pub))
		wlc_pio_cntupd(WLC_HW_PIO(wlc, queue), totlen);

#ifdef STA
	/* Roaming decision based upon too many packets at the min tx rate */
	if (scb != NULL && !SCB_ISMULTI(scb) &&
	    bsscfg != NULL && BSSCFG_STA(bsscfg) &&
	    bsscfg->BSS && bsscfg->associated &&
#ifdef WLP2P
	    !BSS_P2P_ENAB(wlc, bsscfg) &&
#endif
	    !bsscfg->roam->off)
		wlc_txrate_roam(wlc, scb, txs, pkt_sent, pkt_max_retries);

	if (scb != NULL && lastframe)
		wlc_adv_pspoll_upd(wlc, scb, p, TRUE, queue);
#endif /* STA */
#ifdef MACOSX
	if (wlc->oscallback_dotxstatus)
		wlc->oscallback_dotxstatus(wlc, (void*)scb, (void*)p, (void *)txs, (uint)0);
#endif /* MACOSX */
	/* call any matching pkt callbacks */
	if (free_pdu) {
		if (supr_pkt) {
#ifdef WLAMPDU
			/* for suppressed regmpdu */
			if (WLPKTFLAG_AMPDU(pkttag))
				wlc_ampdu_ini_adjust(wlc->ampdu_tx, scb, p);
#endif
		}
#ifdef WLAMSDU_TX
		/* amsdu pkt callbacks will be taken care of within ampdu module via */
		/* amsdu_dotxstatus; pcb only used in pkt free scenario, so clear here */
		if (AMSDU_TX_ENAB(wlc->pub) && (WLF2_PCB1(p) == WLF2_PCB1_AMSDU)) {
			WLF2_PCB1_UNREG(p);
		}
#endif
		wlc_pcb_fn_invoke(wlc->pcb, p,
		                  (uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))));
	}

#ifdef WLP2P
	if (WL_P2P_ON()) {
		if (txs->status.was_acked) {
			WL_P2P(("wl%d: sent packet %p successfully, seqnum %d\n",
			        wlc->pub->unit, p, ltoh16(h->seq) >> SEQNUM_SHIFT));
		}
	}
#endif

	PKTDBG_TRACE(osh, p, PKTLIST_TXDONE);

	/* free the mpdu */
	if (free_pdu) {
#ifdef WLTXMONITOR
#endif /* WLTXMONITOR */
		PKTFREE(osh, p, TRUE);
	}

	if (lastframe)
		wlc->txretried = 0;

	return FALSE;

fatal:
	if (p != NULL) {
		PKTDBG_TRACE(osh, p, PKTLIST_TXFAIL);
		PKTFREE(osh, p, TRUE);
	}

#ifdef WLC_HIGH_ONLY
	/* If this is a split driver, do the big-hammer here.
	 * If this is a monolithic driver, wlc_bmac.c:wlc_dpc() will do the big-hammer.
	 */
	WL_ERROR(("wl%d: %s HAMMERING \n", wlc->pub->unit, __FUNCTION__));
	wlc_fatal_error(wlc);
#endif
	BCM_REFERENCE(pkt_sent);
	BCM_REFERENCE(pkt_max_retries);
	return TRUE;
}

#ifdef PROP_TXSTATUS
void wlc_txfifo_suppress(wlc_info_t *wlc, struct scb* pmq_scb)
{
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
	/* valid SCB (SCB_PS) :Peek and suppress only those pkts that belong to PMQ-ed scb */
	/* null SCB (BSSCFG_ABS) :all pkts in TxFIFO will be suppressed */
	void **phdr_list = NULL;
	hnddma_t* di = NULL;
	int i, j;
	int size;

	int fifo_list[] = { TX_AC_BE_FIFO, TX_AC_BK_FIFO, TX_AC_VI_FIFO, TX_AC_VO_FIFO };
	for (j = 0; j < (sizeof(fifo_list)/sizeof(int)); j++)
	{
		int fifo = fifo_list[j];
		int n = TXPKTPENDGET(wlc, fifo);
		if (n == 0)
			continue;

		size = (n * sizeof(void*));
		phdr_list = MALLOC(wlc->osh, size);
		if (phdr_list == NULL) continue;

		bzero(phdr_list, size);
		di = WLC_HW_DI(wlc, fifo);
		if (di && (dma_peekntxp(di, &n, phdr_list, HNDDMA_RANGE_ALL) == BCME_OK)) {
		for (i = 0; i < n; i++) {
			void *p;
			p = phdr_list[i];
			ASSERT(p);
			if (p != NULL) {
				struct scb *scb = WLPKTTAGSCBGET(p);

				if ((pmq_scb == NULL) || (pmq_scb && scb && (pmq_scb == scb))) {
					if (!(scb && SCB_ISMULTI(scb))) {
						if ((WL_TXSTATUS_GET_FLAGS
							(WLPKTTAG(p)->wl_hdr_information)
							& WLFC_PKTFLAG_PKTFROMHOST)) {
							wlc_suppress_sync_fsm(wlc, scb, p, TRUE);
							wlc_process_wlhdr_txstatus(wlc,
								WLFC_CTL_PKTFLAG_WLSUPPRESS,
								p, i < (n-1) ?
								TRUE : FALSE);
						}
					}
				}
			} /* if p!=NULL */
		} /* for each p */
		} /* if dma_peek */
		MFREE(wlc->osh, (void*) phdr_list, size);
	} /* for each fifo */
	} /* PROP_TX ENAB */
}
#endif /* PROP_TXSTATUS */

bool wlc_should_retry_suppressed_pkt(wlc_info_t *wlc, void *p, uint status)
{
	/* TX_STATUS_SUPR_PPS is purposely not tested here */
	if ((status == TX_STATUS_SUPR_PMQ) || (P2P_ABS_SUPR(wlc, status))) {
		/* Retry/Requeue pkt in any of the following cases:
		 * 1. NIC / PropTx Not enabled
		 * 2. Dongle / PropTx Disabled.
		 * 3. Dongle / PropTx Enabled, but Host PropTx inactive
		 * 4. Dongle / PropTx Enabled and Host PropTx active, but dongle(!host) packet
		 */
		if ((!PROP_TXSTATUS_ENAB(wlc->pub)) ||
#ifdef PROP_TXSTATUS
			(!HOST_PROPTXSTATUS_ACTIVATED(wlc)) ||
			(PROP_TXSTATUS_ENAB(wlc->pub) && HOST_PROPTXSTATUS_ACTIVATED(wlc) &&
			(!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST))) ||
#endif /* PROP_TXSTATUS */
			FALSE) {
#ifdef PROP_TXSTATUS_DEBUG
			wlc_txh_info_t txh_info;
			struct dot11_header *h;
			uint16 fc;
			wlc_get_txh_info(wlc, p, &txh_info);
			h = (struct dot11_header *)(txh_info.d11HdrPtr);
			fc = ltoh16(h->fc);
			char eabuf[ETHER_ADDR_STR_LEN];
			bcm_ether_ntoa((struct ether_addr *)(txh_info.TxFrameRA), eabuf);
			printf("retry ptk:0x%x FC%x t/st:%x/%x for [%s]\n",
				(WLPKTTAG(p)->wl_hdr_information), fc,
				FC_TYPE(fc), FC_SUBTYPE(fc), eabuf);
#endif /* PROP_TXSTATUS_DEBUG */
			return TRUE;
		}
	}
	return FALSE;
}

#ifdef STA
/* update cfg->PMpending and wlc->PMpending */
void
wlc_set_pmpending(wlc_bsscfg_t *cfg, bool pm0to1)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool oldstate = pm->PMpending;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, pm0to1));

	pm->PMpending = pm0to1;
#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		wlc_mchan_set_pmpending(wlc->mchan, cfg, pm0to1);
	}
#endif /* WLMCHAN */

	if (oldstate == pm0to1)
		return;

	if (pm0to1) {
		pm->pm2_ps0_allowed = FALSE;
		wlc->PMpending = TRUE;
	}
	/* leave setting wlc->PMpending to FALSE to wlc_pm_pending_complete()
	 * else {
	 * 	int idx;
	 *	wlc_bsscfg_t *bc;
	 *	FOREACH_AS_STA(wlc, idx, bc) {
	 *		if (bc->pm->PMpending)
	 *			return;
	 *	}
	 *	wlc->PMpending = FALSE;
	 * }
	 */

	wlc_set_wake_ctrl(wlc);
}

/* update cfg->PMawakebcn and wlc->PMawakebcn */
void
wlc_set_pmawakebcn(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool oldstate = pm->PMawakebcn;
	wlc_cxn_t *cxn = cfg->cxn;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, state));

	if (state == oldstate)
		return;

	if (state) {
		if (cxn->ign_bcn_lost_det != 0)
			return;
		pm->PMawakebcn = TRUE;
		wlc->PMawakebcn = TRUE;
	}
	else {
		int idx;
		wlc_bsscfg_t *bc;

		pm->PMawakebcn = FALSE;
		FOREACH_AS_STA(wlc, idx, bc) {
			if (bc->pm->PMawakebcn)
				return;
		}
		wlc->PMawakebcn = FALSE;
	}

#ifdef WLOFFLD
	if (WLOFFLD_BCN_ENAB(wlc->pub)) {
		if (state)
			wlc_ol_rx_deferral(wlc->ol, OL_AWAKEBCN_MASK, OL_AWAKEBCN_MASK);
		else
			wlc_ol_rx_deferral(wlc->ol, OL_AWAKEBCN_MASK, 0);
	}
#endif
	wlc_set_wake_ctrl(wlc);
}

/* update cfg->PSpoll and wlc->PSpoll */
void
wlc_set_pspoll(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool oldstate = pm->PSpoll;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, state));

	pm->PSpoll = state;
	if (oldstate == state)
		return;

	if (state)
		wlc->PSpoll = TRUE;
	else {
		int idx;
		wlc_bsscfg_t *bc;

		FOREACH_AS_STA(wlc, idx, bc) {
			if (bc->pm->PSpoll)
				return;
		}
		wlc->PSpoll = FALSE;
	}
	wlc_set_wake_ctrl(wlc);
}

/* update cfg->apsd_sta_usp and wlc->apsd_sta_usp */
void
wlc_set_apsd_stausp(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool oldstate = pm->apsd_sta_usp;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, state));

	pm->apsd_sta_usp = state;
	if (state == oldstate)
		return;

	if (state)
		wlc->apsd_sta_usp = TRUE;
	else {
		int idx;
		wlc_bsscfg_t *bc;
		FOREACH_BSS(wlc, idx, bc) {
			if (BSSCFG_STA(bc) && (bc->associated || BSS_TDLS_ENAB(wlc, bc)))
				if (bc->pm->apsd_sta_usp)
					return;
		}
		wlc->apsd_sta_usp = FALSE;
	}
	wlc_set_wake_ctrl(wlc);
}

/* update cfg->check_for_unaligned_tbtt and wlc->check_for_unaligned_tbtt */
void
wlc_set_uatbtt(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool oldstate = pm->check_for_unaligned_tbtt;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, state));

	pm->check_for_unaligned_tbtt = state;
	if (state == oldstate)
		return;

	if (state) {
		wlc->check_for_unaligned_tbtt = TRUE;
		wlc->ebd->uatbtt_start_time = OSL_SYSUPTIME();
#ifdef WLMCNX
		if (MCNX_ENAB(wlc->pub))
			wlc_mcnx_tbtt_inv(wlc->mcnx, cfg);
#endif
	}
	else {
		int idx;
		wlc_bsscfg_t *bc;
		FOREACH_AS_STA(wlc, idx, bc) {
			if (bc->pm->check_for_unaligned_tbtt)
				return;
		}
		wlc->check_for_unaligned_tbtt = FALSE;
	}
#ifdef WL_BCN_COALESCING
	wlc_bcn_clsg_disable(wlc->bc, BCN_CLSG_UATBTT_MASK,
		wlc->check_for_unaligned_tbtt ? BCN_CLSG_UATBTT_MASK:0);
#endif /* WL_BCN_COALESCING */

#ifdef WLOFFLD
	if (WLOFFLD_BCN_ENAB(wlc->pub)) {
		if (!wlc->check_for_unaligned_tbtt)
			wlc_ol_rx_deferral(wlc->ol, OL_TBTT_MASK, 0);

		if (wlc->check_for_unaligned_tbtt) {
			wlc_ol_rx_deferral(wlc->ol, OL_TBTT_MASK, OL_TBTT_MASK);
		}
	}
#endif

	wlc_set_wake_ctrl(wlc);
}

/* update cfg->PMenabled */
void
wlc_set_pmenabled(wlc_bsscfg_t *cfg, bool state)
{
	wlc_pm_st_t *pm = cfg->pm;
#if defined(BCMDBG) || defined(WLMSG_PS)
	bool oldstate = pm->PMenabled;
#endif
	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, state));

	pm->PMenabled = state;

	wlc_set_ps_ctrl(cfg);
}

/* update cfg->PM_override */
void
wlc_set_pmoverride(wlc_bsscfg_t *cfg, bool state)
{
	wlc_pm_st_t *pm = cfg->pm;
	bool oldstate = pm->PM_override;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
	       cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, oldstate, state));

	cfg->pm->PM_override = state;
	if (oldstate == state)
		return;

	wlc_set_pmstate(cfg, cfg->pm->PMenabled);
}

/* update cfg->dtim_programmed */
void
wlc_set_dtim_programmed(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;

	(void)wlc;

	WL_PS(("wl%d.%d: %s: old %d new %d\n",
		wlc->pub->unit,    WLC_BSSCFG_IDX(cfg), __FUNCTION__, cfg->dtim_programmed, state));

	if (cfg->dtim_programmed == state)
		return;

	cfg->dtim_programmed = state;
}

void
wlc_update_bcn_info(wlc_bsscfg_t *cfg, bool state)
{
	wlc_set_dtim_programmed(cfg, state);

	wlc_set_ps_ctrl(cfg);

#ifdef WL_BCN_COALESCING
	wlc_bcn_clsg_disable(cfg->wlc->bc, BCN_CLSG_ASSOC_MASK, state? 0: BCN_CLSG_ASSOC_MASK);
#endif /* WL_BCN_COALESCING */
#ifdef WLOFFLD
	if (WLOFFLD_ENAB(cfg->wlc->pub) && (cfg == cfg->wlc->cfg)) {
		if (state) {
			wlc_ol_enable(cfg->wlc->ol, cfg);
		} else {
			wlc_ol_disable(cfg->wlc->ol, cfg);
		}
	}
#endif /* WLOFFLD */
}

/* Set the module id that triggered PS mode and call wlc_set_pmstate */
void wlc_module_set_pmstate(wlc_bsscfg_t *cfg, bool state, mbool moduleId)
{
	mboolset(cfg->pm->PMenabledModuleId, moduleId);
	wlc_set_pmstate(cfg, state);
}

void
wlc_cfg_set_pmstate_upd(wlc_bsscfg_t *cfg, bool pmenabled)
{
	wlc_info_t *wlc = cfg->wlc;
	bool wake = !pmenabled;

	if (wake)
		wlc->core_wake |= (1 << WLC_BSSCFG_IDX(cfg));
	else
		wlc->core_wake &= ~(1 << WLC_BSSCFG_IDX(cfg));

	if (wlc->core_wake && wlc->pub->associated)
		wl_indicate_maccore_state(wlc->wl, LTR_ACTIVE);
	else
		wl_indicate_maccore_state(wlc->wl, LTR_SLEEP);
}

/*
 * Set PS mode and communicate new state to AP by sending null data frame.
 */
void
wlc_set_pmstate(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	uint txstatus = TX_STATUS_ACK_RCV;

	wlc_cfg_set_pmstate_upd(cfg, state);

	if (!BSSCFG_STA(cfg) || !cfg->BSS)
		return;

	WL_PS(("wl%d.%d: PM-MODE: wlc_set_pmstate: set PMenabled to %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), state));

	/* Disable U-APSD when STA in active mode */
	if (state == FALSE && pm->apsd_sta_usp)
		wlc_set_apsd_stausp(cfg, FALSE);

	if (state == FALSE) {
			pm->PMenabledModuleId = 0;
			pm->pm_immed_retries = 0;
			if (wlc->pub->up && BSSCFG_STA(cfg) && cfg->BSS && cfg->associated)
				wlc_pm2_ret_upd_last_wake_time(cfg, NULL);
		}

	wlc_set_pmenabled(cfg, state);

	wlc_set_pmpending(cfg, PS_ALLOWED(cfg));

	if (!wlc->pub->up)
		return;


	if (!cfg->associated || !wlc_portopen(cfg))
		return;

	WL_RTDC(wlc, "wlc_set_pmstate: tx PMep=%02u AW=%02u",
	        (pm->PMenabled ? 10 : 0) | pm->PMpending,
	        (PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));

	/* If wlc->PMPending, fake a status of TX_STATUS_PMINDCTD | TX_STATUS_ACK_RCV
	 * else, fake TX_STATUS_ACK_RCV, in case the upper layers need to know of the
	 * conditions.
	 */
	if (pm->PMpending)
		txstatus |= TX_STATUS_PMINDCTD;

	/* announce PM change */
	/* send NULL data frame to communicate PM state to each associated APs */
	/* don't bother sending a null data frame if we lost our AP connection */
	if (!WLC_BSS_CONNECTED(cfg) ||
#ifdef WLMCNX
	    (!MCNX_ENAB(wlc->pub) && cfg != wlc->cfg) ||
#endif
	    FALSE) {
		WL_PS(("wl%d.%d: skip the PM null frame, fake a PM0->PM1 transition\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		wlc_bss_pm_pending_upd(cfg, txstatus);
	}
	else if (0 ||
#ifdef WL_MULTIQUEUE
	         /* if excursion is already active, no point in sending null pkt out.
	          * null pkts are queued to bsscfg specific queues so they will not
	          * go out until excursion is complete.
	          */
	         wlc->excursion_active ||
#endif /* WL_MULTIQUEUE */
	         !wlc_sendpmnotif(wlc, cfg, &cfg->BSSID, 0, -1,
	                          SCAN_IN_PROGRESS(wlc->scan) && state)) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
#ifdef WL_MULTIQUEUE
		bool excrx = wlc->excursion_active;
#else
		bool excrx = 0;
#endif
#endif /* BCMDBG || BCMDBG_ERR */
		WL_ERROR(("wl%d.%d: failed to send PM null frame, "
		          "fake a PM0->PM1 transition, excursion_active %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), excrx));
		wlc_bss_pm_pending_upd(cfg, txstatus);
	}

	/* start periodic pspoll timer if we are indeed in PS mode */
	wlc_pspoll_timer_upd(cfg, state);
	wlc_apsd_trigger_upd(cfg, state);
}

#ifdef WLPM_BCNRX
static bool
wlc_pm_bcnrx_cap(wlc_info_t *wlc)
{
	if (D11REV_GE(wlc->pub->corerev, 40))
		return TRUE;

	return FALSE;
}

bool
wlc_pm_bcnrx_allowed(wlc_info_t *wlc)
{
	return wlc_stf_rxchain_ishwdef(wlc);
}

int
wlc_pm_bcnrx_set(wlc_info_t *wlc, bool enable)
{
	bool val;

	if (!wlc_pm_bcnrx_cap(wlc))
		return BCME_UNSUPPORTED;

	if (!si_iscoreup(wlc->pub->sih))
		return BCME_NOTUP;

	val = (wlc_mhf_get(wlc, MHF3, WLC_BAND_AUTO) & MHF3_PM_BCNRX) ? TRUE : FALSE;
	if (val == enable)
		return 0;

	wlc_suspend_mac_and_wait(wlc);
	wlc_mhf(wlc, MHF3, MHF3_PM_BCNRX, enable ? MHF3_PM_BCNRX : 0, WLC_BAND_ALL);
	wlc_enable_mac(wlc);

	return 0;
}

void
wlc_pm_bcnrx_disable(wlc_info_t *wlc)
{
	int err;

	err = wlc_pm_bcnrx_set(wlc, FALSE);

	/* If it fails because core was down,
	 * don't clear flag so we get another
	 * chance in wlc_init
	 */
	if (!err) {
		WL_ERROR(("Forcing PM bcnrx disable\n"));
		wlc->pub->_pm_bcnrx = FALSE;
	}
}

static void
wlc_pm_bcnrx_init(wlc_info_t *wlc)
{
#if defined(AP) && (defined(RADIO_PWRSAVE) || defined(RXCHAIN_PWRSAVE))
	if (RADIO_PWRSAVE_ENAB(wlc->ap) || RXCHAIN_PWRSAVE_ENAB(wlc->ap)) {
		wlc_pm_bcnrx_disable(wlc);
	} else
#endif /* AP && (RADIO_PWRSAVE || RXCHAIN_PWRSAVE) */
	if (wlc_pm_bcnrx_allowed(wlc))
		wlc_pm_bcnrx_set(wlc, TRUE);
	else
		wlc_pm_bcnrx_set(wlc, FALSE);
}
#endif /* WLPM_BCNRX */

static void
wlc_radio_shutoff_dly_timer_upd(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

	wl_del_timer(wlc->wl, wlc->pm2_radio_shutoff_dly_timer);

	if (wlc->pm2_radio_shutoff_dly == 0)
		return;

	/* we are done if no one is in FAST PS mode */
	cfg = NULL;
	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->pm->PMenabled && cfg->pm->PM == PM_FAST)
			break;
	}
	if (cfg == NULL)
		return;

	WL_PS(("wl%d: wlc_radio_shutoff_dly_timer_upd: arm periodic(%u) "
	       "radio shutoff delay timer\n", wlc->pub->unit, wlc->pm2_radio_shutoff_dly));

	wlc->pm2_radio_shutoff_pending = TRUE;
	wl_add_timer(wlc->wl, wlc->pm2_radio_shutoff_dly_timer, wlc->pm2_radio_shutoff_dly, FALSE);
	wlc_set_wake_ctrl(wlc);
}

int
wlc_pspoll_timer_upd(wlc_bsscfg_t *cfg, bool allow)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	int callbacks = 0;

	if (pm->pspoll_timer == NULL) {
		WL_ERROR(("wl%d.%d: %s: Trying to update NULL pspoll timer.\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return 0;
	}

	if (!wl_del_timer(wlc->wl, pm->pspoll_timer))
		callbacks ++;

	if (!allow || !cfg->associated || !pm->PMenabled || pm->pspoll_prd == 0)
		return callbacks;

	WL_PS(("wl%d.%d: %s: arm periodic(%u) pspoll timer\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pm->pspoll_prd));

	wl_add_timer(wlc->wl, pm->pspoll_timer, pm->pspoll_prd, TRUE);
	return 0;
}

/* Update local PM state(s) and change local PS mode based on tx status.
 * It will also resume any pending work ie. scan that depend on the PM
 * transition, or allow local to doze if there is nothing pending.
 */
void
wlc_update_pmstate(wlc_bsscfg_t *cfg, uint txstatus)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool pmindctd;

	/* we only work on associated BSS STA... */
	if (!BSSCFG_STA(cfg) || ((!cfg->BSS || !cfg->associated) && !BSS_TDLS_ENAB(wlc, cfg)))
		return;

	/* If we get a suppressed frame due to lifetime expiration when waiting for PM indication,
	 * consider it as a PM indication since it indicates a strong interference. Since ucode
	 * doesn't give us PM indication bit in this case, we need to fake it.
	 */

	if (pm->PMpending) {
		WL_PS(("wl%d.%d: PM-MODE: wlc_update_pmstate: txstatus 0x%x\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), txstatus));

		if (((txstatus & TX_STATUS_SUPR_MASK) >> TX_STATUS_SUPR_SHIFT) ==
			TX_STATUS_SUPR_EXPTIME) {
			WL_PS(("wl%d.%d: PM-MODE: frame lifetime expired\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
			txstatus |= TX_STATUS_PMINDCTD;
		}
		if ((txstatus & TX_STATUS_MASK) == TX_STATUS_NO_ACK &&
			SCAN_IN_PROGRESS(wlc->scan)) {
			WL_PS(("wl%d.%d: PM-MODE: frame recv'd no ACK\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
			txstatus |= TX_STATUS_ACK_RCV|TX_STATUS_PMINDCTD;
		}
	}

	pmindctd = (txstatus & TX_STATUS_PMINDCTD) == TX_STATUS_PMINDCTD;

#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, cfg) && pm->PMpending)
		wlc_tdls_update_pm(wlc->tdls, cfg, txstatus);
#endif

	/* PM change */
	if (pm->priorPMstate != pmindctd) {
		wlc_bss_pm_pending_upd(cfg, txstatus);
		pm->priorPMstate = pmindctd;
	}

	/* if an Ack was received for a PM1 Null tx, stop any retransmits of
	 * PM1 Null at each received infra bss beacon.
	 */
	if (pmindctd && (txstatus & TX_STATUS_ACK_RCV)) {
		pm->pm_immed_retries = 0;
	}

	/* If this evaluates to true, the PM1->PM0 transition was lost and we didn't catch the
	 * transition. So, fake the transition (for upper layers that need to be aware of
	 * PM1->PM0 transitions, and then send the real PM0->PM1 transition
	 */
	if (pm->PMpending && pmindctd) {
		if (SCAN_IN_PROGRESS(wlc->scan) && (!(txstatus & TX_STATUS_ACK_RCV))) {
			WL_PS(("wl%d.%d: PM-MODE: fake a lost PM1->PM0 transition\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
			wlc_bss_pm_pending_upd(cfg, (txstatus | TX_STATUS_ACK_RCV));
			pm->pm_immed_retries = 0;
		}
		else if (txstatus & TX_STATUS_ACK_RCV) {
			wlc_bss_pm_pending_upd(cfg, txstatus);
		}
		/* reaching here means our PM1 tx was not acked by the AP */
		else if (pm->ps_resend_mode == PS_RESEND_MODE_BCN_NO_SLEEP ||
			pm->ps_resend_mode == PS_RESEND_MODE_BCN_SLEEP) {
			/* If we have not reached the max immediate PM1 retries,
			 *   Increment the immediate PM1 retry count
			 *   Send a new PM1 Null frame with a new seq#.
			 */
			if (pm->pm_immed_retries < PM_IMMED_RETRIES_MAX) {
				++pm->pm_immed_retries;
				wlc_sendnulldata(wlc, cfg, &cfg->BSSID, 0, 0, -1,
					NULL, NULL);
			/* Else we have reached the max# of immediate PM1 resends.
			 * Leave pm_immed_retries at the max retries value to
			 * indicate to the the beacon handler to resend PM1 Null
			 * frames at every DTIM beacon until an Ack is received.
			 */
			} else {
				/* If configured, go to sleep while waiting for the
				 * next beacon.
				 */
				if (pm->ps_resend_mode == PS_RESEND_MODE_BCN_SLEEP) {
					wlc_bss_pm_pending_upd(cfg,
						(txstatus | TX_STATUS_ACK_RCV));
				}
			}
		}
	}

	/* update the PMpending state and move things forward when possible */
	wlc_pm_pending_complete(wlc);
	wlc_vht_pm_pending_complete(wlc->vhti, cfg);
}

static void
wlc_bss_pm_pending_reset(wlc_bsscfg_t *cfg, uint txs)
{
	WL_PS(("wl%d.%d: PM-MODE: fake a lost PM0->PM1 transition\n",
	       cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

	wlc_bss_pm_pending_upd(cfg, txs | TX_STATUS_PMINDCTD | TX_STATUS_ACK_RCV);
}

/* we are only interested in PM0->PM1 transition! */
void
wlc_pm_pending_complete(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

	if (!wlc->PMpending)
		return;

	WL_PS(("wl%d: PM-MODE: wlc_pm_pending_complete: wlc->PMpending %d\n",
	       wlc->pub->unit, wlc->PMpending));

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		/* wlc_mchan_pm_pending_complete relies on chan_context->pm_pending being FALSE */
		wlc_mchan_pm_pending_complete(wlc->mchan);
	}
#endif /* WLMCHAN */

	FOREACH_STA(wlc, idx, cfg) {
		if (cfg->associated) {
			if (cfg->pm->PMpending)
				return;
		}
#ifdef WLTDLS
		else if (BSS_TDLS_ENAB(wlc, cfg) && (cfg->pm->PMpending))
			return;
#endif
	}

	WL_PS(("wl%d: PM-MODE: PM indications are all finished, "
	       "set wlc->PMpending to FALSE\n", wlc->pub->unit));

	wlc->PMpending = FALSE;
	wlc_set_wake_ctrl(wlc);

	wlc_scan_pm_pending_complete(wlc->scan);
#ifdef WLRM
	if (WLRM_ENAB(wlc->pub))
		wlc_rm_pm_pending_complete(wlc->rm_info);
#endif

#ifdef WL11K
	if (WL11K_ENAB(wlc->pub))
		wlc_rrm_pm_pending_complete(wlc->rrm_info);
#endif /* WL11K */

	FOREACH_AS_STA(wlc, idx, cfg) {
	}

	/* non-p2p APs are out there don't shutdown the radio */
	FOREACH_UP_AP(wlc, idx, cfg) {
		if (cfg != wlc->cfg &&
#ifdef WLP2P
		    !BSS_P2P_ENAB(wlc, cfg) &&
#endif
		    TRUE)
			return;
	}
	wlc_radio_shutoff_dly_timer_upd(wlc);

}

/* we are only interested in PM0->PM1 transition! */
static void
wlc_bss_pm_pending_upd(wlc_bsscfg_t *cfg, uint txstatus)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	bool pmindctd = (txstatus & TX_STATUS_PMINDCTD) == TX_STATUS_PMINDCTD;

	/* Get rid of warning */
	(void)wlc;

	WL_PS(("wl%d.%d: PM-MODE: wlc_bss_pm_pending_upd: PMpending %d pmindctd %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), pm->PMpending, pmindctd));

	if (!pm->PMpending)
		return;

	if (pm->PMblocked || (txstatus & TX_STATUS_ACK_RCV))
		wlc_set_pmpending(cfg, !pmindctd);

	if (pm->PMpending)
		return;

	/* when done waiting PM0->PM1 transition, let core go to sleep */
	WL_RTDC(wlc, "wlc_bss_pm_pending_upd: PMep=%02u PMaw=%02u",
	        (pm->PMenabled ? 10 : 0) | pm->PMpending,
	        (PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
	WL_PS(("wl%d.%d: PM-MODE: PS_ALLOWED() %d, %s\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), PS_ALLOWED(cfg),
	       STAY_AWAKE(wlc)?"but remaining awake":"going to sleep"));

	if (PM2_RCV_DUR_ENAB(cfg) && pm->PM == PM_FAST) {
		if (pm->pm2_rcv_state == PM2RD_WAIT_RTS_ACK) {
			pm->pm2_rcv_state = PM2RD_WAIT_BCN;
			WL_RTDC(wlc, "pm2_rcv_state=PM2RD_WAIT_BCN", 0, 0);
		}
	}

#ifdef WLMCHAN
	/* update chan_context pm_pending flag */
	if (MCHAN_ENAB(wlc->pub)) {
		wlc_mchan_update_pmpending(wlc->mchan, cfg);
	}
#endif /* WLMCHAN */
}

void
wlc_reset_pmstate(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;

	(void)wlc;

	WL_PS(("wl%d.%d: PM-MODE: wlc_reset_pmstate:\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

	wlc_set_pmawakebcn(cfg, FALSE);

	if (pm->PMpending)
		wlc_update_pmstate(cfg, TX_STATUS_BE);

	wlc_set_pspoll(cfg, FALSE);
	wlc_set_apsd_stausp(cfg, FALSE);
	wlc_set_uatbtt(cfg, FALSE);
	pm->WME_PM_blocked = FALSE;
	/* clear all PMblocked bits */
	pm->PMblocked = 0;
	pm->PMenabled = (pm->PM != PM_OFF);
	pm->PMenabledModuleId = 0;
	wlc_set_ps_ctrl(cfg);
}

bool
wlc_bssid_is_current(wlc_bsscfg_t *cfg, struct ether_addr *bssid)
{
	return WLC_IS_CURRENT_BSSID(cfg, bssid);
}

bool BCMFASTPATH
wlc_bss_connected(wlc_bsscfg_t *cfg)
{
	return WLC_BSS_CONNECTED(cfg);
}

uint8
wlc_stas_connected(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	uint8 stas_connected = 0;

	/* count STAs currently still talking with their associated APs */
	FOREACH_AS_STA(wlc, idx, cfg) {
		if (WLC_BSS_CONNECTED(cfg))
			stas_connected ++;
	}

	return stas_connected;
}

bool
wlc_portopen(wlc_bsscfg_t *cfg)
{
	return WLC_PORTOPEN(cfg);
}

#endif /* STA */

/* Can be called in either STA or AP context.
 * In STA mode, we received a Channel Switch announcement.
 * In AP, we detected radar and need to move off channel.
 */
void
wlc_do_chanswitch(wlc_bsscfg_t *cfg, chanspec_t newchspec)
{
	wlc_info_t *wlc = cfg->wlc;
	bool band_chg = FALSE;
	bool bw_chg = FALSE;
	bool from_radar = FALSE;
	bool to_radar = FALSE;
#if defined(STA) && defined(WLOFFLD)
	uint16 offload_state = 0;
#endif
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char chanbuf[CHANSPEC_STR_LEN];

	WL_INFORM(("wl%d: %s: new chanspec %s\n", wlc->pub->unit, __FUNCTION__,
		wf_chspec_ntoa_ex(newchspec, chanbuf)));
#endif /* BCMDBG || WLMSG_INFORM */

	if (!wlc_valid_chanspec_db(wlc->cmi, newchspec)) {
		WL_REGULATORY(("%s: Received invalid channel - ignoring\n", __FUNCTION__));
		/* something needs to be done here ?? */
		return;
	}

#if defined(STA) && defined(WLOFFLD)
	/*
	 * Get offloads status
	 * if offloads is enabled then Disable offloads
	 * At the bottom of this function before we return
	 * if offloads status was ENABLE then re-enable offloads
	 * Push new RX_TX_PARAMS
	 */
	offload_state = wlc_ol_get_state(wlc->ol);
	if (offload_state) {
		wlc_ol_disable(wlc->ol, cfg);
	}
#endif


	/* With 11H AP active:
	 * NOTE: Non-Radar (NR), Radar (R).
	 * 1. oldchanspec = R, newchanspec = R
	 * Action:
	 * 1. turn radar_detect OFF befor channel switch.
	 * 2. Turn radar_detect ON affter channel switch.
	 *
	 * 2. oldchanspec = R, newchanspec = NR
	 * Action:
	 *	1. Turn radar OFF before channel switch.
	 *
	 * 3. oldchanspec = NR, newchanspec = R
	 * Action:
	 *	1. Turn radar ON After switch to new channel.
	 *
	 * Turning on radar in case 1 and 3 are take care in wlc_restart_ap().
	 */
	if (WL11H_AP_ENAB(wlc) && AP_ACTIVE(wlc)) {
		from_radar = (wlc_radar_chanspec(wlc->cmi, cfg->current_bss->chanspec) == TRUE);
		to_radar = (wlc_radar_chanspec(wlc->cmi, newchspec));
	}

	/* evaluate band or bandwidth change here before wlc->home_chanspec gets changed */
	if (CHSPEC_WLCBANDUNIT(wlc->home_chanspec) != CHSPEC_WLCBANDUNIT(newchspec))
		band_chg = TRUE;
	if (CHSPEC_BW(wlc->home_chanspec) != CHSPEC_BW(newchspec))
		bw_chg = TRUE;

	if (WL11H_ENAB(wlc)) {
		/* restore the current channel's quiet bit */
		if (wlc_radar_chanspec(wlc->cmi, wlc->home_chanspec) ||
		    wlc_restricted_chanspec(wlc->cmi, wlc->home_chanspec)) {
			wlc_set_quiet_chanspec(wlc->cmi, wlc->home_chanspec);
			WL_REGULATORY(("%s: Setting quiet bit for chanspec %s\n",
				__FUNCTION__, wf_chspec_ntoa_ex(newchspec, chanbuf)));
		}
	}

	if (from_radar)
		wlc_set_dfs_cacstate(wlc->dfs, OFF);

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub) &&
	    !BSS_TDLS_ENAB(wlc, cfg)) {
		WL_MCHAN(("wl%d.%d: %s: chansw command issued, change chanctx to 0x%x\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, newchspec));
		if (wlc_mchan_create_bss_chan_context(wlc, cfg, newchspec)) {
			WL_ERROR(("wl%d : Channel Switch :"
				" Failed to create bss_chan_context\n", wlc->pub->unit));
			goto out;
		}
		/* Make sure cfg->current_bss and cfg->target_bss chanspec changed also */
		cfg->target_bss->chanspec = newchspec;
		wlc_bsscfg_set_current_bss_chan(cfg, newchspec);
	}
#endif
	WL_REGULATORY(("wl%d.%d: %s: switching to chanspec %s\n",
		wlc->pub->unit, cfg->_idx, __FUNCTION__, wf_chspec_ntoa_ex(newchspec, chanbuf)));

	/* Skip for proxySTA bsscfg */
	if (!BSSCFG_PSTA(cfg)) {
		wlc_set_home_chanspec(wlc, newchspec);
		wlc_suspend_mac_and_wait(wlc);
		wlc_set_chanspec(wlc, newchspec);
		/* Schedule a PHY cal right after channel change */
		wlc_phy_mphase_cal_schedule(WLC_PI(wlc), PHY_MPH_CAL_DELAY);
	}


#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		wlc_mchan_set_priq(wlc->mchan, cfg);
	}
#endif
	if (BSSCFG_AP(cfg)) {
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, FALSE);
	}

	/* Skip for proxySTA bsscfg */
	if (!BSSCFG_PSTA(cfg))
		wlc_enable_mac(wlc);

	WL_REGULATORY(("wl%d.%d: %s: switched to chanspec %s\n",
		wlc->pub->unit, cfg->_idx, __FUNCTION__, wf_chspec_ntoa_ex(newchspec, chanbuf)));

	if (band_chg) {
		wlc_scb_update_band_for_cfg(wlc, cfg, newchspec);
	}

	if (band_chg || to_radar) {
		/* in case of band switch, restart ap */
		if (AP_ACTIVE(wlc))
			wlc_restart_ap(wlc->ap);
	}

	if (bw_chg)
		/* force a rate selection init. */
		wlc_scb_ratesel_init_bss(wlc, cfg);
	else
		/* reset the rate sel state to init state */
		wlc_scb_ratesel_rfbr_bss(wlc, cfg);

	/* perform a reassoc procedure to sync up everything with the AP.
	 * 1. If band or BW change.
	 * 2. If for some reason (bcn_lost etc) STA disconnected
	 * with AP (cfg->BSSID is cleared when this happens).
	*/
	if (band_chg || bw_chg || ETHER_ISNULLADDR(&cfg->BSSID)) {
#ifdef STA
		if (BSSCFG_STA(cfg) && cfg->BSS) {
			wl_reassoc_params_t assoc_params;

			memset(&assoc_params, 0, sizeof(wl_reassoc_params_t));

			/* If we got disconnected by now try
			 * non-directeed scan.
			 */
			assoc_params.bssid = (ETHER_ISNULLADDR(&cfg->BSSID)) ?
				ether_bcast:cfg->BSSID;
			assoc_params.chanspec_num = 1;
			assoc_params.chanspec_list[0] = newchspec;
			wlc_reassoc(cfg, &assoc_params);
		}
#endif /* STA */
	}

#ifdef WLMCHAN
out:
#endif

#if defined(STA) && defined(WLOFFLD)
	if (offload_state) {
		wlc_ol_enable(wlc->ol, cfg);
	}
#endif
	return;
}

/* Decode the WMM 2.1.1 QoS Info field, as sent by WMM STA, and save in scb */
void
wlc_qosinfo_update(struct scb *scb, uint8 qosinfo)
{
	/* Nybble swap; QoS Info bits are in backward order from AC bitmap */
	static const ac_bitmap_t qi_bitmap_to_ac_bitmap[16] = {
		0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15
	};
	/* Max USP Length field decoding (WMM 2.2.1) */
	static const uint16 maxsp[4] = { WLC_APSD_USP_UNB, 2, 4, 6 };

	scb->apsd.maxsplen =
	        maxsp[(qosinfo & WME_QI_STA_MAXSPLEN_MASK) >> WME_QI_STA_MAXSPLEN_SHIFT];

	/* ACs negotiated to be APSD-enabled during association. */
	scb->apsd.ac_defl = qi_bitmap_to_ac_bitmap[qosinfo & WME_QI_STA_APSD_ALL_MASK];

	/* Initial config; can be updated by TSPECs, and revert back to ac_defl on DELTS. */
	scb->apsd.ac_trig = scb->apsd.ac_defl;
	scb->apsd.ac_delv = scb->apsd.ac_defl;
}

#ifdef STA
/* Update beacon listen interval in shared memory */
void
wlc_bcn_li_upd(wlc_info_t *wlc)
{
	/* wake up every DTIM is the default */
	if (wlc->bcn_li_dtim == 1)
		wlc_write_shm(wlc, M_BCN_LI, 0);
	else
		wlc_write_shm(wlc, M_BCN_LI, (wlc->bcn_li_dtim << 8) | wlc->bcn_li_bcn);
}
#endif /* STA */

void
wlc_enable_probe_req(wlc_info_t *wlc, uint32 mask, uint32 val)
{

	wlc->prb_req_enable_mask = (wlc->prb_req_enable_mask & ~mask) | (val & mask);

	if (!wlc->pub->up) {
#ifndef ATE_BUILD
		WL_ERROR(("wl%d: %s: state down, deferring setting of host flags\n",
			wlc->pub->unit, __FUNCTION__));
#endif
		return;
	}

	if (wlc->prb_req_enable_mask)
		wlc_mhf(wlc, MHF5, MHF5_SUPPRESS_PRB_REQ, 0, WLC_BAND_ALL);
	else
		wlc_mhf(wlc, MHF5, MHF5_SUPPRESS_PRB_REQ, MHF5_SUPPRESS_PRB_REQ, WLC_BAND_ALL);
}

static void
wlc_config_ucode_probe_resp(wlc_info_t *wlc)
{
	if (wlc->prb_rsp_disable_mask) {
		/* disable ucode ProbeResp */
		wlc_mhf(wlc, MHF2, MHF2_DISABLE_PRB_RESP,
			MHF2_DISABLE_PRB_RESP, WLC_BAND_ALL);
	} else {
		/* enable ucode ProbeResp */
		wlc_mhf(wlc, MHF2, MHF2_DISABLE_PRB_RESP,
			0, WLC_BAND_ALL);
	}
}

void
wlc_disable_probe_resp(wlc_info_t *wlc, uint32 mask, uint32 val)
{
	bool disable = (wlc->prb_rsp_disable_mask != 0);
	wlc->prb_rsp_disable_mask = (wlc->prb_rsp_disable_mask & ~mask) | (val & mask);

	/* deferring setting in wlc_up */
	if (!wlc->pub->up) {
		return;
	}

	if (disable != (wlc->prb_rsp_disable_mask != 0))
		wlc_config_ucode_probe_resp(wlc);
}

chanspec_t
wlc_get_cur_wider_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_bsscfg_t	*cfg;
	int idx;
	chanspec_t	chspec = bsscfg->current_bss->chanspec;

	FOREACH_BSS(wlc, idx, cfg) {
		if (cfg->associated || (BSSCFG_AP(cfg) && cfg->up) ||
#ifdef WLTDLS
			(BSS_TDLS_ENAB(wlc, cfg) && BSS_TDLS_AS(wlc, cfg)) ||
#endif /* WLTDLS */
			FALSE) {

#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !wlc_mchan_shared_chanctx(wlc->mchan, bsscfg, cfg)) {
				continue;
			}
#endif /* WLMCHAN */
			/* check for highest operating bandwidth */
			if (CHSPEC_BW(cfg->current_bss->chanspec) >
				CHSPEC_BW(chspec)) {
				chspec = cfg->current_bss->chanspec;
			}
		}
	}
	return chspec;
}

void
wlc_update_bandwidth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	chanspec_t new_chspec)
{
	if (!new_chspec || new_chspec == INVCHANSPEC) {
		return;
	}

	wlc_bsscfg_set_current_bss_chan(bsscfg, new_chspec);

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub) &&
	    !wlc_mchan_has_chanctx(wlc->mchan, bsscfg)) {
		WL_ERROR(("wl%d: %s: MCHAN Enabled but bsscfg is without MCHAN context \r\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}
#endif /* WLMCHAN */

	new_chspec = wlc_get_cur_wider_chanspec(wlc, bsscfg);

	/* if current phy chanspec is not the higest oper. bw
	 * update the phy chanspec
	 */
	if (wlc->chanspec != new_chspec) {
#ifdef WLMCHAN
		if (MCHAN_ENAB(wlc->pub)) {
			wlc_mchan_update_bss_chan_context(wlc, bsscfg, new_chspec, FALSE);
		}
#endif /* WLMCHAN */
		wlc->home_chanspec = new_chspec;

		wlc_suspend_mac_and_wait(wlc);
		wlc_set_chanspec(wlc, new_chspec);
		wlc_enable_mac(wlc);

		/* Updated the PHY chanspec, calibrate the channel */
		wlc_full_phy_cal(wlc, bsscfg, PHY_PERICAL_UP_BSS);
	}

	wlc_scb_ratesel_init_bss(wlc, bsscfg);
}

chanspec_t
wlc_max_valid_bw_chanspec(wlc_info_t *wlc, wlcband_t *band, wlc_bsscfg_t *cfg, chanspec_t chanspec)
{
	WL_INFORM(("wl%d: %s: given chanspec: 0x%x", wlc->pub->unit, __FUNCTION__, chanspec));

	/* Sanitize user setting for 80MHz against current settings
	 * Reduce an 80MHz chanspec to 40MHz if needed.
	 */
	if (CHSPEC_IS8080_UNCOND(chanspec) &&
		!VALID_8080CHANSPEC(wlc, chanspec)) {
		/* select the 80MHz primary channel in case80 is allowed */
		chanspec = wf_chspec_primary80_chspec(chanspec);
	}
	if (CHSPEC_IS160_UNCOND(chanspec) &&
		!VALID_160CHANSPEC(wlc, chanspec)) {
		/* select the 80MHz primary channel in case80 is allowed */
		chanspec = wf_chspec_primary80_chspec(chanspec);
	}
	if (CHSPEC_IS80_UNCOND(chanspec) &&
	    !VALID_80CHANSPEC(wlc, chanspec)) {
		/* select the 40MHz primary channel in case 40 is allowed */
		chanspec = wf_chspec_primary40_chspec(chanspec);
	}

	/*
	 * Sanitize chanspec for 40MHz against current settings
	 * (not in NMODE or the locale does not allow 40MHz
	 * or the band is not configured for 40MHz operation).
	 * Note that the unconditional version of the CHSPEC_IS40 is used so that
	 * code compiled without MIMO support will still recognize and convert
	 * a 40MHz chanspec.
	 */
	if (CHSPEC_IS40_UNCOND(chanspec) &&
	    (!N_ENAB(wlc->pub) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) &
	      WLC_NO_40MHZ) || !WL_BW_CAP_40MHZ(band->bw_cap) ||
	     (BAND_2G(band->bandtype) && WLC_INTOL40_DET(wlc, cfg)) ||
	     !wlc_valid_chanspec_db(wlc->cmi, chanspec)))
	{
		chanspec = wf_chspec_ctlchspec(chanspec);
	}

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		WL_ERROR(("wl%d: %s: chanspec 0x%x is invalid",
			wlc->pub->unit, __FUNCTION__, chanspec));
		return INVCHANSPEC;
	}

	WL_INFORM(("wl%d: %s: returning chanspec 0x%x", wlc->pub->unit, __FUNCTION__, chanspec));
	return chanspec;
}

static const struct {
	int8 rate;
	int8 pwr;
} nominal_req_pwr_table[] = {
	{WLC_RATE_1M, -80},
	{WLC_RATE_2M, -80},
	{WLC_RATE_5M5, -76},
	{WLC_RATE_11M, -76},
	{WLC_RATE_6M, -82},
	{WLC_RATE_9M, -81},
	{WLC_RATE_12M, -79},
	{WLC_RATE_18M, -77},
	{WLC_RATE_24M, -74},
	{WLC_RATE_36M, -70},
	{WLC_RATE_48M, -66},
	{WLC_RATE_54M, -65},
	{0, -100}
};

int
wlc_find_nominal_req_pwr(ratespec_t rspec)
{
	int i;
	int rate;
	int nominal_req_pwr;

	if (RSPEC_ISLEGACY(rspec)) {
		rate = RSPEC2RATE(rspec);
	} else {
		rate = WLC_RATE_6M;
	}

	/* Find the nominal required power for the request packet's rate */
	i = 0;
	nominal_req_pwr = 0;
	while (nominal_req_pwr_table[i].rate != 0) {
		if (nominal_req_pwr_table[i].rate == rate) {
			nominal_req_pwr = nominal_req_pwr_table[i].pwr;
			break;
		}
		i++;
	}

	return nominal_req_pwr;
}

#if defined(BCM_DCS) && defined(STA)
static int
wlc_send_action_brcm_dcs(wlc_info_t *wlc, wl_bcmdcs_data_t *data, struct scb *scb)
{
	void *p;
	uint8 *pbody;
	uint body_len;
	dot11_action_vs_frmhdr_t *action_vs_hdr;
	uint32 reason;
	chanspec_t channel_spec;

	if (!scb) {
		WL_ERROR(("wl%d: %s: scb = NULL\n", wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	body_len = DOT11_ACTION_VS_HDR_LEN + sizeof(wl_bcmdcs_data_t);

	if ((p = wlc_frame_get_mgmt(wlc, FC_ACTION, &scb->ea, &scb->bsscfg->cur_etheraddr,
		&scb->bsscfg->BSSID, body_len, &pbody)) == NULL)
		return BCME_NOMEM;

	action_vs_hdr = (dot11_action_vs_frmhdr_t *)pbody;

	action_vs_hdr->category  = DOT11_ACTION_CAT_VS;
	action_vs_hdr->OUI[0]    = BCM_ACTION_OUI_BYTE0;
	action_vs_hdr->OUI[1]    = BCM_ACTION_OUI_BYTE1;
	action_vs_hdr->OUI[2]    = BCM_ACTION_OUI_BYTE2;
	action_vs_hdr->type      = BCM_ACTION_RFAWARE;
	action_vs_hdr->subtype   = BCM_ACTION_RFAWARE_DCS;

	reason = htol32(data->reason);
	bcopy(&reason, &action_vs_hdr->data[0], sizeof(uint32));
	channel_spec = htol16(data->chspec);
	bcopy(&channel_spec, &action_vs_hdr->data[4], sizeof(chanspec_t));

#ifdef BCMDBG
	{
	char da[ETHER_ADDR_STR_LEN];
	WL_INFORM(("wl%d: %s: send action frame to %s\n", wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, da)));
	}
#endif
	wlc_sendmgmt(wlc, p, SCB_WLCIFP(scb)->qi, scb);

	return 0;
}

#endif /* BCM_DCS && STA */

void
wlc_send_action_err(wlc_info_t *wlc, struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	void *p;
	uint8* pbody;
	uint action_category;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */
	wlc_bsscfg_t *bsscfg = wlc->cfg;

	if (ETHER_ISMULTI(&hdr->da)) {
		WL_INFORM(("wl%d: %s: suppressing error reply to %s",
			wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
		WL_INFORM((" due to multicast address %s\n",
			bcm_ether_ntoa(&hdr->da, eabuf)));
		return;
	}

	action_category = (uint)body[0];

	if (body_len < 2) {
		WL_INFORM(("wl%d: %s: sending error reply to %s for Category %d "
		           " with missing Action field\n", wlc->pub->unit, __FUNCTION__,
		           bcm_ether_ntoa(&hdr->sa, eabuf), action_category));
	} else {
		WL_INFORM(("wl%d: %s: sending error reply to %s for Category/Action %d/%d\n",
		           wlc->pub->unit, __FUNCTION__,
		           bcm_ether_ntoa(&hdr->sa, eabuf), action_category, (int)body[1]));
	}

	action_category |= DOT11_ACTION_CAT_ERR_MASK;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, &hdr->sa, &bsscfg->cur_etheraddr, &bsscfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_INFORM(("wl%d: %s: no memory for Action Error\n",
		           wlc->pub->unit, __FUNCTION__));
		return;
	}

	bcopy(body, pbody, body_len);
	pbody[0] = (action_category | DOT11_ACTION_CAT_ERR_MASK);

	wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, NULL);
}

void
wlc_bss_list_free(wlc_info_t *wlc, wlc_bss_list_t *bss_list)
{
	uint indx;
	wlc_bss_info_t *bi;

	/* inspect all BSS descriptor */
	for (indx = 0; indx < bss_list->count; indx++) {
		bi = bss_list->ptrs[indx];
		if (bi) {
			if (bi->bcn_prb) {
				MFREE(wlc->osh, bi->bcn_prb, bi->bcn_prb_len);
			}
			MFREE(wlc->osh, bi, sizeof(wlc_bss_info_t));
			bss_list->ptrs[indx] = NULL;
		}
	}
	bss_list->count = 0;
}

void
wlc_bss_list_xfer(wlc_bss_list_t *from, wlc_bss_list_t *to)
{
	uint size;

	ASSERT(to->count == 0);
	size = from->count * sizeof(wlc_bss_info_t*);
	bcopy((char*)from->ptrs, (char*)to->ptrs, size);
	bzero((char*)from->ptrs, size);

	to->count = from->count;
	from->count = 0;
}

#if defined(PKTC_DONGLE) && defined(WLAMSDU)
static bool
wlc_pktc_dongle_rx_chainable(wlc_info_t *wlc, struct scb *scb, uint16 fc)
{
	if (WLEXTSTA_ENAB(wlc->pub) || MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
		return FALSE;

	/* Disable this for IBSS & WDS now */
	if (((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS)) ||
		((fc & (FC_TODS | FC_FROMDS)) == 0))
		return FALSE;

	if (!((scb->bsscfg->pm->PM == PM_OFF) || (scb->bsscfg->pm->PM == PM_FAST)))
		return FALSE;


	return TRUE;
}
#else /* PKTC_DONGLE & WLAMSDU */
#define wlc_pktc_dongle_rx_chainable(wlc, scb, fc)	TRUE
#endif /* PKTC_DONGLE & WLAMSDU */

#if defined(STA) && defined(PKTC_DONGLE)
static int32
wlc_pktc_wsec_process(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, void *p, d11rxhdr_t *rxh,
                      struct dot11_header *h, struct ether_header *eh, uint32 *tsf_l,
                      uint16 body_offset)
{
	if (wlc_keymgmt_b4m4_enabled(wlc->keymgmt, bsscfg) &&
	    (ntoh16(eh->ether_type) == ETHER_TYPE_802_1X)) {
		if (wlc_is_4way_msg(wlc, p, ETHER_HDR_LEN - DOT11_LLC_SNAP_HDR_LEN, PMSG1)) {
			WL_WSEC(("wl%d: %s: allowing unencrypted 802.1x 4-Way M1\n",
				wlc->pub->unit, __FUNCTION__));
			wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_M1_RX, bsscfg,
				NULL, NULL, NULL);
		}
	}

	if (BSSCFG_STA(bsscfg)) {
#ifdef WL11K
		if (WL11K_ENAB(wlc->pub)) {
			wlc_rrm_upd_data_activity_ts(wlc->rrm_info);
		}
#endif /* WL11K */
		wlc_pm2_ret_upd_last_wake_time(bsscfg, tsf_l);
		if (bsscfg->pm->dfrts_logic != WL_DFRTS_LOGIC_OFF)
			wlc_update_sleep_ret(bsscfg, TRUE, FALSE,
				pkttotlen(wlc->osh, p), 0);
	}

#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
	if (SUP_ENAB(wlc->pub) &&
		(BSS_SUP_TYPE(wlc->idsup, bsscfg) != SUP_UNUSED) &&
		(ntoh16(eh->ether_type) == ETHER_TYPE_802_1X)) {

		eapol_header_t *eapol_hdr = (eapol_header_t *)eh;

		/* ensure that we have a valid eapol - header/length */
		if (!wlc_valid_pkt_field(wlc->osh, p, (void*)eapol_hdr,
			OFFSETOF(eapol_header_t, body)) ||
			!wlc_valid_pkt_field(wlc->osh, p, (void*)&eapol_hdr->body[0],
			(int)ntoh16(eapol_hdr->length))) {
			WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
				wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return BCME_ERROR;
		}

		if (wlc_sup_eapol(wlc->idsup, bsscfg,
			eapol_hdr, ((h->fc & htol16(FC_WEP)) != 0)))
			return BCME_ERROR;
	}
#endif 


	return BCME_OK;
}
#else	/* STA && PKTC_DONGLE */
#define wlc_pktc_wsec_process(wlc, bsscfg, p, rxh, h, eh, tsf_l, body_offset) BCME_OK
#endif	/* STA && PKTC_DONGLE */

#if defined(PKTC) || defined(PKTC_DONGLE)
static int32 BCMFASTPATH
wlc_wsec_recv(wlc_info_t *wlc, struct scb *scb, void *p, d11rxhdr_t *rxh, struct dot11_header *h,
              wlc_rfc_t *rfc, uint16 *body_offset)
{
	struct wlc_frminfo f;
	osl_t *osh = wlc->osh;
	int err = BCME_OK;

	f.p = p;
	f.h = h;
	f.fc = ltoh16(h->fc);

	(void)osh;

	f.rx_wep = f.fc & FC_WEP;
	ASSERT(f.rx_wep);

	f.pbody = (uint8*)h + *body_offset;
	f.len = PKTLEN(osh, p);
	f.body_len = f.len - *body_offset;
	f.ismulti = FALSE;
	rxh->RxStatus1 = ltoh16(rxh->RxStatus1);
	rxh->RxStatus2 = ltoh16(rxh->RxStatus2);
	f.rxh = rxh;
	f.WPA_auth = scb->WPA_auth;
	f.isamsdu = (*(uint16 *)(f.pbody - 2) & htol16(QOS_AMSDU_MASK)) != 0;
	f.prio = rfc->prio;

	WL_NONE(("wl%d: %s: RxStatus1 0x%x\n", wlc->pub->unit, __FUNCTION__, rxh->RxStatus1));

	/* the key, if any, will be provided by wlc_keymgmt_recvdata */
	if ((h->fc & htol16(FC_TODS)) == 0) {
		f.sa = &h->a3;
		f.da = &h->a1;
	} else {
		f.sa = &h->a2;
		f.da = &h->a3;
	}

	/* the key, if any, will be provided by wsec_recvdata */
	f.key = NULL;
	if (!wlc_keymgmt_recvdata(wlc->keymgmt, &f)) {
		WLCNTSCB_COND_INCR((f.rx_wep), scb->scb_stats.rx_decrypt_failures);
		return BCME_ERROR;
	}
	WLCNTSCB_COND_INCR((f.rx_wep), scb->scb_stats.rx_decrypt_succeeds);

	/* lose  iv from body */
	*body_offset = f.pbody - (uint8 *)f.h;

	if (f.rx_wep) {
		struct ether_header eh, *ehp;
		uint push_len;

		ASSERT(f.key != NULL);
		memcpy(&eh, f.pbody - sizeof(eh), sizeof(eh)); /* save data from pkt */

		push_len = *body_offset - ETHER_HDR_LEN;
		ehp = (struct ether_header*)PKTPULL(osh, f.p, push_len);
		eacopy((char*)(f.da), ehp->ether_dhost);
		eacopy((char*)(f.sa), ehp->ether_shost);
		ehp->ether_type = hton16((uint16)f.body_len);

		err = wlc_key_rx_msdu(f.key, f.p, f.rxh);
		if (err != BCME_OK)
			goto done;

		memcpy(ehp, &eh, sizeof(eh));
		PKTPUSH(osh, f.p, push_len);
	}

done:
	WL_NONE(("wl%d: %s: f.key %p\n", wlc->pub->unit, __FUNCTION__, f.key));
	return err;
}

#define PKTC_MIN_FRMLEN	(D11_PHY_HDR_LEN + DOT11_A3_HDR_LEN + DOT11_QOS_LEN + DOT11_FCS_LEN)

bool BCMFASTPATH
wlc_rxframe_chainable(wlc_info_t *wlc, void **pp, uint16 index)
{
#ifdef WLAMSDU
	void *p = *pp;
	d11rxhdr_t *rxh = (d11rxhdr_t *)PKTDATA(wlc->osh, p);
	struct dot11_header *h;
	uint16 fc, prio = 0, seq, pad = 0;
	wlc_rfc_t *rfc;
	struct scb *scb;
	bool amsdu, chainable = TRUE;
	uchar *pbody;

	pad = ((rxh->RxStatus1 & htol16(RXS_PBPRES)) ? 2 : 0);
	amsdu = ((rxh->RxStatus2 & htol16(RXS_AMSDU_MASK)) != 0);

	if (amsdu) {
		if (!wlc->_rx_amsdu_in_ampdu)
			return FALSE;

		/* Remove rx header */
		PKTPULL(wlc->osh, p, wlc->hwrxoff + pad);
		rxh->RxStatus2 = ltoh16(rxh->RxStatus2);

		/* check, qualify and chain MSDUs */
		*pp = wlc_recvamsdu(wlc->ami, (wlc_d11rxhdr_t *)rxh, p, TRUE);
		rxh->RxStatus2 = htol16(rxh->RxStatus2);

		if (*pp != NULL) {
			/* amsdu deagg successful */
			p = *pp;
			rxh = (d11rxhdr_t *)PKTPUSH(wlc->osh, p, wlc->hwrxoff + pad);
		} else {
			/* non-tail or out-of-order sub-frames */
			return TRUE;
		}
	} else
		wlc_amsdu_flush(wlc->ami);

	h = (struct dot11_header *)(PKTDATA(wlc->osh, p) + wlc->hwrxoff + pad + D11_PHY_HDR_LEN);

	if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &h->a2))
		return FALSE;
	fc = ltoh16(h->fc);
	seq = ltoh16(h->seq);

	if ((FC_TYPE(fc) != FC_TYPE_DATA) ||
	    (PKTLEN(wlc->osh, p) - wlc->hwrxoff <= PKTC_MIN_FRMLEN + pad))
		return FALSE;

	rfc = &wlc->pktc_info->rfc[(fc & FC_TODS) >> FC_TODS_SHIFT];
	scb = rfc->scb;
	/* if chaining is not allowed for rfc scb return false */
	if ((scb == NULL) || !SCB_PKTC_ENABLED(scb) || (scb->pktc_pps <= 16))
		return FALSE;

	if (!wlc_pktc_dongle_rx_chainable(wlc, scb, fc))
		return FALSE;

	if (!amsdu) {
		uint8 *da;
#ifdef PSTA
		da = (uchar *)(BSSCFG_STA(rfc->bsscfg) ?
		              (PSTA_IS_REPEATER(wlc) ? &rfc->ds_ea : &h->a1) : &h->a3);
#else
		da = (uchar *)(BSSCFG_STA(rfc->bsscfg) ? &h->a1 : &h->a3);
#endif
		if (ETHER_ISNULLDEST(da))
			return FALSE;

		/* Init DA for the first valid packet of the chain */
		if (wlc->pktc_info->h_da == NULL)
			wlc->pktc_info->h_da = (struct ether_addr *)da;

#if defined(BCM_GMAC3)
		chainable = !ETHER_ISMULTI(da) &&
#else  /* ! BCM_GMAC3 */
		chainable = TRUE &&
#ifdef HNDCTF
			CTF_HOTBRC_CMP(wlc->pub->brc_hot, da, rfc->wlif_dev) &&
#else
			!ETHER_ISMULTI(da) &&
#endif /* ! HNDCTF */
#endif /* ! BCM_GMAC3 */
			!eacmp(wlc->pktc_info->h_da, da);
	}

	pbody = (uchar*)h + DOT11_A3_HDR_LEN;

	chainable = (chainable && !eacmp(&h->a1, &rfc->bsscfg->cur_etheraddr) &&
	             !eacmp(&h->a2, &scb->ea));

	chainable = (chainable &&
	             (FC_SUBTYPE_ANY_QOS((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT) &&
	             (prio = QOS_PRIO(ltoh16_ua(pbody)), (prio == rfc->prio)) &&
	             (!FC_SUBTYPE_ANY_NULL((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT)) &&
	             ((seq & FRAGNUM_MASK) == 0) && ((fc & FC_MOREFRAG) == 0) &&
	             (((fc & FC_RETRY) == 0) || ((scb->seqctl[prio] >> 4) + index != (seq >> 4)))));

	if (chainable && wlc_ampdu_chainable(wlc->ampdu_rx, p, scb, seq, prio))
		return TRUE;

	WL_NONE(("wl%d: %s: pkt %p not chainable\n", wlc->pub->unit, __FUNCTION__, p));
	WL_NONE(("wl%d: %s: prio %d rfc seq %d pkt seq %d index %d\n",  wlc->pub->unit,
	         __FUNCTION__, prio, scb->seqctl[prio] >> 4, seq >> 4, index));
#endif /* WLAMSDU */
	return FALSE;
}

void BCMFASTPATH
wlc_sendup_chain(wlc_info_t *wlc, void *head)
{
	struct dot11_llc_snap_header *lsh;
	struct ether_header *eh = NULL;
	struct dot11_header *h;
	wlc_d11rxhdr_t *wrxh;
	d11rxhdr_t *rxh;
	ratespec_t rspec;
	uint16 body_offset, count = 0;
	uint pkt_len, byte_count = 0;
	wlc_rfc_t *rfc = NULL;
	struct scb *scb = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	void *temp, *next, *p = head, *prev = NULL;
	uint8 *plcp;
	bool chained_sendup;
#ifdef PKTC_DONGLE
	uint32 tsf_l, tsf_h;

	wlc_bmac_read_tsf(wlc->hw, &tsf_l, &tsf_h);
#endif
#ifdef PKTC_INTRABSS
	bool intrabss_fwd = FALSE;
#endif

	/* Clear DA for pktc */
	wlc->pktc_info->h_da = NULL;

	chained_sendup = (wlc->pktc_info->policy == PKTC_POLICY_SENDUPC);

	while (p != NULL) {
		wrxh = (wlc_d11rxhdr_t *)PKTDATA(wlc->osh, p);
#ifdef PKTC_DONGLE
		wrxh->tsf_l = htol32(tsf_l);
#endif
		rxh = &wrxh->rxhdr;
		plcp = PKTPULL(wlc->osh, p, wlc->hwrxoff +
		               ((rxh->RxStatus1 & htol16(RXS_PBPRES)) ? 2 : 0));
		rspec = wlc_recv_compute_rspec(wrxh, plcp);
		h = (struct dot11_header *)PKTPULL(wlc->osh, p, D11_PHY_HDR_LEN);
		body_offset = DOT11_A3_HDR_LEN + DOT11_QOS_LEN;
		pkt_len = pkttotlen(wlc->osh, p);

		if ((rxh->RxStatus2 & htol16(RXS_AMSDU_MASK)) == 0) {
			/* For split RX case, FCS bytes are in host */
			/* drop length of host fragment */
			PKTFRAG_TRIM_TAILBYTES(wlc->osh, p, DOT11_FCS_LEN);
		} else {
			ASSERT(WLPKTTAG(p)->flags & WLF_HWAMSDU);
		}

		rfc = &wlc->pktc_info->rfc[(h->fc & htol16(FC_TODS)) ? 1 : 0];
		scb = rfc->scb;
		WLPKTTAGSCBSET(p, scb);
		ASSERT(scb != NULL);
		ASSERT(!SCB_A4_DATA(scb));
		bsscfg = SCB_BSSCFG(scb);

		/* update per-scb stat counters */
		WLCNTSCBINCR(scb->scb_stats.rx_ucast_pkts);
		WLCNTSCBADD(scb->scb_stats.rx_ucast_bytes, pkt_len);
#ifdef WL11N
		/* for first MPDU in an A-MPDU collect ratesel stats for antsel */
		rxh->PhyRxStatus_0 = ltoh16(rxh->PhyRxStatus_0);
		if (plcp[0] | plcp[1] | plcp[2]) {
			wlc_scb_ratesel_upd_rxstats(wlc->wrsi, rspec, ltoh16(rxh->RxStatus2));
			WLCNTSCBSET(scb->scb_stats.rx_rate, rspec);
		}
#else
		WLCNTSCBSET(scb->scb_stats.rx_rate, rspec);
#endif

#ifdef PKTC_DONGLE
		/* for pktc dongle only */
		bsscfg->roam->time_since_bcn = 0;

		/* compute the RSSI from d11rxhdr and record it in wlc_rxd11hr */
		phy_rssi_compute_rssi((phy_info_t *)wlc->hw->band->pi, wrxh);

		if (BSSCFG_STA(bsscfg)) {
			int8 rssi;
			int snr = WLC_SNR_INVALID;

			if ((rssi = wlc_lq_rssi_pktrxh_cal(wlc, wrxh)) != WLC_RSSI_INVALID) {
				wlc_lq_rssi_update_ma(bsscfg, rssi, wrxh->rssi_qdb, TRUE);
				snr = wlc_lq_recv_snr_compute(wlc, wrxh, wlc->noise_lvl);
				wlc_lq_snr_update_ma(bsscfg, snr, TRUE);
			}

			/* update rssi & snr */
			WLPKTTAG(p)->pktinfo.misc.rssi = rssi;
			WLPKTTAG(p)->pktinfo.misc.snr = (int8)snr;
		}
#endif /* PKTC_DONGLE */

		if ((h->fc & htol16(FC_WEP)) ||
		    (WSEC_ENABLED(bsscfg->wsec) && bsscfg->wsec_restrict)) {
			if (wlc_wsec_recv(wlc, scb, p, rxh, h, rfc, &body_offset) != BCME_OK) {
				/* remove the packet from chain */
				goto toss;
			}
		}

		/* update cached seqctl */
		scb->seqctl[rfc->prio] = ltoh16(h->seq);

		next = PKTCLINK(p);

		/* WME: set packet priority based on first received MPDU */
		PKTSETPRIO(p, rfc->prio);

		wlc_ampdu_update_rxcounters(wlc->ampdu_rx,
		                            rxh->PhyRxStatus_0 & PRXS0_FT_MASK, scb, h);

#ifdef MCAST_REGEN
		/* apply WMF reverse translation only the STA side */
		if (chained_sendup && MCAST_REGEN_ENAB(bsscfg) && BSSCFG_STA(bsscfg))
			chained_sendup = FALSE;
#endif

		if (WLPKTTAG(p)->flags & WLF_HWAMSDU) {
			PKTPULL(wlc->osh, p, body_offset);
#if defined(PKTC) || defined(PKTC_DONGLE)
			if (wlc_amsdu_pktc_deagg_hw(wlc->ami, &p, rfc,
			                            &count, &chained_sendup) != BCME_OK)
				goto toss;
#endif
		} else

		{
			/* 802.11 -> 802.3/Ethernet header conversion */
			lsh = (struct dot11_llc_snap_header *)
			      (PKTDATA(wlc->osh, p) + body_offset);
			if (lsh->dsap == 0xaa && lsh->ssap == 0xaa && lsh->ctl == 0x03 &&
			    lsh->oui[0] == 0 && lsh->oui[1] == 0 &&
			    ((lsh->oui[2] == 0x00 && !sstlookup(wlc, ntoh16(lsh->type))) ||
			     (lsh->oui[2] == 0xf8 && sstlookup(wlc, ntoh16(lsh->type))))) {
				/* RFC894 */
				eh = (struct ether_header *)PKTPULL(wlc->osh, p,
					body_offset + DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);
			} else {
				/* RFC1042 */
				uint body_len = PKTLEN(wlc->osh, p) - body_offset;
				eh = (struct ether_header *)PKTPULL(wlc->osh, p,
					body_offset - ETHER_HDR_LEN);
				eh->ether_type = HTON16((uint16)body_len);
			}

			if ((h->fc & htol16(FC_TODS)) == 0) {
				ether_copy(&h->a3, eh->ether_shost);
				ether_rcopy(&h->a1, eh->ether_dhost);
			} else {
				ether_copy(&h->a2, eh->ether_shost);
				ether_rcopy(&h->a3, eh->ether_dhost);
			}

#ifdef WLTDLS
			if (TDLS_ENAB(wlc->pub) && BSSCFG_STA(bsscfg) &&
				eh->ether_type == HTON16(ETHER_TYPE_89_0D)) {
				void *tmp_pkt = PKTDUP(wlc->osh, p);

				if (tmp_pkt) {
					struct wlc_frminfo f;
					f.p = tmp_pkt;
					f.sa = (struct ether_addr *)&eh->ether_shost;
					f.da = (struct ether_addr *)&eh->ether_dhost;

					wlc_tdls_rcv_action_frame(wlc->tdls, scb, &f,
						ETHER_HDR_LEN);

					goto toss;
				}
			}
#endif /* WLTDLS */

			if (chained_sendup)
				chained_sendup = ((eh->ether_type == HTON16(ETHER_TYPE_IP)) ||
				                  (eh->ether_type == HTON16(ETHER_TYPE_IPV6)));

			if (wlc_pktc_wsec_process(wlc, bsscfg, p, rxh, h, eh,
			                          &tsf_l, body_offset) != BCME_OK)
				goto toss;

			PKTCADDLEN(head, PKTLEN(wlc->osh, p));
			PKTSETCHAINED(wlc->osh, p);
			count++;
			byte_count += pkt_len;
		}

		prev = p;
		p = next;
		continue;
	toss:
		temp = p;
		if (prev != NULL)
			PKTSETCLINK(prev, PKTCLINK(p));
		else
			head = PKTCLINK(p);
		p = PKTCLINK(p);
		PKTSETCLINK(temp, NULL);
		PKTCLRCHAINED(wlc->osh, temp);
		WLCNTINCR(wlc->pub->_wme_cnt->rx_failed[rfc->ac].packets);
		WLCNTADD(wlc->pub->_wme_cnt->rx_failed[rfc->ac].bytes,
			pkttotlen(wlc->osh, temp));
		PKTFREE(wlc->osh, temp, FALSE);
		/* empty chain, return */
		if (head == NULL)
			return;

	}

#ifdef PKTC_INTRABSS
	if (BSSCFG_AP(bsscfg) && !bsscfg->ap_isolate) {
		struct scb *dst;
		eh = (struct ether_header *)PKTDATA(wlc->osh, head);
		if (eacmp(eh->ether_dhost, wlc->pub->cur_etheraddr.octet) &&
			(dst = wlc_scbfind(wlc, bsscfg, (struct ether_addr *)eh->ether_dhost))) {
			if (SCB_ASSOCIATED(dst)) {
				intrabss_fwd = TRUE;
				chained_sendup = FALSE;
			}
		}
	}
#endif /* PKTC_INTRABSS */

	if (chained_sendup) {
		WLCNTSET(wlc->pub->_cnt->currchainsz, count);
		WLCNTSET(wlc->pub->_cnt->maxchainsz,
		         MAX(count, wlc->pub->_cnt->maxchainsz));

		/* update stat counters */
		WLCNTADD(wlc->pub->_cnt->rxframe, count);
		WLCNTADD(wlc->pub->_cnt->rxbyte, byte_count);
		WLCNTADD(wlc->pub->_wme_cnt->rx[rfc->ac].packets, count);
		WLCNTADD(wlc->pub->_wme_cnt->rx[rfc->ac].bytes, byte_count);
		WLPWRSAVERXFADD(wlc, count);

		/* update interface stat counters */
		WLCNTADD(bsscfg->wlcif->_cnt.rxframe, count);
		WLCNTADD(bsscfg->wlcif->_cnt.rxbyte, byte_count);

		scb->used = wlc->pub->now;

#ifdef WLWNM_AP
		/* update rx time stamp for BSS Max Idle Period */
		if (BSSCFG_AP(bsscfg) && WLWNM_ENAB(wlc->pub) && scb && SCB_ASSOCIATED(scb))
			wlc_wnm_rx_tstamp_update(wlc, scb);
#endif

#ifdef PSTA
		/* restore the original src addr before sending up */
		if (BSSCFG_PSTA(bsscfg)) {
			eh = (struct ether_header *)PKTDATA(wlc->osh, head);
			wlc_psta_recv_proc(wlc->psta, head, eh, &bsscfg);
			wlc_bsscfg_primary(wlc)->roam->time_since_bcn = 0;
		}
#endif

		PKTCSETCNT(head, count);
		WLCNTADD(wlc->pub->_cnt->chained, count);

		wl_sendup(wlc->wl, bsscfg->wlcif->wlif, head, 1);
	} else {
		void *n;
		FOREACH_CHAINED_PKT(head, n) {
			PKTCLRCHAINED(wlc->osh, head);
			PKTCCLRFLAGS(head);
			WLCNTINCR(wlc->pub->_cnt->chainedsz1);

#ifdef PKTC_INTRABSS
			if (intrabss_fwd) {
				if (WME_ENAB(wlc->pub)) {
					wl_traffic_stats_t *forward =
					&wlc->pub->_wme_cnt->forward[WME_PRIO2AC(PKTPRIO(head))];
					WLCNTINCR(forward->packets);
					WLCNTADD(forward->bytes, pkttotlen(wlc->osh, head));
				}
				wlc_sendpkt(wlc, head, bsscfg->wlcif);
			} else
#endif /* PKTC_INTRABSS */
			{
				wlc_recvdata_sendup(wlc, WLPKTTAGSCBGET(head), FALSE,
					(struct ether_addr *)PKTDATA(wlc->osh, head), head);
			}
		}
	}
}
#endif /* PKTC || PKTC_DONGLE */

int
wlc_wsec(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint32 val)
{
	uint32 wsec_prev;

	WL_WSEC(("wl%d: wlc_wsec: setting wsec 0x%x\n", wlc->pub->unit, val));

	wsec_prev = bsscfg->wsec;

	bsscfg->wsec = val & (WEP_ENABLED|TKIP_ENABLED|AES_ENABLED|WSEC_SWFLAG|
	                      SES_OW_ENABLED);

#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub)) {
		/* fail if result will have MFPR but MFPC clear */
		if ((val & MFP_REQUIRED) && !(val & MFP_CAPABLE)) {
			WL_WSEC(("wl%d: %s: MFPR/MFPC mismatch, old wsec: 0x%04x,"
				" attempted wsec: 0x%04x\n",
				WLCWLUNIT(wlc), __FUNCTION__, wsec_prev, val));
			bsscfg->wsec = wsec_prev;
			return (BCME_BADARG);
		}
	/* wl join triggers wsec command which resets MFP_SHA256 bit here.
	 * So it is needed to keep MFP_SHA256.
	 */
#ifdef MFP_TEST
		bsscfg->wsec |= val & (MFP_CAPABLE|MFP_REQUIRED|MFP_SHA256);
#else
		bsscfg->wsec |= val & (MFP_CAPABLE|MFP_REQUIRED);
#endif /* MFP_TEST */
	}
#else
	(void)wsec_prev;
#endif /* MFP */

#ifdef STA
	/* change of wsec may modify the PS_ALLOWED state */
	if (BSSCFG_STA(bsscfg))
		wlc_set_pmstate(bsscfg, bsscfg->pm->PMenabled);
#endif /* STA */

	if (!wlc->pub->up)
		return (0);

	wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_BSS_WSEC_CHANGED,
		bsscfg, NULL /* scb */, NULL /* key */, NULL /* pkt */);
	if (AIBSS_ENAB(wlc->pub)) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}
	return (0);
}

/* return the multicast cipher defined as the weakest
 * enabled encryption in the order: WEP, TKIP, AES
 */
uint8
wlc_wpa_mcast_cipher(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint32 wsec = cfg->wsec;
	uint8 cipher = 0;

	if (WSEC_WEP_ENABLED(wsec)) {
		wlc_key_info_t key_info;

		/* wsec does not specify WEP key size, so use the default key algo */
		wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, cfg, FALSE, &key_info);
		if (key_info.algo == CRYPTO_ALGO_OFF) {
			WL_ERROR(("wl%d.%d: %s: WEP encryption enabled but"
				" key is not plumbed\n", WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
				__FUNCTION__));
			goto done;
		}

		switch (key_info.algo) {
		case CRYPTO_ALGO_WEP128:
			cipher = WPA_CIPHER_WEP_104;
			break;
		case CRYPTO_ALGO_WEP1:
			cipher = WPA_CIPHER_WEP_40;
			break;
		default:
			WL_ERROR(("wl%d: %s: WEP encryption enabled but"
				" default key algo (%d) is not WEP1 or WEP128\n",
				wlc->pub->unit, __FUNCTION__, key_info.algo));
			cipher = WPA_CIPHER_WEP_40;
		}
	} else if (WSEC_TKIP_ENABLED(wsec)) {
		cipher = WPA_CIPHER_TKIP;
	} else if (WSEC_AES_ENABLED(wsec)) {
		cipher = WPA_CIPHER_AES_CCM;
	}

done:
	return cipher;
}

/* calculate frame duration for Mixed-mode L-SIG spoofing, return
 * number of bytes goes in the length field
 *
 * Formula given by HT PHY Spec v 1.13
 *   len = 3(nsyms + nstream + 3) - 3
 */
uint16
wlc_calc_lsig_len(wlc_info_t *wlc, ratespec_t ratespec, uint mac_len)
{
	uint nsyms, len = 0, kNdps;

	WL_TRACE(("wl%d: %s: rate %d, len%d\n", wlc->pub->unit, __FUNCTION__,
		RSPEC2RATE(ratespec), mac_len));

	if (RSPEC_ISHT(ratespec)) {
		uint mcs = ratespec & RSPEC_RATE_MASK;
		int tot_streams = wlc_ratespec_nsts(ratespec);

		ASSERT(WLC_PHY_11N_CAP(wlc->band));
		ASSERT(VALID_MCS(mcs));

		if (!VALID_MCS(mcs)) {
			mcs = 0;
		}

		/* the payload duration calculation matches that of regular ofdm */
		/* 1000Ndbps = kbps * 4 */
		kNdps = MCS_RATE(mcs, RSPEC_IS40MHZ(ratespec), RSPEC_ISSGI(ratespec)) * 4;

		if (RSPEC_ISSTBC(ratespec) == 0)
			/* NSyms = CEILING((SERVICE + 8*NBytes + TAIL) / Ndbps) */
			nsyms = CEIL((APHY_SERVICE_NBITS + 8 * mac_len + APHY_TAIL_NBITS)*1000,
			             kNdps);
		else
			/* STBC needs to have even number of symbols */
			nsyms = 2 * CEIL((APHY_SERVICE_NBITS + 8 * mac_len + APHY_TAIL_NBITS)*1000,
			                 2 * kNdps);

		nsyms += (tot_streams + 3);	/* (+3) account for HT-SIG(2) and HT-STF(1) */
		/* 3 bytes/symbol @ legacy 6Mbps rate */
		len = (3 * nsyms) - 3;	/* (-3) excluding service bits and tail bits */
	} else if (RSPEC_ISVHT(ratespec)) {
		WL_ERROR(("wl%d: %s: Need VHT L-SIG length calc\n", wlc->pub->unit, __FUNCTION__));
	}

	return (uint16)len;
}

/* calculate frame duration of a given rate and length, return time in usec unit */
uint
wlc_calc_frame_time(wlc_info_t *wlc, ratespec_t ratespec, uint8 preamble_type, uint mac_len)
{
	uint nsyms, dur = 0, Ndps;
	uint rate = RSPEC2RATE(ratespec);

	if (rate == 0) {
		ASSERT(0);
		WL_ERROR(("wl%d: WAR: using rate of 1 mbps\n", wlc->pub->unit));
		rate = WLC_RATE_1M;
	}

	WL_TRACE(("wl%d: wlc_calc_frame_time: rspec 0x%x, preamble_type %d, len%d\n",
		wlc->pub->unit, ratespec, preamble_type, mac_len));

	if (RSPEC_ISVHT(ratespec)) {
		/* this is very approximate, ucode controls retries; not acct for vht overhead */
		dur = ((mac_len + rate-1) / rate);
	} else if (RSPEC_ISHT(ratespec)) {
		dur = wlc_ht_calc_frame_time(wlc->hti, ratespec, preamble_type, mac_len);
	} else if (RSPEC_ISOFDM(rate)) {
		dur = APHY_PREAMBLE_TIME;
		dur += APHY_SIGNAL_TIME;
		/* Ndbps = Mbps * 4 = rate(500Kbps) * 2 */
		Ndps = rate*2;
		/* NSyms = CEILING((SERVICE + 8*NBytes + TAIL) / Ndbps) */
		nsyms = CEIL((APHY_SERVICE_NBITS + 8 * mac_len + APHY_TAIL_NBITS), Ndps);
		dur += APHY_SYMBOL_TIME * nsyms;
		if (BAND_2G(wlc->band->bandtype))
			dur += DOT11_OFDM_SIGNAL_EXTENSION;
	} else {
		/* calc # bits * 2 so factor of 2 in rate (1/2 mbps) will divide out */
		mac_len = mac_len * 8 * 2;
		/* calc ceiling of bits/rate = microseconds of air time */
		dur = (mac_len + rate-1) / rate;
		if (preamble_type & WLC_SHORT_PREAMBLE)
			dur += BPHY_PLCP_SHORT_TIME;
		else
			dur += BPHY_PLCP_TIME;
	}
	return dur;
}

static uint
wlc_calc_ack_time(wlc_info_t *wlc, ratespec_t rspec, uint8 preamble_type)
{
	uint dur = 0;

	WL_TRACE(("wl%d: wlc_calc_ack_time: rspec 0x%x, preamble_type %d\n", wlc->pub->unit, rspec,
		preamble_type));
	/* Spec 9.6: ack rate is the highest rate in BSSBasicRateSet that is less than
	 * or equal to the rate of the immediately previous frame in the FES
	 */
	rspec = WLC_BASIC_RATE(wlc, rspec);
	ASSERT(VALID_RATE_DBG(wlc, rspec));

	/* ACK frame len == 14 == 2(fc) + 2(dur) + 6(ra) + 4(fcs) */
	dur = wlc_calc_frame_time(wlc, rspec, preamble_type, (DOT11_ACK_LEN + DOT11_FCS_LEN));
	return dur;
}

uint
wlc_calc_cts_time(wlc_info_t *wlc, ratespec_t rspec, uint8 preamble_type)
{
	WL_TRACE(("wl%d: wlc_calc_cts_time: ratespec 0x%x, preamble_type %d\n", wlc->pub->unit,
	        rspec, preamble_type));
	return wlc_calc_ack_time(wlc, rspec, preamble_type);
}

/* derive wlc->band->basic_rate[] table from 'rateset' */
void
wlc_rate_lookup_init(wlc_info_t *wlc, wlc_rateset_t *rateset)
{
	uint8 rate;
	uint8 mandatory;
	uint8 cck_basic = 0;
	uint8 ofdm_basic = 0;
	uint8 *br = &wlc->band->basic_rate[0];
	uint i;

	/* incoming rates are in 500kbps units as in 802.11 Supported Rates */
	bzero(br, WLC_MAXRATE + 1);

	/* For each basic rate in the rates list, make an entry in the
	 * best basic lookup.
	 */
	for (i = 0; i < rateset->count; i++) {
		/* only make an entry for a basic rate */
		if (!(rateset->rates[i] & WLC_RATE_FLAG))
			continue;

		/* mask off basic bit */
		rate = (rateset->rates[i] & RATE_MASK);

		if (rate > WLC_MAXRATE) {
			WL_ERROR(("%s: invalid rate 0x%X in rate set\n",
				__FUNCTION__, rateset->rates[i]));
			continue;
		}

		br[rate] = rate;
	}

	/* The rate lookup table now has non-zero entries for each
	 * basic rate, equal to the basic rate: br[basicN] = basicN
	 *
	 * To look up the best basic rate corresponding to any
	 * particular rate, code can use the basic_rate table
	 * like this
	 *
	 * basic_rate = wlc->band->basic_rate[tx_rate]
	 *
	 * Make sure there is a best basic rate entry for
	 * every rate by walking up the table from low rates
	 * to high, filling in holes in the lookup table
	 */

	for (i = 0; i < wlc->band->hw_rateset.count; i++) {
		int is_ofdm;

		rate = wlc->band->hw_rateset.rates[i];
		ASSERT(rate <= WLC_MAXRATE);

		is_ofdm = RATE_ISOFDM(rate);

		if (br[rate] != 0) {
			/* This rate is a basic rate.
			 * Keep track of the best basic rate so far by
			 * modulation type.
			 */
			if (is_ofdm)
				ofdm_basic = rate;
			else
				cck_basic = rate;

			continue;
		}

		/* This rate is not a basic rate so figure out the
		 * best basic rate less than this rate and fill in
		 * the hole in the table
		 */

		br[rate] = is_ofdm ? ofdm_basic : cck_basic;

		if (br[rate] != 0)
			continue;

		/* This is a weird setup where the lowest basic rate
		 * of a modulation type is non-existent or higher than
		 * some supported rates in the same modulation
		 */
		WL_RATE(("wl%d: no basic rate with the same modulation less than or equal to rate"
			" %d%s\n",
			 wlc->pub->unit, rate/2, (rate % 2)?".5":""));

		if (is_ofdm) {
			/* In 11g and 11a, the OFDM mandatory rates are 6, 12, and 24 Mbps */
			if (rate >= WLC_RATE_24M)
				mandatory = WLC_RATE_24M;
			else if (rate >= WLC_RATE_12M)
				mandatory = WLC_RATE_12M;
			else
				mandatory = WLC_RATE_6M;
		} else {
			/* In 11b, all the CCK rates are mandatory 1 - 11 Mbps */
			mandatory = rate;
		}

		br[rate] = mandatory;
	}
}

/* read rate table direct address map from shm */
static void
wlc_read_rt_dirmap(wlc_info_t *wlc)
{
	int i;
	for (i = 0; i < D11_RT_DIRMAP_SIZE; i ++)
		wlc->rt_dirmap_a[i] = wlc_read_shm(wlc, (M_RT_DIRMAP_A + i * 2));
	for (i = 0; i < D11_RT_DIRMAP_SIZE; i ++)
		wlc->rt_dirmap_b[i] = wlc_read_shm(wlc, (M_RT_DIRMAP_B + i * 2));
}

static void
wlc_write_rate_shm(wlc_info_t *wlc, uint8 rate, uint8 basic_rate)
{
	uint8 phy_rate, indx;
	uint8 basic_phy_rate, basic_index;
	uint16 basic_table;
	uint16 *rt_dirmap;

	/* Find the direct address map table we are reading */
	rt_dirmap = IS_OFDM(basic_rate) ? wlc->rt_dirmap_a : wlc->rt_dirmap_b;

	/* Shared memory address for the table we are writing */
	basic_table = IS_OFDM(rate) ? M_RT_BBRSMAP_A : M_RT_BBRSMAP_B;

	/*
	 * for a given rate, the LS-nibble of the PLCP SIGNAL field is
	 * the index into the rate table.
	 */
	phy_rate = rate_info[rate] & RATE_MASK;
	basic_phy_rate = rate_info[basic_rate] & RATE_MASK;
	indx = phy_rate & 0xf;
	basic_index = basic_phy_rate & 0xf;

	/* Update the SHM BSS-basic-rate-set mapping table with the pointer
	 * to the correct basic rate for the given incoming rate
	 */
	wlc_write_shm(wlc, (basic_table + indx * 2), rt_dirmap[basic_index]);
}

static const wlc_rateset_t *
wlc_rateset_get_hwrs(wlc_info_t *wlc)
{
	const wlc_rateset_t *rs_dflt;

	if (WLC_PHY_11N_CAP(wlc->band)) {
		if (BAND_5G(wlc->band->bandtype))
			rs_dflt = &ofdm_mimo_rates;
		else
			rs_dflt = &cck_ofdm_mimo_rates;
	} else if (WLCISLPPHY(wlc->band)) {
		if (BAND_5G(wlc->band->bandtype))
			rs_dflt = &ofdm_rates;
		else
			rs_dflt = &cck_ofdm_rates;
	} else if (WLCISAPHY(wlc->band))
		rs_dflt = &ofdm_rates;
	else if (wlc->band->gmode)
		rs_dflt = &cck_ofdm_rates;
	else
		rs_dflt = &cck_rates;

	return rs_dflt;
}

bool
wlc_hwrs_iterator_finished(wlc_hwrs_iterator *hwrs_walker)
{
	return ((int)hwrs_walker->rs->count == hwrs_walker->cur_idx);
}

uint8
wlc_hwrs_iterator_next(wlc_hwrs_iterator *hwrs_walker)
{
	uint8 rate = hwrs_walker->rs->rates[hwrs_walker->cur_idx];
	hwrs_walker->cur_idx++;
	return rate;
}

void
wlc_hwrs_iterator_init(wlc_info_t *wlc, wlc_hwrs_iterator *hwrs_walker)
{
	hwrs_walker->rs = wlc_rateset_get_hwrs(wlc);
	hwrs_walker->cur_idx = 0;
}

void
wlc_set_ratetable(wlc_info_t *wlc)
{
	const wlc_rateset_t *rs_dflt;
	wlc_rateset_t rs;
	uint8 rate, basic_rate;
	uint i;

	rs_dflt = wlc_rateset_get_hwrs(wlc);
	ASSERT(rs_dflt != NULL);

	wlc_rateset_copy(rs_dflt, &rs);

	wlc_rateset_mcs_upd(&rs, wlc->stf->txstreams);

	/* walk the phy rate table and update SHM basic rate lookup table */
	for (i = 0; i < rs.count; i++) {
		rate = rs.rates[i] & RATE_MASK;

		/* for a given rate WLC_BASIC_RATE returns the rate at
		 * which a response ACK/CTS should be sent.
		 */
		basic_rate = WLC_BASIC_RATE(wlc, rate);
		if (basic_rate == 0) {
			/* This should only happen if we are using a
			 * restricted rateset.
			 */
			WL_RATE(("wl%d: set_ratetable: Adding rate %d to ratetable\n",
				wlc->pub->unit, rate));
			basic_rate = rs.rates[0] & RATE_MASK;
		}

		wlc_write_rate_shm(wlc, rate, basic_rate);
	}
}

/**
 * Return true if the specified rate is supported by the specified band.
 * WLC_BAND_AUTO indicates the current band.
 */
bool
wlc_valid_rate(wlc_info_t *wlc, ratespec_t rspec, int band, bool verbose)
{
	wlc_rateset_t *hw_rateset;
	uint i;

	if ((band == WLC_BAND_AUTO) || (band == wlc->band->bandtype)) {
		hw_rateset = &wlc->band->hw_rateset;
	} else if (NBANDS(wlc) > 1) {
		hw_rateset = &wlc->bandstate[OTHERBANDUNIT(wlc)]->hw_rateset;
	} else {
		/* other band specified and we are a single band device */
		return (FALSE);
	}

	/* check for a VHT (11ac) rate that is supported by the hardware */
	if (RSPEC_ISVHT(rspec)) {
		uint mcs = (rspec & RSPEC_VHT_MCS_MASK);
		uint nss = ((rspec & RSPEC_VHT_NSS_MASK) >> RSPEC_VHT_NSS_SHIFT);

		if (mcs > 9 || nss > 3) {
			goto error;
		} else {
			return TRUE;
		}
	}

	/* check for an HT (11n) rate that is supported by the hardware */
	if (RSPEC_ISHT(rspec)) {
		uint mcs = (rspec & RSPEC_RATE_MASK);
		if (!VALID_MCS(mcs))
			goto error;

		return isset(hw_rateset->mcs, mcs);
	}

	for (i = 0; i < hw_rateset->count; i++)
		if (hw_rateset->rates[i] == (rspec & RSPEC_RATE_MASK))
			return (TRUE);
error:
	if (verbose) {
		WL_ERROR(("wl%d: %s: rate spec 0x%x not in hw_rateset\n",
		          wlc->pub->unit, __FUNCTION__, rspec));
#ifdef BCMDBG
		wlc_rateset_show(wlc, hw_rateset, NULL);
#endif
	}

	return (FALSE);
}

void
wlc_mod_prb_rsp_rate_table(wlc_info_t *wlc, uint frame_len)
{
	const wlc_rateset_t *rs_dflt;
	wlc_rateset_t rs;
	uint8 rate;
	uint16 entry_ptr;
	uint8 plcp[D11_PHY_HDR_LEN];
	uint16 dur, sifs;
	uint i;

	sifs = SIFS(wlc->band);

	rs_dflt = wlc_rateset_get_hwrs(wlc);
	ASSERT(rs_dflt != NULL);

	wlc_rateset_copy(rs_dflt, &rs);

	wlc_rateset_mcs_upd(&rs, wlc->stf->txstreams);

	/* walk the phy rate table and update MAC core SHM basic rate table entries */
	for (i = 0; i < rs.count; i++) {
		rate = rs.rates[i] & RATE_MASK;

		entry_ptr = wlc_rate_shm_offset(wlc, rate);

		/* Calculate the Probe Response PLCP for the given rate and addr */
		/* FROMDS -- not necessarily but a good first approximation for "to STA" */
		wlc_compute_plcp(wlc, rate, frame_len, FC_FROMDS, plcp);

		/* Calculate the duration of the Probe Response frame plus SIFS for the MAC */
		dur = (uint16)wlc_calc_frame_time(wlc, rate, WLC_LONG_PREAMBLE, frame_len);
		dur += sifs;

		/* Update the SHM Rate Table entry Probe Response values */
		/* different location for 802.11ac chips, per ucode */
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			wlc_write_shm(wlc, entry_ptr + D11AC_M_RT_PRS_PLCP_POS,
				(uint16)(plcp[0] + (plcp[1] << 8)));
			wlc_write_shm(wlc, entry_ptr + D11AC_M_RT_PRS_PLCP_POS + 2,
				(uint16)(plcp[2] + (plcp[3] << 8)));
			wlc_write_shm(wlc, entry_ptr + D11AC_M_RT_PRS_DUR_POS, dur);
		}
		else {
			wlc_write_shm(wlc, entry_ptr + D11_M_RT_PRS_PLCP_POS,
				(uint16)(plcp[0] + (plcp[1] << 8)));
			wlc_write_shm(wlc, entry_ptr + D11_M_RT_PRS_PLCP_POS + 2,
				(uint16)(plcp[2] + (plcp[3] << 8)));
			wlc_write_shm(wlc, entry_ptr + D11_M_RT_PRS_DUR_POS, dur);
		}
	}
}

/* compute the offset in usec from the first symbol in the payload (PHY-RXSTART),
 * to the first symbol of the TSF
 */
uint16
wlc_compute_bcn_payloadtsfoff(wlc_info_t *wlc, ratespec_t rspec)
{
	uint bcntsfoff = 0;

	if (!RSPEC_ISLEGACY(rspec)) {
		WL_ERROR(("wl%d: recd beacon with mcs rate; rspec 0x%x\n",
			wlc->pub->unit, rspec));
	} else if (IS_OFDM(rspec)) {
		/* PLCP SERVICE + MAC header time (SERVICE + FC + DUR + A1 + A2 + A3 + SEQ == 26
		 * bytes at beacon rate)
		 */
		bcntsfoff += wlc_compute_airtime(wlc, rspec, APHY_SERVICE_NBITS/8 +
			DOT11_MAC_HDR_LEN);
	} else {
		/* MAC header time (FC + DUR + A1 + A2 + A3 + SEQ == 24 bytes at beacon rate) */
		bcntsfoff += wlc_compute_airtime(wlc, rspec, DOT11_MAC_HDR_LEN);
	}
	return (uint16)(bcntsfoff);
}

/* compute the offset in usec from the beginning of the preamble,
 * to the first symbol of the TSF
 */
uint16
wlc_compute_bcntsfoff(wlc_info_t *wlc, ratespec_t rspec, bool short_preamble, bool phydelay)
{
	uint bcntsfoff = 0;

	/* compute the air time for the preamble */
	if (!RSPEC_ISLEGACY(rspec)) {
		WL_ERROR(("wl%d: recd beacon with mcs rate; rspec 0x%x\n",
			wlc->pub->unit, rspec));
	} else if (IS_OFDM(rspec)) {
		/* tx delay from MAC through phy to air (2.1 usec) +
		 * phy header time (preamble + PLCP SIGNAL == 20 usec) +
		 * PLCP SERVICE + MAC header time (SERVICE + FC + DUR + A1 + A2 + A3 + SEQ == 26
		 * bytes at beacon rate)
		 */
		bcntsfoff += phydelay ? D11A_PHY_TX_DELAY : 0;
		bcntsfoff += APHY_PREAMBLE_TIME + APHY_SIGNAL_TIME;
	} else {
		/* tx delay from MAC through phy to air (3.4 usec) +
		 * phy header time (long preamble + PLCP == 192 usec) +
		 * MAC header time (FC + DUR + A1 + A2 + A3 + SEQ == 24 bytes at beacon rate)
		 */
		bcntsfoff += phydelay ? D11B_PHY_TX_DELAY : 0;
		bcntsfoff += short_preamble ? D11B_PHY_SPREHDR_TIME: D11B_PHY_LPREHDR_TIME;
	}

	/* add the time from the end of the preamble/beginning of payload to the tsf field */
	bcntsfoff += wlc_compute_bcn_payloadtsfoff(wlc, rspec);

	return (uint16)(bcntsfoff);
}

bool
wlc_erp_find(wlc_info_t *wlc, void *body, uint body_len, uint8 **erp, int *len)
{
	bcm_tlv_t *ie;
	uint8 *tlvs = (uint8 *)body + DOT11_BCN_PRB_LEN;
	int tlvs_len = body_len - DOT11_BCN_PRB_LEN;

	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_ERP_ID);

	if (ie == NULL)
		return FALSE;

	*len = ie->len;
	*erp = ie->data;

	return TRUE;
}

/*	Max buffering needed for beacon template/prb resp template is 142 bytes.
 *
 *	PLCP header is 6 bytes.
 *	802.11 A3 header is 24 bytes.
 *	Max beacon frame body template length is 112 bytes.
 *	Max probe resp frame body template length is 110 bytes.
 *
 *      *len on input contains the max length of the packet available.
 *
 *	The *len value is set to the number of bytes in buf used, and starts with the PLCP
 *	and included up to, but not including, the 4 byte FCS.
 */
void
wlc_bcn_prb_template(wlc_info_t *wlc, uint type, ratespec_t bcn_rspec, wlc_bsscfg_t *cfg,
                     uint16 *buf, int *len)
{
	uint8 *body;
	struct dot11_management_header *h;
	int hdr_len, body_len;

	ASSERT(*len >= 142);
	ASSERT(type == FC_BEACON || type == FC_PROBE_RESP);

	if ((MBSS_BCN_ENAB(wlc, cfg) && type == FC_BEACON) ||
	    (D11REV_GE(wlc->pub->corerev, 40) && type == FC_PROBE_RESP)) {
		h = (struct dot11_management_header *)buf;
		hdr_len = DOT11_MAC_HDR_LEN;
	}
	else {
		int plcp_hdr_len = wlc->pub->d11tpl_phy_hdr_len;
		h = (struct dot11_management_header *)((uint8 *)buf + plcp_hdr_len);
		hdr_len = plcp_hdr_len + DOT11_MAC_HDR_LEN;
	}
	bzero(buf, hdr_len);

	body_len = *len - hdr_len; /* calc buffer size provided for frame body */
	body = (uint8 *)buf + hdr_len;

	wlc_bcn_prb_body(wlc, type, cfg, body, &body_len, FALSE);

	*len = hdr_len + body_len; /* return actual size */

	/* PLCP for Probe Response frames are filled in from core's rate table */
	if (!MBSS_BCN_ENAB(wlc, cfg) && type == FC_BEACON) {
		int mf_len = DOT11_MAC_HDR_LEN + body_len;
		uint8 *plcp;
		/* fill in PLCP */
		plcp = (uint8 *)buf + wlc_template_plcp_offset(wlc, bcn_rspec);
		wlc_compute_plcp(wlc, bcn_rspec, mf_len + DOT11_FCS_LEN, FC_FROMDS, plcp);
	}

	/* fill in 802.11 header */
	h->fc = htol16((uint16)type);

	/* DUR is 0 for multicast bcn, or filled in by MAC for prb resp */
	/* A1 filled in by MAC for prb resp, broadcast for bcn */
	if (type == FC_BEACON)
		bcopy((const char*)&ether_bcast, (char*)&h->da, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->cur_etheraddr, (char*)&h->sa, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&h->bssid, ETHER_ADDR_LEN);

	/* SEQ filled in by MAC */

	return;
}

int
wlc_write_hw_bcnparams(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint16 *bcn, int len, ratespec_t bcn_rspec, bool suspend)
{
	if (HWBCN_ENAB(cfg))
		wlc_beacon_phytxctl_txant_upd(wlc, bcn_rspec);

	if (!cfg->associated || BSSCFG_IBSS(cfg)) {
		uint16 bcn_offset;

		if (HWBCN_ENAB(cfg)) {
#ifdef WLMCHAN
			wlc_beacon_phytxctl(wlc, bcn_rspec, cfg->current_bss->chanspec);
#else
			wlc_beacon_phytxctl(wlc, bcn_rspec, wlc->chanspec);
#endif /* WLMCHAN */

		}
		/* VSDB TODO: wlc_beacon_phytxctl_percfg  */

		/* beacon timestamp symbol tsf offset */
		bcn_offset = wlc_compute_bcntsfoff(wlc, bcn_rspec, FALSE, TRUE);
		ASSERT(bcn_offset != 0);
		wlc->band->bcntsfoff = bcn_offset;
		wlc_write_shm(wlc, M_BCN_TXTSF_OFFSET, bcn_offset);
	}

	if (BSSCFG_AP(cfg) && !MBSS_BCN_ENAB(wlc, cfg)) {
		uint8 *cp;
		uint16 tim_offset;

		/* find the TIM elt offset in the bcn template */
		cp = (uint8 *)bcn + wlc->pub->d11tpl_phy_hdr_len +
		        DOT11_MAC_HDR_LEN + DOT11_BCN_PRB_LEN;
		len -= (int)(cp - (uint8 *)bcn);
		if (len < 0) {
			WL_ERROR(("wl%d.%d: %s: beacon length is wrong\n",
			          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			return BCME_BADARG;
		}
		cp = (uint8 *)bcm_parse_tlvs(cp, len, DOT11_MNG_TIM_ID);
		if (cp == NULL) {
			WL_ERROR(("wl%d.%d: %s: unable to find TIM in beacon frame\n",
			          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			return BCME_ERROR;
		}
		tim_offset = (uint16)(cp - (uint8 *)bcn);
		wlc_write_shm(wlc, M_TIMBPOS_INBEACON, tim_offset);
	}

	return BCME_OK;
}

static void
wlc_bss_update_dtim_period(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint16 dtim_period;
	uint32 val = 0;

	/* set DTIM count to 0 in SCR */
	wlc_bmac_copyto_objmem(wlc->hw, S_DOT11_DTIMCOUNT << 2, &val, sizeof(val), OBJADDR_SCR_SEL);

	dtim_period = cfg->associated ? cfg->current_bss->dtim_period :
		wlc->default_bss->dtim_period;
	wlc_write_shm(wlc, M_DOT11_DTIMPERIOD, dtim_period);
}

/* save bcn/prbresp in current_bss for IBSS */
static void
wlc_ibss_bcn_prb_save(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *bcn, int len)
{
	wlc_bss_info_t *current_bss = cfg->current_bss;

	if (current_bss->bcn_prb != NULL)
		return;

	ASSERT(current_bss->bcn_prb_len == 0);

	len -= wlc->pub->d11tpl_phy_hdr_len + DOT11_MAC_HDR_LEN;
	if (len <= 0)
		return;

	current_bss->bcn_prb = (struct dot11_bcn_prb *)MALLOC(wlc->osh, len);
	if (current_bss->bcn_prb == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return;
	}

	bcopy(bcn + wlc->pub->d11tpl_phy_hdr_len + DOT11_MAC_HDR_LEN,
		(uint8 *)current_bss->bcn_prb, len);
	current_bss->bcn_prb_len = (uint16)len;
}

/* Update a beacon for a particular BSS
 * For MBSS, this updates the software template and sets "latest" to the index of the
 * template updated.
 * Otherwise, it updates the hardware template.
 */
static void
_wlc_bss_update_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	/* Clear the soft intmask */
	wlc_bmac_set_defmacintmask(wlc->hw, MI_BCNTPL, ~MI_BCNTPL);

#ifdef MBSS
	if (MBSS_BCN_ENAB(wlc, cfg)) {
		wlc_mbss_update_beacon(wlc, cfg);
	} else
#endif /* MBSS */
	if (HWBCN_ENAB(cfg)) { /* Hardware beaconing for this config */
		uint16 *bcn;
		uint32	both_valid = MCMD_BCN0VLD | MCMD_BCN1VLD;
		d11regs_t *regs = wlc->regs;
		osl_t *osh = wlc->osh;
		wlc_bss_info_t *current_bss = cfg->current_bss;
		ratespec_t bcn_rspec;
		int len, bcn_tmpl_len;

		bcn_tmpl_len = wlc->pub->bcn_tmpl_len;
		len = bcn_tmpl_len;

		/* Check if both templates are in use, if so sched. an interrupt
		 *	that will call back into this routine
		 */
		if ((R_REG(osh, &regs->maccommand) & both_valid) == both_valid) {
			/* clear any previous status */
			W_REG(osh, &regs->macintstatus, MI_BCNTPL);
		}
		/* Check that after scheduling the interrupt both of the
		 *	templates are still busy. if not clear the int. & remask
		 */
		if ((R_REG(osh, &regs->maccommand) & both_valid) == both_valid) {
			wlc_bmac_set_defmacintmask(wlc->hw, MI_BCNTPL, MI_BCNTPL);
			return;
		}

		if ((bcn = (uint16 *)MALLOC(osh, bcn_tmpl_len)) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, %d bytes malloced\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(osh)));
			return;
		}

		/* Use the lowest basic rate for beacons if no user specified bcn rate */
		bcn_rspec = (BSSCFG_AP(cfg) && wlc_force_bcn_rspec(wlc))? wlc_force_bcn_rspec(wlc):
			wlc_lowest_basic_rspec(wlc, &current_bss->rateset);
		ASSERT(wlc_valid_rate(wlc, bcn_rspec,
		        CHSPEC_IS2G(current_bss->chanspec) ? WLC_BAND_2G : WLC_BAND_5G,
		        TRUE));
		wlc->bcn_rspec = bcn_rspec;

		/* update the template and ucode shm */
		wlc_bcn_prb_template(wlc, FC_BEACON, bcn_rspec, cfg, bcn, &len);
		wlc_write_hw_bcnparams(wlc, cfg, bcn, len, bcn_rspec, FALSE);
		wlc_write_hw_bcntemplates(wlc, bcn, len, FALSE);

		/* set a default bcn_prb for IBSS */
		if (BSSCFG_STA(cfg) && !cfg->BSS)
			wlc_ibss_bcn_prb_save(wlc, cfg, (uint8 *)bcn, len);

		MFREE(osh, (void *)bcn, bcn_tmpl_len);
	}
	else {
		wlc_bss_tplt_upd_notif(wlc, cfg, FC_BEACON);
	}
}

void
wlc_bss_update_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	if (!cfg->up)
		return;
	_wlc_bss_update_beacon(wlc, cfg);
}

/*
 * Update all beacons for the system.
 */
void
wlc_update_beacon(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *bsscfg;

	/* update AP or IBSS beacons */
	FOREACH_BSS(wlc, idx, bsscfg) {
		if (bsscfg->up &&
		    (BSSCFG_AP(bsscfg) || !bsscfg->BSS))
			wlc_bss_update_beacon(wlc, bsscfg);
	}
}

/* Write ssid into shared memory */
void
wlc_shm_ssid_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint8 *ssidptr = cfg->SSID;
	uint16 base = M_SSID;
	uint8 ssidbuf[DOT11_MAX_SSID_LEN];


#ifdef MBSS
	if (MBSS_ENAB(wlc->pub)) {
		wlc_mbss_shm_ssid_upd(wlc, cfg, &base);

		if (MBSS_ENAB16(wlc->pub))
			return;
	}
#endif /* MBSS */

	/* padding the ssid with zero and copy it into shm */
	bzero(ssidbuf, DOT11_MAX_SSID_LEN);
	bcopy(ssidptr, ssidbuf, cfg->SSID_len);

	wlc_copyto_shm(wlc, base, ssidbuf, DOT11_MAX_SSID_LEN);

	if (!MBSS_BCN_ENAB(wlc, cfg))
		wlc_write_shm(wlc, M_SSIDLEN, (uint16)cfg->SSID_len);
}

void
wlc_update_probe_resp(wlc_info_t *wlc, bool suspend)
{
	int idx;
	wlc_bsscfg_t *bsscfg;

	/* update AP or IBSS probe responses */
	FOREACH_BSS(wlc, idx, bsscfg) {
		if (bsscfg->up &&
		    (BSSCFG_AP(bsscfg) || !bsscfg->BSS))
			wlc_bss_update_probe_resp(wlc, bsscfg, suspend);
	}
}

void
wlc_bss_update_probe_resp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool suspend)
{
	int len, bcn_tmpl_len;

	bcn_tmpl_len = wlc->pub->bcn_tmpl_len;
	len = bcn_tmpl_len;

#ifdef WLPROBRESP_SW
	if (WLPROBRESP_SW_ENAB(wlc))
		return;
#endif /* WLPROBRESP_SW */

	/* write the probe response to hardware, or save in the config structure */
#if defined(MBSS)
	if (MBSS_PRB_ENAB(wlc, cfg)) {
		/* Generating probe resp in sw; update local template */
		wlc_mbss_update_probe_resp(wlc, cfg, suspend);
	} else
#endif /* MBSS */
	if (HWPRB_ENAB(cfg)) { /* Hardware probe resp for this config */
		uint16 *prb_resp;
		int offset;

		if ((prb_resp = (uint16 *)MALLOC(wlc->osh, bcn_tmpl_len)) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return;
		}

		/* create the probe response template */
		wlc_bcn_prb_template(wlc, FC_PROBE_RESP, 0, cfg, prb_resp, &len);

		if (suspend)
			wlc_suspend_mac_and_wait(wlc);

		if (D11REV_GE(wlc->pub->corerev, 40))
			offset = D11AC_T_PRS_TPL_BASE;
		else
			offset = D11_T_PRS_TPL_BASE;

		/* write the probe response into the template region */
		wlc_bmac_write_template_ram(wlc->hw, offset, (len + 3) & ~3, prb_resp);

		/* write the length of the probe response frame (+PLCP/-FCS) */
		wlc_write_shm(wlc, M_PRB_RESP_FRM_LEN, (uint16)len);

		/* write the SSID and SSID length */
		wlc_shm_ssid_upd(wlc, cfg);

		/*
		 * Write PLCP headers and durations for probe response frames at all rates.
		 * Use the actual frame length covered by the PLCP header for the call to
		 * wlc_mod_prb_rsp_rate_table() by adding the FCS.
		 */
		len += DOT11_FCS_LEN;

		/* pre-rev40 macs include space for the PLCP in the template area
		 * so subtract that from the frame len
		 */

		if (D11REV_LT(wlc->pub->corerev, 40))
			len -= D11_PHY_HDR_LEN;

		wlc_mod_prb_rsp_rate_table(wlc, (uint16)len);

		if (suspend)
			wlc_enable_mac(wlc);

		MFREE(wlc->osh, (void *)prb_resp, bcn_tmpl_len);
	}
	else {
		wlc_bss_tplt_upd_notif(wlc, cfg, FC_PROBE_RESP);
	}
}

/* Beacon and Probe Response frame body
 *  max sz  order
 *  8       1 Timestamp
 *  2       2 Beacon interval
 *  2       3 Capability information
 *  34      4 SSID
 *  10      5 Supported rates
 *  (0)     6 FH Params             (not supported)
 *  3       7 DS Parameter Set
 *  (0)     8 CF Params             (not supported)
 *  4       9 IBSS Parameter Set    (IBSS only)
 *  6(256)  10 TIM                  (BSS-Beacon only)
 *  8(113)  11 Country Information  (11d)
 *  3       15 Channel Switch Announcement (11h)
 *  6       16 Quiet period	    (11h)
 *  3       19 ERP Information      (11g)
 *  10(257) 20 Ext. Sup. Rates      (11g)
 *  7          BRCM proprietary
 *  27         WME params
 *
 *  The TIM can be 256 bytes long in general. For a beacon template,
 *  a NULL TIM is used, which is 6 bytes long.
 *
 *  Country element is minimum of 8 which includes a base of 5 plus
 *  possibly 3 bytes per supported channel (less if the channel numbers
 *  are monotonic and can be compressed.
 *
 *  Max template beacon frame body length is 112 bytes. (items 1-7 and 10, 19, 20, 221)
 *  Max template probe response frame body length is 110 bytes. (items 1-7 and 9, 19, 20, 221)
 */

void
wlc_bcn_prb_body(wlc_info_t *wlc, uint type, wlc_bsscfg_t *bsscfg, uint8 *bcn, int *len,
	bool legacy_tpl)
{
	struct dot11_bcn_prb *fixed;
	wlc_rateset_t sup_rates, ext_rates;
	uint8 *pbody;
	uint body_len;
	wlc_bss_info_t *current_bss;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	uint8 ht_wsec_restrict;
	bool ht_cap = TRUE;

	ASSERT(bsscfg != NULL);
	ASSERT(len != NULL);

	current_bss = bsscfg->current_bss;

	ASSERT(*len >= 112);
	ASSERT(ISALIGNED(bcn, sizeof(uint16)));
	ASSERT(type == FC_BEACON || type == FC_PROBE_RESP);

	bzero(&sup_rates, sizeof(wlc_rateset_t));
	bzero(&ext_rates, sizeof(wlc_rateset_t));

	/* check for a supported rates override, add back iBSS Gmode setting */
	if (wlc->sup_rates_override.count > 0)
		wlc_rateset_copy(&wlc->sup_rates_override, &sup_rates);
	else if ((!AP_ENAB(wlc->pub)) && (wlc->band->gmode == GMODE_LRS))
		wlc_rateset_copy(&cck_rates, &sup_rates);
	else if ((!AP_ENAB(wlc->pub)) && (wlc->band->gmode == GMODE_PERFORMANCE))
		wlc_default_rateset(wlc, &sup_rates);

	wlc_rateset_elts(wlc, bsscfg, &current_bss->rateset, &sup_rates, &ext_rates);

	body_len = DOT11_BCN_PRB_LEN;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.bcn.sup = &sup_rates;
	ftcbparm.bcn.ext = &ext_rates;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	ht_wsec_restrict = wlc_ht_get_wsec_restriction(wlc->hti);
	if (((ht_wsec_restrict & WLC_HT_WEP_RESTRICT) &&
		WSEC_WEP_ENABLED(bsscfg->wsec)) ||
		((ht_wsec_restrict & WLC_HT_TKIP_RESTRICT) &&
		WSEC_TKIP_ENABLED(bsscfg->wsec) && !(WSEC_AES_ENABLED(bsscfg->wsec)))) {
		ht_cap = FALSE;
	}

	cbparm.ht = BSS_N_ENAB(wlc, bsscfg) && !legacy_tpl && ht_cap ? TRUE : FALSE;
#ifdef WL11AC
	cbparm.vht = BSS_VHT_ENAB(wlc, bsscfg) && !legacy_tpl && ht_cap ? TRUE : FALSE;
#endif

	/* calculate the IEs' length */
	body_len += wlc_iem_calc_len(wlc->iemi, bsscfg, (uint16)type, NULL, &cbparm);
	if (body_len > (uint)*len) {
		WL_ERROR(("wl%d: %s: buffer too short, buf %d, needs %u\n",
		          wlc->pub->unit, __FUNCTION__, *len, body_len));
		return;
	}

	fixed = (struct dot11_bcn_prb *)bcn;
	pbody = bcn;

	/* fill in fixed info, Timestamp, Beacon interval, and Capability */
	bzero((char*)fixed, sizeof(*fixed));
	/* Timestamp is zero in bcn template, filled in by MAC */
	/* [N.B.: fixed->timestamp is misaligned but unreferenced, so "safe".] */

	fixed->beacon_interval = htol16((uint16)current_bss->beacon_period);
	if (BSSCFG_AP(bsscfg)) {
		fixed->capability = DOT11_CAP_ESS;
#ifdef WL11K
		if (WL11K_ENAB(wlc->pub) && wlc_rrm_enabled(wlc->rrm_info, bsscfg))
			fixed->capability |= DOT11_CAP_RRM;
#endif
	}
#ifdef WLP2P
	else if (BSS_P2P_DISC_ENAB(wlc, bsscfg))
		fixed->capability = 0;
#endif
	else
		fixed->capability = DOT11_CAP_IBSS;
	if (WSEC_ENABLED(bsscfg->wsec) && (bsscfg->wsec_restrict)) {
		fixed->capability |= DOT11_CAP_PRIVACY;
	}
	/* Advertise short preamble capability if we have not been forced long AND we
	 * are not an APSTA
	 */
	if (!APSTA_ENAB(wlc->pub) && BAND_2G(wlc->band->bandtype) &&
	    (current_bss->capability & DOT11_CAP_SHORT))
		fixed->capability |= DOT11_CAP_SHORT;
	if (wlc->band->gmode && wlc->shortslot)
		fixed->capability |= DOT11_CAP_SHORTSLOT;
	if (BSS_WL11H_ENAB(wlc, bsscfg) && BSSCFG_AP(bsscfg))
		fixed->capability |= DOT11_CAP_SPECTRUM;

	fixed->capability = htol16(fixed->capability);

	pbody += DOT11_BCN_PRB_LEN;
	body_len -= DOT11_BCN_PRB_LEN;

	/* fill in Info elts, SSID, Sup Rates, DS params, and IBSS params */
	if (wlc_iem_build_frame(wlc->iemi, bsscfg, (uint16)type,
	                        NULL, &cbparm, pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
	}

	pbody -= DOT11_BCN_PRB_LEN;
	body_len += DOT11_BCN_PRB_LEN;

	*len = body_len;

	return;
}

uchar
wlc_assocscb_getcnt(wlc_info_t *wlc)
{
	struct scb *scb;
	uchar assoc = 0;
	struct scb_iter scbiter;
	int32 idx;
	wlc_bsscfg_t *bsscfg;
	ASSERT(wlc->pub->tunables->maxscb <= 255);

	FOREACH_BSS(wlc, idx, bsscfg) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (SCB_ASSOCIATED(scb))
				assoc++;
		}
	}
	return assoc;
}

static uint8
wlc_bss_scb_getcnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool (*scb_test_cb)(struct scb *scb))
{
	struct scb *scb;
	uint8 assoc = 0;
	struct scb_iter scbiter;
	int32 idx;
	wlc_bsscfg_t *bsscfg;
	ASSERT(wlc->pub->tunables->maxscb <= 255);

	FOREACH_BSS(wlc, idx, bsscfg) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (scb->bsscfg == cfg && scb_test_cb(scb))
				assoc++;
		}
	}
	return assoc;
}

static bool
wlc_scb_associated_cb(struct scb *scb)
{
	return SCB_ASSOCIATED(scb);
}

uint8
wlc_bss_assocscb_getcnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	return wlc_bss_scb_getcnt(wlc, cfg, wlc_scb_associated_cb);
}

/* return TRUE if cfg->brcm_ie[] changed, FALSE if not */
bool
wlc_update_brcm_ie(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	bool update = FALSE;

	FOREACH_BSS(wlc, idx, cfg) {
		update |= wlc_bss_update_brcm_ie(wlc, cfg);
	}

	return update;
}

bool
wlc_bss_update_brcm_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint8 assoc;
	brcm_ie_t *brcm_ie;
	volatile uint8 flags = 0, flags1 = 0;


	brcm_ie = (brcm_ie_t *)&cfg->brcm_ie[0];

	flags = 0;

#ifdef AP
	if (BSSCFG_AP(cfg)) {
#ifdef WDS
		if (wlc_wds_lazywds_is_enable(wlc->mwds) && cfg == wlc->cfg)
			flags |= BRF_LZWDS;
#endif
#ifdef BCM_DCS
		/* enable BCM_DCS for BCM AP */
		if (wlc->ap->dcs_enabled)
			flags1 |= BRF1_RFAWARE_DCS;
#endif
		/* As long as WME is enabled, AP does the right thing
		 * when a WME STA goes to PS mode as opposed to some versions of BRCM AP
		 * that would not buffer traffic going to a WME STA in PS mode
		 */
		if (WME_ENAB(wlc->pub))
			flags1 |= BRF1_WMEPS;
#ifdef SOFTAP
		flags1 |= BRF1_SOFTAP;
#endif
		flags1 |= BRF1_PSOFIX;
	}
#endif /* AP */

	/* enable capability of receiving large aggregate if STA is
	 * HTPHY and AMPDU_ENAB
	 */
	if ((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) && AMPDU_ENAB(wlc->pub)) {
#ifndef WLC_HIGH_ONLY
		flags1 |= BRF1_RX_LARGE_AGG;
#else
		if (wlc->pub->is_ss) {
			flags1 |= BRF1_RX_LARGE_AGG;
		}
#endif
	}


#ifdef DWDS
	if (DWDS_ENAB(cfg)) {
		flags1 |= BRF1_DWDS;
	}
#endif /* DWDS */

	/* count the total number of associated stas */
	assoc = wlc_bss_assocscb_getcnt(wlc, cfg);

	/* no change? */
	if ((brcm_ie->assoc == assoc) && (brcm_ie->flags == flags) && (brcm_ie->flags1 == flags1))
		return (FALSE);

	brcm_ie->assoc = assoc;
	brcm_ie->flags = flags;
	brcm_ie->flags1 = flags1;

	ASSERT((cfg->brcm_ie[TLV_LEN_OFF]+TLV_HDR_LEN) < WLC_MAX_BRCM_ELT);

	return (TRUE);
}

/** process received BRCM info element */
void
wlc_process_brcm_ie(wlc_info_t *wlc, struct scb *scb, brcm_ie_t *brcm_ie)
{

	if (!brcm_ie || brcm_ie->len < 4) {
#ifdef BRCMAPIVTW
		wlc_keymgmt_ivtw_enable(wlc->keymgmt, scb, FALSE);
#endif
		return;
	}

	ASSERT(scb != NULL);

	/* remote station is BRCM AP/STA */
	scb->flags |= SCB_BRCM;

	/* legacy AES implementations */
	if (brcm_ie->ver <= BRCM_IE_LEGACY_AES_VER)
		scb->flags |= SCB_LEGACY_AES;
	else
		scb->flags &= ~SCB_LEGACY_AES;

	/* early brcm_ie is only 7 bytes in length */
	if (brcm_ie->len <= (OFFSETOF(brcm_ie_t, flags) - TLV_HDR_LEN)) {
		return;
	}

#ifdef STA

	if (brcm_ie->len <= (OFFSETOF(brcm_ie_t, flags1) - TLV_HDR_LEN)) {
#ifdef BRCMAPIVTW
		wlc_keymgmt_ivtw_enable(wlc->keymgmt, scb, TRUE);
#endif
		return;
	}

	if (brcm_ie->flags1 & BRF1_WMEPS)
		scb->flags |= SCB_WMEPS;
	else
		scb->flags &= ~SCB_WMEPS;

#ifdef BCM_DCS
	if (brcm_ie->flags1 & BRF1_RFAWARE_DCS)
		scb->flags2 |= SCB2_BCMDCS;
	else
		scb->flags2 &= ~SCB2_BCMDCS;

#endif /* BCM_DCS */

#ifdef BRCMAPIVTW
	wlc_keymgmt_ivtw_enable(wlc->keymgmt,  scb,
		((brcm_ie->flags1 & BRF1_PSOFIX) ? FALSE : TRUE));
#endif

#endif /* STA */

	scb->flags2 &= ~SCB2_RX_LARGE_AGG;

	if (brcm_ie->len > (OFFSETOF(brcm_ie_t, flags1) - TLV_HDR_LEN))
		if (brcm_ie->flags1 & BRF1_RX_LARGE_AGG) {
#ifndef WLC_HIGH_ONLY
			scb->flags2 |= SCB2_RX_LARGE_AGG;
#else
			if (wlc->pub->is_ss) {
				scb->flags2 |= SCB2_RX_LARGE_AGG;
			}
#endif
		}

#ifdef DWDS
	if (DWDS_ENAB(SCB_BSSCFG(scb))) {
		if (brcm_ie->flags1 & BRF1_DWDS)
			scb->flags3 |= SCB3_DWDS_CAP;
		else {
#ifdef PSTA
			/*
			 * In case if driver has been dynamically switched
			 * to DWDS mode from one of the PSTA mode then the
			 * function below will restore that particulate PSTA
			 * mode back.
			 */
			wlc_psta_mode_update(wlc->psta, SCB_BSSCFG(scb), NULL,
				PSTA_MODE_UPDATE_ACTION_RESTORE);
#endif /* PSTA */
			scb->flags3 &= ~SCB3_DWDS_CAP;
		}
	}
#endif /* DWDS */
}

/* recover 32bit TSF value from the 16bit TSF value */
/* assumption is time in rxh is within 65ms of the current tsf */
/* local TSF inserted in the rxh is at RxStart which is before 802.11 header */
uint32
wlc_recover_tsf32(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh)
{
	uint16 	rxh_tsf;
	uint32	ts_tsf;
	wlcband_t *band;
	int bandunit;
	uint16 rfdly;

	rxh_tsf = wrxh->rxhdr.RxTSFTime;
	ts_tsf = wrxh->tsf_l;

	/* adjust rx dly added in RxTSFTime */
	bandunit = CHSPEC_BANDUNIT(wrxh->rxhdr.RxChan);
	band = wlc->bandstate[bandunit];

	/* TODO: add PHY type specific value here... */
	if (WLCISAPHY(band))
		rfdly = M_APHY_PLCPRX_DLY;
	else
		rfdly = M_BPHY_PLCPRX_DLY;

	rxh_tsf -= rfdly;

	return (((ts_tsf - rxh_tsf) & 0xFFFF0000) | rxh_tsf);
}

/* recover 64bit TSF value from the 16bit TSF value */
/* assumption is time in rxh is within 65ms of the current tsf */
/* 'tsf' carries in the current TSF time */
/* local TSF inserted in the rxh is at RxStart which is before 802.11 header */
void
wlc_recover_tsf64(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, uint32 *tsf_h, uint32 *tsf_l)
{
	uint32 rxh_tsf;

	rxh_tsf = wlc_recover_tsf32(wlc, wrxh);

	/* a lesser TSF time indicates the low 32 bits of
	 * TSF wrapped, so decrement the high 32 bits.
	 */
	if (*tsf_l < rxh_tsf) {
		*tsf_h -= 1;
		WL_NONE(("TSF has wrapped (rxh %x tsf %x), adjust to %x%08x\n",
		         rxh_tsf, *tsf_l, *tsf_h, rxh_tsf));
	}
	*tsf_l = rxh_tsf;
}


#ifdef STA
static void *
wlc_frame_get_ps_ctl(wlc_info_t *wlc, wlc_bsscfg_t *cfg, const struct ether_addr *bssid,
	const struct ether_addr *sa)
{
	void *p;
	struct dot11_ps_poll_frame *hdr;

	if ((p = wlc_frame_get_ctl(wlc, DOT11_PS_POLL_LEN)) == NULL) {
		return (NULL);
	}

	/* construct a PS-Poll frame */
	hdr = (struct dot11_ps_poll_frame *)PKTDATA(wlc->osh, p);
	hdr->fc = htol16(FC_PS_POLL);
	hdr->durid = htol16(cfg->AID);
	bcopy((const char*)bssid, (char*)&hdr->bssid, ETHER_ADDR_LEN);
	bcopy((const char*)sa, (char*)&hdr->ta, ETHER_ADDR_LEN);

	return (p);
}
#endif /* STA */

void*
wlc_frame_get_ctl(wlc_info_t *wlc, uint len)
{
	void *p;
	osl_t *osh;

	ASSERT(len != 0);

	osh = wlc->osh;
	if ((p = PKTGET(osh, (TXOFF + len), TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
			wlc->pub->unit, __FUNCTION__, ((int)TXOFF + len)));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return (NULL);
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len);

	PKTSETPRIO(p, 0);

	return (p);
}

static void
wlc_fill_mgmt_hdr(struct dot11_management_header *hdr, uint16 fc,
	const struct ether_addr *da, const struct ether_addr *sa, const struct ether_addr *bssid)
{
	/* construct a management frame */
	hdr->fc = htol16(fc);
	hdr->durid = 0;
	bcopy((const char*)da, (char*)&hdr->da, ETHER_ADDR_LEN);
	bcopy((const char*)sa, (char*)&hdr->sa, ETHER_ADDR_LEN);
	bcopy((const char*)bssid, (char*)&hdr->bssid, ETHER_ADDR_LEN);
	hdr->seq = 0;
}

static void*
wlc_frame_get_mgmt_int(wlc_info_t *wlc, uint16 fc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len,
	uint8 **pbody, uint8 cat)
{
	void *p;
#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub))
		p = wlc_mfp_frame_get_mgmt(wlc->mfp, fc, cat, da, sa, bssid, body_len, pbody);
	else
		p = wlc_frame_get_mgmt_ex(wlc, fc, da, sa, bssid, body_len, pbody, 0, 0);
#else
	p = wlc_frame_get_mgmt_ex(wlc, fc, da, sa, bssid, body_len, pbody, 0, 0);
#endif 
	return p;
}

void*
wlc_frame_get_mgmt_ex(wlc_info_t *wlc, uint16 fc, const struct ether_addr *da,
    const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len,
    uint8 **pbody, uint iv_len, uint tail_len)
{
	uint len;
	void *p = NULL;
	osl_t *osh;
	struct dot11_management_header *hdr;

	osh = wlc->osh;

	len = DOT11_MGMT_HDR_LEN + iv_len + body_len + tail_len;

	if ((p = PKTGET(osh, (TXOFF + len), TRUE)) == NULL) {
#ifdef BCMPKTPOOL
		uint16 fckind = (fc & FC_KIND_MASK);
		if ((fckind != FC_NULL_DATA) && (fckind != FC_QOS_NULL)) {
			if (POOL_ENAB(wlc->pub->pktpool) &&
				((TXOFF + len) <= pktpool_plen(wlc->pub->pktpool)))
				p = pktpool_get(wlc->pub->pktpool);
		}
#endif
		if (p == NULL) {
			WL_ERROR(("wl%d: wlc_frame_get_mgmt: pktget error for len %d fc %x\n",
				wlc->pub->unit, ((int)TXOFF + len), fc));
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			return (NULL);
		}
	}

	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len - tail_len);

	/* construct a management frame */
	hdr = (struct dot11_management_header *)PKTDATA(osh, p);
	wlc_fill_mgmt_hdr(hdr, fc, da, sa, bssid);

	*pbody = (uint8*)&hdr[1] + iv_len;

	/* Set MAX Prio for MGMT packets */
	PKTSETPRIO(p, MAXPRIO);

#if defined(WLP2P) && defined(BCMDBG)
	if (fc == FC_ACTION || fc == FC_PROBE_REQ || fc == FC_PROBE_RESP) {
		char eabuf[ETHER_ADDR_STR_LEN];
		char *frmtype_str = NULL;
		switch (fc) {
			case FC_ACTION:
				frmtype_str = "ACTION";
				break;
			case FC_PROBE_REQ:
				frmtype_str = "PROBE_REQ";
				break;
			case FC_PROBE_RESP:
				frmtype_str = "PROBE_RESP";
				break;
		}
		WL_P2P(("wl%d: prep %s frame tx to %s, pkt %p\n",
		        wlc->pub->unit, frmtype_str,
		        bcm_ether_ntoa(&hdr->da, eabuf), p));
	}
	WL_WSEC(("wl%d: %s: allocated %u bytes; iv %u and tail %u bytes\n",
	         wlc->pub->unit, __FUNCTION__, (uint)(TXOFF + len), iv_len, tail_len));
#endif /* WLP2P && BCMDBG */

	return (p);
}

void*
wlc_frame_get_mgmt(wlc_info_t *wlc, uint16 fc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len,
	uint8 **pbody)
{
	return wlc_frame_get_mgmt_int(wlc, fc, da, sa, bssid, body_len, pbody, 0);
}

void*
wlc_frame_get_action(wlc_info_t *wlc, uint16 fc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len,
	uint8 **pbody, uint8 cat)
{
	return wlc_frame_get_mgmt_int(wlc, fc, da, sa, bssid, body_len, pbody, cat);
}

bool
wlc_queue_80211_frag(wlc_info_t *wlc, void *p, wlc_txq_info_t *qi, struct scb *scb,
	wlc_bsscfg_t *bsscfg, bool short_preamble, wlc_key_t *key, ratespec_t rate_override)
{
	int prio;
	uint fifo = TX_CTL_FIFO;

	ASSERT(wlc->pub->up);


	/* use hw rateset scb if the one passed in is NULL */
	if (scb == NULL) {
		scb = wlc->band->hwrs_scb;
		if (!bsscfg)
			bsscfg = scb->bsscfg;
	}

	if (!bsscfg)
		goto toss;

	prio = PKTPRIO(p);
	ASSERT(prio <= MAXPRIO);

	/* In WME mode, send mgmt frames based on priority setting */
	if (SCB_QOS(scb) && EDCF_ENAB(wlc->pub))
		fifo = prio2fifo[prio];


	WLPKTTAGSCBSET(p, scb);

#ifdef MFP
	/* setup key for MFP */
	if (WLC_MFP_ENAB(wlc->pub) && (WLPKTTAG(p)->flags & WLF_MFP) && key == NULL) {
		struct dot11_management_header *hdr;
		hdr = (struct dot11_management_header*)PKTDATA(wlc->osh, p);
		if (ETHER_ISMULTI(&hdr->da) && BSSCFG_AP(bsscfg))
			key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, TRUE, NULL);
		else if (!ETHER_ISMULTI(&hdr->da) && SCB_MFP(scb))
			key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
				WLC_KEY_FLAG_NONE, NULL);
	}
#endif /* MFP */

	WLPKTTAGBSSCFGSET(p, WLC_BSSCFG_IDX(bsscfg));

	/* save the tx options for wlc_prep_pdu */
	wlc_pdu_push_txparams(wlc, p, short_preamble ? WLC_TX_PARAMS_SHORTPRE : 0,
	                      key, rate_override, fifo);

	if (BSSCFG_AP(bsscfg) &&
	    SCB_PS(scb) &&
	    !(WLPKTTAG(p)->flags & WLF_PSDONTQ)) {
		if (wlc_apps_psq(wlc, p, WLC_PRIO_TO_HI_PREC(prio)))
			return TRUE;
		goto toss;
	}

#if defined(NEW_TXQ) && defined(USING_MUX)
	if (wlc_txq_immediate_enqueue(wlc, qi, p, WLC_PRIO_TO_HI_PREC(prio))) {
		return TRUE;
	}
#else
	if (wlc_prec_enq(wlc, &qi->q, p, WLC_PRIO_TO_HI_PREC(prio))) {
		wlc_send_q(wlc, qi);
		return TRUE;
	}
#endif /* defined(NEW_TXQ) && defined(USING_MUX) */

toss:
	PKTFREE(wlc->osh, p, TRUE);
	WLCNTINCR(wlc->pub->_cnt->txnobuf);
	WLCIFCNTINCR(scb, txnobuf);
	WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_failures);
	return FALSE;
}

bool
wlc_sendmgmt(wlc_info_t *wlc, void *p, wlc_txq_info_t *qi, struct scb *scb)
{

	return wlc_queue_80211_frag(wlc, p, qi, scb,
		(scb ? scb->bsscfg : NULL), FALSE, NULL,
		(scb ? WLC_LOWEST_SCB_RSPEC(scb) : WLC_LOWEST_BAND_RSPEC(wlc->band)));
}

bool
wlc_sendctl(wlc_info_t *wlc, void *p, wlc_txq_info_t *qi, struct scb *scb, uint fifo,
	ratespec_t rate_override, bool enq_only)
{
	int prio;

	ASSERT(wlc->pub->up);

	/* SCB must not be NULL */
	ASSERT(scb != NULL);

	if (!scb->bsscfg)
		goto toss;


	prio = PKTPRIO(p);
	ASSERT(prio <= MAXPRIO);

	/* WME: send frames based on priority setting */
	if (SCB_QOS(scb) && EDCF_ENAB(wlc->pub))
		fifo = prio2fifo[prio];

	WLPKTTAGSCBSET(p, scb);
	if (scb->bsscfg)
		WLPKTTAGBSSCFGSET(p, WLC_BSSCFG_IDX(scb->bsscfg));

#if defined(NEW_TXQ) && defined(USING_MUX)
	/* pick the lowest rate if a rate_override is not specified */
	if (!RSPEC_ACTIVE(rate_override)) {
		rate_override = WLC_LOWEST_SCB_RSPEC(scb);
	}
#endif /* defined(NEW_TXQ) && defined(USING_MUX) */

	/* save the tx options for wlc_prep_pdu */
	wlc_pdu_push_txparams(wlc, p, 0, NULL, rate_override, fifo);

	if (BSSCFG_AP(SCB_BSSCFG(scb)) &&
	    SCB_PS(scb) &&
	    !(WLPKTTAG(p)->flags & WLF_PSDONTQ)) {
		if (wlc_apps_psq(wlc, p, WLC_PRIO_TO_HI_PREC(prio)))
			return TRUE;
		goto toss;
	}
#if defined(NEW_TXQ) && defined(USING_MUX)
	if (wlc_txq_immediate_enqueue(wlc, qi, p, WLC_PRIO_TO_HI_PREC(prio))) {
		return TRUE;
	}
#else
	if (wlc_prec_enq(wlc, &qi->q, p, WLC_PRIO_TO_HI_PREC(prio))) {
		if (!enq_only)
			wlc_send_q(wlc, qi);
		return TRUE;
	}
#endif /* defined(NEW_TXQ) && defined(USING_MUX) */
toss:
	PKTFREE(wlc->osh, p, TRUE);
	WLCNTINCR(wlc->pub->_cnt->txnobuf);
	return FALSE;
}

static const uint8 ac2prio[] = {
	PRIO_8021D_BE,	/* AC_BE index */
	PRIO_8021D_BK, 	/* AC_BK index */
	PRIO_8021D_VI, 	/* AC_VI index */
	PRIO_8021D_VO	/* AC_VO index */
};

/* prep raw pkt via header additions etc. */
uint16
wlc_prep80211_raw(wlc_info_t *wlc, wlc_if_t *wlcif, uint ac,
	void *p, ratespec_t rspec, uint *outfifo)
{
	struct scb *scb;
	uint fifo;
	int prio;
	uint16 fid;

	ASSERT(ac < ARRAYSIZE(ac2prio));
	prio = ac2prio[ac];
	scb = wlc->band->hwrs_scb;

	/* WME: send frames based on priority setting */
	if (EDCF_ENAB(wlc->pub)) {
		fifo = prio2fifo[prio];
	} else {
		fifo = TX_CTL_FIFO;
	}
	if (outfifo) {
		*outfifo = fifo;
	}

	PKTSETPRIO(p, prio);
	WL_TRACE(("%s: fifo=%d prio=%d\n", __FUNCTION__, fifo, prio));
	/* add headers */
	fid = wlc_d11hdrs(wlc, p, scb, 0, 0, 1, fifo, 0, NULL, rspec, NULL);

	WLPKTTAGSCBSET(p, scb);
	if (scb->bsscfg)
		WLPKTTAGBSSCFGSET(p, WLC_BSSCFG_IDX(scb->bsscfg));
	return fid;
}

/* push a tx_params structure on the head of a packet when creating an
 * MPDU packet without a d11 hardware txhdr
 */
static void
wlc_pdu_push_txparams(wlc_info_t *wlc, void *p,
                      uint32 flags, wlc_key_t *key, ratespec_t rate_override, uint fifo)
{
	wlc_pdu_tx_params_t tx_params;

#if defined(NEW_TXQ) && defined(USING_MUX)
	/* rate needs to be 'active' (non-zero) */
	ASSERT(RSPEC_ACTIVE(rate_override));
	/* rate needs to have a BW specified */
	ASSERT(RSPEC_BW(rate_override) != RSPEC_BW_UNSPECIFIED);	/* push on the tx params */
#endif /* defined(NEW_TXQ) and defined(USING_MUX) */

	tx_params.flags = flags;
	tx_params.key = key;
	tx_params.rspec_override = rate_override;
	tx_params.fifo = fifo;

	PKTPUSH(wlc->osh, p, sizeof(wlc_pdu_tx_params_t));
	memcpy(PKTDATA(wlc->osh, p), &tx_params, sizeof(wlc_pdu_tx_params_t));

	/* Mark the pkt as an MPDU (tx_params and 802.11 header) but no ucode txhdr */
	WLPKTTAG(p)->flags |= WLF_MPDU;
}

void
wlc_get_sup_ext_rates(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_rateset_t *sup_rates,
	wlc_rateset_t *ext_rates)
{
	const wlc_rateset_t *rs;

	bzero(sup_rates, sizeof(wlc_rateset_t));
	bzero(ext_rates, sizeof(wlc_rateset_t));
	if ((BAND_2G(wlc->band->bandtype)) && (wlc->band->gmode == GMODE_LEGACY_B))
		rs = &cck_rates;	/* only advertise 11b rates */
	else
		rs = &wlc->band->hw_rateset;

	/* For 2.4Ghz band probe requests, CCK-only in sup_rates, OFDM in ext_rates */
	if ((BAND_2G(wlc->band->bandtype)) && wlc->legacy_probe)
		wlc_rateset_copy(&cck_rates, sup_rates);

	wlc_rateset_elts(wlc, bsscfg, rs, sup_rates, ext_rates);
}

void
wlc_sendprobe(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	const uint8 ssid[], int ssid_len,
	const struct ether_addr *da, const struct ether_addr *bssid,
	ratespec_t ratespec_override, uint8 *extra_ie, int extra_ie_len)
{
	void *pkt;
	wlc_rateset_t sup_rates, ext_rates;
	uint8 *pbody;
	uint body_len;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	if (!bsscfg||!bsscfg->brcm_ie)
		return;

	wlc_get_sup_ext_rates(wlc, bsscfg, &sup_rates, &ext_rates);

	/* prepare the IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.prbreq.mcs = wlc->band->hw_rateset.mcs;
	ftcbparm.prbreq.ssid = (const uint8 *)ssid;
	ftcbparm.prbreq.ssid_len = (uint8)ssid_len;
	ftcbparm.prbreq.sup = &sup_rates;
	ftcbparm.prbreq.ext = &ext_rates;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;
	cbparm.ht = BSS_N_ENAB(wlc, bsscfg);
#ifdef WL11AC
	cbparm.vht = BSS_VHT_ENAB(wlc, bsscfg);
#endif

	/* calculate IEs' length */
	body_len = wlc_iem_calc_len(wlc->iemi, bsscfg, FC_PROBE_REQ, NULL, &cbparm);

	/* allocate a packet */
	if ((pkt = wlc_frame_get_mgmt(wlc, FC_PROBE_REQ, da,
		&bsscfg->cur_etheraddr, bssid, body_len, &pbody)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_frame_get_mgmt failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* populte IEs */
	if (wlc_iem_build_frame(wlc->iemi, bsscfg, FC_PROBE_REQ, NULL, &cbparm,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, pkt, TRUE);
		return;
	}

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		wlc_p2p_sendprobe(wlc->p2p, bsscfg, pkt);
	else
#endif
	wlc_sendmgmt(wlc, pkt, wlc->active_queue, NULL);
}

void *
wlc_sendauth(
	wlc_bsscfg_t *cfg,
	struct ether_addr *ea,
	struct ether_addr *bssid,
	struct scb *scb,
	int auth_alg,
	int auth_seq,
	int auth_status,
	uint8 *challenge_text,
	bool short_preamble
)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_txq_info_t *qi;
	void *pkt = NULL;
	uint8 *pbody;
	uint body_len;
	struct dot11_auth *auth;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	ASSERT(cfg != NULL);

	/* assert:  (status == success) -> (scb is not NULL) */
	ASSERT((auth_status != DOT11_SC_SUCCESS) || (scb != NULL));

	/* All Authentication frames have a common 6 byte contents:
	 * 2 bytes Authentication Alg Number
	 * 2 bytes Authentication Transaction Number
	 * 2 bytes Status Code
	 */
	body_len = sizeof(struct dot11_auth);

	wlc_key_get_info(NULL, &key_info); /* init key info, algo=off */

	/* Authentications frames 2 and 3 for Shared Key auth have a 128 byte
	 * Challenge Text Info element, with frame 3 WEP encrypted
	 */
	if ((auth_status == DOT11_SC_SUCCESS) &&
		(auth_alg == DOT11_SHARED_KEY) && (auth_seq == 3)) {
		key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, &key_info);
		if (key_info.algo == CRYPTO_ALGO_OFF) {
			wlc_bsscfg_t *tx_cfg;
			tx_cfg = (BSSCFG_PSTA(cfg)) ? wlc_bsscfg_primary(wlc) : cfg;
			key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, tx_cfg, FALSE, &key_info);
		}

		if (key_info.algo == CRYPTO_ALGO_OFF) {
#ifdef STA
			wlc_auth_tx_complete(wlc, TX_STATUS_NO_ACK, (void *)(uintptr)cfg->ID);
#endif /* STA */
			WL_ERROR(("wl%d: transmit key absent\n", wlc->pub->unit));
			return NULL;
		}
	}

	/* prepare the IE mgmt calls, and calculate IE's length */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.auth.alg = auth_alg;
	ftcbparm.auth.seq = auth_seq;
	ftcbparm.auth.scb = scb;
	ftcbparm.auth.challenge = challenge_text;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	body_len += wlc_iem_calc_len(wlc->iemi, cfg, FC_AUTH, NULL, &cbparm);

	/* allocate a packet */
	pkt = wlc_frame_get_mgmt_ex(wlc, FC_AUTH, ea, &cfg->cur_etheraddr,
		bssid, body_len, &pbody, key_info.iv_len, key_info.icv_len);
	if (pkt == NULL) {
		WL_ERROR(("wl%d: %s: pkt alloc failed\n", wlc->pub->unit, __FUNCTION__));
#ifdef STA
#endif /* STA */
		return NULL;
	}

	/* init fixed portion */
	auth = (struct dot11_auth *)pbody;
	auth->alg = htol16((uint16)auth_alg);
	auth->seq = htol16((uint16)auth_seq);
	auth->status = htol16((uint16)auth_status);

	pbody += sizeof(struct dot11_auth);
	body_len -= sizeof(struct dot11_auth);

	/* generate IEs */
	if (wlc_iem_build_frame(wlc->iemi, cfg, FC_AUTH, NULL, &cbparm,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
	}
	if (auth_status == DOT11_SC_SUCCESS &&
	    ftcbparm.auth.status != DOT11_SC_SUCCESS) {
		WL_INFORM(("wl%d: %s: wlc_iem_build_frame status %u\n",
		           wlc->pub->unit, __FUNCTION__, ftcbparm.auth.status));
		auth->status = htol16(ftcbparm.auth.status);
	}

	/* As an AP, send using the bsscfg queue so that the auth response will go out on the bsscfg
	 * channel.
	 *
	 * As a STA, send using the active_queue instead of the bsscfg queue. When roaming, the
	 * bsscfg queue will be associated with the current associated channel, not the roam target
	 * channel.
	 */
	if (BSSCFG_AP(cfg))
		qi = cfg->wlcif->qi;
	else
		qi = wlc->active_queue;
	ASSERT(qi != NULL);

	if (!wlc_queue_80211_frag(wlc, pkt, qi, scb, cfg, short_preamble, key,
		(scb ? WLC_LOWEST_SCB_RSPEC(scb) : WLC_LOWEST_BAND_RSPEC(wlc->band)))) {
		WL_ERROR(("wl%d: %s: wlc_queue_80211_frag failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	return pkt;
}

void *
wlc_senddisassoc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	const struct ether_addr *da, const struct ether_addr *bssid,
	const struct ether_addr *sa, uint16 reason_code)
{
	void *pkt = NULL;
	uint8 *pbody;
	uint body_len;
	uint16 *reason;

	ASSERT(cfg != NULL);

	/*
	 * get a packet - disassoc pkt has the following contents:
	 * 2 bytes Reason Code
	 */
	body_len = sizeof(uint16);

	/* calculate IEs' length */
	body_len += wlc_iem_calc_len(wlc->iemi, cfg, FC_DISASSOC, NULL, NULL);

	if ((pkt = wlc_frame_get_mgmt(wlc, FC_DISASSOC, da, sa, bssid,
	                              body_len, &pbody)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_frame_get_mgmt failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	/* fill out the disassociation reason code */
	reason = (uint16 *) pbody;
	reason[0] = htol16(reason_code);

	pbody += sizeof(uint16);
	body_len -= sizeof(uint16);

	/* generate IEs */
	if (wlc_iem_build_frame(wlc->iemi, cfg, FC_DISASSOC, NULL, NULL,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, pkt, TRUE);
		return NULL;
	}

	if (wlc_sendmgmt(wlc, pkt, cfg->wlcif->qi, scb))
		return pkt;

	wlc_smfstats_update(wlc, cfg, SMFS_TYPE_DISASSOC_TX, reason_code);

	return NULL;
}

/*
 * For non-WMM association: sends a Null Data frame.
 *
 * For WMM association: if prio is -1, sends a Null Data frame;
 * otherwise sends a QoS Null frame with priority prio.
 */
static void *
wlc_alloc_nulldata(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *da,
                 uint32 pktflags, int prio)
{
	void *p;
	uint8 *pbody;
	uint16 fc;
	int qos, body_len;
	struct ether_addr *bssid;
	struct scb *scb;

	ASSERT(bsscfg != NULL);
	if (!(scb = wlc_scbfind(wlc, bsscfg, da)))
		return NULL;

	if (prio < 0) {
		qos = 0;
		prio = PRIO_8021D_BE;
	} else
		qos = SCB_QOS(scb);

	if (qos) {
		fc = FC_QOS_NULL;
		body_len = 2;
	} else {
		fc = FC_NULL_DATA;
		body_len = 0;
	}

	if (SCB_A4_NULLDATA(scb)) {
		fc |= FC_FROMDS | FC_TODS;
		/* WDS Data Frame...so four addresses */
		body_len += ETHER_ADDR_LEN;
	}
	else if (BSSCFG_STA(bsscfg)) {
		if (bsscfg->BSS)
			fc |= FC_TODS;
	} else {
		fc |= FC_FROMDS;
	}

	bssid = &bsscfg->BSSID;

	if ((p = wlc_frame_get_mgmt(wlc, fc, da, &bsscfg->cur_etheraddr,
		bssid, body_len, &pbody)) == NULL)
		return p;

	WLPKTTAG(p)->flags |= pktflags;

	PKTSETPRIO(p, prio);

	/* For WDS addresses, mgmt has correct RA and TA, but DA and SA need fixup */
	if (SCB_A4_NULLDATA(scb)) {
		struct dot11_header *hdr = (struct dot11_header *)PKTDATA(wlc->osh, p);
		ASSERT(PKTLEN(wlc->osh, p) >= DOT11_A4_HDR_LEN);
		bcopy(da->octet, hdr->a3.octet, ETHER_ADDR_LEN);
		bcopy((char*)&wlc->pub->cur_etheraddr, hdr->a4.octet, ETHER_ADDR_LEN);
	}

	if (qos) {
		uint16 *pqos;
		wlc_wme_t *wme = bsscfg->wme;

		/* Initialize the QoS Control field that comes after all addresses. */
		pqos = (uint16 *)((uint8 *)PKTDATA(wlc->osh, p) +
			DOT11_MGMT_HDR_LEN + body_len - DOT11_QOS_LEN);
		ASSERT(ISALIGNED(pqos, sizeof(*pqos)));
		if (wme->wme_noack == QOS_ACK_NO_ACK)
			WLPKTTAG(p)->flags |= WLF_WME_NOACK;
		*pqos = htol16(((prio << QOS_PRIO_SHIFT) & QOS_PRIO_MASK) |
			((wme->wme_noack << QOS_ACK_SHIFT) & QOS_ACK_MASK));
#ifdef WLTDLS
		if (BSS_TDLS_ENAB(wlc, SCB_BSSCFG(scb)) &&
#ifdef AP
		    SCB_PS(scb) &&
		    !wlc_apps_scb_apsd_cnt(wlc, scb) &&
#endif
		    TRUE)
			*pqos |= htol16(QOS_EOSP_MASK);
#endif /* WLTDLS */
	}
	return p;
}

bool
wlc_sendnulldata(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *da,
	ratespec_t rate_override, uint32 pktflags, int prio,
	int (*pre_send_fn)(wlc_info_t*, wlc_bsscfg_t*, void*, void *), void *data)
{
	void *p;
	struct scb *scb;
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	int err;

	WL_TRACE(("wl%d: %s: DA %s rspec 0x%08X pktflags 0x%08X prio %d bsscfg %p\n",
		wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(da, eabuf), rate_override, pktflags, prio, bsscfg));

	ASSERT(bsscfg != NULL);
	if ((scb = wlc_scbfind(wlc, bsscfg, da)) == NULL) return FALSE;

#ifdef STA
	/* If we're in the middle of a join, don't send any null data frames.
	 * It could end up being sent between the time we sent the auth req to
	 * the time the AP received our assoc req.
	 * If that happened, the AP will disassoc this cfg.
	 */
	if ((wlc->block_datafifo & DATA_BLOCK_JOIN) && (wlc->as_info->assoc_req[0] == bsscfg)) {
		WL_ASSOC(("wl%d.%d: %s trying to send null data during JOIN, discard\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		return FALSE;
	}
#endif /* STA */
	if ((p = wlc_alloc_nulldata(wlc, bsscfg, da, pktflags, prio)) == NULL)
		return FALSE;

	/* If caller registered function to process pkt buffer before send, call it */
	if (pre_send_fn)
		if ((err = pre_send_fn(wlc, bsscfg, p, data))) {
			WL_ERROR(("%s: callback failed with error %d\n", __FUNCTION__, err));
			return FALSE;
		}

	WL_PS(("wl%d.%d: sending null frame to %s\n", wlc->pub->unit,
		WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(da, eabuf)));

	return wlc_sendctl(wlc, p, bsscfg->wlcif->qi, scb, TX_DATA_FIFO, rate_override, FALSE);
}
#ifdef STA
static int wlc_sendpmnotif_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt, void *data)
{
	bool track = (bool)(uintptr) data;
	/* Sometimes this Null data packet is not sent to the air and read back
	 * from txfifo in wlc_tx_fifo_sync_complete(), because other packets
	 * already in txfifo have been transmitted and called wlc_update_pmstate()
	 * in wlc_dotxstatus().
	 * In case it initiates excursion start, then we don't have to re-enqueue
	 * this null data packet as PM indication is already done.
	 */
	WLPKTTAG(pkt)->flags3 |= WLF3_TXQ_SHORT_LIFETIME;

	/* register the completion callback */
	if (track)
		WLF2_PCB1_REG(pkt, WLF2_PCB1_PM_NOTIF);
	return BCME_OK;
}

static bool
wlc_sendpmnotif(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	ratespec_t rate_override, int prio, bool track)
{
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	WL_TRACE(("wl%d: %s: DA %s rspec 0x%08X prio %d cfg %p\n", wlc->pub->unit, __FUNCTION__,
	          bcm_ether_ntoa(da, eabuf), rate_override, prio, cfg));

	return wlc_sendnulldata(wlc, cfg, da, rate_override, 0, prio,
		wlc_sendpmnotif_cb, (void *)(uintptr)track);
}
#endif /* STA */

void *
wlc_nulldata_template(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *da)
{
	void *pkt;
	bool shortpreamble;
	struct scb *scb;
	ratespec_t rspec;

	ASSERT(bsscfg != NULL);

	if ((scb = wlc_scbfind(wlc, bsscfg, da)) == NULL)
		return NULL;

	pkt = wlc_alloc_nulldata(wlc, bsscfg, &wlc->cfg->BSSID, 0, -1);
	if (pkt == NULL)
		return NULL;

	if (wlc->cfg->PLCPHdr_override == WLC_PLCP_LONG)
		shortpreamble = FALSE;
	else
		shortpreamble = TRUE;

	rspec = wlc_lowest_basic_rspec(wlc, &wlc->cfg->current_bss->rateset);
	ASSERT(wlc_valid_rate(wlc, rspec,
	                      CHSPEC_IS2G(wlc->cfg->current_bss->chanspec) ?
	                      WLC_BAND_2G : WLC_BAND_5G, TRUE));

	/* add headers */
	wlc_d11hdrs(wlc, pkt, scb, shortpreamble, 0, 1,
	            TX_DATA_FIFO, 0, NULL, rspec, NULL);
	return pkt;
}

#ifdef STA
bool
wlc_sendapsdtrigger(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *scb;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	ASSERT(cfg != NULL);
	ASSERT(cfg->associated);

	scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);
	if (scb == NULL) {
		WL_ERROR(("wl%d.%d: %s: unable to find scb for %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		          bcm_ether_ntoa(&cfg->BSSID, eabuf)));
		return FALSE;
	}

	if (!_wlc_sendapsdtrigger(wlc, scb)) {
		WL_ERROR(("wl%d.%d: %s: unable to send apsd trigger frame to %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		          bcm_ether_ntoa(&cfg->BSSID, eabuf)));
		return FALSE;
	}

	return TRUE;
}

static const uint8 apsd_trig_acbmp2maxprio[] = {
	PRIO_8021D_BE, PRIO_8021D_BE, PRIO_8021D_BK, PRIO_8021D_BK,
	PRIO_8021D_VI, PRIO_8021D_VI, PRIO_8021D_VI, PRIO_8021D_VI,
	PRIO_8021D_VO, PRIO_8021D_VO, PRIO_8021D_VO, PRIO_8021D_VO,
	PRIO_8021D_VO, PRIO_8021D_VO, PRIO_8021D_VO, PRIO_8021D_VO
};

static bool
_wlc_sendapsdtrigger(wlc_info_t *wlc, struct scb *scb)
{
	wlc_bsscfg_t *cfg;
	wlc_wme_t *wme;
	int prio;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	wme = cfg->wme;

	/* Select the prio based on valid values and user override. */
	prio = (int)apsd_trig_acbmp2maxprio[wme->apsd_trigger_ac & scb->apsd.ac_trig];

	return wlc_sendnulldata(wlc, cfg, &scb->ea, 0, 0, prio, NULL, NULL);
}

static int wlc_rateprobe_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt, void *data)
{
	WLF2_PCB1_REG(pkt, WLF2_PCB1_RATE_PRB);
	return BCME_OK;
}

void
wlc_rateprobe(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *ea, ratespec_t rate_override)
{
	wlc_sendnulldata(wlc, cfg, ea, rate_override, 0, PRIO_8021D_BE, wlc_rateprobe_cb, NULL);
}

/** Called after a tx completion of a 'rate probe' (a NULL data frame) */
static void
wlc_rateprobe_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	uint8 mcsallow = 0;

	/* no ack */
	if (!(txs & TX_STATUS_ACK_RCV))
		return;

	/* make sure the scb is still around */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	/* upgrade the scb rateset to all supported network rates */
	if (SCB_HT_CAP(scb))
		mcsallow |= WLC_MCS_ALLOW;
	if (SCB_VHT_CAP(scb))
		mcsallow |= WLC_MCS_ALLOW_VHT;
	wlc_rateset_filter((bsscfg->associated ? &bsscfg->current_bss->rateset :
		&bsscfg->target_bss->rateset), &scb->rateset,
		FALSE, WLC_RATES_CCK_OFDM, RATE_MASK, mcsallow);
	wlc_scb_ratesel_init(wlc, scb);
}

/** Called after a tx completion of a 'rate probe' (a NULL data frame) */
static void
wlc_rateprobe_scan(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	struct scb *scb;
	struct scb_iter scbiter;

	/*
	 * Send ofdm rate probe to any station
	 * which does not yet have an ofdm rate in its scb rateset
	 * and has been active within the past 60 seconds.
	 */

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (!IS_OFDM(scb->rateset.rates[scb->rateset.count-1]) &&
		    ((wlc->pub->now - scb->used) < 60))
			wlc_rateprobe(wlc, cfg, &scb->ea, WLC_RATEPROBE_RATE);
	}
}

bool
wlc_sendpspoll(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	void *p;
	struct scb *scb;
	struct ether_addr *bssid = &bsscfg->BSSID;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	if (!(scb = wlc_scbfind(wlc, bsscfg, bssid))) {
		WL_ERROR(("wl%d: %s: wlc_scbfind failed, BSSID %s bandunit %d\n",
		          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(bssid, eabuf),
		          wlc->band->bandunit));
		return FALSE;
	}

	/* If a pspoll is still pending, don't send another one */
	if (scb->flags & SCB_PENDING_PSPOLL)
		return TRUE;

	if ((p = wlc_frame_get_ps_ctl(wlc, bsscfg, bssid, &bsscfg->cur_etheraddr)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_frame_get_ps_ctl failed\n", wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* Force d11 to wake */
	wlc_set_pspoll(bsscfg, TRUE);

	/* register the completion callback */
	WLF2_PCB1_REG(p, WLF2_PCB1_PSP);

	if (!wlc_sendctl(wlc, p, bsscfg->wlcif->qi, scb, TX_CTL_FIFO, 0, FALSE)) {
		WL_ERROR(("wl%d: %s: wlc_sendctl failed\n", wlc->pub->unit, __FUNCTION__));

		/* Failed, allow d11 to sleep */
		wlc_set_pspoll(bsscfg, FALSE);
		return FALSE;
	}
	scb->flags |= SCB_PENDING_PSPOLL;

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		wlc_p2p_pspoll_resend_upd(wlc->p2p, bsscfg, TRUE);
#endif
	return TRUE;
}

static void
wlc_sendpspoll_complete(wlc_info_t *wlc, void *pkt, uint txstatus)
{
	struct scb *scb;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || BCMDBG_ERR */

	/* Is this scb still around */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

	if ((txstatus & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
		wlc_bsscfg_t *cfg;

		WL_ERROR(("wl%d: %s: no ACK from %s for PS-Poll\n",
		          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf)));

		cfg = SCB_BSSCFG(scb);
		ASSERT(cfg != NULL);

		wlc_set_pspoll(cfg, FALSE);
	}

	scb->flags &= ~SCB_PENDING_PSPOLL;
}

static void
_wlc_pspoll_timer(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;

	if (pm->PMblocked || AS_IN_PROGRESS(wlc))
		return;

	if (pm->PMpending)
		return;

	if (wlc_sendpspoll(wlc, cfg) == FALSE) {
		WL_ERROR(("wl%d: %s: wlc_sendpspoll() failed\n", wlc->pub->unit, __FUNCTION__));
	}
}

void
wlc_pspoll_timer(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;

	ASSERT(cfg->associated && cfg->pm->PMenabled && cfg->pm->pspoll_prd != 0);

	_wlc_pspoll_timer(cfg);
}

static void
wlc_pm2_radio_shutoff_dly_timer(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;

	wlc->pm2_radio_shutoff_pending = FALSE;
	wlc_set_wake_ctrl(wlc);
	return;
}

static void
wlc_wake_event_timer(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;

	wlc->wake_event_timeout = 0;
	wlc_mac_event(wlc, WLC_E_WAKE_EVENT, NULL, WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
	return;
}

static void
wlc_iscan_timeout(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char chanbuf[CHANSPEC_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	if (ISCAN_IN_PROGRESS(wlc) && wlc->custom_iscan_results_state != WL_SCAN_RESULTS_SUCCESS) {
		WL_INFORM(("wlc_iscan_timeout ending on chanspec %s, i %d\n",
		           wf_chspec_ntoa_ex(wlc->iscan_chanspec_last, chanbuf),
		           wlc->scan_results->count));
		wlc_scan_terminate(wlc->scan, WLC_E_STATUS_PARTIAL);
	}
}
#endif /* STA */

void *
wlc_senddeauth(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	struct ether_addr *da, struct ether_addr *bssid,
	struct ether_addr *sa, uint16 reason_code)
{
	void *pkt;
	uint8 *pbody;
	uint body_len;
	uint16 *reason;

	ASSERT(cfg != NULL);

	/*
	 * get a packet - deauth pkt has the following contents:
	 * 2 bytes Reason Code
	 */
	body_len = sizeof(uint16);

	/* calculate IEs' length */
	body_len += wlc_iem_calc_len(wlc->iemi, cfg, FC_DEAUTH, NULL, NULL);

	/* allocate a packet */
	if ((pkt = wlc_frame_get_mgmt(wlc, FC_DEAUTH, da, sa, bssid,
	                              body_len, &pbody)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_frame_get_mgmt failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	/* fill out the deauthentication reason code */
	reason = (uint16 *)pbody;
	reason[0] = htol16(reason_code);

	pbody += sizeof(uint16);
	body_len -= sizeof(uint16);

	/* generate IEs */
	if (wlc_iem_build_frame(wlc->iemi, cfg, FC_DEAUTH, NULL, NULL,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, pkt, TRUE);
		return NULL;
	}

	if (wlc_sendmgmt(wlc, pkt, cfg->wlcif->qi, scb))
		return pkt;

	wlc_smfstats_update(wlc, cfg, SMFS_TYPE_DEAUTH_TX, reason_code);

	return NULL;
}

#if ((defined(BCMDBG) || defined(WLMSG_PRPKT)) && defined(STA)) || defined(BCMDBG)
static const char*
wlc_lookup_name(const wlc_id_name_table_t tbl, int id)
{
	const struct wlc_id_name_entry *elt = tbl;
	static char unknown[16];

	for (elt = tbl; elt->name != NULL; elt++) {
		if (id == elt->id)
			return elt->name;
	}

	snprintf(unknown, sizeof(unknown), "ID:%d", id);

	return unknown;
}
#endif 

#if defined(BCMDBG)
void
wlc_dump_ie(wlc_info_t *wlc, bcm_tlv_t *ie, struct bcmstrbuf *b)
{
	const char* name = wlc_lookup_name(dot11_ie_names, ie->id);
	int j;

	bcm_bprintf(b, "ID: %d %s [%d]: ", ie->id, name, ie->len);
	for (j = 0; j < ie->len; j ++)
		bcm_bprintf(b, "%02x", ie->data[j]);
}
#endif 

#if defined(BCMDBG) || defined(WLMSG_PRPKT)
void
wlc_print_ies(wlc_info_t *wlc, uint8 *ies, uint ies_len)
{
	bcm_tlv_t *ie;
	char hexbuf[65];

	ie = (bcm_tlv_t*)ies;
	while (ies_len >= 2) {
		const char* name;
		char* valstr = NULL;
		uint8 id = ie->id;
		char ssidbuf[SSID_FMT_BUF_LEN + 2];

		if (ies_len < (uint)(ie->len + 2))
			break;

		name = wlc_lookup_name(dot11_ie_names, id);

		if (id == DOT11_MNG_SSID_ID) {
			int i = wlc_format_ssid(ssidbuf+1, ie->data, ie->len);
			ssidbuf[0] = '\"';
			ssidbuf[i+1] = '\"';
			ssidbuf[i+2] = '\0';
			valstr = ssidbuf;
		} else if (ie->len <= 32) {
			bcm_format_hex(hexbuf, ie->data, ie->len);
			valstr = hexbuf;
		}

		if (valstr != NULL) {
			printf("wl%d: ID %d %s [%d]: %s\n",
			       wlc->pub->unit, id, name, ie->len, valstr);
		} else {
			printf("wl%d: ID %d %s [%d]:\n",
			       wlc->pub->unit, id, name, ie->len);
			prhex(NULL, ie->data, ie->len);
		}

		ies += (2 + ie->len);
		ies_len -= (2 + ie->len);
		ie = (bcm_tlv_t*)ies;
	}

	if (ies_len > 0) {
		printf("wl%d: Unknown bytes at end of IEs [%d bytes]\n", wlc->pub->unit, ies_len);
		prhex(NULL, ies, ies_len);
	}
}
#endif /* BCMDBG || WLMSG_PRPKT */

#ifdef STA

uint16
wlc_assoc_capability(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_bss_info_t *bi)
{
	wlc_assoc_t *as;
	uint16 capability;

	ASSERT(bsscfg != NULL);

	as = bsscfg->assoc;

	/* Capability information */
	if (as->capability)
		return htol16(as->capability);
	else {
		capability = DOT11_CAP_ESS;
		if (WSEC_ENABLED(bsscfg->wsec))
			capability |= DOT11_CAP_PRIVACY;
		if (BAND_2G(wlc->band->bandtype) &&
		    ((bsscfg->PLCPHdr_override == WLC_PLCP_SHORT) ||
		     (bsscfg->PLCPHdr_override == WLC_PLCP_AUTO)))
			capability |= DOT11_CAP_SHORT;
		/* Nokia A032 AP (11b-only AP) rejects association req if
		 * capability - [short slot time, spectrum management] bit is set.
		 * The solution for interoperability
		 * with such AP is to
		 * (1) set DOT11_CAP_SHORTSLOT only for gmode-ofdm case
		 * (2) set DOT11_CAP_SPECTRUM only if beacon has it set.
		 */
		if (wlc->band->gmode &&
		    wlc_rateset_isofdm(bi->rateset.count, bi->rateset.rates) &&
		    (wlc->shortslot_override != WLC_SHORTSLOT_OFF))
			capability |= DOT11_CAP_SHORTSLOT;
		if (BSS_WL11H_ENAB(wlc, bsscfg) &&
		    (bi->capability & DOT11_CAP_SPECTRUM))
			capability |= DOT11_CAP_SPECTRUM;
#ifdef WL11K
		if (WL11K_ENAB(wlc->pub) &&
			wlc_rrm_enabled(wlc->rrm_info, bsscfg) &&
			(bi->capability & DOT11_CAP_RRM))
			capability |= DOT11_CAP_RRM;
#endif
		capability = htol16(capability);
	}

	return capability;
}

int wlc_remove_wpa2_pmkid(wlc_info_t *wlc, bcm_tlv_t *wpa2ie)
{
	int len = wpa2ie->len;		/* value length */
	int i;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint16 count;

	/* Check min length and version */
	if ((len < WPA2_VERSION_LEN)||
		(ltoh16_ua(wpa2ie->data) != WPA2_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA2 version %d\n", wlc->pub->unit,
		ltoh16_ua(wpa2ie->data)));
		return BCME_UNSUPPORTED;
	}
	len -= WPA2_VERSION_LEN;

	/* Check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("wl%d: no multicast cipher suite\n", wlc->pub->unit));
		/* it is ok to not have multicast cipher */
		return 0;
	}
	/* skip the multicast cipher */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	len -= WPA_SUITE_LEN;
	/* Check for unicast suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: no unicast suite\n", wlc->pub->unit));
		/* it is ok to not have unicast cipher(s) */
		return 0;
	}
	/* skip unicast cipher list */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;

	for (i = 0; i < count && len > WPA_SUITE_LEN; i++, len -= WPA_SUITE_LEN);

	/* jump to auth key mgmt suites */
	len -= (count - i) * WPA_SUITE_LEN;
	/* Check for auth key management suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: auth key mgmt suite\n", wlc->pub->unit));
		/* it is ok to not have auth key mgmt suites */
		return 0;
	}

	/* skip the auth_key_mgmt */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0; i < count && len >= WPA_SUITE_LEN; i++, len -= WPA_SUITE_LEN);

	/* jump to RSN Cap */
	len -= (count - i) * WPA_SUITE_LEN;

	if (len < RSN_CAP_LEN) {
		WL_INFORM(("wl%d: no rsn cap\n", wlc->pub->unit));
		/* it is ok to not have RSN Cap */
		return 0;
	}
	len -= RSN_CAP_LEN;
	if (len > 0) {
		wpa2ie->len -= (uint8)len;
		WL_INFORM(("wl%d : removed pmkid length %d\n", wlc->pub->unit, len));
	}
	return 0;
}

static bool
wlc_assoc_req_ins_cb(void *ctx, wlc_iem_ins_data_t *data)
{
	/* insert all non Vendor Specific IEs that haven't been
	 * proclaimed so far before Vendor Specific IEs
	 */
	if (data->tag == DOT11_MNG_VS_ID)
		return TRUE;
	return FALSE;
}

static bool
wlc_assoc_req_mod_cb(void *ctx, wlc_iem_mod_data_t *data)
{
	/* modify Supported Rates, Extended Supported Rates and RSN IEs */
	if (data->ie[TLV_TAG_OFF] == DOT11_MNG_RATES_ID ||
	    data->ie[TLV_TAG_OFF] == DOT11_MNG_EXT_RATES_ID ||
		data->ie[TLV_TAG_OFF] == DOT11_MNG_RSN_ID)
		return TRUE;
	/* insert other IEs as is */
	return FALSE;
}

static uint8
wlc_assoc_req_vsie_cb(void *ctx, wlc_iem_cbvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

void *
wlc_sendassocreq(wlc_info_t *wlc, wlc_bss_info_t *bi, struct scb *scb, bool reassoc)
{
	void *pkt = NULL;
	uint8 *pbody;
	uint body_len;
	struct dot11_assoc_req *assoc;
	uint16 type;
	wlc_bsscfg_t *bsscfg;
	wlc_assoc_t *as;
	wlc_rateset_t sup_rates, ext_rates;
	wlc_pre_parse_frame_t ppf;
	wlc_iem_uiel_t uiel, *puiel;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	as = bsscfg->assoc;

	body_len = DOT11_ASSOC_REQ_FIXED_LEN;
	/* Current AP address */
	if (reassoc)
		body_len += ETHER_ADDR_LEN;

	type = (uint16)(reassoc ? FC_REASSOC_REQ : FC_ASSOC_REQ);

	/* special handling: Rates Overrides */
	bzero(&sup_rates, sizeof(sup_rates));
	bzero(&ext_rates, sizeof(ext_rates));
	bzero(&ppf, sizeof(ppf));
	ppf.sup = &sup_rates;
	ppf.ext = &ext_rates;

	if (wlc_arq_pre_parse_frame(wlc, bsscfg, type, as->ie, as->ie_len, &ppf) != BCME_OK) {
		WL_ERROR(("%s: wlc_arq_pre_parse_frame failed\n", __FUNCTION__));
		return NULL;
	}

	if ((bi->flags & WLC_BSS_54G) &&
	    (wlc->band->gmode != GMODE_LEGACY_B)) {
		/* Legacy 54g AP, so ignore rates overrides and put
		 * all rates in the supported rates elt
		 */
		sup_rates.count = wlc->band->hw_rateset.count;
		bcopy(wlc->band->hw_rateset.rates, sup_rates.rates, wlc->band->hw_rateset.count);
#ifdef WLP2P
		if (BSS_P2P_ENAB(wlc, bsscfg))
			wlc_p2p_rateset_filter(wlc->p2p, bsscfg, &sup_rates);
#endif
		ext_rates.count = 0;
	} else {
		/* create the ratesets with overrides taken into account
		 * target rateset has been filtered against our hw_rateset
		 */
		wlc_rateset_elts(wlc, bsscfg, &bi->rateset, &sup_rates, &ext_rates);
	}

	/* prepare IE mgmt calls */
	puiel = NULL;
	if (as->ie != NULL) {
		bzero(&uiel, sizeof(uiel));
		uiel.ies = as->ie;
		uiel.ies_len = as->ie_len;
		uiel.ins_fn = wlc_assoc_req_ins_cb;
		uiel.mod_fn = wlc_assoc_req_mod_cb;
		uiel.vsie_fn = wlc_assoc_req_vsie_cb;
		uiel.ctx = wlc;
		puiel = &uiel;
	}
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.assocreq.scb = scb;
	ftcbparm.assocreq.target = bi;
	ftcbparm.assocreq.sup = &sup_rates;
	ftcbparm.assocreq.ext = &ext_rates;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;
	cbparm.ht = N_ENAB(wlc->pub) && SCB_HT_CAP(scb) ? TRUE : FALSE;
#ifdef WL11AC
	cbparm.vht = VHT_ENAB(wlc->pub) && SCB_VHT_CAP(scb) ? TRUE : FALSE;
	if (cbparm.vht) {
		ftcbparm.assocreq.narrow_bw = CHSPEC_IS40(wlc->chanspec) ? NARROW_BW_40 :
			(CHSPEC_IS20(wlc->chanspec) ? NARROW_BW_20 : NARROW_BW_NONE);
	}
	else {
		/* clear the extend capabilities for Operating mode notification-VHT(11ac) */
		wlc_bsscfg_set_ext_cap(bsscfg, DOT11_EXT_CAP_OPER_MODE_NOTIF, FALSE);
	}
#endif
#ifdef WL11K
	if (!(WL11K_ENAB(wlc->pub) && wlc_rrm_enabled(wlc->rrm_info, bsscfg))) {
		/* clear the extend capabilities for Extended Channel Switching */
		wlc_bsscfg_set_ext_cap(bsscfg, DOT11_EXT_CAP_EXT_CHAN_SWITCHING, FALSE);
	}
#endif

	/* calculate IEs length */
	body_len += wlc_iem_calc_len(wlc->iemi, bsscfg, type, puiel, &cbparm);

	/* get a control packet */
	if ((pkt = wlc_frame_get_mgmt(wlc, type, &bi->BSSID, &bsscfg->cur_etheraddr, &bi->BSSID,
	                              body_len, &pbody)) == NULL) {
		wlc_assocreq_complete(wlc, TX_STATUS_NO_ACK, (void *)(uintptr)bsscfg->ID);
		WL_ERROR(("wl%d: %s: wlc_frame_get_mgmt failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	/* fill out the association request body */
	assoc = (struct dot11_assoc_req *)pbody;

	/* Capability information */
	assoc->capability = wlc_assoc_capability(wlc, bsscfg, bi);

	/* Listen interval */
	assoc->listen = htol16(as->listen);


	pbody += DOT11_ASSOC_REQ_FIXED_LEN;
	body_len -= DOT11_ASSOC_REQ_FIXED_LEN;

	/* Current AP address */
	if (reassoc) {
		if (!ETHER_ISNULLADDR(&as->bssid))
			bcopy(&as->bssid, pbody, ETHER_ADDR_LEN);
		else
			bcopy(&bsscfg->prev_BSSID, pbody, ETHER_ADDR_LEN);
		pbody += ETHER_ADDR_LEN;
		body_len -= ETHER_ADDR_LEN;
	}

	/* generate IEs */
	if (wlc_iem_build_frame(wlc->iemi, bsscfg, type, puiel, &cbparm,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, pkt, TRUE);
		return NULL;
	}

	wlc_bss_mac_event(wlc, bsscfg, WLC_E_ASSOC_REQ_IE, NULL, 0, 0, 0, pbody, body_len);

	/* Save a copy of the last association request */
	if (as->req != NULL) {
		MFREE(wlc->osh, as->req, as->req_len);
		as->req = NULL;
	}

	if (reassoc) {
		pbody -= ETHER_ADDR_LEN;
		body_len += ETHER_ADDR_LEN;
			}
	pbody -= DOT11_ASSOC_REQ_FIXED_LEN;
	body_len += DOT11_ASSOC_REQ_FIXED_LEN;
	as->req_is_reassoc = reassoc;
	if ((as->req = MALLOC(wlc->osh, body_len)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		as->req_len = 0;
	}
	else {
		bcopy(pbody, as->req, body_len);
		as->req_len = body_len;
	}

#if defined(BCMDBG) || defined(WLMSG_PRPKT)
	WL_ASSOC(("wl%d: JOIN: sending %s REQ ...\n",
	            WLCWLUNIT(wlc), (reassoc ? "REASSOC" : "ASSOC")));

	if (WL_ASSOC_ON()) {
		struct dot11_management_header *h =
		        (struct dot11_management_header *)PKTDATA(wlc->osh, pkt);
		uint l = PKTLEN(wlc->osh, pkt);
		wlc_print_assoc(wlc, h, l);
	}
#endif

	/* Send using the active_queue instead of the bsscfg queue. When roaming, the bsscfg queue
	 * will be associated with the current associated channel, not the roam target channel.
	 */
	if (!wlc_queue_80211_frag(wlc, pkt, wlc->active_queue, scb, bsscfg,
	                ((bi->capability & DOT11_CAP_SHORT) != 0),
	                NULL, WLC_LOWEST_SCB_RSPEC(scb))) {
		WL_ERROR(("wl%d: %s: wlc_queue_80211_frag failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	return pkt;
}



/* Enable/Disable IBSS operations in hardware */
static void
wlc_ibss_hw_enab(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool enab)
{
	uint32 infra;
	uint16 mhf;

	WL_TRACE(("wl%d.%d: wlc_ibss_hw_enab %d\n",
	          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), enab));

	ASSERT(!cfg->BSS);
	ASSERT(IBSS_HW_ENAB(cfg));

	if (enab) {
		/* enable bcn promisc mode */
		wlc->bcnmisc_ibss = TRUE;

		/* Switch back to IBSS STA mode */
		infra = 0;
		/* enable IBSS security mode */
		mhf = MHF4_IBSS_SEC;

		wlc_validate_bcn_phytxctl(wlc, cfg);
	}
	else {
		/* disable bcn promisc mode */
		wlc->bcnmisc_ibss = FALSE;

		/* Switch to BSS STA mode so that MAC will not send IBSS Beacons
		 * or respond to probe requests
		 */
		infra = MCTL_INFRA;
		/* disable IBSS security mode */
		mhf = 0;
	}

	/* update h/w */
	wlc_mac_bcn_promisc(wlc);
	wlc_mctrl(wlc, MCTL_INFRA, infra);
	if (IBSS_PEER_GROUP_KEY_ENAB(wlc->pub))
		wlc_mhf(wlc, MHF4, MHF4_IBSS_SEC, mhf, WLC_BAND_ALL);
}

void
wlc_ibss_enable_all(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

#ifdef WL_PM_MUTE_TX
	wlc_pm_mute_tx_pm_pending_complete(wlc->pm_mute_tx);
#endif

	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->BSS)
			continue;
		wlc_ibss_enable(cfg);
	}
}

void
wlc_ibss_disable_all(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->BSS)
			continue;
		wlc_ibss_disable(cfg);
	}
}

void
wlc_ibss_disable(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	ASSERT(!cfg->BSS);

	if (IBSS_HW_ENAB(cfg))
		wlc_ibss_hw_enab(wlc, cfg, FALSE);
	else
		wlc_ibss_mute_upd_notif(wlc, cfg, ON);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		/* enable P2P mode host flag in BSS mode only */
		wlc_mhf(wlc, MHF5, MHF5_P2P_MODE, MHF5_P2P_MODE, WLC_BAND_ALL);
	}
#endif /* WLMCNX */
}

void
wlc_ibss_enable(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	ASSERT(!cfg->BSS);

	if (IBSS_HW_ENAB(cfg))
		wlc_ibss_hw_enab(wlc, cfg, TRUE);
	else
		wlc_ibss_mute_upd_notif(wlc, cfg, OFF);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		/* disable P2P mode host flag in IBSS mode to allow beacon coalescing */
		wlc_mhf(wlc, MHF5, MHF5_P2P_MODE, 0, WLC_BAND_ALL);
	}
#endif /* WLMCNX */
}

static void
wlc_tkip_countermeasures(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;

	WL_WSEC(("wl%d: %s: TKIP countermeasures: tx status for 2nd"
		" MIC failure indication...\n", WLCWLUNIT(wlc), __FUNCTION__));

	/* make sure the scb still exists */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL) {
		WL_ERROR(("wl%d: %s: unable to find scb from the pkt %p\n",
		          wlc->pub->unit, __FUNCTION__, pkt));
		return;
	}

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	if ((txs & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
		WL_ERROR(("wl%d.%d: %s: No ack for TKIP countermeasures MIC report.\n",
			WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
	}

	/* notify keymgmt that we sent mic error report. It is up to the
	 * event handler to send deauth if necessary. This should be done
	 * only for STA BSSCFG
	 */
	wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_TKIP_CM_REPORTED,
		bsscfg, scb, NULL, pkt);
}

/* Use this packet tx completion callback as the last resort to advance
 * the scan engine state machine.
 */
static void
wlc_pm_notif_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	if (SCAN_IN_PROGRESS(wlc->scan)) {
		wlc_bsscfg_t *cfg = WLC_BSSCFG(wlc, WLPKTTAGBSSCFGGET(pkt));

		if (cfg != NULL && cfg->pm->PMpending)
			wlc_bss_pm_pending_reset(cfg, txs);
	}
}


#ifdef BCMSUP_PSK
/* called from per-port mic_error handlers because they can't see sup ptr */
bool
wlc_sup_mic_error(wlc_bsscfg_t *cfg, bool group)
{
	return wlc_sup_send_micfailure(cfg->wlc->idsup, cfg, group);
}
#endif /* BCMSUP_PSK */
#endif /* STA */

/* return true if pkt idx matches with idx value */
bool
wlc_ifpkt_chk_cb(void *p, int idx)
{
	return (WLPKTTAGBSSCFGGET(p) == idx);
}

void
wlc_getrand(wlc_info_t *wlc, uint8 *buf, int buflen)
{
	int rnd;

	if (buf != NULL) {
		while (buflen > 1) {
			rnd = R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random);
			*buf++ = (rnd >> 8) & 0xff;
			*buf++ = rnd & 0xff;
			buflen -= 2;
		}
		/* Was buflen odd? */
		if (buflen == 1) {
			rnd = R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random);
			*buf = rnd & 0xff;
		}
	}
}

static uint8
wlc_get_antennas(wlc_info_t *wlc)
{
	if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band))
		return ANTENNA_NUM_3;
	else if (WLCISNPHY(wlc->band))
		return ANTENNA_NUM_2;
	else if ((wlc->stf->ant_rx_ovr == ANT_RX_DIV_FORCE_0) ||
	    (wlc->stf->ant_rx_ovr == ANT_RX_DIV_FORCE_1))
		return ANTENNA_NUM_1;
	else
		return ANTENNA_NUM_2;
}

/* init tx reported rate mechanism */
void
wlc_reprate_init(wlc_info_t *wlc)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	FOREACH_BSS(wlc, i, bsscfg) {
		wlc_bsscfg_reprate_init(bsscfg);
	}
}


uint32
wlc_get_current_highest_rate(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	/* return a default value for rate */
	ratespec_t reprspec;
	uint8 bw = BW_20MHZ;
	bool sgi = (WLC_HT_GET_SGI_TX(wlc->hti)) ? TRUE : FALSE;
	wlcband_t *cur_band = wlc->band;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	if (cfg->associated) {
		uint8 vht_ratemask = 0;
		bool vht_ldpc = FALSE;

		if (N_ENAB(wlc->pub) && CHSPEC_IS40(current_bss->chanspec))
			bw = BW_40MHZ;
#ifdef WL11AC
		if (VHT_ENAB_BAND(wlc->pub, cur_band->bandtype)) {
			if (CHSPEC_IS80(current_bss->chanspec)) {
				bw = BW_80MHZ;
			}

			vht_ldpc = ((wlc->stf->ldpc_tx == AUTO) || (wlc->stf->ldpc_tx == ON));
			vht_ratemask = BAND_5G(wlc->band->bandtype) ?
				WLC_VHT_FEATURES_RATES_5G(wlc->pub) :
				WLC_VHT_FEATURES_RATES_2G(wlc->pub);
			/*
			* STA Mode  Look for scb corresponding to the bss.
			* APSTA Mode default to LDPC setting for AP mode.
			*/
			if (!AP_ENAB(wlc->pub)) {
				struct scb *scb = wlc_scbfind(wlc, cfg, &current_bss->BSSID);
				if (scb) {
					vht_ratemask = wlc_vht_get_scb_ratemask(wlc->vhti, scb);
					vht_ldpc &= (SCB_VHT_LDPC_CAP(wlc->vhti, scb));
				}
			}
		}
#endif /* WL11AC */
		/* associated, so return the max supported rate */
		reprspec = wlc_get_highest_rate(&current_bss->rateset, bw, sgi,
			vht_ldpc, vht_ratemask, wlc->stf->txstreams);
	} else {
		if (N_ENAB(wlc->pub)) {
			if (wlc_ht_is_40MHZ_cap(wlc->hti) &&
				((wlc_channel_locale_flags_in_band(wlc->cmi, cur_band->bandunit) &
				WLC_NO_40MHZ) == 0) &&
				WL_BW_CAP_40MHZ(cur_band->bw_cap)) {
				bw = BW_40MHZ;
			}
		}
		if (VHT_ENAB_BAND(wlc->pub, cur_band->bandtype)) {
			if (((wlc_channel_locale_flags_in_band(wlc->cmi, cur_band->bandunit) &
				WLC_NO_80MHZ) == 0) && WL_BW_CAP_80MHZ(cur_band->bw_cap)) {
				bw = BW_80MHZ;
			}
		}

		/* not associated, so return the max rate we can send */
		reprspec = wlc_get_highest_rate(&cur_band->hw_rateset, bw, sgi,
			(wlc->stf->ldpc_tx == AUTO), (BAND_5G(cur_band->bandtype)?
			WLC_VHT_FEATURES_RATES_5G(wlc->pub) : WLC_VHT_FEATURES_RATES_2G(wlc->pub)),
			wlc->stf->txstreams);

		/* Report the highest CCK rate if gmode == GMODE_LEGACY_B */
		if (BAND_2G(cur_band->bandtype) && (cur_band->gmode == GMODE_LEGACY_B)) {
			wlc_rateset_t rateset;
			wlc_rateset_filter(&cur_band->hw_rateset, &rateset, FALSE, WLC_RATES_CCK,
			                   RATE_MASK_FULL, wlc_get_mcsallow(wlc, cfg));
			reprspec = rateset.rates[rateset.count - 1];
		}
	}
	return reprspec;
}

/* return the "current tx rate" as a ratespec */
uint32
wlc_get_rspec_history(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wl_mac_ratehisto_res_t rate_histo;
	ratespec_t reprspec, highest_used_ratespec = 0;
	wlcband_t *cur_band = wlc->band;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	chanspec_t chspec;

	bzero(&rate_histo, sizeof(wl_mac_ratehisto_res_t));

	if (cfg->associated)
		cur_band = wlc->bandstate[CHSPEC_IS2G(current_bss->chanspec) ?
			BAND_2G_INDEX : BAND_5G_INDEX];

	reprspec = cur_band->rspec_override ? cur_band->rspec_override : 0;
	if (reprspec) {
		if (RSPEC_ISHT(reprspec) || RSPEC_ISVHT(reprspec)) {
			/* If bw in override spec not specified, use bw from chanspec */
			if (RSPEC_BW(reprspec) == RSPEC_BW_UNSPECIFIED) {

				if (cfg->associated) {
					chspec = current_bss->chanspec;
				} else {
					chspec = WLC_BAND_PI_RADIO_CHANSPEC;
				}

				if (CHSPEC_IS20(chspec)) {
					reprspec |= RSPEC_BW_20MHZ;
				} else if (CHSPEC_IS40(chspec)) {
					reprspec |= RSPEC_BW_40MHZ;
				} else if (CHSPEC_IS80(chspec)) {
					reprspec |= RSPEC_BW_80MHZ;
				} else if (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec)) {
					reprspec |= RSPEC_BW_160MHZ;
				}

			}
			if (WLC_HT_GET_SGI_TX(wlc->hti) == ON)
				reprspec |= RSPEC_SHORT_GI;
#if defined(WL_BEAMFORMING)
			if (TXBF_ENAB(wlc->pub) && wlc_txbf_get_applied2ovr(wlc->txbf))
				reprspec |= RSPEC_TXBF;
#endif
		}
		return reprspec;
	}

	/*
	 * Loop over txrspec history, looking up rate bins, and summing
	 * nfrags into appropriate supported rate bin.
	 */
	wlc_get_rate_histo_bsscfg(cfg, &rate_histo, &reprspec, &highest_used_ratespec);

	/* check for an empty history */
	if (reprspec == 0)
		return wlc_get_current_highest_rate(cfg);

	return (wlc->rpt_hitxrate ? highest_used_ratespec : reprspec);
}

/* Retrieve a consolidated set of revision information,
 * typically for the WLC_GET_REVINFO ioctl
 */
int
wlc_get_revision_info(wlc_info_t *wlc, void *buf, uint len)
{
	wlc_rev_info_t *rinfo = (wlc_rev_info_t *)buf;

	if (len < WL_REV_INFO_LEGACY_LENGTH)
		return BCME_BUFTOOSHORT;

	rinfo->vendorid = wlc->vendorid;
	rinfo->deviceid = wlc->deviceid;
	if (WLCISACPHY(wlc->band))
		rinfo->radiorev = (wlc->band->radiorev << IDCODE_ACPHY_REV_SHIFT) |
		                  (wlc->band->radioid << IDCODE_ACPHY_ID_SHIFT);
	else
		rinfo->radiorev = (wlc->band->radiorev << IDCODE_REV_SHIFT) |
		                  (wlc->band->radioid << IDCODE_ID_SHIFT);
	rinfo->chiprev = wlc->pub->sih->chiprev;
	rinfo->corerev = wlc->pub->corerev;
	rinfo->boardid = wlc->pub->sih->boardtype;
	rinfo->boardvendor = wlc->pub->sih->boardvendor;
	rinfo->boardrev = wlc->pub->boardrev;
	rinfo->ucoderev = wlc->ucode_rev;
	rinfo->driverrev = wlc->pub->cmn->driverrev;
	rinfo->bus = wlc->pub->sih->bustype;
	rinfo->chipnum = wlc->pub->sih->chip;

	if (len >= (OFFSETOF(wlc_rev_info_t, chippkg))) {
		rinfo->phytype = wlc->band->phytype;
		rinfo->phyrev = wlc->band->phyrev;
		rinfo->anarev = 0;	/* obsolete stuff, suppress */
	}

	if (len >= (OFFSETOF(wlc_rev_info_t, nvramrev))) {
		rinfo->chippkg = wlc->pub->sih->chippkg;
	}

	if (len >= sizeof(*rinfo)) {
		rinfo->nvramrev = wlc->nvramrev;
	}

	return BCME_OK;
}

/**
 * Several rate related functions have an 'mcsallow' parameter. That parameter is a bit mask that
 * indicates if 11n, 11n proprietary and/or 11ac MCS rates are allowed.
 *
 * Return value: the mcsallow flags based on status in pub and bsscfg.
 */
uint8
wlc_get_mcsallow(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint8 mcsallow = 0;

	if (wlc) {
		if (cfg) {
			if (BSS_N_ENAB(wlc, cfg))
				mcsallow |= WLC_MCS_ALLOW;
			if (BSS_VHT_ENAB(wlc, cfg))
				mcsallow |= WLC_MCS_ALLOW_VHT;
		} else {
			if (N_ENAB(wlc->pub))
				mcsallow |= WLC_MCS_ALLOW;
			if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype))
				mcsallow |= WLC_MCS_ALLOW_VHT;
		}
	}

	return mcsallow;
}

/*
 * Get the default rateset.
 */
void
wlc_default_rateset(wlc_info_t *wlc, wlc_rateset_t *rs)
{
	wlc_rateset_default(rs, NULL, wlc->band->phytype, wlc->band->bandtype, FALSE,
	                    RATE_MASK_FULL, wlc_get_mcsallow(wlc, NULL),
	                    CHSPEC_WLC_BW(wlc->default_bss->chanspec),
	                    wlc->stf->op_rxstreams);

	WL_RATE(("wl%d: %s: bandunit 0x%x phy_type 0x%x gmode 0x%x\n", wlc->pub->unit,
	        __FUNCTION__, wlc->band->bandunit, wlc->band->phytype, wlc->band->gmode));
#ifdef BCMDBG
	wlc_rateset_show(wlc, rs, &wlc->default_bss->BSSID);
#endif
}

static void
BCMATTACHFN(wlc_bss_default_init)(wlc_info_t *wlc)
{
	chanspec_t chanspec;
	wlcband_t * band;
	wlc_bss_info_t * bi = wlc->default_bss;

	/* init default and target BSS with some sane initial values */
	bzero((char*)(bi), sizeof(wlc_bss_info_t));
	bi->beacon_period = ISSIM_ENAB(wlc->pub->sih) ? BEACON_INTERVAL_DEF_QT :
		BEACON_INTERVAL_DEFAULT;
	bi->dtim_period = ISSIM_ENAB(wlc->pub->sih) ? DTIM_INTERVAL_DEF_QT :
	        DTIM_INTERVAL_DEFAULT;

	/* fill the default channel as the first valid channel
	 * starting from the 2G channels
	 */
	chanspec = wlc_next_chanspec(wlc->cmi, CH20MHZ_CHSPEC(0), CHAN_TYPE_ANY, TRUE);

	/* if there are no valid channels, use the first phy supported channel
	 * for the current band
	 */
	if (chanspec == INVCHANSPEC)
		chanspec = phy_utils_chanspec_band_firstch((phy_info_t *)WLC_PI(wlc),
			wlc->band->bandtype);

	wlc->home_chanspec = bi->chanspec = chanspec;

	/* find the band of our default channel */
	band = wlc->band;
	if (NBANDS(wlc) > 1 && band->bandunit != CHSPEC_WLCBANDUNIT(chanspec))
		band = wlc->bandstate[OTHERBANDUNIT(wlc)];

	/* init bss rates to the band specific default rate set */
	wlc_rateset_default(&bi->rateset, NULL, band->phytype, band->bandtype, FALSE,
		RATE_MASK_FULL, wlc_get_mcsallow(wlc, NULL),
		CHSPEC_WLC_BW(chanspec), wlc->stf->op_rxstreams);

	if (N_ENAB(wlc->pub))
		bi->flags |= WLC_BSS_HT;

	if (VHT_ENAB_BAND(wlc->pub, band->bandtype))
		bi->flags2 |= WLC_BSS_VHT;
}

/*
 * Parse a legacy rateset from a TLV section, optionally copy the raw version
 * into a *bss, sanitize and sort it, then optionally copy it into an scb.
 * Return 0 on success, nonzero on error.
 */
int
wlc_parse_rates(wlc_info_t *wlc, uchar *tlvs, uint tlvs_len, wlc_bss_info_t *bi,
	wlc_rateset_t *rates)
{
	wlc_rateset_t rs;
	int ext_count;
	bcm_tlv_t *ie;

	bzero(&rs, sizeof(wlc_rateset_t));

	/* pick out rateset from tlv section */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RATES_ID)) == NULL)
		return (-1);

	rs.count = MIN(ie->len, WLC_NUMRATES);
	bcopy(ie->data, (char*)rs.rates, rs.count);

	/* pick out extended rateset from tlv section */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_EXT_RATES_ID)) != NULL) {
		ext_count = MIN(ie->len, WLC_NUMRATES - (int)rs.count);
		bcopy(ie->data, (char*)(rs.rates + rs.count), ext_count);
		rs.count += ext_count;
	}

	/* copy raw set to bss_info */
	if (bi)
		bcopy((char*)&rs, (char*) &bi->rateset, sizeof(wlc_rateset_t));

	/* sort and sanitize them */
	if (!wlc_rate_hwrs_filter_sort_validate(&rs, &wlc->band->hw_rateset, FALSE,
		wlc->stf->txstreams)) {
		/* filter bss membership out of rateset */
		if (!wlc_ht_get_phy_membership(wlc->hti) || !wlc_bss_membership_filter(&rs))
			return (-1);
		if (bi)
			bcopy((char*)&rs, (char*) &bi->rateset, sizeof(wlc_rateset_t));
	}

	ASSERT(rs.count <= WLC_NUMRATES);

	if (rates)
		bcopy((char*)&rs, (char*)rates, sizeof(wlc_rateset_t));

	return (0);
}

/*
 * Combine Supported Rates and Extended Supported Rates in to a rateset,
 * sanitize and sort it.
 * Return BCME_OK on success, BCME_XXXX on error.
 */
int
wlc_combine_rateset(wlc_info_t *wlc, wlc_rateset_t *sup, wlc_rateset_t *ext, wlc_rateset_t *rates)
{
	int ext_count;

	ASSERT(sup != NULL);
	ASSERT(ext != NULL);
	ASSERT(rates != NULL);

	bzero(rates, sizeof(*rates));

	rates->count = MIN(sup->count, WLC_NUMRATES);
	bcopy(sup->rates, rates->rates, rates->count);

	ext_count = MIN(ext->count, WLC_NUMRATES - rates->count);
	bcopy(ext->rates, rates->rates + rates->count, ext_count);
	rates->count += ext_count;

	/* sort and sanitize them */
	if (!wlc_rate_hwrs_filter_sort_validate(rates, &wlc->band->hw_rateset,
	                FALSE, wlc->stf->txstreams)) {
		/* filter bss membership out of rateset */
		if (!wlc_ht_get_phy_membership(wlc->hti) || !wlc_bss_membership_filter(rates))
			return BCME_ERROR;
	}

	return BCME_OK;
}

void
wlc_rateset_elts(wlc_info_t *wlc, wlc_bsscfg_t *cfg, const wlc_rateset_t *rates,
	wlc_rateset_t *sup_rates, wlc_rateset_t *ext_rates)
{
	uint8 sup_bin[WLC_MAXRATE+1];
	uint8 ext_bin[WLC_MAXRATE+1];
	uint8 sup_pref_bin[WLC_MAXRATE+1];
	const wlc_rateset_t *sup_pref_rates;
	const wlc_rateset_t *hw_rs = &wlc->band->hw_rateset;
	uint8 r;
	uint sup_count, ext_count;
	uint i;
	int j;

	ASSERT(rates->count <= WLC_NUMRATES);
	ASSERT(sup_rates != NULL);

	bzero(sup_bin, sizeof(sup_bin));
	bzero(ext_bin, sizeof(ext_bin));
	bzero(sup_pref_bin, sizeof(sup_pref_bin));

	if (!wlc->band->gmode ||
#ifdef WLP2P
	    BSS_P2P_ENAB(wlc, cfg) ||
#endif
	    FALSE) {
#ifdef WLP2P
		wlc_rateset_t rs;

		if (BSS_P2P_ENAB(wlc, cfg)) {
			wlc_rateset_copy(rates, &rs);
			wlc_p2p_rateset_filter(wlc->p2p, cfg, &rs);
			rates = &rs;
		}
#endif
		/* All rates end up in the Supported Rates elt */
		sup_rates->count = rates->count;
		bcopy(rates->rates, sup_rates->rates, rates->count);
		/* setting ht phy membership */
		if (wlc_ht_get_phy_membership(wlc->hti)) {
			ASSERT(ext_rates != NULL);
			if (sup_rates->count < 8)
				sup_rates->rates[sup_rates->count++] =
					wlc_ht_get_phy_membership(wlc->hti);
			else if (ext_rates->count < 8)
				ext_rates->rates[ext_rates->count++] =
					wlc_ht_get_phy_membership(wlc->hti);
			else
				WL_ERROR(("Cannot support HT PHY Membership, rate IEs are full\n"));
		}
		return;
	}

	/* check for a supported rates override */
	if (sup_rates->count > 0) {
		/* split the rateset into supported and extended using the provided sup_rates
		 * as the guide
		 */
		sup_pref_rates = sup_rates;
	} else {
		/* use the default rate split of LRS rates (wlc_lrs_rates -> 1 2 5.5 11 18 24 36 54)
		 * in the supported rates, and others in extended rates
		 */
		sup_pref_rates = &wlc_lrs_rates;
	}

	/* init the supported rates preferred lookup */
	for (i = 0; i < sup_pref_rates->count; i++) {
		r = sup_pref_rates->rates[i] & RATE_MASK;
		sup_pref_bin[r] = r;
	}

	sup_count = 0;
	ext_count = 0;

	/* split the rates into a supported and ext supported using the preferred split. */
	for (i = 0; i < rates->count; i++) {
		uint8 rate = rates->rates[i];
		r = rate & RATE_MASK;
		if (sup_pref_bin[r]) {
			sup_bin[r] = rate;
			sup_count++;
		} else {
			ext_bin[r] = rate;
			ext_count++;
		}
	}

	if (sup_rates->count == 0 && ext_rates->count == 0) {
		/* fill up the supported rates to the full 8 rates unless there was an override */
		j = (int)hw_rs->count;
		for (i = sup_count; i < 8 && ext_count > 0; i++) {
			for (; j >= 0; j--) {
				r = hw_rs->rates[j] & RATE_MASK;
				if (ext_bin[r] != 0) {
					sup_bin[r] = ext_bin[r];
					sup_count++;
					ext_bin[r] = 0;
					ext_count--;
					break;
				}
			}
		}
	}

	/* fill out the sup_rates and ext_rates in sorted order */
	bzero(sup_rates, sizeof(wlc_rateset_t));
	bzero(ext_rates, sizeof(wlc_rateset_t));
	for (i = 0; i < hw_rs->count; i++) {
		r = hw_rs->rates[i] & RATE_MASK;
		if (sup_bin[r] != 0)
			sup_rates->rates[sup_rates->count++] = sup_bin[r];
		if (ext_bin[r] != 0)
			ext_rates->rates[ext_rates->count++] = ext_bin[r];
	}

	/* setting ht phy membership */
	if (wlc_ht_get_phy_membership(wlc->hti)) {
		if (sup_rates->count < 8)
			sup_rates->rates[sup_rates->count++] = wlc_ht_get_phy_membership(wlc->hti);
		else if (ext_rates->count < 8)
			ext_rates->rates[ext_rates->count++] = wlc_ht_get_phy_membership(wlc->hti);
		else
			WL_ERROR(("Cannot support BSS Membership, rate IEs are full\n"));
	}
}

/* Deferred event processing */
static void
wlc_process_eventq(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;
	wlc_event_t *etmp;

	while ((etmp = wlc_eventq_deq(wlc->eventq))) {
		/* Perform OS specific event processing */
		wl_event(wlc->wl, etmp->event.ifname, etmp);
		/* Perform common event notification */
		wlc_eventq_handle_ind(wlc->eventq, etmp);

		if (etmp->data) {
			MFREE(wlc->osh, etmp->data, etmp->event.datalen);
			etmp->data = NULL;
		}
		wlc_event_free(wlc->eventq, etmp);
	}
}

/* Immediate event processing.
 * Process the event and any events generated during the processing,
 * and queue all these events for deferred processing as well.
 */
void
wlc_process_event(wlc_info_t *wlc, wlc_event_t *e)
{
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	wlc_print_event(wlc, e);
#endif /* BCMDBG || BCMDBG_ERR */

#if defined(WOWLPF)
	if (WOWLPF_ENAB(wlc->pub)) {
		wlc_wowlpf_event_cb(wlc, e->event.event_type, e->event.reason);
	}
#endif

	/* deliver event to the port OS right now */
	wl_event_sync(wlc->wl, e->event.ifname, e);

	/* queue the event for 0 delay timer delivery */
	wlc_eventq_enq(wlc->eventq, e);
}

/* Record the destination interface pointer dst_wlcif in e->wlcif field and
 * init e->event.ifname field with a string that each port maintains for
 * the OS interface associated with the source interface pointed by src_wlcif.
 * dst_wlcif may become stale if one deletes the bsscfg associated with it
 * due to the async nature of events processing therefore an explicit flush
 * of all generated events is required when deleting a bsscfg.
 * See wlc_eventq_flush() in wlc_bsscfg_free().
 */
void
_wlc_event_if(wlc_info_t *wlc, wlc_event_t *e,
	wlc_bsscfg_t *src_bsscfg, wlc_if_t *src_wlcif, wlc_if_t *dst_wlcif)
{
	wl_if_t *wlif = NULL;

	ASSERT(dst_wlcif != NULL);

	e->wlcif = dst_wlcif;

	if (src_bsscfg != NULL)
		e->event.bsscfgidx = WLC_BSSCFG_IDX(src_bsscfg);
	else {
		e->event.bsscfgidx = 0;
		e->event.flags |= WLC_EVENT_MSG_UNKBSS;
	}
	if (src_wlcif != NULL)
		e->event.ifidx = src_wlcif->index;
	else {
		e->event.ifidx = 0;
		e->event.flags |= WLC_EVENT_MSG_UNKIF;
	}

	if (src_wlcif != NULL)
		wlif = src_wlcif->wlif;

	strncpy(e->event.ifname, wl_ifname(wlc->wl, wlif), sizeof(e->event.ifname) - 1);
	e->event.ifname[sizeof(e->event.ifname) - 1] = 0;
}

/* Send the event to the OS interface associated with the scb (used by WDS,
 * looked up by the addr parm if valid) or the bsscfg. In case the bsscfg
 * parm is unknown (NULL) or the bsscfg doesn't have an OS visible interface
 * send the event to the primary interface.
 */
void
wlc_event_if(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_event_t *e, const struct ether_addr *addr)
{
	struct scb *scb;
	wlc_if_t *wlcif;

	if (addr != NULL && !ETHER_ISMULTI(addr)) {
		bcopy(addr->octet, e->event.addr.octet, ETHER_ADDR_LEN);
		e->addr = &(e->event.addr);
	}

	if (addr != NULL && !ETHER_ISMULTI(addr) && cfg != NULL &&
	    (scb = wlc_scbfind(wlc, cfg, addr)) != NULL &&
	    (wlcif = SCB_WDS(scb)) != NULL && wlcif->wlif != NULL && !SCB_DWDS(scb))
		_wlc_event_if(wlc, e, SCB_BSSCFG(scb), wlcif, wlcif);
	else if (cfg != NULL && (wlcif = cfg->wlcif) != NULL) {
		if (wlcif->wlif != NULL)
			_wlc_event_if(wlc, e, cfg, wlcif, wlcif);
		else
			_wlc_event_if(wlc, e, cfg, wlcif, wlc->cfg->wlcif);
	}
	else
		_wlc_event_if(wlc, e, NULL, NULL, wlc->cfg->wlcif);
}

/* Check whether an event requires a wl_bss_info_t TLV to be appended to it.
 * If yes, returns the bssinfo to append.  If no, returns NULL.
 */
static wlc_bss_info_t*
wlc_event_needs_bssinfo_tlv(uint msg, wlc_bsscfg_t *bsscfg)
{
	if (msg == WLC_E_AUTH || msg == WLC_E_ASSOC || msg == WLC_E_REASSOC)
		return bsscfg->target_bss;
	else if (msg == WLC_E_SET_SSID || msg == WLC_E_DEAUTH_IND ||
		msg == WLC_E_DISASSOC_IND || msg == WLC_E_ROAM)
		return bsscfg->current_bss;
	else
		return NULL;
}

/* Some events require specifying the BSS config (assoc in particular). */
void
wlc_bss_mac_event(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg, uint msg, const struct ether_addr* addr,
	uint result, uint status, uint auth_type, void *data, int datalen)
{
	wlc_bss_mac_rxframe_event(wlc, bsscfg, msg, addr, result, status, auth_type,
	                          data, datalen, NULL);
}

void
wlc_bss_mac_rxframe_event(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg, uint msg,
                          const struct ether_addr* addr,
                          uint result, uint status, uint auth_type, void *data, int datalen,
                          wl_event_rx_frame_data_t *rxframe_data)
{
	wlc_event_t *e;
	wlc_bss_info_t *current_bss = NULL; /* if not NULL, add bss_info to event data */
	uint8 *bi_data = NULL;
	uint edatlen = 0;
	brcm_prop_ie_t *event_ie;

	/* Get the bss_info TLV to add to selected events */
	if (EVDATA_BSSINFO_ENAB(wlc->pub) && (bsscfg != NULL) && (rxframe_data == NULL))
		current_bss = wlc_event_needs_bssinfo_tlv(msg, bsscfg);

	/* There are many WLC_E_TXFAIL but currently no special handling;
	 * possible optimization is to drop it here unless requested.
	 *
	 * if (msg == WLC_E_TXFAIL)
	 * 	if (!wlc_eventq_test_ind(eq, e->event.event_type))
	 *		return;
	 */
	e = wlc_event_alloc(wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: %s wlc_event_alloc failed\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	e->event.event_type = msg;
	e->event.status = result;
	e->event.reason = status;
	e->event.auth_type = auth_type;

	if (rxframe_data)
		datalen += sizeof(wl_event_rx_frame_data_t);

	e->event.datalen = datalen;
	/* If a bss_info TLV needs to be appended to the event data
	 *   Add the bss_info TLV size to event data size and allocate memory
	 *   for the event data.
	 *   If that fails then try alloc from the eventd pool.
	 *   If that fails then the NULL bi_data indicates do not include the
	 *   bss_info TLV in the event data.
	 */
	if (EVDATA_BSSINFO_ENAB(wlc->pub) && (current_bss != NULL)) {
		edatlen = datalen + BRCM_PROP_IE_LEN + sizeof(wl_bss_info_t);
		e->data = MALLOC(wlc->osh, edatlen);
		if (e->data != NULL)
			bi_data = e->data;
	}
	if ((e->event.datalen > 0) && (data != NULL)) {
		if (EVDATA_BSSINFO_ENAB(wlc->pub)) {
			if (e->data == NULL)
				e->data = MALLOC(wlc->osh, e->event.datalen);
		} else
			e->data = MALLOC(wlc->osh, e->event.datalen);
		if (e->data == NULL) {
			wlc_event_free(wlc->eventq, e);
			WL_ERROR(("wl%d: %s MALLOC failed\n", wlc->pub->unit, __FUNCTION__));
			return;
		}

		if (rxframe_data) {
			bcopy(rxframe_data, e->data, sizeof(wl_event_rx_frame_data_t));
			bcopy(data, ((wl_event_rx_frame_data_t *)(e->data)) + 1,
			      e->event.datalen - sizeof(wl_event_rx_frame_data_t));
		} else
			bcopy(data, e->data, e->event.datalen);
		if (EVDATA_BSSINFO_ENAB(wlc->pub) && (bi_data != NULL))
			bi_data =  (uint8*)e->data + e->event.datalen;
	}

	/* Add bssinfo TLV to bi_data */
	if (EVDATA_BSSINFO_ENAB(wlc->pub) && (bi_data != NULL)) {
		e->event.datalen = edatlen;
		event_ie = (brcm_prop_ie_t*)bi_data;
		event_ie->id = DOT11_MNG_PROPR_ID;
		event_ie->len = (BRCM_PROP_IE_LEN - TLV_HDR_LEN) + sizeof(wl_bss_info_t);
		bcopy(BRCM_PROP_OUI, event_ie->oui, DOT11_OUI_LEN);
		event_ie->type = BRCM_EVT_WL_BSS_INFO;

		if (wlc_bss2wl_bss(wlc, current_bss,
		                   (wl_bss_info_t*)((uint8 *)event_ie + BRCM_PROP_IE_LEN),
		                   sizeof(wl_bss_info_t), FALSE)) {
			/* Really shouldn't happen, but if so then omit the bssinfo TLV */
			e->event.datalen = datalen;
		}

	}

	wlc_event_if(wlc, bsscfg, e, addr);

	wlc_process_event(wlc, e);
}

/* Wrapper for wlc_bss_mac_event when no BSS is specified. */
void
wlc_mac_event(wlc_info_t* wlc, uint msg, const struct ether_addr* addr,
	uint result, uint status, uint auth_type, void *data, int datalen)
{
	wlc_bss_mac_event(wlc, NULL, msg, addr, result, status, auth_type, data, datalen);
}

/*
 * The struct ether_addr arg is (when non-NULL) the mac addr
 * of the AP to which we're associating. It's used by nas supplicant
 * in the ap-sta build variant.
 *
 */
void
wlc_link(wlc_info_t *wlc, bool isup, struct ether_addr *addr, wlc_bsscfg_t *bsscfg, uint reason)
{
	wlc_event_t *e;
	wlc_bss_info_t *current_bss;
#ifdef STA
	uint8 *edata, *secdata = NULL;
	uint edatlen, secdatlen = 0;
	brcm_prop_ie_t *event_ie;
#endif /* STA */

	ASSERT(bsscfg != NULL);

	current_bss = bsscfg->current_bss;
	BCM_REFERENCE(current_bss);

#ifdef STA
	/* When the link goes down, driver is no longer preserving the association */
	if (!isup)
		bsscfg->assoc->preserved = FALSE;
#endif

	/* Allow for link-up suppression */
	if (isup && wlc->nolinkup)
		return;

	e = wlc_event_alloc(wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: %s wlc_event_alloc failed\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	e->event.event_type = WLC_E_LINK;
	e->event.flags = isup ? WLC_EVENT_MSG_LINK : 0;
	e->event.reason = reason;

#ifdef STA

	/* send WPA/WPA2 IEs in the probe resp as event data */
	if (BSSCFG_STA(bsscfg) && bsscfg->associated &&
	    WSEC_ENABLED(bsscfg->wsec) &&
	    current_bss->bcn_prb != NULL &&
	    current_bss->bcn_prb_len > DOT11_BCN_PRB_LEN) {
		uint8 *parse;
		uint len;
		bcm_tlv_t *ie;

		parse = (uint8 *)current_bss->bcn_prb + sizeof(struct dot11_bcn_prb);
		len = current_bss->bcn_prb_len - sizeof(struct dot11_bcn_prb);

		if ((bcmwpa_is_wpa_auth(bsscfg->WPA_auth) &&
		     (ie = (bcm_tlv_t *)bcm_find_wpaie(parse, len)) != NULL) ||
		    (bcmwpa_is_wpa2_auth(bsscfg->WPA_auth) &&
		     (ie = bcm_parse_tlvs(parse, len, DOT11_MNG_RSN_ID)) != NULL) ||
		    FALSE) {

#if defined(WLFBT)
			/* Don't report the association event if it's a fast transition */
			if (WLFBT_ENAB(wlc->pub) && (bcmwpa_is_wpa2_auth(wlc->cfg->WPA_auth)) &&
				isup && reason) {
				wlc_event_free(wlc->eventq, e);
				return;
			}
#endif /* WLFBT */

			if (EVDATA_BSSINFO_ENAB(wlc->pub)) {
				secdata = (uint8*)ie;
				secdatlen = ie->len + TLV_HDR_LEN;
			} else {
				e->data = MALLOC(wlc->osh, ie->len + TLV_HDR_LEN);
				if (e->data != NULL) {
					e->event.datalen = ie->len + TLV_HDR_LEN;
					bcopy(ie, e->data, e->event.datalen);
				}
			}
		}
	}

	if (EVDATA_BSSINFO_ENAB(wlc->pub)) {
		/* Malloc a block for event data: wl_bss_info_t plus any security IE */
		edatlen = secdatlen;
		edatlen += BRCM_PROP_IE_LEN + sizeof(wl_bss_info_t);
		edata = MALLOC(wlc->osh, edatlen);

		/* If we got enough for both, do bss_info first, move later if need room for IE */
		if (edata != NULL) {
			event_ie = (brcm_prop_ie_t*)edata;
			event_ie->id = DOT11_MNG_PROPR_ID;
			event_ie->len = (BRCM_PROP_IE_LEN - TLV_HDR_LEN) + sizeof(wl_bss_info_t);
			bcopy(BRCM_PROP_OUI, event_ie->oui, DOT11_OUI_LEN);
			event_ie->type = BRCM_EVT_WL_BSS_INFO;

			if (wlc_bss2wl_bss(wlc, current_bss,
				(wl_bss_info_t*)((uint8 *)event_ie + BRCM_PROP_IE_LEN),
				sizeof(wl_bss_info_t), FALSE)) {
				/* Really shouldn't happen, but if so skip it */
				ASSERT(0);
				MFREE(wlc->osh, edata, edatlen);
				edata = NULL;
			}
		}

		/* Now deal with security IE: slide bss_info over, or malloc new if needed */
		if (secdatlen) {
			if (edata) {
				memmove(edata + secdatlen, edata,
					BRCM_PROP_IE_LEN + sizeof(wl_bss_info_t));
			} else {
				/* Malloc a block for event data without wl_bss_info_t */
				edatlen = secdatlen;
				edata = MALLOC(wlc->osh, edatlen);
			}

			if (edata) {
				bcopy(secdata, edata, secdatlen);
			}
		}

		/* Either way, if we have data then link it to the event */
		if (edata) {
			e->data = edata;
			e->event.datalen = edatlen;
		} else {
			e->data = NULL;
			e->event.datalen = 0;
		}
	}
#endif /* STA */

	wlc_event_if(wlc, bsscfg, e, addr);

	wlc_process_event(wlc, e);
}

#if !defined(WLNOEIND)
/* Send BRCM encapsulated EAPOL Events to applications. */
void
wlc_bss_eapol_event(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea,
	uint8 *data, uint32 len)
{
	wlc_pub_t *pub = wlc->pub;
	wlc_event_t *e;

	/* `data' should point to a WPA info element */
	if (data == NULL || len == 0) {
		WL_ERROR(("wl%d: 802.1x missing", pub->unit));
		return;
	}

	e = wlc_event_alloc(wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: %s wlc_event_alloc failed\n", pub->unit, __FUNCTION__));
		return;
	}

	e->event.event_type = WLC_E_EAPOL_MSG;
	e->event.status = WLC_E_STATUS_SUCCESS;
	e->event.reason = 0;
	e->event.auth_type = 0;

	e->event.datalen = len;
	e->data = MALLOC(pub->osh, e->event.datalen);
	if (e->data == NULL) {
		wlc_event_free(wlc->eventq, e);
		WL_ERROR(("wl%d: %s MALLOC failed\n", pub->unit, __FUNCTION__));
		return;
	}

	bcopy(data, e->data, e->event.datalen);

	wlc_event_if(wlc, bsscfg, e, ea);

	WL_WSEC(("wl%d: notify NAS of 802.1x frame data len %d\n", pub->unit, len));
	wlc_process_event(wlc, e);
}

void
wlc_bss_eapol_wai_event_indicate(wlc_info_t *wlc, void *p, struct scb *scb, wlc_bsscfg_t *bsscfg,
	struct ether_header *eh)
{
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) */

	if (!WLEIND_ENAB(wlc->pub) || wlc->mac_spoof)
		return;

	/* encapsulate 802.1x frame within 802.3 LLC/SNAP */
	/*
	* Encap. is for userland applications such as NAS to tell the
	* origin of the data frame when they can't directly talk to the
	* interface, for example, they run behind a software bridge
	* where interface info is lost (it can't be identified only
	* from MAC address, for example, both WDS interface and wireless
	* interface have the save MAC address).
	*
	* If IBSS, only send up if this is from a station that is a peer.
	* Also send original one up.
	*/
	if (ntoh16(eh->ether_type) == ETHER_TYPE_802_1X) {
		if (BSSCFG_AP(bsscfg)) {
			WL_APSTA_RX(("wl%d: %s: gen EAPOL event for AP %s\n",
			             wlc->pub->unit, __FUNCTION__,
			             bcm_ether_ntoa(&scb->ea, eabuf)));
		}
		if (bsscfg->BSS || SCB_IS_IBSS_PEER(scb)) {
			/*
			 * Generate EAPOL event ONLY when it is destined
			 * to the local interface address.
			 */
			if (bcmp(&eh->ether_dhost, &bsscfg->cur_etheraddr, ETHER_ADDR_LEN) == 0)
				wlc_bss_eapol_event(wlc, bsscfg, &scb->ea,
				PKTDATA(wlc->osh, p) + ETHER_HDR_LEN,
				PKTLEN(wlc->osh, p) - ETHER_HDR_LEN);
#if defined(WLMSG_INFORM)
			else {
				char eabuf_dst[ETHER_ADDR_STR_LEN];
				char eabuf_src[ETHER_ADDR_STR_LEN];
				struct ether_addr *dst = (struct ether_addr *)&eh->ether_dhost[0];
				WL_INFORM(("wl%d: %s: Prevent gen EAPOL event from %s to %s\n",
					wlc->pub->unit, __FUNCTION__,
					bcm_ether_ntoa(&scb->ea, eabuf_src),
					bcm_ether_ntoa(dst, eabuf_dst)));
			}
#endif /* WLMSG_INFORM */
		}
	}
}
#endif /* !defined(WLNOEIND) */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
static void
wlc_print_event(wlc_info_t *wlc, wlc_event_t *e)
{
	uint msg = e->event.event_type;
	struct ether_addr *addr = e->addr;
	uint result = e->event.status;
	char eabuf[ETHER_ADDR_STR_LEN];
#if defined(BCMDBG) || defined(WLMSG_INFORM)

	uint auth_type = e->event.auth_type;
	const char *auth_str;
	const char *event_name;
	uint status = e->event.reason;
	char ssidbuf[SSID_FMT_BUF_LEN];

	event_name = bcmevent_get_name(msg);
#endif /* BCMDBG || WLMSG_INFORM */

	if (addr != NULL)
		bcm_ether_ntoa(addr, eabuf);
	else
		strncpy(eabuf, "<NULL>", 7);

	switch (msg) {
	case WLC_E_START:
	case WLC_E_DEAUTH:
	case WLC_E_ASSOC_IND:
	case WLC_E_REASSOC_IND:
	case WLC_E_DISASSOC:
	case WLC_E_EAPOL_MSG:
	case WLC_E_BCNRX_MSG:
	case WLC_E_BCNSENT_IND:
	case WLC_E_ROAM_PREP:
	case WLC_E_BCNLOST_MSG:
	case WLC_E_PROBREQ_MSG:
#ifdef WLP2P
	case WLC_E_PROBRESP_MSG:
	case WLC_E_P2P_PROBREQ_MSG:
#endif
#if defined(NDIS) && (NDISVER >= 0x0620)
	case WLC_E_ASSOC_IND_NDIS:
	case WLC_E_REASSOC_IND_NDIS:
	case WLC_E_IBSS_COALESCE:
#endif /* NDIS && (NDISVER >= 0x0620) */
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s\n",
		           WLCWLUNIT(wlc), event_name, eabuf));
		break;

	case WLC_E_ASSOC:
	case WLC_E_REASSOC:
		if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, SUCCESS\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_TIMEOUT) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, TIMEOUT\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_ABORT) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, ABORT\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_NO_ACK) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, NO_ACK\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_UNSOLICITED) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, UNSOLICITED\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_FAIL) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, FAILURE, status %d\n",
			           WLCWLUNIT(wlc), event_name, eabuf, (int)status));
		} else {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, unexpected status %d\n",
			           WLCWLUNIT(wlc), event_name, eabuf, (int)result));
		}
		break;

	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC_IND:
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, reason %d\n",
		           WLCWLUNIT(wlc), event_name, eabuf, (int)status));
		break;

#if defined(BCMDBG) || defined(WLMSG_INFORM)
	case WLC_E_AUTH:
	case WLC_E_AUTH_IND: {
		char err_msg[32];

		if (auth_type == DOT11_OPEN_SYSTEM)
			auth_str = "Open System";
		else if (auth_type == DOT11_SHARED_KEY)
			auth_str = "Shared Key";
		else {
			snprintf(err_msg, sizeof(err_msg), "AUTH unknown: %d", (int)auth_type);
			auth_str = err_msg;
		}

		if (msg == WLC_E_AUTH_IND) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, %s\n",
			           WLCWLUNIT(wlc), event_name, eabuf, auth_str));
		} else if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, %s, SUCCESS\n",
			           WLCWLUNIT(wlc), event_name, eabuf, auth_str));
		} else if (result == WLC_E_STATUS_TIMEOUT) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, %s, TIMEOUT\n",
			           WLCWLUNIT(wlc), event_name, eabuf, auth_str));
		} else if (result == WLC_E_STATUS_FAIL) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, %s, FAILURE, status %d\n",
			           WLCWLUNIT(wlc), event_name, eabuf, auth_str, (int)status));
		}
		break;
	}
#endif /* BCMDBG || WLMSG_INFORM */
	case WLC_E_JOIN:
	case WLC_E_ROAM:
	case WLC_E_BSSID:
	case WLC_E_SET_SSID:
		if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_FAIL) {
			WL_INFORM(("wl%d: MACEVENT: %s, failed\n",
			           WLCWLUNIT(wlc), event_name));
		} else if (result == WLC_E_STATUS_NO_NETWORKS) {
			WL_INFORM(("wl%d: MACEVENT: %s, no networks found\n",
			           WLCWLUNIT(wlc), event_name));
		} else if (result == WLC_E_STATUS_ABORT) {
			WL_INFORM(("wl%d: MACEVENT: %s, ABORT\n",
			           wlc->pub->unit, event_name));
		} else {
			WL_INFORM(("wl%d: MACEVENT: %s, unexpected status %d\n",
			           WLCWLUNIT(wlc), event_name, (int)result));
		}
		break;

	case WLC_E_BEACON_RX:
		if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s, SUCCESS\n",
			           WLCWLUNIT(wlc), event_name));
		} else if (result == WLC_E_STATUS_FAIL) {
			WL_INFORM(("wl%d: MACEVENT: %s, FAIL\n",
			           WLCWLUNIT(wlc), event_name));
		} else {
			WL_INFORM(("wl%d: MACEVENT: %s, result %d\n",
			           WLCWLUNIT(wlc), event_name, result));
		}
		break;

	case WLC_E_NDIS_LINK:

	case WLC_E_LINK:
		WL_INFORM(("wl%d: MACEVENT: %s %s\n",
		           WLCWLUNIT(wlc), event_name,
		           (e->event.flags&WLC_EVENT_MSG_LINK)?"UP":"DOWN"));
		break;

	case WLC_E_MIC_ERROR:
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, Group: %s, Flush Q: %s\n",
		           WLCWLUNIT(wlc), event_name, eabuf,
		           (e->event.flags&WLC_EVENT_MSG_GROUP)?"Yes":"No",
		           (e->event.flags&WLC_EVENT_MSG_FLUSHTXQ)?"Yes":"No"));
		break;

	case WLC_E_ICV_ERROR:
	case WLC_E_UNICAST_DECODE_ERROR:
	case WLC_E_MULTICAST_DECODE_ERROR:
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s\n",
		           WLCWLUNIT(wlc), event_name, eabuf));
		break;

	case WLC_E_TXFAIL:
		/* TXFAIL messages are too numerous for WL_INFORM() */
		break;

	case WLC_E_COUNTRY_CODE_CHANGED: {
		char cstr[16];
		memset(cstr, 0, sizeof(cstr));
		memcpy(cstr, e->data, MIN(e->event.datalen, sizeof(cstr) - 1));
		WL_INFORM(("wl%d: MACEVENT: %s New Country: %s\n", WLCWLUNIT(wlc),
		           event_name, cstr));
		break;
	}

	case WLC_E_RETROGRADE_TSF:
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s\n",
		           WLCWLUNIT(wlc), event_name, eabuf));
		break;

#ifdef WIFI_ACT_FRAME
	case WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE:
#endif
	case WLC_E_SCAN_COMPLETE:
		if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s, SUCCESS\n",
			           WLCWLUNIT(wlc), event_name));
		} else if (result == WLC_E_STATUS_ABORT) {
			WL_INFORM(("wl%d: MACEVENT: %s, ABORTED\n",
			           WLCWLUNIT(wlc), event_name));
		} else {
			WL_INFORM(("wl%d: MACEVENT: %s, result %d\n",
			           WLCWLUNIT(wlc), event_name, result));
		}
		break;

	case WLC_E_AUTOAUTH:
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, result %d\n",
		           WLCWLUNIT(wlc), event_name, eabuf, (int)result));
		break;

	case WLC_E_ADDTS_IND:
		if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, SUCCESS\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_TIMEOUT) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, TIMEOUT\n",
			           WLCWLUNIT(wlc), event_name, eabuf));
		} else if (result == WLC_E_STATUS_FAIL) {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, FAILURE, status %d\n",
			           WLCWLUNIT(wlc), event_name, eabuf, (int)status));
		} else {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, unexpected status %d\n",
			           WLCWLUNIT(wlc), event_name, eabuf, (int)result));
		}
		break;

	case WLC_E_DELTS_IND:
		if (result == WLC_E_STATUS_SUCCESS) {
			WL_INFORM(("wl%d: MACEVENT: %s success ...\n",
			           WLCWLUNIT(wlc), event_name));
		} else if (result == WLC_E_STATUS_UNSOLICITED) {
			WL_INFORM(("wl%d: MACEVENT: DELTS unsolicited %s\n",
			           WLCWLUNIT(wlc), event_name));
		} else {
			WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, unexpected status %d\n",
			           WLCWLUNIT(wlc), event_name, eabuf, (int)result));
		}
		break;

	case WLC_E_PFN_NET_FOUND:
	case WLC_E_PFN_NET_LOST:
		WL_INFORM(("wl%d: PFNEVENT: %s, SSID %s, SSID len %d\n",
		           WLCWLUNIT(wlc), event_name,
		           (wlc_format_ssid(ssidbuf, e->data, e->event.datalen), ssidbuf),
		           e->event.datalen));
		break;

	case WLC_E_PSK_SUP:
		WL_INFORM(("wl%d: MACEVENT: %s, state %d, reason %d\n", WLCWLUNIT(wlc),
		           event_name, result, status));
		break;

#if defined(IBSS_PEER_DISCOVERY_EVENT)
	case WLC_E_IBSS_ASSOC:
		WL_INFORM(("wl%d: MACEVENT: %s, PEER %s\n", WLCWLUNIT(wlc), event_name, eabuf));
		break;
#endif /* defined(IBSS_PEER_DISCOVERY_EVENT) */


	case WLC_E_PSM_WATCHDOG:
		WL_INFORM(("wl%d: MACEVENT: %s, psmdebug 0x%x, phydebug 0x%x, psm_brc 0x%x\n",
		           WLCWLUNIT(wlc), event_name, result, status, auth_type));
		break;

	case WLC_E_TRACE:
		/* We don't want to trace the trace event */
		break;

#ifdef WIFI_ACT_FRAME
	case WLC_E_ACTION_FRAME_COMPLETE:
		WL_INFORM(("wl%d: MACEVENT: %s status: %s\n", WLCWLUNIT(wlc), event_name,
		           (result == WLC_E_STATUS_NO_ACK?"NO ACK":"ACK")));
		break;
#endif /* WIFI_ACT_FRAME */

	/*
	 * Events that don't require special decoding
	 */
	case WLC_E_ASSOC_REQ_IE:
	case WLC_E_ASSOC_RESP_IE:
	case WLC_E_PMKID_CACHE:
	case WLC_E_PRUNE:
	case WLC_E_RADIO:
	case WLC_E_IF:
	case WLC_E_EXTLOG_MSG:
	case WLC_E_RSSI:
	case WLC_E_ESCAN_RESULT:
	case WLC_E_PFN_SCAN_COMPLETE:
	case WLC_E_DCS_REQUEST:
	case WLC_E_CSA_COMPLETE_IND:
#ifdef WIFI_ACT_FRAME
	case WLC_E_ACTION_FRAME:
	case WLC_E_ACTION_FRAME_RX:
#endif
#ifdef WLP2P
	case WLC_E_P2P_DISC_LISTEN_COMPLETE:
#endif
#if defined(NDIS) && (NDISVER >= 0x0620)
	case WLC_E_PRE_ASSOC_IND:
	case WLC_E_PRE_REASSOC_IND:
	case WLC_E_AP_STARTED:
	case WLC_E_DFS_AP_STOP:
	case WLC_E_DFS_AP_RESUME:
#endif
#if defined(NDIS) && (NDISVER >= 0x0630)
	case WLC_E_ACTION_FRAME_RX_NDIS:
	case WLC_E_AUTH_REQ:
	case WLC_E_SPEEDY_RECREATE_FAIL:
	case WLC_E_ASSOC_RECREATED:
#endif /* NDIS && (NDISVER >= 0x0630) */
#ifdef PROP_TXSTATUS
	case WLC_E_FIFO_CREDIT_MAP:
	case WLC_E_BCMC_CREDIT_SUPPORT:
#endif
#ifdef P2PO
	case WLC_E_SERVICE_FOUND:
	case WLC_E_P2PO_ADD_DEVICE:
	case WLC_E_P2PO_DEL_DEVICE:
#endif
#if defined(P2PO) || defined(ANQPO)
	case WLC_E_GAS_FRAGMENT_RX:
	case WLC_E_GAS_COMPLETE:
#endif
	case WLC_E_WAKE_EVENT:
	case WLC_E_NATIVE:
		WL_INFORM(("wl%d: MACEVENT: %s\n", WLCWLUNIT(wlc), event_name));
		break;
#ifdef WLTDLS
	case WLC_E_TDLS_PEER_EVENT:
		WL_INFORM(("wl%d: MACEVENT: %s, MAC %s, reason %d\n",
			WLCWLUNIT(wlc), event_name, eabuf, (int)status));
		break;
#endif /* WLTDLS */
#if defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND)
	case WLC_E_PKTDELAY_IND:
		WL_INFORM(("wl%d: MACEVENT: %s\n", WLCWLUNIT(wlc), event_name));
		break;
#endif /* defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND) */
	case WLC_E_PSTA_PRIMARY_INTF_IND:
		WL_INFORM(("wl%d: MACEVENT: %s\n", WLCWLUNIT(wlc), event_name));
		break;
#if defined(WL_PROXDETECT)
	case WLC_E_PROXD:
		WL_INFORM(("wl%d: MACEVENT: %s\n", WLCWLUNIT(wlc), event_name));
		break;
#endif /* defined(WL_PROXDETECT) */

	case WLC_E_CCA_CHAN_QUAL:
		WL_INFORM(("wl%d: MACEVENT: %s\n", WLCWLUNIT(wlc), event_name));
		break;
#ifdef WLINTFERSTAT
	case WLC_E_TXFAIL_THRESH:
	WL_INFORM(("wl%d: MACEVENT: %s\n", WLCWLUNIT(wlc), event_name));
		break;
#endif  /* WLINTFERSTAT */

	default:
		WL_INFORM(("wl%d: MACEVENT: UNSUPPORTED %d, MAC %s, result %d, status %d,"
			" auth %d\n", WLCWLUNIT(wlc), msg, eabuf, (int)result, (int)status,
			(int)auth_type));
		break;
	}
}
#endif /* BCMDBG || BCMDBG_ERR */

/* Clean up monitor resources if driver goes down
 */
static void
wlc_monitor_down(wlc_info_t *wlc)
{
	if (wlc->monitor_amsdu_pkts)
		PKTFREE(wlc->osh, wlc->monitor_amsdu_pkts, FALSE);
	wlc->monitor_amsdu_pkts = NULL;
}

#ifdef WLTXMONITOR
/* Convert TX hardware status to standard format and send to wl_tx_monitor
 * assume p points to plcp header
 */
void
wlc_tx_monitor(wlc_info_t *wlc, d11txh_t *txh, tx_status_t *txs, void *p, struct wlc_if *wlcif)
{
	struct wl_txsts sts;
	ratespec_t rspec;
	uint16 chan_bw, chan_band, chan_num;
	uint16 xfts;
	uint8 frametype, *plcp;
	uint16 phytxctl1, phytxctl;
	struct dot11_header *h;
	if (p == NULL)
		WL_ERROR(("%s: null ptr2", __FUNCTION__));

	ASSERT(p != NULL);

	bzero((void *)&sts, sizeof(wl_txsts_t));

	sts.wlif = wlcif ? wlcif->wlif : NULL;

	sts.mactime = txs->lasttxtime;
	sts.retries = (txs->status.frag_tx_cnt);

	plcp = (uint8 *)(txh + 1);
	PKTPULL(wlc->osh, p, sizeof(d11txh_t));

	xfts = ltoh16(txh->XtraFrameTypes);
	chan_num = xfts >> XFTS_CHANNEL_SHIFT;
#if defined(WL11N) || defined(WL11AC)
	chan_band = (chan_num > CH_MAX_2G_CHANNEL) ? WL_CHANSPEC_BAND_5G : WL_CHANSPEC_BAND_2G;
#endif /* defined(WL11N) || defined(WL11AC) */
	phytxctl = ltoh16(txh->PhyTxControlWord);
	phytxctl1 = ltoh16(txh->PhyTxControlWord_1);
#ifdef WL11AC
	if (WLCISACPHY(wlc->band)) {
		wlc_vht_txmon_chspec(phytxctl, phytxctl1,
			chan_band, chan_num,
			&sts, &chan_bw);
	} else
#endif /* WL11AC */
#ifdef WL11N
	{
		wlc_ht_txmon_chspec(wlc->hti, phytxctl, phytxctl1,
			chan_band, chan_num,
			&sts, &chan_bw);
	}
#endif /* WL11N */

	frametype = phytxctl & PHY_TXC_FT_MASK;
#ifdef WL11AC
	if (frametype == FT_VHT) {
		wlc_vht_txmon_htflags(phytxctl, phytxctl1, plcp, chan_bw,
			chan_band, chan_num, &rspec, &sts);
	} else
#endif /* WL11AC */
#ifdef WL11N
	if (frametype == FT_HT) {
		wlc_ht_txmon_htflags(wlc->hti, phytxctl, phytxctl1, plcp, chan_bw,
			chan_band, chan_num, &rspec, &sts);
	} else
#endif /* WL11N */
		if (frametype == FT_OFDM) {
		rspec = ofdm_plcp_to_rspec(plcp[0]);
		sts.datarate = RSPEC2KBPS(rspec)/500;
		sts.encoding = WL_RXS_ENCODING_OFDM;
		sts.preamble = WL_RXS_PREAMBLE_SHORT;

		sts.phytype = WL_RXS_PHY_G;
	} else {
		ASSERT(frametype == FT_CCK);
		sts.datarate = plcp[0]/5;
		sts.encoding = WL_RXS_ENCODING_DSSS_CCK;
		sts.preamble = (phytxctl & PHY_TXC_SHORT_HDR) ?
		        WL_RXS_PREAMBLE_SHORT : WL_RXS_PREAMBLE_LONG;

		sts.phytype = WL_RXS_PHY_B;
	}

	sts.pktlength = PKTLEN(wlc->pub->osh, p) - D11_PHY_HDR_LEN;

	h = (struct dot11_header *)(plcp + D11_PHY_HDR_LEN);

	if (!ETHER_ISMULTI(txh->TxFrameRA) &&
	    !txs->status.was_acked)
	        sts.txflags |= WL_TXS_TXF_FAIL;
	if (ltoh16(txh->MacTxControlLow) & TXC_SENDCTS)
		sts.txflags |= WL_TXS_TXF_CTS;
	else if (ltoh16(txh->MacTxControlLow) & TXC_SENDRTS)
		sts.txflags |= WL_TXS_TXF_RTSCTS;

	wlc_ht_txmon_agg_ft(wlc->hti, p, h, frametype, &sts);

	wl_tx_monitor(wlc->wl, &sts, p);
}
#endif /* WLTXMONITOR */

#ifdef STA
static void
wlc_freqtrack(wlc_info_t *wlc)
{
	if (wlc->freqtrack_override != FREQTRACK_AUTO)
		return;

	ASSERT(wlc->pub->associated);

	if (!wlc->freqtrack && CHSPEC_IS2G(wlc->home_chanspec)) {
		WL_INFORM(("wl%d: FREQTRACK: Increasing freq tracking bw\n", wlc->pub->unit));
		wlc->freqtrack = TRUE;
		wlc->freqtrack_starttime = wlc->pub->now;
		wlc->freqtrack_attempts++;

		if (WLCISGPHY(wlc->band))
			wlc_phy_freqtrack_start(WLC_PI(wlc));
	}
}

void
wlc_freqtrack_reset(wlc_info_t *wlc)
{
	if (wlc->freqtrack_override != FREQTRACK_AUTO)
		return;

	wlc->freqtrack_attempts = 0;
	wlc->freqtrack_starttime = 0;

	if (wlc->freqtrack) {
		wlc->freqtrack = FALSE;

		if (WLCISGPHY(wlc->band))
			wlc_phy_freqtrack_end(WLC_PI(wlc));
	}
}

static void
wlc_freqtrack_verify(wlc_info_t *wlc)
{
	if (wlc->freqtrack_override != FREQTRACK_AUTO)
		return;

	ASSERT((wlc->freqtrack_attempts > 0) &&
		(wlc->freqtrack_attempts <= WLC_FREQTRACK_MIN_ATTEMPTS));

	if (wlc->freqtrack) {
		int idx;
		wlc_bsscfg_t *cfg;
		int count = 0;
		FOREACH_AS_STA(wlc, idx, cfg) {
			if (cfg->BSS && cfg->roam->time_since_bcn != 0)
				count ++;
		}
		if (count == 0) {
			/* Restore the frequency tracking bandwidth and check if we get
			 * beacons with the default settings. We will try this a few times
			 * before committing to the wide bandwidth
			 */
			WL_INFORM(("wl%d: FREQTRACK: Restoring freq tracking bw (attempts = %d)\n",
			           wlc->pub->unit, wlc->freqtrack_attempts));

			wlc->freqtrack = FALSE;

			if (WLCISGPHY(wlc->band))
				wlc_phy_freqtrack_end(WLC_PI(wlc));
		} else if ((wlc->pub->now - wlc->freqtrack_starttime) > WLC_FREQTRACK_DETECT_TIME) {
			/* We increased the bandwidth but did not get any beacons */
			wlc_freqtrack_reset(wlc);
		}
	}
	else if ((wlc->pub->now - wlc->freqtrack_starttime) > WLC_FREQTRACK_TIMEOUT) {
		/* Either the link is down or we have been receiving beacons with
		 * the default freq tracking b/w, for quite some time now. Clear
		 * the number of attempts.
		 */
		wlc_freqtrack_reset(wlc);
	}
}

#endif	/* STA */

/* Create the internal ratespec_t from the wl_ratespec */
static int
wlc_wlrspec2ratespec(uint32 wlrspec, ratespec_t *pratespec)
{
	bool islegacy = ((wlrspec & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_RATE);
	bool isht     = ((wlrspec & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_HT);
	bool isvht    = ((wlrspec & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_VHT);
	uint8 rate    =  (wlrspec & WL_RSPEC_RATE_MASK);
	uint32 bw     =  (wlrspec & WL_RSPEC_BW_MASK);
	uint txexp    = ((wlrspec & WL_RSPEC_TXEXP_MASK) >> WL_RSPEC_TXEXP_SHIFT);
	bool isstbc   = ((wlrspec & WL_RSPEC_STBC) != 0);
	bool issgi    = ((wlrspec & WL_RSPEC_SGI) != 0);
	bool isldpc   = ((wlrspec & WL_RSPEC_LDPC) != 0);
	ratespec_t rspec;

	*pratespec = 0;

	/* check for an uspecified rate */
	if (wlrspec == 0) {
		/* pratespec has already been cleared, return OK */
		return BCME_OK;
	}

	/* set the encoding to legacy, HT, or VHT as specified, and set the rate/mcs field */
	if (islegacy) {
		rspec = LEGACY_RSPEC(rate);
	} else if (isht) {
		rspec = HT_RSPEC(rate);
	} else if (isvht) {
		uint mcs = (WL_RSPEC_VHT_MCS_MASK & wlrspec);
		uint nss = ((WL_RSPEC_VHT_NSS_MASK & wlrspec) >> WL_RSPEC_VHT_NSS_SHIFT);
		rspec = VHT_RSPEC(mcs, nss);
	} else {
		return BCME_BADARG;
	}

	/* Tx chain expansion */
	rspec |= (txexp << RSPEC_TXEXP_SHIFT);

	/* STBC, LDPC and Short GI */
	if (isstbc) {
		rspec |= RSPEC_STBC;
	}

	if (isldpc) {
		 rspec |= RSPEC_LDPC_CODING;
	}

	if (issgi) {
		 rspec |= RSPEC_SHORT_GI;
	}

	/* Bandwidth */
	if (bw == WL_RSPEC_BW_20MHZ) {
		rspec |= RSPEC_BW_20MHZ;
	} else if (bw == WL_RSPEC_BW_40MHZ) {
		rspec |= RSPEC_BW_40MHZ;
	} else if (bw == WL_RSPEC_BW_80MHZ) {
		rspec |= RSPEC_BW_80MHZ;
	} else if (bw == WL_RSPEC_BW_160MHZ) {
		rspec |= RSPEC_BW_160MHZ;
	}

	*pratespec = rspec;

	return BCME_OK;
}

/* Create the internal ratespec_t from the wl_ratespec */
static int
wlc_ratespec2wlrspec(ratespec_t ratespec, uint32 *pwlrspec)
{
	uint8 rate = (ratespec & RSPEC_RATE_MASK);
	ratespec_t wlrspec;

	*pwlrspec = 0;

	/* set the encoding to legacy, HT, or VHT as specified, and set the rate/mcs field */
	if (RSPEC_ISLEGACY(ratespec)) {
		wlrspec = (WL_RSPEC_ENCODE_RATE | (rate & WL_RSPEC_RATE_MASK));
	} else if (RSPEC_ISHT(ratespec)) {
		wlrspec = (WL_RSPEC_ENCODE_HT | (rate & WL_RSPEC_RATE_MASK));
	} else if (RSPEC_ISVHT(ratespec)) {
		uint mcs = (RSPEC_VHT_MCS_MASK & ratespec);
		uint nss = ((RSPEC_VHT_NSS_MASK & ratespec) >> RSPEC_VHT_NSS_SHIFT);

		wlrspec = (WL_RSPEC_ENCODE_VHT |
		           ((nss << WL_RSPEC_VHT_NSS_SHIFT) & WL_RSPEC_VHT_NSS_MASK) |
		           (mcs & WL_RSPEC_VHT_MCS_MASK));
	} else {
		return BCME_BADARG;
	}

	/* Tx chain expansion, STBC, LDPC and Short GI */
	wlrspec |= RSPEC_TXEXP(ratespec) << WL_RSPEC_TXEXP_SHIFT;
	wlrspec |= RSPEC_ISSTBC(ratespec) ? WL_RSPEC_STBC : 0;
	wlrspec |= RSPEC_ISLDPC(ratespec) ? WL_RSPEC_LDPC : 0;
	wlrspec |= RSPEC_ISSGI(ratespec) ? WL_RSPEC_SGI : 0;
#ifdef WL_BEAMFORMING
	if (RSPEC_ISTXBF(ratespec)) {
		wlrspec |= WL_RSPEC_TXBF;
	}
#endif
	/* Bandwidth */
	if (RSPEC_IS20MHZ(ratespec)) {
		wlrspec |= WL_RSPEC_BW_20MHZ;
	} else if (RSPEC_IS40MHZ(ratespec)) {
		wlrspec |= WL_RSPEC_BW_40MHZ;
	} else if (RSPEC_IS80MHZ(ratespec)) {
		wlrspec |= WL_RSPEC_BW_80MHZ;
	} else if (RSPEC_IS160MHZ(ratespec)) {
		wlrspec |= WL_RSPEC_BW_160MHZ;
	}

	if (ratespec & RSPEC_OVERRIDE_RATE)
		wlrspec |= WL_RSPEC_OVERRIDE_RATE;
	if (ratespec & RSPEC_OVERRIDE_MODE)
		wlrspec |= WL_RSPEC_OVERRIDE_MODE;

	*pwlrspec = wlrspec;

	return BCME_OK;
}

int
wlc_set_iovar_ratespec_override(wlc_info_t *wlc, int band_id, uint32 wl_rspec, bool mcast)
{
	ratespec_t rspec;
	int err;

	WL_TRACE(("wl%d: %s: band %d wlrspec 0x%08X mcast %d\n", wlc->pub->unit, __FUNCTION__,
	          band_id, wl_rspec, mcast));

	err = wlc_wlrspec2ratespec(wl_rspec, &rspec);

	if (!err) {
		err = wlc_set_ratespec_override(wlc, band_id, rspec, mcast);
	} else {
		WL_TRACE(("wl%d: %s: rspec translate err %d\n",
		          wlc->pub->unit, __FUNCTION__, err));
	}

	return err;
}

/** transmit related, called when e.g. the user configures a fixed tx rate using the wl utility */
int
wlc_set_ratespec_override(wlc_info_t *wlc, int band_id, ratespec_t rspec, bool mcast)
{
	wlcband_t *band;
	bool islegacy = RSPEC_ISLEGACY(rspec) && (rspec != 0);
	bool isht = RSPEC_ISHT(rspec);
	bool isvht = RSPEC_ISVHT(rspec);
	uint8 rate = (rspec & RSPEC_RATE_MASK);
	uint32 bw = RSPEC_BW(rspec);
	uint txexp = RSPEC_TXEXP(rspec);
	bool isstbc = RSPEC_ISSTBC(rspec);
	bool issgi = RSPEC_ISSGI(rspec);
	bool isldpc = RSPEC_ISLDPC(rspec);
	uint Nss, Nsts;
	int bcmerror = 0;

	/* Default number of space-time streams for all legacy OFDM/DSSS/CCK is 1 */
	Nsts = 1;

	if (band_id == WLC_BAND_2G) {
		band = wlc->bandstate[BAND_2G_INDEX];
	} else {
		band = wlc->bandstate[BAND_5G_INDEX];
	}

	WL_TRACE(("wl%d: %s: band %d rspec 0x%08X mcast %d\n", wlc->pub->unit, __FUNCTION__,
	          band_id, rspec, mcast));

	/* check for HT/VHT mode items being set on a legacy rate */
	if (islegacy &&
	    (isldpc || issgi || isstbc)) {
		bcmerror = BCME_BADARG;
		WL_NONE(("wl%d: %s: err, legacy rate with ldpc:%d sgi:%d stbc:%d\n",
		         wlc->pub->unit, __FUNCTION__, isldpc, issgi, isstbc));
		goto done;
	}

	/* validate the combination of rate/mcs/stf is allowed */
	if (VHT_ENAB_BAND(wlc->pub, band_id) && isvht) {
		uint8 mcs = rate & WL_RSPEC_VHT_MCS_MASK;

		Nss = (rate & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;

		WL_NONE(("wl%d: %s: VHT, mcs:%d Nss:%d\n",
		         wlc->pub->unit, __FUNCTION__, mcs, Nss));

		/* VHT (11ac) supports MCS 0-9 */
		if (mcs > 9) {
			bcmerror = BCME_BADARG;
			goto done;
		}

		/* VHT (11ac) supports Nss 1-8 */
		if (Nss == 0 || Nss > 8) {
			bcmerror = BCME_BADARG;
			goto done;
		}

		/* STBC only supported by some phys */
		if (isstbc && !WLC_STBC_CAP_PHY(wlc)) {
			bcmerror = BCME_RANGE;
			goto done;
		}

		/* STBC expansion doubles the Nss */
		if (isstbc) {
			Nsts = Nss * 2;
		} else {
			Nsts = Nss;
		}
	} else if (N_ENAB(wlc->pub) && isht) {
		/* mcs only allowed when nmode */
		uint8 mcs = rate;

		/* none of the 11n phys support the defined HT MCS 33-76 */
		if (mcs > 32) {
			bcmerror = BCME_RANGE;
			goto done;
		}

		/* calculate the number of spatial streams, Nss.
		 * MCS 32 is a 40 MHz single stream, otherwise
		 * 0-31 follow a pattern of 8 MCS per Nss.
		 */
		Nss = (mcs == 32) ? 1 : (1 + (mcs / 8));

		/* STBC only supported by some phys */
		if (isstbc && !WLC_STBC_CAP_PHY(wlc)) {
			bcmerror = BCME_RANGE;
			goto done;
		}

		/* BRCM 11n phys only support STBC expansion doubling the num spatial streams */
		if (isstbc) {
			Nsts = Nss * 2;
		} else {
			Nsts = Nss;
		}

		/* mcs 32 is a special case, DUP mode 40 only */
		if (mcs == 32) {
			if (CHSPEC_IS20(wlc->home_chanspec) ||
			    (isstbc && (txexp > 1))) {
				bcmerror = BCME_RANGE;
				goto done;
			}
		}
	} else if (islegacy) {
		if (RATE_ISCCK(rate)) {
			/* DSSS/CCK only allowed in 2.4G */
			if (band->bandtype != WLC_BAND_2G) {
				bcmerror = BCME_RANGE;
				goto done;
			}
		} else if (BAND_2G(band->bandtype) && (band->gmode == GMODE_LEGACY_B)) {
			/* allow only CCK if gmode == GMODE_LEGACY_B */
			bcmerror = BCME_RANGE;
			goto done;
		} else if (!RATE_ISOFDM(rate)) {
			bcmerror = BCME_RANGE;
			goto done;
		}
	} else if (rspec != 0) {
		bcmerror = BCME_RANGE;
		goto done;
	}

	/* Validate a BW override if given
	 *
	 * Make sure the phy is capable
	 * Only VHT for 80MHz/80+80Mhz/160Mhz
	 * VHT/HT/OFDM for 40MHz, not CCK/DSSS
	 * MCS32 cannot be 20MHz, always 40MHz
	 */

	if (bw == RSPEC_BW_160MHZ) {
		if (!(WLC_8080MHZ_CAP_PHY(wlc) || WLC_160MHZ_CAP_PHY(wlc))) {
			bcmerror = BCME_RANGE;
			goto done;
		}
		/* only VHT & legacy mode can be specified for >40MHz */
		if (!isvht) {
			bcmerror = BCME_BADARG;
			goto done;
		}
	} else if (bw == RSPEC_BW_80MHZ) {
		if (!WLC_80MHZ_CAP_PHY(wlc)) {
			bcmerror = BCME_RANGE;
			goto done;
		}
		/* only VHT & legacy mode can be specified for >40MHz */
		if (!isvht && !islegacy) {
			bcmerror = BCME_BADARG;
			goto done;
		}
	} else if (bw == RSPEC_BW_40MHZ) {
		if (!WLC_40MHZ_CAP_PHY(wlc)) {
			bcmerror = BCME_RANGE;
			goto done;
		}
		/* DSSS/CCK are only 20MHz */
		if (RSPEC_ISCCK(rspec)) {
			bcmerror = BCME_BADARG;
			goto done;
		}
	} else if (bw == RSPEC_BW_20MHZ) {
		/* MCS 32 is 40MHz, cannot be forced to 20MHz */
		if (isht && rate == 32) {
			bcmerror = BCME_BADARG;
		}
	}

	/* more error checks if the rate is not 0 == 'auto' */
	if (rspec != 0) {
		/* add the override flags */
		rspec = (rspec | RSPEC_OVERRIDE_RATE | RSPEC_OVERRIDE_MODE);

		/* make sure there are enough tx streams available for what is specified */
		if ((Nsts + txexp) > wlc->stf->txstreams) {
			WL_TRACE(("wl%d: %s: err, (Nsts %d + txexp %d) > txstreams %d\n",
			          wlc->pub->unit, __FUNCTION__, Nsts, txexp, wlc->stf->txstreams));
			bcmerror = BCME_RANGE;
			goto done;
		}

		if ((rspec != 0) && !wlc_valid_rate(wlc, rspec, band->bandtype, TRUE)) {
			WL_TRACE(("wl%d: %s: err, failed wlc_valid_rate()\n",
			          wlc->pub->unit, __FUNCTION__));
			bcmerror = BCME_RANGE;
			goto done;
		}
	}

	/* set the rspec_override/mrspec_override for the given band */
	if (mcast) {
		band->mrspec_override = rspec;
	} else {
		band->rspec_override = rspec;
	}

	if (!mcast) {
		wlc_reprate_init(wlc); /* reported rate initialization */
	}

	/* invalidate txcache as transmit rate has changed */
	if (WLC_TXC_ENAB(wlc))
		wlc_txc_inv_all(wlc->txc);

done:
	return bcmerror;
}

int
wlc_sta_info(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea, void *buf, int len)
{
	sta_info_t sta;
	struct scb *scb;
	int bandunit, i;
	int copy_len;

	ASSERT(ea != NULL);
	if (ea == NULL)
		return (BCME_BADARG);

	ASSERT(bsscfg != NULL);
	if (bsscfg->up)
		bandunit = CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec);
	else
		bandunit = CHSPEC_WLCBANDUNIT(wlc->home_chanspec);

	if ((scb = wlc_scbfindband(wlc, bsscfg, ea, bandunit)) == NULL)
		return (BCME_BADADDR);

	/* fill in the sta_info struct */
	bzero(&sta, sizeof(sta_info_t));
	sta.ver = WL_STA_VER;
	sta.len = sizeof(sta_info_t);
	bcopy(ea, &sta.ea, ETHER_ADDR_LEN);
	bcopy(&scb->rateset, &sta.rateset, sizeof(wl_rateset_t));
	sta.idle = wlc->pub->now - scb->used;
	sta.flags = 0;
	sta.in = 0;
	if (scb->flags & SCB_BRCM)
		sta.flags |= WL_STA_BRCM;
	if (SCB_WME(scb))
		sta.flags |= WL_STA_WME;
	if (SCB_AUTHENTICATED(scb))
		sta.flags |= WL_STA_AUTHE;
	if (SCB_ASSOCIATED(scb)) {
		sta.flags |= WL_STA_ASSOC;
		sta.in = wlc->pub->now - scb->assoctime;
	}
	if (SCB_AUTHORIZED(scb))
		sta.flags |= WL_STA_AUTHO;
	if (AC_BITMAP_TST(scb->apsd.ac_defl, AC_BE))
		sta.flags |= WL_STA_APSD_BE;
	if (AC_BITMAP_TST(scb->apsd.ac_defl, AC_BK))
		sta.flags |= WL_STA_APSD_BK;
	if (AC_BITMAP_TST(scb->apsd.ac_defl, AC_VI))
		sta.flags |= WL_STA_APSD_VI;
	if (AC_BITMAP_TST(scb->apsd.ac_defl, AC_VO))
		sta.flags |= WL_STA_APSD_VO;
	sta.listen_interval_inms = 0;

	if (SCB_LEGACY_WDS(scb)) {
		sta.flags |= WL_STA_WDS;
		if (scb->flags & SCB_WDS_LINKUP)
			sta.flags |= WL_STA_WDS_LINKUP;
	}
	if (BSSCFG_AP(scb->bsscfg)) {
		if (SCB_PS(scb))
			sta.flags |= WL_STA_PS;
#ifdef AP
		sta.aid = scb->aid;
#endif /* AP */
		sta.cap = scb->cap;
		sta.listen_interval_inms = scb->listen * wlc->default_bss->beacon_period;
	}
	wlc_ht_fill_sta_fields(wlc->hti, scb, &sta);

	if (scb->wpaie && bcm_find_wpsie(scb->wpaie,
		scb->wpaie[TLV_LEN_OFF] + TLV_HDR_LEN))
		sta.flags |= WL_STA_WPS;

	if (scb->flags & SCB_NONERP)
		sta.flags |= WL_STA_NONERP;
	if (SCB_AMPDU(scb))
		sta.flags |= WL_STA_AMPDU_CAP;
	if (SCB_AMSDU(scb))
		sta.flags |= WL_STA_AMSDU_CAP;
#ifdef WL11N
	if (WLC_HT_GET_SCB_MIMOPS_ENAB(wlc->hti, scb))
		sta.flags |= WL_STA_MIMO_PS;
	if (WLC_HT_GET_SCB_MIMOPS_RTS_ENAB(wlc->hti, scb))
		sta.flags |= WL_STA_MIMO_RTS;
#endif /* WL11N */
	if (scb->flags & SCB_RIFSCAP)
		sta.flags |= WL_STA_RIFS_CAP;
	if (SCB_VHT_CAP(scb)) {
		sta.flags |= WL_STA_VHT_CAP;
#ifdef WL11AC
		sta.vht_flags = wlc_vht_get_scb_flags(wlc->vhti, scb);
#endif
	}

	/* update per antenna rssi and noise floor */
	/* to be done */

	if (len < (int)sizeof(sta_info_t))
		copy_len = WL_OLD_STAINFO_SIZE;
	else {
		copy_len = sizeof(sta_info_t);
#ifdef WLCNTSCB
		sta.flags |= WL_STA_SCBSTATS;
		sta.tx_failures = scb->scb_stats.tx_failures;
		sta.rx_ucast_pkts = scb->scb_stats.rx_ucast_pkts;
		sta.rx_mcast_pkts = scb->scb_stats.rx_mcast_pkts;
		sta.tx_rate = RSPEC2KBPS(scb->scb_stats.tx_rate);
		sta.tx_rate_fallback = RSPEC2KBPS(scb->scb_stats.tx_rate_fallback);
		sta.rx_rate = RSPEC2KBPS(scb->scb_stats.rx_rate);
		sta.rx_decrypt_succeeds = scb->scb_stats.rx_decrypt_succeeds;
		sta.rx_decrypt_failures = scb->scb_stats.rx_decrypt_failures;

		if (BSSCFG_AP(bsscfg)) {
			struct scb *bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);

			/* scb_pkts hold total unicast for AP SCB */
			sta.tx_pkts = scb->scb_stats.tx_pkts;	/* unicast TX */
			sta.tx_mcast_pkts = bcmc_scb ? bcmc_scb->scb_stats.tx_pkts : 0;
			sta.tx_mcast_bytes = bcmc_scb ? bcmc_scb->scb_stats.tx_ucast_bytes : 0;
		} else {
			/* scb_pkts hold total # for non-AP SCB */
			sta.tx_pkts = scb->scb_stats.tx_pkts - scb->scb_stats.tx_mcast_pkts;
			sta.tx_mcast_pkts = scb->scb_stats.tx_mcast_pkts;
			sta.tx_mcast_bytes = scb->scb_stats.tx_mcast_bytes;
		}
		sta.tx_ucast_bytes = scb->scb_stats.tx_ucast_bytes;
		sta.tx_tot_pkts = sta.tx_pkts + sta.tx_mcast_pkts;
		sta.tx_tot_bytes = sta.tx_ucast_bytes + sta.tx_mcast_bytes;
		sta.rx_ucast_bytes = scb->scb_stats.rx_ucast_bytes;
		sta.rx_mcast_bytes = scb->scb_stats.rx_mcast_bytes;
		sta.rx_tot_pkts = sta.rx_ucast_pkts + sta.rx_mcast_pkts;
		sta.rx_tot_bytes = sta.rx_ucast_bytes + sta.rx_mcast_bytes;
		sta.tx_pkts_retried = scb->scb_stats.tx_pkts_retried;
		sta.rx_pkts_retried = scb->scb_stats.rx_pkts_retried;

		/* WLAN TX statistics for link monitor */
		sta.tx_pkts_total = scb->scb_stats.tx_pkts_total;
		sta.tx_pkts_retries = scb->scb_stats.tx_pkts_retries;
		sta.tx_pkts_retry_exhausted = scb->scb_stats.tx_pkts_retry_exhausted;
		sta.tx_pkts_fw_total = scb->scb_stats.tx_pkts_fw_total;
		sta.tx_pkts_fw_retries = scb->scb_stats.tx_pkts_fw_retries;
		sta.tx_pkts_fw_retry_exhausted = scb->scb_stats.tx_pkts_fw_retry_exhausted;
#endif /* WLCNTSCB */

		for (i = WL_ANT_IDX_1; i < WL_RSSI_ANT_MAX; i++) {
			sta.rssi[i] = wlc_scb_rssi_chain(scb, i);
		}

		for (i = WL_ANT_IDX_1; i < WL_RSSI_ANT_MAX; i++)
			sta.rx_lastpkt_rssi[i] = wlc_scb_pkt_rssi_chain(scb, i);

		/* per core noise floor */
		for (i = 0; i < WL_RSSI_ANT_MAX; i++)
			sta.nf[i] = wlc_phy_noise_avg_per_antenna(WLC_PI(wlc), i);
	}


	/* bcopy to avoid alignment issues */
	bcopy(&sta, buf, copy_len);

	return (0);
}



#ifdef BCMDBG
/* reset the TSF register to the given value */
static void
wlc_tsf_set(wlc_info_t *wlc, uint32 tsf_l, uint32 tsf_h)
{
	osl_t *osh;
	osh = wlc->osh;

	W_REG(osh, &wlc->regs->tsf_timerlow, tsf_l);
	W_REG(osh, &wlc->regs->tsf_timerhigh, tsf_h);
}
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(WLMCHAN) || defined(SRHWVSDB)
/* adjust the TSF register by a 32 bit delta */
void
wlc_tsf_adjust(wlc_info_t *wlc, int delta)
{
	uint32 tsf_l, tsf_h;
	uint32 delta_h;
	osl_t *osh = wlc->osh;

	/* adjust the tsf time by offset */
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);
	/* check for wrap:
	 * if we are close to an overflow (2 ms) from tsf_l to high,
	 * make sure we did not read tsf_h after the overflow
	 */
	if (tsf_l > (uint32)(-2000)) {
		uint32 tsf_l_new;
		tsf_l_new = R_REG(osh, &wlc->regs->tsf_timerlow);
		/* update the tsf_h if tsf_l rolled over since we do not know if we read tsf_h
		 * before or after the roll over
		 */
		if (tsf_l_new < tsf_l)
			tsf_h = R_REG(osh, &wlc->regs->tsf_timerhigh);
		tsf_l = tsf_l_new;
	}

	/* sign extend delta to delta_h */
	if (delta < 0)
		delta_h = -1;
	else
		delta_h = 0;

	wlc_uint64_add(&tsf_h, &tsf_l, delta_h, (uint32)delta);

	W_REG(osh, &wlc->regs->tsf_timerlow, tsf_l);
	W_REG(osh, &wlc->regs->tsf_timerhigh, tsf_h);
}
#endif /* BCMDBG || WLMCHAN  || SRHWVSDB */

#if defined(WLTEST) || defined(WLPKTENG)
static void*
wlc_tx_testframe_get(wlc_info_t *wlc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len)
{
	int i, start = 0, len, buflen, filllen;
	void *p = NULL, *prevp = NULL, *headp = NULL;
	osl_t *osh;
	struct dot11_management_header *hdr;
	uint8 *pbody;
	bool first = TRUE;

	osh = wlc->osh;
	len = DOT11_MGMT_HDR_LEN + TXOFF + body_len;

	while (len > 0) {
		buflen = (len > PKTBUFSZ) ? PKTBUFSZ : len;

		if ((p = PKTGET(osh, buflen, TRUE)) == NULL) {
			WL_ERROR(("wl%d: wlc_frame_get_mgmt_test: pktget error for len %d \n",
				wlc->pub->unit, buflen));
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			PKTFREE(osh, headp, TRUE);
			return (NULL);
		}
		ASSERT(ISALIGNED((uintptr)PKTDATA(osh, p), sizeof(uint32)));

		if (first) {
			/* reserve TXOFF bytes of headroom */
			PKTPULL(osh, p, TXOFF);
			PKTSETLEN(osh, p, buflen - TXOFF);

			/* Set MAX Prio for MGMT packets */
			PKTSETPRIO(p, MAXPRIO);

			/* Remember the head pointer */
			headp = p;

			/* construct a management frame */
			hdr = (struct dot11_management_header *)PKTDATA(osh, p);
			wlc_fill_mgmt_hdr(hdr, FC_DATA, da, sa, bssid);

			/* Find start of data and length */
			pbody = (uint8 *)&hdr[1];
			filllen = buflen - TXOFF - sizeof(struct dot11_management_header);

			first = FALSE;
		} else {
			pbody = (uint8 *)PKTDATA(osh, p);
			filllen = buflen;
		}

		for (i = start; i < start + filllen; i++) {
			pbody[i - start] = (uint8)i;
		}
		start = i;

		/* chain the pkt buffer */
		if (prevp)
			PKTSETNEXT(osh, prevp, p);

		prevp = p;
		len -= PKTBUFSZ;
	};

	return (headp);
}

/* Create a test frame and enqueue into tx fifo */
void *
wlc_tx_testframe(wlc_info_t *wlc, struct ether_addr *da, struct ether_addr *sa,
                 ratespec_t rate_override, int length)
{
	void *p;
	bool shortpreamble;

	if ((p = wlc_tx_testframe_get(wlc, da, sa, sa, length)) == NULL)
		return NULL;

	/* check if the rate overrides are set */
	if (!RSPEC_ACTIVE(rate_override)) {
#if defined(NEW_TXQ) && defined(USING_MUX)
		if (ETHER_ISMULTI(da)) {
			rate_override = wlc->band->mrspec_override;
		} else {
			rate_override = wlc->band->rspec_override;
		}

		if (!RSPEC_ACTIVE(rate_override)) {
			rate_override = LEGACY_RSPEC(wlc->band->hwrs_scb->rateset.rates[0]);
		}

		/* default to channel BW if the rate overrides had BW unspecified */
		if (RSPEC_BW(rate_override) == RSPEC_BW_UNSPECIFIED) {
			if (CHSPEC_IS20(wlc->chanspec)) {
				rate_override |= RSPEC_BW_20MHZ;
			} else if (CHSPEC_IS40(wlc->chanspec)) {
				rate_override |= RSPEC_BW_40MHZ;
			} else if (CHSPEC_IS80(wlc->chanspec)) {
				rate_override |= RSPEC_BW_80MHZ;
			} else if (CHSPEC_IS8080(wlc->chanspec) || CHSPEC_IS160(wlc->chanspec)) {
				rate_override |= RSPEC_BW_160MHZ;
			} else {
				/* un-handled bandwidth */
				ASSERT(0);
			}
		}
#else
		if (ETHER_ISMULTI(da))
			rate_override = wlc->band->mrspec_override;
		else
			rate_override = wlc->band->rspec_override;

		if (!RSPEC_ACTIVE(rate_override))
			rate_override = wlc->band->hwrs_scb->rateset.rates[0] & RATE_MASK;
#endif /* defined(NEW_TXQ) && defined(USING_MUX) */
	}
	if (wlc->cfg->PLCPHdr_override == WLC_PLCP_LONG)
		shortpreamble = FALSE;
	else
		shortpreamble = TRUE;

	/* add headers */
	wlc_d11hdrs(wlc, p, wlc->band->hwrs_scb, shortpreamble, 0, 1,
	            TX_DATA_FIFO, 0, NULL, rate_override, NULL);

#ifndef WL_MULTIQUEUE
	WLPKTTAGSCBCLR(p);
#endif

	if (WLC_WAR16165(wlc))
		wlc_war16165(wlc, TRUE);

	/* Originally, we would call wlc_txfifo() here */
	/* wlc_txfifo(wlc, TX_DATA_FIFO, p, TRUE, 1); */
	/* However, that job is now the job of wlc_pkteng() */
	/* We return the packet so we can pass it in */
	/* as a parameter to wlc_pkteng() */

	return p;
}
#endif 


#ifdef WIFI_ACT_FRAME
#ifdef STA
static void
wlc_af_off_channel_callback(void * arg, int status, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;
	wlc_scan_info_t	*scan = wlc->scan;

	/* if AF is aborted */
	if (scan->action_frame) {
		wlc_actionframetx_complete(wlc, scan->action_frame, TX_STATUS_NO_ACK);
		PKTFREE(wlc->osh, scan->action_frame, TRUE);
		scan->action_frame = NULL;
	}

	/* Send back the channel switch indication to the host */
	wlc_bss_mac_event(wlc, cfg, WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE, NULL, status,
	                  0, 0, 0, 0);
}

static void
wlc_af_send(wlc_info_t *wlc, void *arg, uint *dwell)
{
	wlc_scan_info_t *scan = (wlc_scan_info_t *)arg;
	wlc_bsscfg_t *cfg = wlc_scan_bsscfg(scan);

	if (!ACT_FRAME_IN_PROGRESS(scan))
		return;

	if (scan->action_frame) {

		WL_SCAN(("wl%d: sending action frame\n", wlc->pub->unit));
		wlc_tx_action_frame_now(wlc, cfg, scan->action_frame);
		scan->action_frame = NULL;
	}
}
#endif /* STA */

static void *
wlc_prepare_action_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg, const struct ether_addr *bssid,
	void *action_frame)
{
	void *pkt;
	uint8* pbody;
	uint16 body_len;
	struct ether_addr da;
	uint32 packetId;
	wlc_pkttag_t *pkttag;

	bcopy((uint8*)action_frame +
		OFFSETOF(wl_action_frame_t, packetId), &packetId, sizeof(uint32));

	bcopy((uint8*)action_frame + OFFSETOF(wl_action_frame_t, len), &body_len, sizeof(body_len));

	bcopy((uint8*)action_frame + OFFSETOF(wl_action_frame_t, da), &da, ETHER_ADDR_LEN);

	if (bssid == NULL) {
		bssid = (WLC_BSS_CONNECTED(cfg)) ? &cfg->BSSID : &ether_bcast;
	}
	pkt = NULL;
	if (body_len) {
		/* get management frame */
		if ((pkt = wlc_frame_get_mgmt(wlc, FC_ACTION, &da, &cfg->cur_etheraddr,
			bssid, body_len, &pbody)) == NULL) {
			return NULL;
		}

		pkttag = WLPKTTAG(pkt);
		pkttag->shared.packetid = packetId;
		WLPKTTAGBSSCFGSET(pkt, cfg->_idx);

		bcopy((uint8*)action_frame + OFFSETOF(wl_action_frame_t, data), pbody, body_len);
	}

	return pkt;
}

static int
wlc_tx_action_frame_now(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt)
{
	ratespec_t rate_override = 0;

	if (BSS_P2P_ENAB(wlc, cfg))  {
		rate_override = WLC_RATE_6M;
	} else {
		rate_override = WLC_LOWEST_BAND_RSPEC(wlc->band);
	}

	if (!wlc_queue_80211_frag(wlc, pkt, wlc->active_queue, NULL, cfg,
		FALSE, NULL, rate_override))
		return BCME_ERROR;

	/* register packet callback */
	WLF2_PCB1_REG(pkt, WLF2_PCB1_AF);

	return BCME_OK;
}

#ifdef STA
static int
wlc_send_action_frame_off_channel(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	uint32 channel, int32 dwell_time, struct ether_addr *bssid, wl_action_frame_t *action_frame)
{
	wlc_ssid_t ssid;
	chanspec_t chanspec_list[1];
	wlc_scan_info_t	*scan = wlc->scan;

	scan->action_frame = wlc_prepare_action_frame(wlc, bsscfg, bssid, action_frame);
	if (!scan->action_frame) {
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_ACTION_FRAME_COMPLETE,
			NULL, WLC_E_STATUS_NO_ACK, 0, 0,
			&action_frame->packetId, sizeof(action_frame->packetId));
		return BCME_NOMEM;
	}

	chanspec_list[0] = CH20MHZ_CHSPEC(channel);
	ssid.SSID_len = 0;

	return wlc_scan_request_ex(wlc, DOT11_BSSTYPE_ANY, &bsscfg->BSSID, 1, &ssid, -1, 1,
	                           dwell_time, dwell_time, -1, chanspec_list,
	                           1, 0, FALSE,
	                           wlc_af_off_channel_callback, wlc,
	                           WLC_ACTION_ACTFRAME, FALSE, bsscfg,
	                           wlc_af_send, scan);
}
#endif /* STA */

int
wlc_send_action_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg, const struct ether_addr *bssid,
	void *action_frame)
{
	void *pkt = wlc_prepare_action_frame(wlc, cfg, bssid, action_frame);

	if (!pkt) {
		return BCME_NOMEM;
	}

	return wlc_tx_action_frame_now(wlc, cfg, pkt);
}

static void
wlc_actionframetx_complete(wlc_info_t *wlc, void *pkt, uint txstatus)
{
	int err;
	int idx;
	wlc_bsscfg_t *cfg;
	uint status = WLC_E_STATUS_SUCCESS;
	uint32 packetId;

	if (!(txstatus & TX_STATUS_ACK_RCV))
		status = WLC_E_STATUS_NO_ACK;

	packetId = WLPKTTAG(pkt)->shared.packetid;

	idx = WLPKTTAGBSSCFGGET(pkt);
	cfg = wlc_bsscfg_find(wlc, idx, &err);

	/* Send back the Tx status to the host */
	wlc_bss_mac_event(wlc, cfg, WLC_E_ACTION_FRAME_COMPLETE, NULL, status,
	                  0, 0, &packetId, sizeof(packetId));
	return;
}
#endif /* WIFI_ACT_FRAME */

/*
 * This routine creates a chanspec with the current band and b/w using the channel
 * number passed in. For 40MHZ chanspecs it always chooses the lower ctrl SB.
 */
chanspec_t
wlc_create_chspec(wlc_info_t *wlc, uint8 channel)
{
	chanspec_t chspec = 0;

	if (BAND_2G(wlc->band->bandtype))
		chspec |= WL_CHANSPEC_BAND_2G;
	else
		chspec |= WL_CHANSPEC_BAND_5G;
	if (N_ENAB(wlc->pub) &&
	    wlc_valid_chanspec_db(wlc->cmi, wlc->chanspec) &&
	    CHSPEC_IS40(wlc->chanspec) &&
	    (wlc_channel_locale_flags(wlc->cmi) & WLC_NO_40MHZ) == 0 &&
	    WL_BW_CAP_40MHZ(wlc->band->bw_cap)) {
		chspec |= WL_CHANSPEC_BW_40;
		chspec |= WL_CHANSPEC_CTL_SB_LOWER;
	} else {
		chspec |= WL_CHANSPEC_BW_20;
	}
	chspec |= channel;


	return chspec;
}

#ifdef STA
/* Change PCIE War override for some platforms */
void
wlc_pcie_war_ovr_update(wlc_info_t *wlc, uint8 aspm)
{
	 wlc_bmac_pcie_war_ovr_update(wlc->hw, aspm);
}

void
wlc_pcie_power_save_enable(wlc_info_t *wlc, bool enable)
{
	wlc_bmac_pcie_power_save_enable(wlc->hw, enable);
}
#endif /* STA */

#if defined(BCMTSTAMPEDLOGS)
/* Asynchronously output a timestamped log message */
void
wlc_log(wlc_info_t *wlc, const char* str, uint32 p1, uint32 p2)
{
	uint32 tstamp;

	/* Read a timestamp from the TSF timer register */
	tstamp = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);

	/* Store the timestamp and the log message in the log buffer */
	bcmtslog(tstamp, (char*)str, p1, p2);
}
#endif /* defined(BCMTSTAMPEDLOGS) */

/*
 * Function downgrades the edcf parameters
 */

#ifdef	WME
int
wlc_wme_downgrade_fifo(wlc_info_t *wlc, uint* p_fifo, struct scb *scb)
{
	wlc_bsscfg_t *cfg;
	wlc_wme_t *wme;

	ASSERT(scb != NULL);

	/* Downgrade the fifo if admission is not yet gained */
	if (CAC_ENAB(wlc->pub) && wlc_cac_is_traffic_admitted(wlc->cac, wme_fifo2ac[*p_fifo], scb))
		return 0;

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	wme = cfg->wme;

	if (AC_BITMAP_TST(wme->wme_admctl, wme_fifo2ac[*p_fifo])) {
		do {
			if (*p_fifo == 0) {
				WL_ERROR(("No AC available; tossing pkt\n"));
				WLCNTINCR(wlc->pub->_cnt->txnobuf);
				WLCIFCNTINCR(scb, txnobuf);
				WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_failures);
				return BCME_ERROR;
			}

			(*p_fifo)--;
		} while (AC_BITMAP_TST(wme->wme_admctl, wme_fifo2ac[*p_fifo]));

		/*
		 * Note: prio in packet is NOT updated.
		 * Original prio is also used below to determine APSD trigger AC.
		 */
	}

	return BCME_OK;
}

uint16
wlc_wme_get_frame_medium_time(wlc_info_t *wlc, ratespec_t rate, uint8 preamble_type, uint mac_len)
{
	uint16 duration;

	duration = (uint16)SIFS(wlc->band);
	duration += (uint16)wlc_calc_ack_time(wlc, rate, preamble_type);
	duration += (uint16)wlc_calc_frame_time(wlc, rate, preamble_type, mac_len);
	/* duration in us */
	return duration;
}
#endif /* WME */

uint
wlc_wme_wmmac_check_fixup(wlc_info_t *wlc, struct scb *scb, void *sdu, uint8 prio, int *bcmerror)
{
	uint fifo_prio;

	fifo_prio = prio2fifo[prio];

#ifdef	WME
	{
		uint fifo_ret = fifo_prio;
		*bcmerror = wlc_wme_downgrade_fifo(wlc, &fifo_ret, scb);
		if (*bcmerror)
			return fifo_prio;

		if (fifo_prio != fifo_ret) {
			prio = fifo2prio[fifo_ret];
			PKTSETPRIO(sdu, prio);
			fifo_prio = fifo_ret;
		}
	}
#endif /* WME */
	return fifo_prio;
}

#if defined(WLPLT)
int
wlc_minimal_down(wlc_info_t *wlc)
{
	uint callbacks;
	wlc_phy_t *pi = WLC_PI(wlc);
#ifdef STA
	int i;
	wlc_bsscfg_t *bsscfg;
#endif

	callbacks = 0;
	WL_TRACE(("wl%d: %s: Enter\n", wlc->pub->unit, __FUNCTION__));
	if (!wlc->pub->up)
		return BCME_NOTUP;

	if (wlc->pub->radio_active == OFF)
		return BCME_ERROR;

	/* Do not access Phy registers if core is not up */
	if (wlc_bmac_si_iscoreup(wlc->hw) == FALSE)
		return BCME_ERROR;

	/* abort any scan in progress */
	wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);

#ifdef STA
#ifdef WLRM
	/* abort any radio measures in progress */
	if (WLRM_ENAB(wlc->pub) && !wlc_rm_abort(wlc))
		callbacks++;
#endif /* WLRM */

	FOREACH_BSS(wlc, i, bsscfg) {
		/* Perform STA down operations if needed */
		if (!BSSCFG_STA(bsscfg))
			continue;
		/* abort any association in progress */
		wlc_assoc_abort(bsscfg);

		/* if config is enabled, take care of deactivating */
		if (bsscfg->enable) {
			/* For WOWL or Assoc Recreate, don't disassociate,
			 * just down the bsscfg.
			 * Otherwise,
			 * disable the config (STA requires active restart)
			 */
			if (ASSOC_RECREATE_ENAB(wlc->pub) &&
			    (bsscfg->flags & WLC_BSSCFG_PRESERVE)) {
				WL_ASSOC(("wl%d: %s: Preserving existing association....\n",
				          wlc->pub->unit, __FUNCTION__));
				wlc_bsscfg_down(wlc, bsscfg);
			} else {
				WL_ASSOC(("wl%d: %s: Disabling bsscfg on down\n",
				          wlc->pub->unit, __FUNCTION__));
				wlc_bsscfg_disable(wlc, bsscfg);
				/* allow time for disassociation packet to
				 * notify associated AP of our departure
				 */
				OSL_DELAY(4 * 1000);
			}
		}
	}
#endif /* STA */

#ifdef WLLED
	wlc_led_down(wlc);
#endif

#ifdef STA
	/* stop pspoll timer to prevent the CPU from running too often */
	FOREACH_AS_STA(wlc, i, bsscfg) {
		callbacks += wlc_pspoll_timer_upd(bsscfg, FALSE);
		callbacks += wlc_apsd_trigger_upd(bsscfg, FALSE);
	}
#endif

	/* suspend d11 core */
	wlc_suspend_mac_and_wait(wlc);

	phy_radio_switch((phy_info_t *)pi, OFF);
	phy_ana_switch((phy_info_t *)pi, OFF);
	wlc->pub->radio_active = OFF;

	wlc_bmac_minimal_radio_hw(wlc->hw, FALSE);

	wlc->pub->up = FALSE;
	return callbacks;
}

int
wlc_minimal_up(wlc_info_t * wlc)
{
	wlc_phy_t *pi = WLC_PI(wlc);
	WL_TRACE(("wl%d: %s: Enter\n", wlc->pub->unit, __FUNCTION__));

	if (wlc->pub->radio_active == ON)
		return BCME_ERROR;

	/* Do not access Phy registers if core is not up */
	if (wlc_bmac_si_iscoreup(wlc->hw) == FALSE)
		return BCME_ERROR;

	wlc_bmac_minimal_radio_hw(wlc->hw, TRUE);

	phy_ana_switch((phy_info_t *)pi, ON);
	phy_radio_switch((phy_info_t *)pi, ON);
	wlc->pub->radio_active = ON;

	/* resume d11 core */
	wlc_enable_mac(wlc);

#ifdef WLLED
	wlc_led_up(wlc);
#endif
	wlc->pub->up = TRUE;

#ifdef STA
	if (ASSOC_RECREATE_ENAB(wlc->pub)) {
		int idx;
		wlc_bsscfg_t *cfg;
		FOREACH_BSS(wlc, idx, cfg) {
			if (BSSCFG_STA(cfg) && cfg->enable) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
				char ssidbuf[SSID_FMT_BUF_LEN];
				wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
#endif

				WL_ASSOC(("wl%d: wlc_up: restarting STA bsscfg 0 \"%s\"\n",
				          wlc->pub->unit, ssidbuf));
				wlc_join_recreate(wlc, cfg);
			}
		}
	}
#endif /* STA */

	return BCME_OK;
}
#endif 

#if defined(STA) && defined(AP)
bool
wlc_apup_allowed(wlc_info_t *wlc)
{
	bool busy = AS_IN_PROGRESS(wlc) ||
		SCAN_IN_PROGRESS(wlc->scan) ||
#ifdef WL11K
		wlc_rrm_inprog(wlc) ||
#endif /* WL11K */
		WLC_RM_IN_PROGRESS(wlc);

#ifdef BCMDBG
	if (busy) {
		WL_APSTA_UPDN(("wl%d: wlc_apup_allowed: defer AP UP, STA associating: "
			"stas/aps/associated %d/%d/%d, AS_IN_PROGRESS() %d, scan %d, rm %d\n",
			wlc->pub->unit, wlc->stas_associated, wlc->aps_associated,
			wlc->pub->associated, AS_IN_PROGRESS(wlc),
			SCAN_IN_PROGRESS(wlc->scan), WLC_RM_IN_PROGRESS(wlc)));
	}
#endif
	return !busy;
}
#endif /* STA && AP */

uint16
wlc_rcvfifo_limit_get(wlc_info_t *wlc)
{
	uint16 rcvfifo;

	/* determine rx fifo. no register/shm, it's hardwired in RTL */

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		/* The hardware now tells us */
		if (D11REV_IS(wlc->pub->corerev, 40) ||
			D11REV_IS(wlc->pub->corerev, 41) ||
			D11REV_IS(wlc->pub->corerev, 42) ||
			D11REV_IS(wlc->pub->corerev, 44) ||
			D11REV_IS(wlc->pub->corerev, 46)) {
			rcvfifo = ((wlc->machwcap & MCAP_RXFSZ_MASK) >> MCAP_RXFSZ_SHIFT) * 2048;
		}
		else {
			rcvfifo = 0;
		}
	}
	else if (D11REV_GE(wlc->pub->corerev, 13)) {
		/* The hardware now tells us */
		rcvfifo = ((wlc->machwcap & MCAP_RXFSZ_MASK) >> MCAP_RXFSZ_SHIFT) * 512;
	} else if (D11REV_GE(wlc->pub->corerev, 11)) {
		rcvfifo = 10240;
	} else if (D11REV_GE(wlc->pub->corerev, 8)) {
		rcvfifo = 4608;
	} else if (D11REV_GE(wlc->pub->corerev, 5)) {
		rcvfifo = 2560;
	} else if (D11REV_IS(wlc->pub->corerev, 4)) {
		rcvfifo = 5 * 1024;
	} else {
		/* EOL'd chips or chips that doesn't exist yet */
		ASSERT(0);
		return 0;
	}
	return rcvfifo;
}

#ifdef WLC_HIGH
/* Read a single uint16 from shared memory.
 * SHM 'offset' needs to be an even address
 */
uint16
wlc_read_shm(wlc_info_t *wlc, uint offset)
{
	return wlc_bmac_read_shm(wlc->hw, offset);
}

/* Write a single uint16 to shared memory.
 * SHM 'offset' needs to be an even address
 */
void
wlc_write_shm(wlc_info_t *wlc, uint offset, uint16 v)
{
	wlc_bmac_write_shm(wlc->hw, offset, v);
}

void
wlc_update_shm(wlc_info_t *wlc, uint offset, uint16 v, uint16 mask)
{
	wlc_bmac_update_shm(wlc->hw, offset, v, mask);
}

/* Set a range of shared memory to a value.
 * SHM 'offset' needs to be an even address and
 * Range length 'len' must be an even number of bytes
 */
void
wlc_set_shm(wlc_info_t *wlc, uint offset, uint16 v, int len)
{
	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	wlc_bmac_set_shm(wlc->hw, offset, v, len);
}

/* Copy a buffer to shared memory.
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 */
void
wlc_copyto_shm(wlc_info_t *wlc, uint offset, const void* buf, int len)
{
	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;
	wlc_bmac_copyto_objmem(wlc->hw, offset, buf, len, OBJADDR_SHM_SEL);

}

/* Copy from shared memory to a buffer.
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 */
void
wlc_copyfrom_shm(wlc_info_t *wlc, uint offset, void* buf, int len)
{
	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	wlc_bmac_copyfrom_objmem(wlc->hw, offset, buf, len, OBJADDR_SHM_SEL);
}
#endif	/* WLC_HIGH */

/* wrapper BMAC functions to for HIGH driver access */
void
wlc_mctrl(wlc_info_t *wlc, uint32 mask, uint32 val)
{
	wlc_bmac_mctrl(wlc->hw, mask, val);
}

void
wlc_corereset(wlc_info_t *wlc, uint32 flags)
{
	wlc_bmac_corereset(wlc->hw, flags);
}

void
wlc_suspend_mac_and_wait(wlc_info_t *wlc)
{
	wlc_bmac_suspend_mac_and_wait(wlc->hw);
}

void
wlc_enable_mac(wlc_info_t *wlc)
{
	wlc_bmac_enable_mac(wlc->hw);
}
void
wlc_mhf(wlc_info_t *wlc, uint8 idx, uint16 mask, uint16 val, int bands)
{
	wlc_bmac_mhf(wlc->hw, idx, mask, val, bands);
}

uint16
wlc_mhf_get(wlc_info_t *wlc, uint8 idx, int bands)
{
	return wlc_bmac_mhf_get(wlc->hw, idx, bands);
}

static int
wlc_xmtfifo_sz_get(wlc_info_t *wlc, uint fifo, uint *blocks)
{
	return wlc_bmac_xmtfifo_sz_get(wlc->hw, fifo, blocks);
}

#if defined(MACOSX)
static int
wlc_xmtfifo_sz_set(wlc_info_t *wlc, uint fifo, uint16 blocks)
{
	if (fifo >= NFIFO || blocks > 299)
		return BCME_RANGE;

	wlc_xmtfifo_sz_upd_high(wlc, fifo, blocks);

	wlc_bmac_xmtfifo_sz_set(wlc->hw, fifo, blocks);

	return BCME_OK;
}
#endif 

static void
wlc_xmtfifo_sz_upd_high(wlc_info_t *wlc, uint fifo, uint16 blocks)
{
	wlc->xmtfifo_szh[fifo] = blocks;

	if (fifo < AC_COUNT) {
		wlc->xmtfifo_frmmaxh[fifo] =
			(wlc->xmtfifo_szh[fifo] * 256 - 1300) / MAX_MPDU_SPACE;
		WL_INFORM(("%s: fifo sz blk %d entries %d\n",
			__FUNCTION__, wlc->xmtfifo_szh[fifo], wlc->xmtfifo_frmmaxh[fifo]));
	}
}

void
wlc_write_template_ram(wlc_info_t *wlc, int offset, int len, void *buf)
{
	wlc_bmac_write_template_ram(wlc->hw, offset, len, buf);
}

void
wlc_write_hw_bcntemplates(wlc_info_t *wlc, void *bcn, int len, bool both)
{
	wlc_bmac_write_hw_bcntemplates(wlc->hw, bcn, len, both);
}

void
wlc_mute(wlc_info_t *wlc, bool on, mbool flags)
{
	wlc_bmac_mute(wlc->hw, on, flags);
}

void
wlc_read_tsf(wlc_info_t* wlc, uint32* tsf_l_ptr, uint32* tsf_h_ptr)
{
	wlc_bmac_read_tsf(wlc->hw, tsf_l_ptr, tsf_h_ptr);
}

void
wlc_set_cwmin(wlc_info_t *wlc, uint16 newmin)
{
	wlc->band->CWmin = newmin;
	wlc_bmac_set_cwmin(wlc->hw, newmin);
}

void
wlc_set_cwmax(wlc_info_t *wlc, uint16 newmax)
{
	wlc->band->CWmax = newmax;
	wlc_bmac_set_cwmax(wlc->hw, newmax);
}

void
wlc_fifoerrors(wlc_info_t *wlc)
{
	wlc_bmac_fifoerrors(wlc->hw);
}

void
wlc_ampdu_mode_upd(wlc_info_t *wlc, uint8 mode)
{
	uint j;
	wlc_bmac_ampdu_set(wlc->hw, mode);

	for (j = 0; j < NFIFO; j++) {
		uint fifo_size;
		wlc_xmtfifo_sz_get(wlc, j, &fifo_size);
		wlc_xmtfifo_sz_upd_high(wlc, j, (uint16)fifo_size);
	}
}

void
wlc_pllreq(wlc_info_t *wlc, bool set, mbool req_bit)
{
	wlc_bmac_pllreq(wlc->hw, set, req_bit);
}

void
wlc_update_phy_mode(wlc_info_t *wlc, uint32 phy_mode)
{
#ifdef WL11N
	/* Notify the rate selection module of the ACI state change */
	wlc_scb_ratesel_aci_change(wlc->wrsi, phy_mode & PHY_MODE_ACI);
#endif
}

void
wlc_reset_bmac_done(wlc_info_t *wlc)
{
#ifdef WLC_HIGH_ONLY
	wlc->reset_bmac_pending = FALSE;
#endif
}


#ifdef MCAST_REGEN
/*
 * Description: This function is called to do the reverse translation
 *
 * Input    eh - pointer to the ethernet header
 */
int32
wlc_mcast_reverse_translation(struct ether_header *eh)
{
	uint8 *iph;
	uint32 dest_ip;

	iph = (uint8 *)eh + ETHER_HDR_LEN;
	dest_ip = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));

	/* Only IP packets are handled */
	if (eh->ether_type != hton16(ETHER_TYPE_IP))
		return BCME_ERROR;

	/* Non-IPv4 multicast packets are not handled */
	if (IP_VER(iph) != IP_VER_4)
		return BCME_ERROR;

	/*
	 * The packet has a multicast IP and unicast MAC. That means
	 * we have to do the reverse translation
	 */
	if (IPV4_ISMULTI(dest_ip) && !ETHER_ISMULTI(&eh->ether_dhost)) {
		ETHER_FILL_MCAST_ADDR_FROM_IP(eh->ether_dhost, dest_ip);
		return BCME_OK;
	}

	return BCME_ERROR;
}
#endif /* MCAST_REGEN */

/* wlc_exptime_start() and wlc_exptime_stop() are just for RF awareness external log
 * filtering so we don't send an extlog for each suppressed frame
 */
void
wlc_exptime_start(wlc_info_t *wlc)
{
	uint32 curr_time = OSL_SYSUPTIME();

	if (wlc->exptime_cnt == 0) {
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_EXPTIME_ID, WL_LOG_LEVEL_ERR, 0,
			wlc->pub->unit, NULL);
	}
	WL_ERROR(("wl%d: %s: packet lifetime (%d ms) expired: frame suppressed\n",
		wlc->pub->unit, __FUNCTION__, wlc->rfaware_lifetime/4));
	wlc->last_exptime = curr_time;
	wlc->exptime_cnt++;
}

/* Enter this only when we're in exptime state */
void
wlc_exptime_check_end(wlc_info_t *wlc)
{
	uint32 curr_time = OSL_SYSUPTIME();

	ASSERT(wlc->exptime_cnt);

	/* No lifetime expiration for WLC_EXPTIME_END_TIME means no interference.
	 * Leave the exptime state.
	 */
	if (curr_time - wlc->last_exptime > WLC_EXPTIME_END_TIME) {
		wlc->exptime_cnt = 0;
		wlc->last_exptime = 0;
	}
}

int
wlc_rfaware_lifetime_set(wlc_info_t *wlc, uint16 lifetime)
{
	int i, ret = BCME_OK;
	bool wme_lifetime = FALSE;

	for (i = 0; i < AC_COUNT; i++)
		if (wlc->lifetime[i])
			wme_lifetime = TRUE;

	/* Enable RF awareness only when wme lifetime is not used because they share same EXPTIME
	 * tx status
	 */
	if (wlc->txmsdulifetime || wme_lifetime) {
		wlc->rfaware_lifetime = 0;
		if ((wlc->rfaware_lifetime == 0) && lifetime) {
			WL_INFORM(("%s: can't turn on RFAWARE if WME is on\n", __FUNCTION__));
			ret = BCME_EPERM;
		}
	} else {
		wlc->rfaware_lifetime = lifetime;
	}

	if (wlc->pub->up) {
		wlc_suspend_mac_and_wait(wlc);
		wlc_write_shm(wlc, M_AGING_THRSH, wlc->rfaware_lifetime);
		wlc_mhf(wlc, MHF4, MHF4_AGING, (wlc->rfaware_lifetime ? MHF4_AGING : 0),
			WLC_BAND_ALL);
		wlc_enable_mac(wlc);
	}

	return ret;
}

#ifdef WLSCANCACHE
/* returns TRUE if scan cache looks valid and recent, FALSE otherwise */
bool
wlc_assoc_cache_validate_timestamps(wlc_info_t *wlc, wlc_bss_list_t *bss_list)
{
	uint current_time = OSL_SYSUPTIME(), oldest_time, i;

	/* if there are no hits, just return with no cache assistance */
	if (bss_list->count == 0)
		return FALSE;

	/* if there are hits, check how old they are */
	oldest_time = current_time;
	for (i = 0; i < bss_list->count; i++)
		oldest_time = MIN(bss_list->ptrs[i]->timestamp, oldest_time);

	/* If the results are all recent enough, then use the cached bss_list
	 * for the association attempt.
	 */
	if (current_time - oldest_time < BCMWL_ASSOC_CACHE_TOLERANCE) {
		WL_ASSOC(("wl%d: %s: %d hits, oldest %d sec, using cache hits\n",
		          wlc->pub->unit, __FUNCTION__,
		          bss_list->count, (current_time - oldest_time)/1000));
		return TRUE;
	}

	return FALSE;
}
#endif /* WLSCANCACHE */

wlc_if_t*
wlc_wlcif_alloc(wlc_info_t *wlc, osl_t *osh, uint8 type, wlc_txq_info_t *qi)
{
	wlc_if_t *wlcif;

	wlcif = (wlc_if_t*)MALLOCZ(osh, sizeof(wlc_if_t));
	if (wlcif != NULL) {
		wlcif->type = type;
		wlcif->qi = qi;

		/* initiaze version fields in per-port counters structure */
		WLCNTSET(wlcif->_cnt.version, WL_IF_STATS_T_VERSION);
		WLCNTSET(wlcif->_cnt.length, sizeof(wl_if_stats_t));

		/* add this interface to the global list */
		wlcif->next = wlc->wlcif_list;
		wlc->wlcif_list = wlcif;
	}

	return wlcif;
}

void
wlc_wlcif_free(wlc_info_t *wlc, osl_t *osh, wlc_if_t *wlcif)
{
	wlc_if_t *p;

	if (wlcif == NULL)
		return;

	/* remove the interface from the interface linked list */
	p = wlc->wlcif_list;
	if (p == wlcif)
		wlc->wlcif_list = p->next;
	else {
		while (p != NULL && p->next != wlcif)
			p = p->next;
		if (p == NULL)
			WL_ERROR(("%s: null ptr2", __FUNCTION__));

		/* assert that we found wlcif before getting to the end of the list */
		ASSERT(p != NULL);

		if (p != NULL) {
			p->next = p->next->next;
		}
	}

	MFREE(osh, wlcif, sizeof(wlc_if_t));
}

/* Return the interface pointer for the BSS indexed by idx */
wlc_if_t *
wlc_wlcif_get_by_index(wlc_info_t *wlc, uint idx)
{
	wlc_bsscfg_t	*bsscfg;
	wlc_if_t	*wlcif = NULL;

	if (wlc == NULL)
		return NULL;

	if (idx >= WLC_MAXBSSCFG)
		return NULL;

	bsscfg = wlc->bsscfg[idx];
	if (bsscfg) {
		/* RSDB: WLCIF incorrect index can cause wlc mixup */
		ASSERT(wlc == bsscfg->wlc);
		if (wlc != bsscfg->wlc) {
			WL_ERROR(("RSDB: WLCIF doesnot belong to WLC[%d] for bsscfg[%d]\n",
				wlc->pub->unit, idx));
		}
		wlcif = bsscfg->wlcif;
	}

	return wlcif;
}

void
wlc_wlcif_stats_get(wlc_info_t *wlc, wlc_if_t *wlcif, wl_if_stats_t *wl_if_stats)
{
	if ((wlcif == NULL) || (wl_if_stats == NULL))
		return;

	/*
	 * Aggregate errors from other errors
	 * These other errors are only updated when it makes sense
	 * that the error should be charged to a logical interface
	 */
	wlcif->_cnt.txerror = wlcif->_cnt.txnobuf + wlcif->_cnt.txrunt;
	wlcif->_cnt.rxerror = wlcif->_cnt.rxnobuf + wlcif->_cnt.rxrunt + wlcif->_cnt.rxfragerr;

	memcpy(wl_if_stats, &(wlcif->_cnt), sizeof(wl_if_stats_t));
}


/* This function will read the PM duration accumulator maintained by ucode */
static uint32
_wlc_get_accum_pmdur(wlc_info_t *wlc)
{
	uint32 res;
	uint16 prev_st = 0, new_st = 0;

	do {
		prev_st = wlc_read_shm(wlc, M_UCODE_DBGST);
		res = wlc_bmac_cca_read_counter(wlc->hw, M_MAC_DOZE_L, M_MAC_DOZE_H);
		new_st = wlc_read_shm(wlc, M_UCODE_DBGST);
	} while (prev_st != new_st);
	/* wlc_pm_dur_cntr is in ms. */
	res = (res - wlc->wlc_pm_dur_last_sample) / 1000;
	wlc->wlc_pm_dur_cntr += res;

	wlc->pm_dur_clear_timeout = TIMEOUT_TO_READ_PM_DUR;
	wlc->wlc_pm_dur_last_sample += (res * 1000);

	return wlc->wlc_pm_dur_cntr;
}

/* This API can be invoked irrespctive of clk available */
uint32
wlc_get_accum_pmdur(wlc_info_t *wlc)
{
	if (wlc->pub->hw_up)
		return _wlc_get_accum_pmdur(wlc);
	else
		return wlc->wlc_pm_dur_cntr;
}

#ifdef BCMDBG
void wlc_update_perf_stats(wlc_info_t *wlc, uint32 mask)
{
	wlc_perf_stats_t *stats = &wlc->perf_stats;

	if (mask & WLC_PERF_STATS_ISR) stats->n_isr++;
	if (mask & WLC_PERF_STATS_DPC) stats->n_dpc++;
	if (mask & WLC_PERF_STATS_TMR_DPC) stats->n_timer_dpc++;
	if (mask & WLC_PERF_STATS_BCN_ISR) stats->n_bcn_isr++;
	if (mask & WLC_PERF_STATS_BCNS) stats->n_beacons++;
	if (mask & WLC_PERF_STATS_PRB_REQ) stats->n_probe_req++;
	if (mask & WLC_PERF_STATS_PRB_RESP) stats->n_probe_resp++;
}

void wlc_update_isr_stats(wlc_info_t *wlc, uint32 macintstatus)
{
	wlc_isr_stats_t *stats = &wlc->perf_stats.isr_stats;
	uint32 i;
	/* Update the overall Stats Count as well */
	if (macintstatus)
		wlc_update_perf_stats(wlc, WLC_PERF_STATS_ISR);

	for (i = 0; i < 32; i++)
		if (macintstatus & (1<<i)) 	{stats->n_counts[i]++;}

}
#endif /* BCMDBG */

#ifdef WLPKTDLYSTAT
void wlc_delay_stats_upd(scb_delay_stats_t *delay_stats, uint32 delay, uint tr, bool ack_recd)
{
	WLCNTSCBINCR(delay_stats->txmpdu_cnt[tr]);
	WLCNTSCBADD(delay_stats->delay_sum[tr], delay);
	if (delay_stats->delay_min == 0) {
		delay_stats->delay_min = delay;
		delay_stats->delay_max = delay;
	} else {
		if (delay < delay_stats->delay_min)
			delay_stats->delay_min = delay;
		else if (delay > delay_stats->delay_max)
			delay_stats->delay_max = delay;
	}
#ifdef WLC_HIGH_ONLY
	WLCNTSCBINCR(delay_stats->delay_hist[MIN(((delay * 1000)/(HZ * 10)),
		WLPKTDLY_HIST_NBINS-1)]);
#else
	WLCNTSCBINCR(delay_stats->delay_hist[MIN(delay/500, WLPKTDLY_HIST_NBINS-1)]);
#endif
	if (!ack_recd)
		WLCNTSCBINCR(delay_stats->txmpdu_lost);
}

#ifdef WLPKTDLYSTAT_IND
/* This function calculate avg txdly and trigger */
void wlc_delay_stats_watchdog(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;

	txdelay_event_t txdelay;
	wlc_chanim_stats_t *chamin_stats;
	int rssi;
	uint8 ac, i;
	uint32 sum_1s, total_1s, avg_1s;
	uint32 avg, delta;
	scb_delay_stats_t *delay_stats;

	char eabuf[ETHER_ADDR_STR_LEN];

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_ASSOCIATED(scb)) {
			scb->txdelay_params.period += 1;
			scb->txdelay_params.period %=
				wlc->pub->txdelay_params.period;

			if (scb->txdelay_params.period)
				continue;

			ac = AC_VI;
			delay_stats = &scb->delay_stats[ac];

			/* Calculate txdelay avg */
			total_1s = 0, sum_1s = 0;
			for (i = 0; i < RETRY_SHORT_DEF; i++) {
				total_1s += delay_stats->txmpdu_cnt[i];
				sum_1s += delay_stats->delay_sum[i];
			}

			if (total_1s != 0) {
				avg_1s = sum_1s / total_1s;
				/* for the first calculation */
				if (delay_stats->delay_avg == 0) {
					delay_stats->delay_avg = avg_1s;
					continue;
				}

				avg = delay_stats->delay_avg;
				delta = (avg * wlc->pub->txdelay_params.ratio)/100;

				/* Collect SCB rssi */
				wlc_scb_rssi_update_enable(scb, TRUE, RSSI_UPDATE_FOR_WLC);
				rssi = wlc_scb_rssi(scb);

				if (wlc->pub->txdelay_params.tune) {
					bcm_ether_ntoa(&scb->ea, eabuf);
					WL_ERROR(("TXDLY:[%s] rssi=%d avg_1s(%5d)-avg(%d)=%d "
						"(avg*ratio)/100=%d\n", eabuf, rssi,
						avg_1s, avg, avg_1s - avg, delta));
				}

				/* Check if need to trigger event */
				if ((avg_1s > avg) && ((avg_1s - avg) > delta)) {
					scb->txdelay_params.cnt++;

					if (scb->txdelay_params.cnt >=
						wlc->pub->txdelay_params.cnt) {
						if (wlc->pub->txdelay_params.tune)
							WL_ERROR(("TXDLY:Event Triggered\n"));

						scb->txdelay_params.cnt = 0;

						/* Issue PKT TX Delay Stats Event */
						memset(&txdelay, 0, sizeof(txdelay_event_t));
						memcpy(&(txdelay.delay_stats),
							scb->delay_stats,
							sizeof(scb_delay_stats_t) * AC_COUNT);

#ifdef WLCHANIM
						chamin_stats = &(wlc->chanim_info->cur_stats);
						memcpy(&(txdelay.chanim_stats),
							&chamin_stats->chanim_stats,
							sizeof(chanim_stats_t));
#endif /* WLCHANIM */
						txdelay.rssi = rssi;

						wlc_bss_mac_event(wlc, scb->bsscfg,
							WLC_E_PKTDELAY_IND,	&scb->ea, 0, 0, 0,
							&txdelay, sizeof(txdelay_event_t));
					}
				}
				else
					delay_stats->delay_avg =
						(delay_stats->delay_avg * 7)/8 + avg_1s / 8;
			}
		}
	}
}
#endif /* WLPKTDLYSTAT_IND */
#endif /* WLPKTDLYSTAT */

#ifdef WLC_HIGH
void
wlc_txstop_intr(wlc_info_t *wlc)
{
	/* handle a scan request for tx suspend */
	wlc_scan_fifo_suspend_complete(wlc->scan);
#ifdef STA
#ifdef WLRM
	/* handle a radio measurement request for tx suspend */
	if (WLRM_ENAB(wlc->pub) && wlc->rm_info->rm_state->step == WLC_RM_WAIT_TX_SUSPEND)
		wl_add_timer(wlc->wl, wlc->rm_info->rm_timer, 0, 0);
#endif

#ifdef WL11K
	/* handle a radio measurement request for tx suspend */
	if (WL11K_ENAB(wlc->pub) && wlc_rrm_wait_tx_suspend(wlc))
		wlc_rrm_start_timer(wlc);
#endif /* WL11K */

	/* move to next quiet state if both DATA and CTL fifos are suspended */
	if (WL11H_ENAB(wlc) && wlc_bmac_tx_fifo_suspended(wlc->hw, TX_CTL_FIFO))
		wlc_quiet_txfifo_suspend_complete(wlc->quiet);
#endif /* STA */
}

void
wlc_rfd_intr(wlc_info_t *wlc)
{
	d11regs_t *regs = wlc->regs;
	uint32 rfd = R_REG(wlc->osh, &regs->phydebug) & PDBG_RFD;

	WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RFDISABLE_ID, WL_LOG_LEVEL_ERR, 0, rfd, NULL);

	WL_ERROR(("wl%d: MAC Detected a change on the RF Disable Input 0x%x\n",
		wlc->pub->unit, rfd));

	WLCNTINCR(wlc->pub->_cnt->rfdisable);

	/* delay the cleanup to wl_down in IBSS case */
	if (rfd) {
		int idx;
		wlc_bsscfg_t *bsscfg;
		FOREACH_BSS(wlc, idx, bsscfg) {
			if ((TRUE &&
#ifdef WLC_HIGH_ONLY
			     !WLEXTSTA_ENAB(wlc->pub) &&
#endif
			     !BSSCFG_STA(bsscfg)) ||
			    !bsscfg->BSS ||
			    !bsscfg->enable)
				continue;
			WL_APSTA_UPDN(("wl%d: wlc_dpc: rfdisable -> wlc_bsscfg_disable()\n",
				wlc->pub->unit));
			wlc_bsscfg_disable(wlc, bsscfg);
		}
	}
}
#endif /* WLC_HIGH */

static uint8
wlc_template_plcp_offset(wlc_info_t *wlc, ratespec_t rspec)
{
	bool ac_phy = FALSE;
	uint8	offset = 0;

	if (D11REV_GE(wlc->pub->corerev, 40))
		ac_phy = TRUE;

	if (!ac_phy)
		return 0;

	if (RSPEC_ISVHT(rspec)) {
		offset = D11AC_PHY_VHT_PLCP_OFFSET;
	} else if (RSPEC_ISHT(rspec)) {
		offset = D11AC_PHY_HTGF_PLCP_OFFSET;
	} else if (IS_OFDM(rspec)) {
		offset = D11AC_PHY_OFDM_PLCP_OFFSET;
	} else {
		offset = D11AC_PHY_CCK_PLCP_OFFSET;
	}
	return offset;
}

uint16
wlc_read_amtinfo_by_idx(wlc_info_t *wlc, uint index)
{
	uint16 ret_val;

	if (D11REV_LT(wlc->pub->corerev, 40))
		return 0;
	ASSERT(index < wlc->pub->max_addrma_idx);

	ret_val =  wlc_read_shm(wlc, (M_AMT_INFO_BLK + (index * 2)));
	return ret_val;
}

void
wlc_write_amtinfo_by_idx(wlc_info_t *wlc, uint index, uint16 val)
{
	if (D11REV_LT(wlc->pub->corerev, 40))
		return;

	ASSERT(index < wlc->pub->max_addrma_idx);

	wlc_write_shm(wlc, M_AMT_INFO_BLK + (index * 2), val);
}

#ifdef BCMDBG
void
wlc_dump_phytxerr(wlc_info_t *wlc, uint16 PhyErr)
{
	/* phytxerr may impact phy structures, so determine phy type here */
	if (D11REV_LT(wlc->pub->corerev, 40)) {
#ifdef WLC_LOW
		if (PhyErr) {
			printf("!!!corerev %d, dump as HT phy\n",
				wlc->pub->corerev);
			wlc_htphy_txerr_dump(PhyErr);
		}
#endif /* WLC_LOW */
		return;
	} else {
		uint16 addr;
#ifdef WLC_LOW
		if (PhyErr) {
			wlc_acphy_txerr_dump(PhyErr);
		}
#endif /* WLC_LOW */

		addr = 2 * wlc_read_shm(wlc, 0x90);
		/* plcp dump format here is L-SIG || Sig-A || Sig-B */
		printf("txerr valid (%d) reason %04x tst %02x pctls %04x %04x %04x\n"
			"plcp %04x %04x || %04x %04x %04x || %04x %04x || rxestats2 %04x\n",
			wlc_read_shm(wlc, addr + 0*2),
			wlc_read_shm(wlc, addr + 1*2),
			wlc_read_shm(wlc, addr + 13*2),
			wlc_read_shm(wlc, addr + 2*2),
			wlc_read_shm(wlc, addr + 3*2),
			wlc_read_shm(wlc, addr + 4*2),
			wlc_read_shm(wlc, addr + 5*2),
			wlc_read_shm(wlc, addr + 6*2),
			wlc_read_shm(wlc, addr + 7*2),
			wlc_read_shm(wlc, addr + 8*2),
			wlc_read_shm(wlc, addr + 9*2),
			wlc_read_shm(wlc, addr + 10*2),
			wlc_read_shm(wlc, addr + 11*2),
			wlc_read_shm(wlc, addr + 12*2));
			wlc_write_shm(wlc, addr, 0);
	}
}
#endif /* BCMDBG */
#ifdef MACOSX
int
wlc_get_pmax_tid(wlc_info_t *wlc, uint tid)
{
	int pmax = 0;

	if (wlc_ampdu_tid_enabled(wlc, tid)) {
		pmax += wlc_ampdu_pmax_tid(wlc->ampdu_tx, tid);
	} else {
		wlc_txq_info_t *qi = wlc->active_queue;
		pmax += pktq_pmax(&qi->q, WLC_PRIO_TO_PREC(tid));
	}
	return pmax;
}

int
wlc_get_prec_txpending(wlc_info_t *wlc, int tid)
{
	int pktcnt = 0;
	wlc_bsscfg_t *bsscfg = wlc->cfg;
	wlc_txq_info_t *qi = bsscfg->wlcif->qi;
	int prec;

	prec = wlc_prio2prec_map[tid];
	pktcnt += pktq_plen(&qi->q, prec);
	pktcnt += pktq_plen(&qi->q, prec+1);
	pktcnt += wlc_ampdu_txpktcnt_tid(wlc->ampdu_tx, tid);
	return (pktcnt);
}
#endif /* MACOSX */

/**
 * On e.g. a change of home channel, srvsdb context has to be zapped. This function resets data
 * members and frees ppr member. Despite the function name, it does not touch hardware.
 */
void
wlc_srvsdb_reset_engine(wlc_info_t *wlc)
{
	wlc_srvsdb_info_t *srvsdb = wlc->srvsdb_info;

	bzero(srvsdb->vsdb_save_valid,    sizeof(srvsdb->vsdb_save_valid));
	bzero(srvsdb->ss_algo_save_valid, sizeof(srvsdb->ss_algo_save_valid));
	bzero(srvsdb->ss_algosaved,       sizeof(srvsdb->ss_algosaved));
	if (srvsdb->vsdb_txpwr_ctl[0] != NULL) {
		ppr_delete(wlc->osh, srvsdb->vsdb_txpwr_ctl[0]);
		srvsdb->vsdb_txpwr_ctl[0] = NULL;
	}
	if (srvsdb->vsdb_txpwr_ctl[1] != NULL) {
		ppr_delete(wlc->osh, srvsdb->vsdb_txpwr_ctl[1]);
		srvsdb->vsdb_txpwr_ctl[1] = NULL;
	}
	bzero(srvsdb->vsdb_chans, sizeof(srvsdb->vsdb_chans));
}

/**
 * Power-per-rate context has to be saved in software, to be restored later when the
 * software switches back to the channel.
 */
static void wlc_srvsdb_save_ppr(wlc_info_t *wlc, int i_ppr)
{
	wlc_srvsdb_info_t *srvsdb = wlc->srvsdb_info;
	ppr_t **ppr;

	ppr = &srvsdb->vsdb_txpwr_ctl[i_ppr];

	srvsdb->vsdb_save_valid[i_ppr] = 1;
	if ((*ppr != NULL) &&
		(ppr_get_ch_bw(*ppr) !=	ppr_get_ch_bw(wlc->stf->txpwr_ctl))) {
		ppr_delete(wlc->osh, *ppr);
		*ppr = NULL;
	}
	if (*ppr == NULL) {
		*ppr = ppr_create(wlc->osh, ppr_get_ch_bw(wlc->stf->txpwr_ctl));
	}
	if (*ppr != NULL) {
		ppr_copy_struct(wlc->stf->txpwr_ctl, *ppr);
	}
} /* wlc_srvsdb_save_ppr */

static void wlc_srvsdb_restore_ppr(wlc_info_t *wlc, int i_ppr)
{
	wlc_srvsdb_info_t *srvsdb = wlc->srvsdb_info;
	ppr_t *ppr = srvsdb->vsdb_txpwr_ctl[i_ppr];

	if (!srvsdb->vsdb_save_valid[i_ppr] || ppr == NULL) {
		return;
	}

	wlc_update_txppr_offset(wlc, ppr);
} /* wlc_srvsdb_restore_ppr */

/**
 * Is called by the BMAC after an (attempted) srvsdb channel change. Tx power related power-per-rate
 * context is not saved/restored by the PHY, so it has to be saved/restored by software. The PHY
 * indicated both if the previous channel was saved ('last_chan_saved') and if the SRVSDB switch
 * succeeded ('vsdb_status'). Depending on the result, channel context is saved and/or restored.
 * Note that despite a switch not being succesfull, the PHY can still have saved channel context.
 */
void wlc_srvsdb_switch_ppr(wlc_info_t *wlc, chanspec_t new_chanspec, bool last_chan_saved,
	bool switched)
{
	uint i_ppr = 0;			/* bank (0 or 1) containing previous channel context */

	/* If phy and radio context were saved by BMAC, initiate a (software) save of ppr also */
	if (last_chan_saved) {
		i_ppr = (new_chanspec == wlc->srvsdb_info->vsdb_chans[0]) ? 1 : 0;
		wlc_srvsdb_save_ppr(wlc, i_ppr);
	}
	if (switched) {
		wlc_srvsdb_restore_ppr(wlc, !i_ppr); /* restore opposite bank on channel switch */
	}
}

/**
 * When srvsdb operation is forced by a higher layer (e.g. from wl utility), both driver and dongle
 * firmware have to store the new setting.
 */
int wlc_srvsdb_force_set(wlc_info_t *wlc, uint8 val)
{
	int err;

	err = wlc_bmac_srvsdb_force_set(wlc->hw, val);
	if (!err) {
		wlc->srvsdb_info->iovar_force_vsdb = val;
	}
	return err;
}

#ifndef WLC_DISABLE_ACI
void
wlc_weakest_link_rssi_chan_stats_upd(wlc_info_t *wlc)
{
	int idx;
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_bsscfg_t *bsscfg;
	int8 rssi = 0;
	wlc_phy_t *pi;

	/* Example chanArray and rssiArray
	  * chanArray		6		11		36		104
	  * rssiArray		-45 	-67 	-87 	-25
	  */
	chanspec_t chanArray[MAX_MCHAN_CHANNELS];
	int8 rssiArray[MAX_MCHAN_CHANNELS];
	int chanLen = 0;
	int j;

#if defined(ISID_STATS) && defined(CCA_STATS)
	int sample_time = 1;
	wlc_congest_channel_req_t *result;
	uint result_len;
	int status;
	chanspec_t chanspec;
#else /* WLCHANIM */
#if defined(WLCHANIM)
	wlc_chanim_stats_t *stats;
	chanspec_t chanspec;
#endif /* WLCHANIM */
#endif /* CCA_STATS  ISID_STATS */


	bzero(chanArray, MAX_MCHAN_CHANNELS * sizeof(chanspec_t));
	bzero(rssiArray, MAX_MCHAN_CHANNELS * sizeof(int8));

	FOREACH_BSS(wlc, idx, bsscfg) {
		/* Should not save channel/rssi in array if not UP. */
		if (!bsscfg->up)
			continue;
		for (j = 0; j < MAX_MCHAN_CHANNELS; j++) {
			if (chanArray[j] == 0) {
				chanArray[j] = bsscfg->current_bss->chanspec;
				/* Use this 'j' to compare RSSI and update the least RSSI */
				chanLen++;
				break;
			} else if (chanArray[j] == bsscfg->current_bss->chanspec) {
				/* Use this 'j' to compare RSSI and update the least RSSI */
				break;
			}
		}
		/* if (chanLen == MAX_MCHAN_CHANNELS)
		  * Number of different channels > MAX_MCHAN_CHANNELS
		  */
		ASSERT(chanLen <= MAX_MCHAN_CHANNELS);
		/* if ('j' == MAX_MCHAN_CHANNELS)
		  * Number of different channels > MAX_MCHAN_CHANNELS
		  */
		ASSERT(j < MAX_MCHAN_CHANNELS);

		if (j == MAX_MCHAN_CHANNELS)
		  continue;

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				if (SCB_ASSOCIATED(scb) ||
					(!bsscfg->BSS && !ETHER_ISBCAST(&scb->ea))) {
				rssi = (int8)wlc_scb_rssi(scb);
				if (rssiArray[j] > rssi) {
					rssiArray[j] = rssi;
				}
			}
		}
	}

#if defined(ISID_STATS) && defined(CCA_STATS)
	result_len = sizeof(wlc_congest_channel_req_t) +
		((sample_time - 1) * sizeof(wlc_congest_t));
	result = (wlc_congest_channel_req_t*)MALLOC(wlc->osh, result_len);
#endif /* CCA_STATS  ISID_STATS */
	pi = WLC_PI(wlc);
	for (j = 0; j < chanLen; j++) {
		wlc_phy_interf_rssi_update(pi, chanArray[j], rssiArray[j]);

#if defined(ISID_STATS) && defined(CCA_STATS)
		chanspec = wf_chspec_ctlchspec(chanArray[j]);  /* ctrl chan based */
		status = cca_query_stats(wlc, chanspec, sample_time, result, result_len);
		if (status == 0)
			wlc_phy_interf_chan_stats_update(pi, chanArray[j],
			   result->secs[0].crsglitch, result->secs[0].bphy_crsglitch,
			   result->secs[0].badplcp, result->secs[0].bphy_badplcp,
			   result->secs[0].duration);
#else /* CCA_STATS  ISID_STATS */
#if defined(WLCHANIM)
		chanspec = chanArray[j];  /* center chan based */
		if (chanspec == wlc->chanim_info->cur_stats.chanim_stats.chanspec) {
		  /* Home channel stats */
		  stats = &(wlc->chanim_info->cur_stats);
		} else {
		  stats = wlc_lq_chanim_chanspec_to_stats(wlc->chanim_info, chanspec);
		}

		if (stats) {
		  WL_CHANINT(("Is valid flag is %d for channel %X \n", stats->is_valid, chanspec));
		  if (stats->is_valid)
		    wlc_phy_interf_chan_stats_update(pi, chanArray[j],
		      stats->chanim_stats.glitchcnt, stats->chanim_stats.bphy_glitchcnt,
		      stats->chanim_stats.badplcp, stats->chanim_stats.bphy_badplcp,
		      1000);
		}
#endif /* WLCHANIM */
#endif /* CCA_STATS  ISID_STATS */

	}

#if defined(ISID_STATS) && defined(CCA_STATS)
	MFREE(wlc->osh, result, result_len);
#endif /* CCA_STATS  ISID_STATS */
}
#endif /* WLC_DISABLE_ACI */

bool
wlc_valid_pkt_field(osl_t *osh, void *pkt, void *ptr, int len)
{
	uint8 *pb = (uint8 *)PKTDATA(osh, pkt);
	uint8 *pe = pb + PKTLEN(osh, pkt);
	uint8 *fb = (uint8 *)ptr;
	uint8 *fe = (uint8 *)fb + len;
	return (len == 0) ||
		((pb <= fb && fb <= pe) && (pb <= fe && fe <= pe));
}

int wlc_is_singleband_5g(unsigned int device)
{
	return (_IS_SINGLEBAND_5G(device));
}

static void
wlc_copy_peer_info(wlc_bsscfg_t *bsscfg, struct scb *scb, bss_peer_info_t *peer_info)
{
	wlc_info_t *wlc = bsscfg->wlc;
#ifdef WLCNTSCB
	uint32 tx_rate = RSPEC2KBPS(scb->scb_stats.tx_rate);
	uint32 rx_rate = RSPEC2KBPS(scb->scb_stats.rx_rate);
	ratespec_t rspec = wlc_get_rspec_history(bsscfg);
	uint32 rate = RSPEC2KBPS(rspec);
	uint32 rate_max = -1;
#endif /* WLCNTSCB */

	store16_ua((uint8 *)&peer_info->version, BSS_PEER_INFO_CUR_VER);
	bcopy((void*)&scb->ea, (void*)&peer_info->ea, ETHER_ADDR_LEN);
	bcopy(&scb->rateset, &peer_info->rateset, sizeof(wl_rateset_t));
	store32_ua((uint8 *)&peer_info->rssi, wlc_scb_rssi(scb));
#ifdef WLCNTSCB
	store32_ua((uint8 *)&peer_info->tx_rate, tx_rate == rate_max ? rate:tx_rate);
	store32_ua((uint8 *)&peer_info->rx_rate, rx_rate == rate_max ? rate:rx_rate);
#endif /* WLCNTSCB */
	store32_ua((uint8 *)&peer_info->age, (wlc->pub->now - scb->used));
}

#ifdef STA
/*
Update the DCF/AC NULLDP Template with all 3 mac addresses.
The order is: BSSID, TA and RA.  Each BSS has its own null frame template.
*/
void
wlc_nulldp_tpl_upd(wlc_bsscfg_t *cfg)
{
	char template[T_P2P_NULL_TPL_SIZE];
	struct dot11_header *mac_header;
	wlc_info_t *wlc = cfg->wlc;
	int bss_idx = 0;
	uint32 base;
	uint8 size;

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub))
		bss_idx = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
#endif /* WLMCNX */

	bzero(template, sizeof(template));

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		/* Non-ac chips - Update PLCP header and mac addresses */
		wlc_compute_plcp(wlc, WLC_RATE_1M, (DOT11_MAC_HDR_LEN + DOT11_FCS_LEN),
			FC_TODS, (uint8*)template);
		mac_header = (struct dot11_header *) ((char *)template + 6);
		mac_header->fc = FC_NULL_DATA;
		mac_header->durid = 0;
		base = (T_P2P_NULL_TPL_BASE + (bss_idx * T_P2P_NULL_TPL_SIZE));
		size = T_P2P_NULL_TPL_SIZE;
	}
	else {
		/* ac chips  - update mac addresses */
		mac_header = (struct dot11_header *) template;
		base = (D11AC_T_NULL_TPL_BASE + (bss_idx * D11AC_T_NULL_TPL_SIZE_BYTES));
		size = D11AC_T_NULL_TPL_SIZE_BYTES;
	}
	bcopy((char*)&cfg->BSSID, (char *)&mac_header->a1, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->cur_etheraddr, (char *)&mac_header->a2, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char *)&mac_header->a3, ETHER_ADDR_LEN);

	wlc_write_template_ram(wlc, base, size, template);
}
#endif /* STA */

void wlc_check_txq_fc(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	if (!EDCF_ENAB(wlc->pub) || (wlc->pub->wlfeatureflag & WL_SWFL_FLOWCONTROL)) {
		if (wlc_txflowcontrol_prio_isset(wlc, qi, ALLPRIO) &&
		    (pktq_len(&qi->q) < wlc->pub->tunables->datahiwat / 2)) {
			wlc_txflowcontrol(wlc, qi, OFF, ALLPRIO);
		}
	} else if (wlc->pub->_priofc) {
		int prio;
		for (prio = MAXPRIO; prio >= 0; prio--) {
			if (wlc_txflowcontrol_prio_isset(wlc, qi, prio) &&
			    (pktq_plen(&qi->q, wlc_prio2prec_map[prio]) <
			     wlc->pub->tunables->datahiwat/2)) {
				wlc_txflowcontrol(wlc, qi, OFF, prio);
			}
		}
	}
}

/* call this function to update wlc->band and wlc->pi pointer with active band_index */
void wlc_pi_band_update(wlc_info_t *wlc, uint band_index)
{
	wlc->band = wlc->bandstate[band_index];
	wlc->pi = WLC_PI_BANDUNIT((wlc), band_index);
}

#ifdef PROP_TXSTATUS
int wlc_link_txflow_scb(wlc_info_t *wlc, struct wlc_if *wlcif, uint16 flowid, uint8 op, uint8 * sa,
	uint8 *da, uint8 tid)
{
	uint16	ret = 0xff;
	switch (op) {
		case FLOW_RING_CREATE :
			ret = wlc_flow_ring_get_scb_handle(wlc, wlcif, da);
			break;
		case FLOW_RING_DELETE :
		case FLOW_RING_FLUSH :
			wlc_flush_flowring_pkts(wlc, wlcif, da, flowid, tid);
			break;
		case FLOW_RING_TIM_SET:
		case FLOW_RING_TIM_RESET:
			if (!flowid)
				break;
			wlc_flow_ring_scb_update_available_traffic_info(wlc, flowid,
				tid, (op == FLOW_RING_TIM_SET));
			break;
		default :
			break;
	}
	return ret;
}
#endif /* PROP_TXSTATUS */

void
wlc_devpwrstchg_change(wlc_info_t *wlc, bool hostmem_access_enabled)
{
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
#ifdef BCMPCIEDEV_ENABLED
	wlc_info_t *wlc_iter;
	int idx;
#endif /* BCMPCIEDEV_ENABLED */
	wlc_cmn->hostmem_access_enabled = hostmem_access_enabled;

#ifdef BCMPCIEDEV_ENABLED
	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
	/* Handle power state change handler for each wlc here to handle RSDB. */
		if (wlc_iter->pub->hw_up) {
			wlc_bmac_enable_rx_hostmem_access(wlc_iter->hw,
				wlc_iter->cmn->hostmem_access_enabled);
		}
	}
#endif /* BCMPCIEDEV_ENABLED */
}

#ifdef BCMSPLITRX
#include <hnd_pktfetch.h>
void
wlc_eapol_schedule_pktfetch(wlc_info_t *wlc, struct scb *scb, wlc_frminfo_t *f, bool promisc_frame)
{
	wlc_eapol_pktfetch_ctx_t *ctx;
	struct pktfetch_info *pinfo = MALLOCZ(wlc->osh, sizeof(struct pktfetch_info));
	if (pinfo) {
		ctx = MALLOCZ(wlc->osh, sizeof(wlc_eapol_pktfetch_ctx_t));
		if (ctx == NULL) {
			MFREE(wlc->osh, pinfo, sizeof(struct pktfetch_info));
			WL_ERROR(("%s: Out of mem: Unable to alloc pktfetch ctx!\n", __FUNCTION__));
			return;
		}
	}
	else {
		WL_ERROR(("%s: Out of mem: Unable to alloc pktfetch_info!\n", __FUNCTION__));
		return;
	}

	ctx->wlc = wlc;
	ctx->scb = scb;
	bcopy(f, &ctx->f, sizeof(wlc_frminfo_t));
#ifdef WLAMPDU
	if (SCB_AMPDU(scb) && !f->ismulti && !promisc_frame)
		ctx->ampdu_path = TRUE;
#endif
	ctx->pinfo = pinfo;

	/* Headroom does not need to be > PKTRXFRAGSZ */
	pinfo->osh = wlc->osh;
	pinfo->headroom = PKTRXFRAGSZ;
	pinfo->host_offset = 0;
	pinfo->lfrag = f->p;
	pinfo->ctx = ctx;
	pinfo->cb = wlc_eapol_pktfetch_cb;
#ifdef BCMPCIEDEV_ENABLED
	hnd_pktfetch(pinfo);
#endif
}

void
wlc_eapol_pktfetch_cb(void *lbuf, void *orig_lfrag, void *ctxt, bool cancelled)
{
	wlc_eapol_pktfetch_ctx_t *ctx = ctxt;
	wlc_info_t *wlc = ctx->wlc;
	struct scb *scb = ctx->scb;
	osl_t *osh = wlc->osh;
	wlc_frminfo_t f;
	bool ampdu_path = ctx->ampdu_path;

	/* Replicate frameinfo buffer */
	bcopy(&ctx->f, &f, sizeof(wlc_frminfo_t));
	wlc_recreate_frameinfo(wlc, lbuf, orig_lfrag, &ctx->f, &f);

	/* Copy PKTTAG */
	bcopy(WLPKTTAG(orig_lfrag), WLPKTTAG(lbuf), sizeof(wlc_pkttag_t));

	/* Cleanup first before dispatch */
	PKTFREE(osh, orig_lfrag, FALSE);
	MFREE(osh, ctx->pinfo, sizeof(struct pktfetch_info));
	MFREE(osh, ctx, sizeof(wlc_eapol_pktfetch_ctx_t));

	if (ampdu_path)
		wlc_ampdu_recvdata(wlc->ampdu_rx, scb, &f);
	else
		wlc_recvdata_ordered(wlc, scb, &f);
}

/* Recreates the frameinfo buffer based on offsets of the pulled packet */
void
wlc_recreate_frameinfo(wlc_info_t *wlc, void *lbuf, void *lfrag,
	wlc_frminfo_t *fold, wlc_frminfo_t *fnew)
{
	osl_t *osh = wlc->osh;
	int16 offset_from_start;
	int16 offset = 0;

	/* During recvd frame processing, Pkt Data pointer was offset by some bytes
	 * Need to restore it to start
	 */
	offset_from_start = PKTDATA(osh, lfrag) - (uchar*)fold->wrxh;

	/* Push up data pointer to start of pkt: RxStatus start (wlc_d11rxhdr_t) */
	PKTPUSH(osh, lfrag, offset_from_start);

	/* lfrag data pointer is now at start of pkt.
	 * Push up lbuf data pointer by PKTLEN of lfrag
	 */
	PKTPUSH(osh, lbuf, PKTLEN(osh, lfrag));

	/* Copy the lfrag data starting from RxStatus (wlc_d11rxhdr_t) */
	bcopy(PKTDATA(osh, lfrag), PKTDATA(osh, lbuf), PKTLEN(osh, lfrag));

	/* wrxh and rxh pointers: both have same address */
	fnew->wrxh = (wlc_d11rxhdr_t *)PKTDATA(osh, lbuf);
	fnew->rxh = (d11rxhdr_t *)PKTDATA(osh, lbuf);

	/* Restore data pointer of lfrag and lbuf */
	PKTPULL(osh, lfrag, offset_from_start);
	PKTPULL(osh, lbuf, offset_from_start);

	/* Calculate offset of dot11_header from original lfrag
	 * and apply the same to lbuf frameinfo
	 */
	offset = ((uchar*)fold->h) - PKTDATA(osh, lfrag);
	fnew->h = (struct dot11_header *)(PKTDATA(osh, lbuf) + offset);

	/* Calculate the offset of Packet body pointer
	 * from original lfrag and apply to lbuf frameinfo
	 */
	offset = ((uchar*)fold->pbody) - PKTDATA(osh, lfrag);
	fnew->pbody = (uchar*)(PKTDATA(osh, lbuf) + offset);

	fnew->p = lbuf;
}
#endif /* BCMSPLITRX */

INLINE bool
sstlookup(wlc_info_t *wlc, uint16 proto)
{
	return (proto == 0x80f3 || proto == 0x8137);
}


INLINE bool
rfc894chk(wlc_info_t *wlc, struct dot11_llc_snap_header *lsh)
{
	return lsh->dsap == 0xaa && lsh->ssap == 0xaa && lsh->ctl == 0x03 &&
	        lsh->oui[0] == 0 && lsh->oui[1] == 0 &&
	        ((lsh->oui[2] == 0x00 && !sstlookup(wlc, ntoh16(lsh->type))) ||
	         (lsh->oui[2] == 0xf8 && sstlookup(wlc, ntoh16(lsh->type))));
}

/** Convert 802.3 back to ethernet per 802.1H (use bridge-tunnel if type in SST) */
void BCMFASTPATH
wlc_8023_etherhdr(wlc_info_t *wlc, osl_t *osh, void *p)
{
	struct ether_header *neh, *eh;
	struct dot11_llc_snap_header *lsh;

	if (PKTLEN(osh, p) < DOT11_LLC_SNAP_HDR_LEN)
		return;

	eh = (struct ether_header *) PKTDATA(osh, p);

	/*
	 * check conversion is necessary
	 * - if ([AA AA 03 00 00 00] and protocol is not in SST) or
	 *   if ([AA AA 03 00 00 F8] and protocol is in SST), convert to RFC894
	 * - otherwise,
	 *	 preserve 802.3 (including RFC1042 with protocol in SST)
	 */
	lsh = (struct dot11_llc_snap_header *)((uint8 *)eh + ETHER_HDR_LEN);
	if (!rfc894chk(wlc, lsh))
		return;

	/* 802.3 MAC header */
	neh = (struct ether_header *) PKTPULL(osh, p, DOT11_LLC_SNAP_HDR_LEN);
	eacopy((char*)eh->ether_shost, (char*)neh->ether_shost);
	eacopy((char*)eh->ether_dhost, (char*)neh->ether_dhost);

	/* no change to the ether_type field */

	return;
}
