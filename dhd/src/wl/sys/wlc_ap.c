/*
 * AP Module
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_ap.c 491883 2014-07-17 23:06:47Z $
 */

#include <wlc_cfg.h>

#ifndef AP
#error "AP must be defined to include this module"
#endif

#include <typedefs.h>
#include <bcmdefs.h>

#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#include <sbconfig.h>
#include <wlioctl.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#include <bcmcrypto/wep.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc_mbss.h>
#include <wlc.h>
#include <wlc_ie_misc_hndlrs.h>
#include <wlc_hw.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_scb_ratesel.h>
#include <wlc_phy_hal.h>
#include <phy_utils_api.h>
#include <wlc_led.h>
#include <wlc_event.h>
#include <wl_export.h>
#include <wlc_apcs.h>
#include <wlc_stf.h>
#include <wlc_ap.h>
#include <wlc_scan.h>
#include <wlc_ampdu_cmn.h>
#include <wlc_amsdu.h>
#ifdef	WLCAC
#include <wlc_cac.h>
#endif
#include <wlc_btcx.h>
#include <wlc_bmac.h>
#ifdef BCMAUTH_PSK
#include <wlc_auth.h>
#endif
#include <wlc_assoc.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif
#ifdef WLP2P
#include <wlc_p2p.h>
#endif
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#include <wlc_lq.h>
#if defined(PROP_TXSTATUS)
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_dfs.h>
#include <wlc_prot_g.h>
#include <wlc_prot_n.h>
#include <wlc_11u.h>
#include <wlc_pcb.h>
#ifdef PSTA
#include <wlc_psta.h>
#endif
#ifdef WL11AC
#include <wlc_vht.h>
#include <wlc_txbf.h>
#endif
#ifdef TRAFFIC_MGMT
#include <wlc_traffic_mgmt.h>
#endif
#ifdef WL11K_AP
#include <wlc_rrm.h>
#endif /* WL11K_AP */
#ifdef MFP
#include <wlc_mfp.h>
#endif
#ifdef WLTOEHW
#include <wlc_tso.h>
#endif /* WLTOEHW */
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#ifdef WLWNM
#include <wlc_wnm.h>
#endif
#include <wlc_stamon.h>
#ifdef WDS
#include <wlc_wds.h>
#endif
#include <wlc_ht.h>
#include <wlc_obss.h>
#include "wlc_txc.h"
#include <wlc_tx.h>
#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif
/* Default pre tbtt time for non mbss case */
#define	PRE_TBTT_DEFAULT_us		2

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)

#define PWRSAVE_RXCHAIN 1
#define PWRSAVE_RADIO 2

/* Bitmap definitions for rxchain power save */
/* rxchain power save enable */
#define PWRSAVE_ENB		0x01
/* enter power save when no STA associated to AP,
 * this flag is valid under the condition
 * PWRSAVE_ENB is on
 */
#define NONASSOC_PWRSAVE_ENB	0x02

#endif /* RXCHAIN_PWRSAVE or RADIO_PWRSAVE */

/*
 * This is a generic structure for power save implementations
 * Defines parameters for packets per second threshold based power save
 */
typedef struct wlc_pwrsave {
	bool	in_power_save;		/* whether we are in power save mode or not */
	uint8	power_save_check;	/* Whether power save mode check need to be done */
	uint8   stas_assoc_check;	/* check for associated STAs before going to power save */
	uint	in_power_save_counter;	/* how many times in the power save mode */
	uint	in_power_save_secs;	/* how many seconds in the power save mode */
	uint	quiet_time_counter;	/* quiet time before we enter the  power save mode */
	uint	prev_pktcount;		/* total pkt count from the previous second */
	uint	quiet_time;		/* quiet time in the network before we go to power save */
	uint	pps_threshold;		/* pps threshold for power save */
} wlc_pwrsave_t;


typedef struct wlc_rxchain_pwrsave {
#ifdef WL11N
	/* need to save rx_stbc HT capability before enter rxchain_pwrsave mode */
	uint8	ht_cap_rx_stbc;		/* configured rx_stbc HT capability */
#endif
	uint	rxchain;		/* configured rxchains */
	wlc_pwrsave_t pwrsave;
} wlc_rxchain_pwrsave_t;


typedef struct wlc_radio_pwrsave {
	uint8  level;			/* Low, Medium or High Power Savings */
	uint16 on_time;			/* number of  TUs radio is 'on' */
	uint16 off_time;		/* number of  TUs radio is 'off' */
	int radio_disabled;		/* Whether the radio needs to be disabled now */
	uint	pwrsave_state;		/* radio pwr save state */
	int32	tbtt_skip;		/* num of tbtt to skip */
	bool	cncl_bcn;		/* whether to stop bcn or not in level 1 & 2. */
	struct wl_timer *timer;		/* timer to keep track of duty cycle */
	wlc_pwrsave_t pwrsave;
} wlc_radio_pwrsave_t;

#ifdef RXCHAIN_PWRSAVE

#define RXCHAIN_PWRSAVE_ENAB_BRCM_NONHT	1
#define RXCHAIN_PWRSAVE_ENAB_ALL	2

#endif

#ifdef RADIO_PWRSAVE
#define RADIO_PWRSAVE_LOW		0
#define RADIO_PWRSAVE_MEDIUM		1
#define RADIO_PWRSAVE_HIGH		2

#define RADIO_PWRSAVE_TIMER_LATENCY	15
#endif

/*
* The operational mode capabilities for STAs required to associate
* to the BSS.
* NOTE: The order of values is important. They must be kept in
* increasing order of capabilities, because the enums are compared
* numerically.
* That is: OMC_VHT > OMC_HT > OMC_ERP > OMC_NONE.
*/
typedef enum _opmode_cap_t {

	OMC_NONE = 0, /* no requirements for STA to associate to BSS */

	OMC_ERP = 1, /* STA must advertise ERP (11g) capabilities
				  * to be allowed to associate to 2G band BSS.
				  */
	OMC_HT = 2,	 /* STA must advertise HT (11n) capabilities to
				  * be allowed to associate to the BSS.
				  */
	OMC_VHT = 3, /* Devices must advertise VHT (11ac) capabilities
				  * to be allowed to associate to the BSS.
				  */

	OMC_MAX
} opmode_cap_t;

/* bsscfg cubby */
typedef struct ap_bsscfg_cubby {
	/*
	 * operational mode capabilities
	 * required for STA association
	 * acceptance with the BSS.
	 */
	opmode_cap_t opmode_cap_reqd;
} ap_bsscfg_cubby_t;

#define AP_BSSCFG_CUBBY(__apinfo, __cfg) \
((ap_bsscfg_cubby_t *)BSSCFG_CUBBY((__cfg), (__apinfo)->cfgh))

/* Private AP data structure */
typedef struct
{
	struct wlc_ap_info	appub;		/* Public AP interface: MUST BE FIRST */
	wlc_info_t		*wlc;
	wlc_pub_t		*pub;
	bool			goto_longslot;	/* Goto long slot on next beacon */
	uint32			maxassoc;	/* Max # associations to allow */
	wlc_radio_pwrsave_t radio_pwrsave;   /* radio duty cycle power save structure */
	uint16			txbcn_inactivity;	/* txbcn inactivity counter */
	uint16			txbcn_snapshot;	/* snapshot of txbcnfrm register */
	int		        cfgh;			/* ap bsscfg cubby handle */
	bool			ap_sta_onradar; /* local AP and STA are on overlapping
						 * radar channel?
						 */
	wlc_rxchain_pwrsave_t  rxchain_pwrsave;	/* rxchain reduction power save structure */
	ratespec_t	force_bcn_rspec; /* force setup beacon ratespec (in unit of 500kbps) */
	bool		wlancoex;	/* flags to save WLAN dual 2G radios coex status */
} wlc_ap_info_pvt_t;

typedef struct wlc_assoc_req
{
	wlc_ap_info_t *ap;
	wlc_bsscfg_t *bsscfg;
	struct dot11_management_header *hdr;
	uint8 *body;
	uint body_len;
	struct scb *scb;
	bool short_preamble;
	uint len;
	uint16 aid;
	bool akm_ie_included;
	wpa_ie_fixed_t *wpaie;
	bcm_tlv_t *ssid;
	uint8 *pbody;
	void *e_data;
	int e_datalen;
	wlc_rateset_t req_rates;
	uint buf_len;
	uint16 status;
} wlc_assoc_req_t;

/* IOVar table */

/* Parameter IDs, for use only internally to wlc -- in the wlc_ap_iovars
 * table and by the wlc_ap_doiovar() function.  No ordering is imposed:
 * the table is keyed by name, and the function uses a switch.
 */
enum {
	IOV_AP_ISOLATE = 1,
	IOV_SCB_ACTIVITY_TIME,
	IOV_AUTHE_STA_LIST,
	IOV_AUTHO_STA_LIST,
	IOV_WME_STA_LIST,
	IOV_BSS,
	IOV_APCSCHSPEC,
	IOV_MAXASSOC,
	IOV_BSS_MAXASSOC,
	IOV_CLOSEDNET,
	IOV_AP,
	IOV_APSTA,		/* enable simultaneously active AP/STA */
	IOV_PREF_CHANSPEC,      /* User supplied chanspec, for when we're not using AutoChannel */
	IOV_AP_ASSERT,		/* User forced crash */
#ifdef RXCHAIN_PWRSAVE
	IOV_RXCHAIN_PWRSAVE_ENABLE,		/* Power Save with single rxchain enable */
	IOV_RXCHAIN_PWRSAVE_QUIET_TIME,		/* Power Save with single rxchain quiet time */
	IOV_RXCHAIN_PWRSAVE_PPS,		/* single rxchain packets per second */
	IOV_RXCHAIN_PWRSAVE,		/* Current power save mode */
	IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK,	/* Whether to check for associated stas */
#endif
#ifdef RADIO_PWRSAVE
	IOV_RADIO_PWRSAVE_ENABLE,		/* Radio duty cycle Power Save enable */
	IOV_RADIO_PWRSAVE_QUIET_TIME,		/* Radio duty cycle Power Save */
	IOV_RADIO_PWRSAVE_PPS,		/* Radio power save packets per second */
	IOV_RADIO_PWRSAVE,		/* Whether currently in power save or not */
	IOV_RADIO_PWRSAVE_LEVEL,		/* Radio power save duty cycle on time */
	IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK,	/* Whether to check for associated stas */
#endif
	IOV_AP_RESET,	/* User forced reset */
	IOV_BCMDCS, 	/* dynamic channel switch (management) */
	IOV_DYNBCN,	/* Dynamic beaconing */
	IOV_SCB_LASTUSED,	/* time (s) elapsed since any of the associated scb is used */
	IOV_SCB_PROBE,		/* get/set scb probe parameters */
	IOV_SCB_ASSOCED,	/* if it has associated SCBs at phy if level */
	IOV_ACS_UPDATE,		/* update after acs_scan and chanspec selection */
	IOV_BEACON_INFO,	/* get beacon management packet */
	IOV_PROBE_RESP_INFO,	/* get probe response management packet */
	IOV_MODE_REQD,		/*
				 * operational mode capabilities required for STA
				 * association acceptance with the BSS
				 */
	IOV_PSPRETEND_THRESHOLD,
	IOV_PSPRETEND_RETRY_LIMIT,
	IOV_BSS_RATESET,	/* Set rateset per BSS */
	IOV_FORCE_BCN_RSPEC,	/* Setup Beacon rate from lowest basic to specific basic rate */
	IOV_WLANCOEX,		/* Setup dual radio usbap coex function on/off */
	IOV_LAST		/* In case of a need to check max ID number */

};

/* AP IO Vars */
static const bcm_iovar_t wlc_ap_iovars[] = {
	{"ap", IOV_AP,
	0, IOVT_INT32, 0
	},
	{"ap_isolate", IOV_AP_ISOLATE,
	(0), IOVT_BOOL, 0
	},
	{"scb_activity_time", IOV_SCB_ACTIVITY_TIME,
	(IOVF_NTRL|IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"authe_sta_list", IOV_AUTHE_STA_LIST,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(uint32)
	},
	{"autho_sta_list", IOV_AUTHO_STA_LIST,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(uint32)
	},
	{"wme_sta_list", IOV_WME_STA_LIST,
	(0), IOVT_BUFFER, sizeof(uint32)
	},
	{"maxassoc", IOV_MAXASSOC,
	(IOVF_WHL|IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"bss_maxassoc", IOV_BSS_MAXASSOC,
	(IOVF_NTRL), IOVT_UINT32, 0
	},
	{"bss", IOV_BSS,
	(0), IOVT_INT32, 0
	},
	{"closednet", IOV_CLOSEDNET,
	(0), IOVT_BOOL, 0
	},
	{"apcschspec", IOV_APCSCHSPEC,
	(0), IOVT_UINT16, 0
	},
#ifdef RXCHAIN_PWRSAVE
	{"rxchain_pwrsave_enable", IOV_RXCHAIN_PWRSAVE_ENABLE,
	(0), IOVT_UINT8, 0
	},
	{"rxchain_pwrsave_quiet_time", IOV_RXCHAIN_PWRSAVE_QUIET_TIME,
	(0), IOVT_UINT32, 0
	},
	{"rxchain_pwrsave_pps", IOV_RXCHAIN_PWRSAVE_PPS,
	(0), IOVT_UINT32, 0
	},
	{"rxchain_pwrsave", IOV_RXCHAIN_PWRSAVE,
	(0), IOVT_UINT8, 0
	},
	{"rxchain_pwrsave_stas_assoc_check", IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK,
	(0), IOVT_UINT8, 0
	},
#endif /* RXCHAIN_PWRSAVE */
#ifdef RADIO_PWRSAVE
	{"radio_pwrsave_enable", IOV_RADIO_PWRSAVE_ENABLE,
	(0), IOVT_UINT8, 0
	},
	{"radio_pwrsave_quiet_time", IOV_RADIO_PWRSAVE_QUIET_TIME,
	(0), IOVT_UINT32, 0
	},
	{"radio_pwrsave_pps", IOV_RADIO_PWRSAVE_PPS,
	(0), IOVT_UINT32, 0
	},
	{"radio_pwrsave_level", IOV_RADIO_PWRSAVE_LEVEL,
	(0), IOVT_UINT8, 0
	},
	{"radio_pwrsave", IOV_RADIO_PWRSAVE,
	(0), IOVT_UINT8, 0
	},
	{"radio_pwrsave_stas_assoc_check", IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK,
	(0), IOVT_UINT8, 0
	},
#endif /* RADIO_PWRSAVE */
#if defined(STA) /* APSTA */
	{"apsta", IOV_APSTA,
	(IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_BOOL, 0,
	},
#endif /* APSTA */
	{"pref_chanspec", IOV_PREF_CHANSPEC,
	(0), IOVT_UINT16, 0
	},
#ifdef BCM_DCS
	{"bcm_dcs", IOV_BCMDCS,
	(0), IOVT_BOOL, 0
	},
#endif /* BCM_DCS */
	{"dynbcn", IOV_DYNBCN,
	(0), IOVT_BOOL, 0
	},
	{"scb_lastused", IOV_SCB_LASTUSED, (0), IOVT_UINT32, 0},
	{"scb_probe", IOV_SCB_PROBE,
	(IOVF_SET_UP|IOVF_RSDB_SET), IOVT_BUFFER, sizeof(wl_scb_probe_t)
	},
	{"scb_assoced", IOV_SCB_ASSOCED, (0), IOVT_BOOL, 0},
	{"acs_update", IOV_ACS_UPDATE,
	(IOVF_SET_UP), IOVT_BOOL, 0
	},
	{"beacon_info", IOV_BEACON_INFO,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(uint32),
	},
	{"probe_resp_info", IOV_PROBE_RESP_INFO,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(uint32),
	},
	{"mode_reqd", IOV_MODE_REQD,
	(IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
#ifdef PSPRETEND
	{"pspretend_threshold", IOV_PSPRETEND_THRESHOLD,
	(0), IOVT_UINT8, 0
	},
	{"pspretend_retry_limit", IOV_PSPRETEND_RETRY_LIMIT,
	(0), IOVT_UINT8, 0
	},
#endif
	{"bss_rateset", IOV_BSS_RATESET,
	(0), IOVT_INT32, 0
	},
	{"force_bcn_rspec", IOV_FORCE_BCN_RSPEC,
	(0), IOVT_UINT8, 0
	},
#ifdef USBAP
	{"wlancoex", IOV_WLANCOEX,
	(0), IOVT_BOOL, 0
	},
#endif
	{NULL, 0, 0, 0, 0 }
};

/* Local Prototypes */
static void wlc_ap_watchdog(void *arg);
static int wlc_ap_wlc_up(void *ctx);
#if !(defined(NDIS) && (NDISVER >= 0x0620))
static void wlc_assoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlif);
#if 0
static void wlc_reassoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlif);
#endif
#endif /* !(NDIS && (NDISVER >= 0x0620)) */
static void wlc_ap_sta_probe(wlc_ap_info_t *ap, struct scb *scb);
static void wlc_ap_sta_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb, void *pkt);
static void wlc_ap_stas_timeout(wlc_ap_info_t *ap);
static int wlc_authenticated_sta_check_cb(struct scb *scb);
static int wlc_authorized_sta_check_cb(struct scb *scb);
static int wlc_wme_sta_check_cb(struct scb *scb);
static int wlc_sta_list_get(wlc_ap_info_t *ap, wlc_bsscfg_t *cfg, uint8 *buf,
                            int len, int (*sta_check)(struct scb *scb));
#ifdef BCMDBG
static int wlc_dump_ap(wlc_ap_info_t *ap, struct bcmstrbuf *b);
#ifdef RXCHAIN_PWRSAVE
static int wlc_dump_rxchain_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b);
#endif
#ifdef RADIO_PWRSAVE
static int wlc_dump_radio_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b);
#endif
#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
static void wlc_dump_pwrsave(wlc_pwrsave_t *pwrsave, struct bcmstrbuf *b);
#endif
#endif /* BCMDBG */

static int wlc_ap_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static int wlc_ap_ioctl(void *hdl, int cmd, void *arg, int len, struct wlc_if *wlcif);

static void wlc_ap_up_upd(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, bool state);

static void wlc_ap_acs_update(wlc_info_t *wlc);
static int wlc_ap_set_opmode_cap_reqd(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	opmode_cap_t opmode);
static opmode_cap_t wlc_ap_get_opmode_cap_reqd(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg);
static int wlc_ap_bsscfg_init(void *context, wlc_bsscfg_t *cfg);

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
#ifdef RXCHAIN_PWRSAVE
static void wlc_disable_pwrsave(wlc_ap_info_t *ap, int type);
#ifdef WDS
static bool wlc_rxchain_wds_detection(wlc_info_t *wlc);
#endif /* WDS */
#endif /* RXCHAIN_PWRSAVE */
static void wlc_reset_pwrsave_mode(wlc_ap_info_t *ap, int type);
static void wlc_pwrsave_mode_check(wlc_ap_info_t *ap, int type);
#endif /* RXCHAIN_PWRSAVE || RADIO_PWRSAVE */
#ifdef RADIO_PWRSAVE
static void wlc_radio_pwrsave_update_quiet_ie(wlc_ap_info_t *ap, uint8 count);
static void wlc_reset_radio_pwrsave_mode(wlc_ap_info_t *ap);
static void wlc_radio_pwrsave_off_time_start(wlc_ap_info_t *ap);
static void wlc_radio_pwrsave_timer(void *arg);
#endif /* RADIO_PWRSAVE */


static void wlc_ap_probe_complete(wlc_info_t *wlc, void *pkt, uint txs);

/* IE mgmt */
static uint wlc_assoc_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_assoc_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_assoc_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_assoc_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_assoc_parse_ssid_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_assoc_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_assoc_parse_ext_rates_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_assoc_parse_wps_ie(void *ctx, wlc_iem_parse_data_t *data);

static bool wlc_ap_on_radarchan(wlc_ap_info_t *ap);
static int wlc_force_bcn_rspec_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_ap_info_t *ap,
	ratespec_t rspec);
#ifdef USBAP
static int wlc_wlancoex_upd(wlc_info_t *wlc, wlc_ap_info_t *ap, bool coex_en);
#endif /* USBAP */
/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>


wlc_ap_info_t*
BCMATTACHFN(wlc_ap_attach)(wlc_info_t *wlc)
{
	int err = 0;
	wlc_ap_info_pvt_t *appvt;
	wlc_ap_info_t* ap;
	wlc_pub_t *pub = wlc->pub;
	uint16 arsfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);

#ifdef RXCHAIN_PWRSAVE
	char *var;
#endif /* RXCHAIN_PWRSAVE */

	appvt = (wlc_ap_info_pvt_t*)MALLOCZ(pub->osh, sizeof(wlc_ap_info_pvt_t));
	if (appvt == NULL) {
		WL_ERROR(("%s: MALLOC wlc_ap_info_pvt_t failed\n", __FUNCTION__));
		goto fail;
	}

	ap = &appvt->appub;
	ap->shortslot_restrict = FALSE;

	ap->scb_timeout = SCB_TIMEOUT;
	ap->scb_activity_time = SCB_ACTIVITY_TIME;
	ap->scb_max_probe = SCB_GRACE_ATTEMPTS;

	appvt->wlc = wlc;
	appvt->pub = pub;
	appvt->maxassoc = wlc_ap_get_maxassoc_limit(ap);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((appvt->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(ap_bsscfg_cubby_t),
		wlc_ap_bsscfg_init, NULL, NULL,	(void *)appvt)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_apps_attach(wlc)) {
		WL_ERROR(("%s: wlc_apps_attach failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef RXCHAIN_PWRSAVE
#ifdef BCMDBG
	wlc_dump_register(pub, "rxchain_pwrsave", (dump_fn_t)wlc_dump_rxchain_pwrsave, (void *)ap);
#endif
	var = getvar(NULL, "wl_nonassoc_rxchain_pwrsave_enable");
	if (var) {
		if (!bcm_strtoul(var, NULL, 0))
			appvt->rxchain_pwrsave.pwrsave.power_save_check &= ~NONASSOC_PWRSAVE_ENB;
		else
			appvt->rxchain_pwrsave.pwrsave.power_save_check |= NONASSOC_PWRSAVE_ENB;
	}
#endif /* RXCHAIN_PWRSAVE */

#ifdef RADIO_PWRSAVE
	if (!(appvt->radio_pwrsave.timer =
		wl_init_timer(wlc->wl, wlc_radio_pwrsave_timer, ap, "radio_pwrsave"))) {
		WL_ERROR(("%s: wl_init_timer for radio powersave timer failed\n", __FUNCTION__));
		goto fail;
	}
#ifdef BCMDBG
	wlc_dump_register(pub, "radio_pwrsave", (dump_fn_t)wlc_dump_radio_pwrsave, (void *)ap);
#endif
#endif /* RADIO_PWRSAVE */

#ifdef BCMDBG
	wlc_dump_register(pub, "ap", (dump_fn_t)wlc_dump_ap, (void *)ap);
#endif
	err = wlc_module_register(pub, wlc_ap_iovars, "ap", appvt, wlc_ap_doiovar,
	                          wlc_ap_watchdog, wlc_ap_wlc_up, NULL);
	if (err) {
		WL_ERROR(("%s: wlc_module_register failed\n", __FUNCTION__));
		goto fail;
	}

	err = wlc_module_add_ioctl_fn(wlc->pub, (void *)ap, wlc_ap_ioctl, 0, NULL);
	if (err) {
		WL_ERROR(("%s: wlc_module_add_ioctl_fn err=%d\n",
		          __FUNCTION__, err));
		goto fail;
	}


	/* register packet class callback */
	err = wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_STA_PRB, wlc_ap_probe_complete);
	if (err != BCME_OK) {
		WL_ERROR(("%s: wlc_pcb_fn_set err=%d\n", __FUNCTION__, err));
		goto fail;
	}

	/* register IE mgmt callback */
	/* calc/build */
	/* assocresp/reassocresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arsfstbmp, DOT11_MNG_RATES_ID,
	      wlc_assoc_calc_sup_rates_ie_len, wlc_assoc_write_sup_rates_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, sup rates in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arsfstbmp, DOT11_MNG_EXT_RATES_ID,
	      wlc_assoc_calc_ext_rates_ie_len, wlc_assoc_write_ext_rates_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, ext rates in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* parse */
	/* assocreq/reassocreq */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_SSID_ID,
	                             wlc_assoc_parse_ssid_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ssid in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_RATES_ID,
	                             wlc_assoc_parse_sup_rates_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, sup rates in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_EXT_RATES_ID,
	                             wlc_assoc_parse_ext_rates_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ext rates in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_WPS,
	                                wlc_assoc_parse_wps_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, wps in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	ap->txbcn_timeout = (getintvar(pub->vars, "watchdog") / 1000);

	return (wlc_ap_info_t*)appvt;

fail:
	wlc_ap_detach((wlc_ap_info_t*)appvt);
	return NULL;
}

void
BCMATTACHFN(wlc_ap_detach)(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt;
	wlc_info_t *wlc;
	wlc_pub_t *pub;

	if (ap == NULL) {
		return;
	}
	appvt = (wlc_ap_info_pvt_t*) ap;
	wlc = appvt->wlc;
	pub = appvt->pub;

	wlc_apps_detach(wlc);

#ifdef RADIO_PWRSAVE
	if (appvt->radio_pwrsave.timer) {
		wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
		wl_free_timer(wlc->wl, appvt->radio_pwrsave.timer);
		appvt->radio_pwrsave.timer = NULL;
	}
#endif

	wlc_module_unregister(pub, "ap", appvt);

	wlc_module_remove_ioctl_fn(wlc->pub, (void *)ap);

	MFREE(pub->osh, appvt, sizeof(wlc_ap_info_pvt_t));
}

static int
wlc_ap_wlc_up(void *ctx)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *)ctx;
	wlc_ap_info_t *ap = &appvt->appub;

	(void)ap;

#ifdef RXCHAIN_PWRSAVE
	wlc_reset_rxchain_pwrsave_mode(ap);
#endif
#ifdef RADIO_PWRSAVE
	wlc_reset_radio_pwrsave_mode(ap);
#endif

	return BCME_OK;
}

int
wlc_ap_up(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*)ap;
	wlc_info_t *wlc = appvt->wlc;
	uint channel;
	wlcband_t *band;
	wlc_bss_info_t *target_bss = bsscfg->target_bss;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
#ifdef WLMCHAN
	chanspec_t chspec;
#endif
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_INFORM)
	char chanbuf[CHANSPEC_STR_LEN];
#endif /* BCMDBG || BCMDBG_ERR || WLMSG_INFORM */

	/* update radio parameters from default_bss */
	/* adopt the default BSS parameters as our BSS */
	/* adopt the default BSS params as the target's BSS params */
	bcopy(wlc->default_bss, target_bss, sizeof(wlc_bss_info_t));

	/* set some values to be appropriate for AP operation */
	target_bss->infra = 1;
	target_bss->atim_window = 0;
	target_bss->capability = DOT11_CAP_ESS;
#ifdef WL11K_AP
	if (WL11K_ENAB(wlc->pub)) {
		if (wlc_rrm_enabled(wlc->rrm_info, bsscfg))
			target_bss->capability |= DOT11_CAP_RRM;
	}
#endif /* WL11K_AP */

	bcopy((char*)&bsscfg->cur_etheraddr, (char*)&target_bss->BSSID, ETHER_ADDR_LEN);


#ifdef WLMCHAN
	/* allow P2P GO to run a possible different channel */
	if (MCHAN_ENAB(wlc->pub) &&
	    !(BSSCFG_AP_MCHAN_DISABLED(wlc, bsscfg) ||
	      wlc_mchan_stago_is_disabled(wlc->mchan)) &&
#ifdef WLP2P
	    BSS_P2P_ENAB(wlc, bsscfg) &&
#endif
	    (chspec = wlc_mchan_configd_go_chanspec(wlc->mchan, bsscfg)) != 0) {
		WL_INFORM(("wl%d: use cfg->chanspec 0x%04x in BSS %s\n", wlc->pub->unit,
		          chspec, bcm_ether_ntoa(&target_bss->BSSID, eabuf)));
		target_bss->chanspec = chspec;
		/* continue to validate the chanspec (channel and channel width)...
		 * they may become invalid due to other user configurations happened
		 * between GO bsscfg creation and now...
		 */
	}
	else
#endif /* WLMCHAN */
	/* use the current operating channel if any */
	if (wlc->pub->associated) {
		WL_INFORM(("wl%d: share wlc->home_chanspec 0x%04x in BSS %s\n", wlc->pub->unit,
		          wlc->home_chanspec, bcm_ether_ntoa(&target_bss->BSSID, eabuf)));
		target_bss->chanspec = wlc->home_chanspec;
		return BCME_OK;
	}

#ifdef RADAR
	/* use dfs selected channel */
	if (WL11H_AP_ENAB(wlc) && CHSPEC_IS5G(target_bss->chanspec) &&
	    !(BSSCFG_SRADAR_ENAB(bsscfg) || BSSCFG_AP_NORADAR_CHAN_ENAB(bsscfg) ||
		BSSCFG_AP_USR_RADAR_CHAN_SELECT(bsscfg))) {
		/* if chanspec_next is uninitialized, e.g. softAP with
		 * DFS_TPC country, pick a random one like wlc_restart_ap()
		 */
		if (ap->chanspec_selected)
			target_bss->chanspec = ap->chanspec_selected;
		else
			target_bss->chanspec = wlc_dfs_sel_chspec(wlc->dfs, FALSE);
		WL_INFORM(("wl%d: use dfs chanspec 0x%04x in BSS %s\n",
		           wlc->pub->unit, target_bss->chanspec,
		           bcm_ether_ntoa(&target_bss->BSSID, eabuf)));
	}
#endif /* RADAR */

	/* validate or fixup default channel value */
	/* also, fixup for GO if radar channel */
	if (!wlc_valid_chanspec_db(wlc->cmi, target_bss->chanspec) ||
	    wlc_restricted_chanspec(wlc->cmi, target_bss->chanspec) ||
#ifdef WLP2P
	   (BSS_P2P_ENAB(wlc, bsscfg) &&
	    BSS_11H_AP_NORADAR_CHAN_ENAB(wlc, bsscfg) &&
	    wlc_radar_chanspec(wlc->cmi, target_bss->chanspec)) ||
#endif
	   FALSE) {
		chanspec_t chspec = wlc_default_chanspec(wlc->cmi, FALSE);
		if ((chspec == INVCHANSPEC) ||
		    wlc_restricted_chanspec(wlc->cmi, chspec)) {
			WL_ERROR(("wl%d: cannot create BSS on chanspec %s\n",
				wlc->pub->unit,
				wf_chspec_ntoa_ex(target_bss->chanspec, chanbuf)));
			bsscfg->up = FALSE;
			return BCME_BADCHAN;
		}
		target_bss->chanspec = chspec;
		WL_INFORM(("wl%d: use default chanspec %s in BSS %s\n",
			wlc->pub->unit, wf_chspec_ntoa_ex(chspec, chanbuf),
			bcm_ether_ntoa(&target_bss->BSSID, eabuf)));
	}

	/* Validate the channel bandwidth */
	band = wlc->bandstate[CHSPEC_IS2G(target_bss->chanspec) ? BAND_2G_INDEX : BAND_5G_INDEX];
	if (CHSPEC_IS40(target_bss->chanspec) &&
	    (!N_ENAB(wlc->pub) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_40MHZ) ||
	     !WL_BW_CAP_40MHZ(band->bw_cap))) {
		channel = wf_chspec_ctlchan(target_bss->chanspec);
		target_bss->chanspec = CH20MHZ_CHSPEC(channel);
		WL_INFORM(("wl%d: use 20Mhz channel width in BSS %s\n",
		           wlc->pub->unit, bcm_ether_ntoa(&target_bss->BSSID, eabuf)));
	}

	return BCME_OK;
}

int
wlc_ap_down(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	int callback = 0; /* Need to fix this timer callback propagation; error prone right now */
	struct scb_iter scbiter;
	struct scb *scb;
	int assoc_scb_count = 0;

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		wlc_mcnx_bss_upd(wlc->mcnx, bsscfg, FALSE);
	}
#endif

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (SCB_ASSOCIATED(scb)) {
			if (WIN7_AND_UP_OS(wlc->pub)) {
				/* indicate sta leaving */
				wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, &scb->ea,
					WLC_E_STATUS_SUCCESS, DOT11_RC_DISASSOC_LEAVING, 0, 0, 0);
			}
			wlc_scbfree(wlc, scb);
			assoc_scb_count++;
		}
	}

	if (assoc_scb_count) {
		if (WIN7_AND_UP_OS(wlc->pub) && wlc->pub->up) {
			/* send broadcast disassoc request to all stas */
			wlc_senddisassoc(wlc, bsscfg, WLC_BCMCSCB_GET(wlc, bsscfg),
			                 &ether_bcast, &bsscfg->cur_etheraddr, &bsscfg->BSSID,
			                 DOT11_RC_DISASSOC_LEAVING);
		}
		else if (!WIN7_AND_UP_OS(wlc->pub)) {
			/* send up a broadcast deauth mac event if there were any
			 * associated STAs
			 */
			wlc_deauth_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS, &ether_bcast,
			                    DOT11_RC_DEAUTH_LEAVING, 0);
		}
	}

	/* adjust associated state(s) accordingly */
	wlc_ap_up_upd(ap, bsscfg, FALSE);

	/* Fix up mac control */
	wlc_macctrl_init(wlc, NULL);

	WL_APSTA_UPDN(("Reporting link down on config %d (AP disabled)\n",
		WLC_BSSCFG_IDX(bsscfg)));

	wlc_link(wlc, FALSE, &bsscfg->cur_etheraddr, bsscfg, WLC_E_LINK_BSSCFG_DIS);


#ifdef RADIO_PWRSAVE
	wlc_reset_radio_pwrsave_mode(ap);
#endif

	/* reset per bss link margin values */
	wlc_ap_bss_tpc_setup(wlc->tpc, bsscfg);

	/* Clear the BSSID */
	bzero(bsscfg->BSSID.octet, ETHER_ADDR_LEN);




	return callback;
}

#ifdef WME
/* Initialize a WME Parameter Info Element with default AP parameters from WMM Spec, Table 14 */
void
wlc_wme_initparams_ap(wlc_ap_info_t *ap, wme_param_ie_t *pe)
{
	static const wme_param_ie_t apdef = {
		WME_OUI,
		WME_OUI_TYPE,
		WME_SUBTYPE_PARAM_IE,
		WME_VER,
		0,
		0,
		{
			{ EDCF_AC_BE_ACI_AP, EDCF_AC_BE_ECW_AP, HTOL16(EDCF_AC_BE_TXOP_AP) },
			{ EDCF_AC_BK_ACI_AP, EDCF_AC_BK_ECW_AP, HTOL16(EDCF_AC_BK_TXOP_AP) },
			{ EDCF_AC_VI_ACI_AP, EDCF_AC_VI_ECW_AP, HTOL16(EDCF_AC_VI_TXOP_AP) },
			{ EDCF_AC_VO_ACI_AP, EDCF_AC_VO_ECW_AP, HTOL16(EDCF_AC_VO_TXOP_AP) }
		}
	};

	STATIC_ASSERT(sizeof(*pe) == WME_PARAM_IE_LEN);
	memcpy(pe, &apdef, sizeof(*pe));
}
#endif /* WME */

static int
wlc_authenticated_sta_check_cb(struct scb *scb)
{
	return SCB_AUTHENTICATED(scb);
}

static int
wlc_authorized_sta_check_cb(struct scb *scb)
{
	return SCB_AUTHORIZED(scb);
}

static int
wlc_wme_sta_check_cb(struct scb *scb)
{
	return SCB_WME(scb);
}

/* Returns a maclist of all scbs that pass the provided check function.
 * The buf is formatted as a struct maclist on return, and may be unaligned.
 * buf must be at least 4 bytes long to hold the maclist->count value.
 * If the maclist is too long for the supplied buffer, BCME_BUFTOOSHORT is returned
 * and the maclist->count val is set to the number of MAC addrs that would
 * have been returned. This allows the caller to allocate the needed space and
 * call again.
 */
static int
wlc_sta_list_get(wlc_ap_info_t *ap, wlc_bsscfg_t *cfg, uint8 *buf,
                 int len, int (*sta_check_cb)(struct scb *scb))
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	int err = 0;
	uint c = 0;
	uint8 *dst;
	struct scb *scb;
	struct scb_iter scbiter;
	ASSERT(len >= (int)sizeof(uint));

	/* make room for the maclist count */
	dst = buf + sizeof(uint);
	len -= sizeof(uint);
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (sta_check_cb(scb)) {
			c++;
			if (len >= ETHER_ADDR_LEN) {
				bcopy(scb->ea.octet, dst, ETHER_ADDR_LEN);
				dst += sizeof(struct ether_addr);
				len -= sizeof(struct ether_addr);
			} else {
				err = BCME_BUFTOOSHORT;
			}
		}
	}

	/* copy the actual count even if the buffer is too short */
	bcopy(&c, buf, sizeof(uint));

	return err;
}


/* AP processes WME Setup Request Management frame */
/* Algorithm for accept/reject is still in flux. For now, reject all requests */
void
wlc_wme_setup_req(wlc_ap_info_t *ap, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	void *outp;
	uint8* pbody;
	struct dot11_management_notification* mgmt_hdr;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif
	wlc_bsscfg_t *cfg = wlc->cfg;

	if ((outp = wlc_frame_get_mgmt(wlc, FC_ACTION, &hdr->sa, &cfg->cur_etheraddr,
	                               &cfg->BSSID, body_len, &pbody)) == NULL) {
		WL_ERROR(("wl%d: failed to get WME response packet \n",
			wlc->pub->unit));
		return;
	}

	/*
	 *  duplicate incoming packet and change
	 *  the few necessary fields
	 */
	bcopy((char *)body, (char *)pbody, body_len);
	mgmt_hdr = (struct dot11_management_notification *)pbody;
	mgmt_hdr->action = WME_ADDTS_RESPONSE;

	/* for now, reject all setup requests */
	mgmt_hdr->status = WME_ADMISSION_REFUSED;
	WL_INFORM(("Sending WME_SETUP_RESPONSE, refusal to %s\n", sa));
	wlc_sendmgmt(wlc, outp, cfg->wlcif->qi, NULL);
}

static void
wlc_ap_stas_timeout(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	struct scb *scb;
	struct scb_iter scbiter;


	WL_INFORM(("%s: run at time = %d\n", __FUNCTION__, wlc->pub->now));
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		wlc_bsscfg_t *cfg = SCB_BSSCFG(scb);

		/* Don't age out the permanent SCB and AP SCB */
		if (scb->permanent ||
		    cfg == NULL || !BSSCFG_AP(cfg))
			continue;

		/* kill any other band scb */
		if (TRUE &&
#ifdef WLMCHAN
		    !MCHAN_ENAB(wlc->pub) &&
#endif
		    wlc_scbband(scb) != wlc->band) {
			wlc_scbfree(wlc, scb);
			continue;
		}

		if (SCB_MARKED_FOR_DELETION(scb)) {
			/* If the SCB has been marked for deletion, let's free the SCB here */
			wlc_scbfree(wlc, scb);
			continue;
		}

		/* probe associated stas if idle for scb_activity_time or reprobe them */
		if (SCB_ASSOCIATED(scb) &&
		    ((ap->scb_activity_time && ((wlc->pub->now - scb->used) >=
		      ap->scb_activity_time)) ||
		     (ap->reprobe_scb && scb->grace_attempts))) {
			wlc_ap_sta_probe(ap, scb);
		}

		/* Authenticated but idle for long time free it now */
		if ((scb->state == AUTHENTICATED) &&
		    ((wlc->pub->now - scb->used) >= SCB_LONG_TIMEOUT)) {
			wlc_scbfree(wlc, scb);
			continue;
		}
	}
}

uint
wlc_ap_stas_associated(wlc_ap_info_t *ap)
{
	wlc_info_t *wlc = ((wlc_ap_info_pvt_t *)ap)->wlc;
	int i;
	wlc_bsscfg_t *bsscfg;
	uint count = 0;

	FOREACH_UP_AP(wlc, i, bsscfg)
		count += wlc_bss_assocscb_getcnt(wlc, bsscfg);

	return count;
}

static void
wlc_auth_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_auth_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

void
wlc_ap_authresp(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr,  uint8 *body, uint body_len,
	void *p, bool short_preamble, d11rxhdr_t *rxh)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	struct dot11_auth *auth;
	struct scb *scb = NULL;
	uint16 auth_alg, auth_seq;
	uint16 status = DOT11_SC_SUCCESS;
	int i;
	wlc_key_id_t key_id;
	uint16 fc;
	int err;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
	wlc_key_t *key;

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_BTA)
	char eabuf[ETHER_ADDR_STR_LEN], *sa;
#endif

	WL_TRACE(("wl%d: wlc_authresp_ap\n", WLCWLUNIT(wlc)));

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_BTA)
	sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif

	auth = (struct dot11_auth *) body;

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_ASSOC(("wl%d: AP Scan in progress, abort auth\n",
			wlc->pub->unit));
		status = SMFS_CODE_IGNORED;
		goto smf_stats;
	}

#ifdef BCMAUTH_PSK
	/* check for tkip countermesures */
	if (BSSCFG_AP(bsscfg) && WSEC_TKIP_ENABLED(bsscfg->wsec) &&
		wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg))
		return;
#endif /* BCMAUTH_PSK */

	/* Only handle Auth requests when the bsscfg is operational */
	if (BSSCFG_AP(bsscfg) && !bsscfg->up) {
		WL_ASSOC(("wl%d: BSS is not operational, abort auth\n", wlc->pub->unit));
		status = SMFS_CODE_IGNORED;
		goto smf_stats;
	}

#ifdef PSTA
	/* Don't allow more than psta max assoc limit */
	if (PSTA_ENAB(wlc->pub) && (wlc_ap_stas_associated(ap) >= PSTA_MAX_ASSOC(wlc))) {
	        WL_ERROR(("wl%d: %s denied association due to max psta association limit\n",
	                  wlc->pub->unit, sa));
		status = SMFS_CODE_IGNORED;
		goto smf_stats;
	}
#endif /* PSTA */

#ifdef BAND5G
	/* Reject auth & assoc while  performing a channel switch */
	if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
		WL_REGULATORY(("wl%d: %s: Authentication denied while in radar"
		               " avoidance mode\n", wlc->pub->unit, __FUNCTION__));
		status = SMFS_CODE_IGNORED;
		goto smf_stats;
	}
#endif /* BAND5G */

	ASSERT(BSSCFG_AP(bsscfg));

	fc = ltoh16(hdr->fc);
	if (fc & FC_WEP) {

		/* frame is protected, assume shared key */
		auth_alg = DOT11_SHARED_KEY;
		auth_seq = 3;

		/* Check if it is logical to have an encrypted packet here */
		if ((scb = wlc_scbfind(wlc, bsscfg, (struct ether_addr *) &hdr->sa)) == NULL) {
			WL_ERROR(("wl%d: %s: auth resp frame encrypted"
				  "from unknown STA %s\n", wlc->pub->unit, __FUNCTION__, sa));
			status = SMFS_CODE_MALFORMED;
			goto smf_stats;
		}
		if (scb->challenge == NULL) {
			WL_ERROR(("wl%d: %s: auth resp frame encrypted "
				"with no challenge recorded from %s\n",
				wlc->pub->unit, __FUNCTION__, sa));
			status = SMFS_CODE_MALFORMED;
			goto smf_stats;
		}

		/* Processing a WEP encrypted AUTH frame:
		 * BSS config0 is allowed to use the HW default keys, all other BSS configs require
		 * software decryption of AUTH frames.  For simpler code:
		 *
		 * If the frame has been decrypted(a default HW key is present at the right index),
		 * always re-encrypt the frame with the key used by HW and then use the BSS config
		 * specific WEP key to decrypt. This means that all WEP encrypted AUTH frames will
		 * be decrypted in software.
		 */

		WL_ASSOC(("wl%d: %s: received wep from %sassociated scb\n",
		          wlc->pub->unit, __FUNCTION__, SCB_ASSOCIATED(scb) ? "" : "non-"));

		WLPKTTAGSCBSET(p, scb);

		status = DOT11_SC_AUTH_CHALLENGE_FAIL;
		key_id = body[3] >> DOT11_KEY_INDEX_SHIFT;
		key = wlc_keymgmt_get_bss_key(wlc->keymgmt, bsscfg, key_id, NULL);

		/* if the frame was incorrectly decrypted with default bss by h/w, rx
		 * processing will encrypt it back and find the right key and
		 * decrypt it.
		 */
		err = wlc_key_rx_mpdu(key, p, rxh);
		if (err != BCME_OK) {
			WL_ASSOC(("wl%d.%d: %s: rx from %s failed with error %d\n",
				WLCWLUNIT(wlc),  WLC_BSSCFG_IDX(bsscfg),
				__FUNCTION__, sa, err));

			/* free the challenge text */
			MFREE(wlc->osh, scb->challenge, 2 + scb->challenge[1]);
			scb->challenge = NULL;
			goto send_result;
		}

		WL_ASSOC(("wl%d: %s: ICV pass : %s: BSSCFG = %d\n",
			WLCWLUNIT(wlc), __FUNCTION__, sa, WLC_BSSCFG_IDX(bsscfg)));

		/* Skip IV */
		body += DOT11_IV_LEN;
		body_len -= DOT11_IV_LEN;

		/* Remove ICV */
		body_len -= DOT11_ICV_LEN;
	}

	auth = (struct dot11_auth *)body;
	auth_alg = ltoh16(auth->alg);
	auth_seq = ltoh16(auth->seq);

	/* if sequence number would be out of range, do nothing */
	if (auth_seq >= 4) {
		status = SMFS_CODE_MALFORMED;
		goto smf_stats;
	}

	body += sizeof(struct dot11_auth);
	body_len -= sizeof(struct dot11_auth);

	if (fc & FC_WEP)
		goto parse_ies;

	switch (auth_seq) {
	case 1:
		for (i = 0; i < (int)NBANDS(wlc); i++) {
			/* Use band 1 for single band 11a */
			if (IS_SINGLEBAND_5G(wlc->deviceid))
				i = BAND_5G_INDEX;

			scb = wlc_scbfindband(wlc, bsscfg, (struct ether_addr *)&hdr->sa, i);
			if (scb) {
				WL_ASSOC(("wl%d: %s: scb for the STA-%s already exists\n",
				          wlc->pub->unit, __FUNCTION__, sa));
				wlc_scbfree(wlc, scb);
			}
		}

		/* allocate an scb */
		if (!(scb = wlc_scblookup(wlc, bsscfg, (struct ether_addr *) &hdr->sa))) {
			WL_ERROR(("wl%d: %s: out of scbs for %s\n",
			          wlc->pub->unit, __FUNCTION__, sa));
			status = DOT11_SC_FAILURE;
			break;
		}

		if (scb->flags & SCB_MYAP) {
			if (APSTA_ENAB(wlc->pub)) {
				WL_APSTA(("wl%d: Reject AUTH request from AP %s\n",
				          wlc->pub->unit, sa));
				status = DOT11_SC_FAILURE;
				break;
			}
			scb->flags &= ~SCB_MYAP;
		}

		wlc_scb_disassoc_cleanup(wlc, scb);


		/* auth_alg is coming from the STA, not us */
		switch (auth_alg) {
		case DOT11_OPEN_SYSTEM:

			if ((WLC_BSSCFG_AUTH(bsscfg) == DOT11_OPEN_SYSTEM) && (!status)) {
				wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED,
				                           WLC_BSSCFG_IDX(bsscfg));
			}
			else {
				wlc_scb_clearstatebit_bsscfg(scb, AUTHENTICATED,
				                             WLC_BSSCFG_IDX(bsscfg));
			}

			/* At this point, we should have at least one valid authentication in open
			 * system
			 */
			if (!(SCB_AUTHENTICATED(scb))) {
				WL_ERROR(("wl%d: %s: Open System auth attempted "
					  "from %s but only Shared Key supported\n",
					  wlc->pub->unit, __FUNCTION__, sa));
				status = DOT11_SC_AUTH_MISMATCH;
				wlc_bss_mac_event(wlc, bsscfg, WLC_E_PRUNE, &hdr->sa, 0,
					WLC_E_PRUNE_ENCR_MISMATCH, 0, 0, 0);
			}
			break;
		case DOT11_SHARED_KEY:
			break;
		default:
			WL_ERROR(("wl%d: %s: unhandled algorithm %d from %s\n",
				wlc->pub->unit, __FUNCTION__, auth_alg, sa));
			status = DOT11_SC_AUTH_MISMATCH;
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_PRUNE, &hdr->sa, 0,
				WLC_E_PRUNE_ENCR_MISMATCH, 0, 0, 0);
			break;
		}
		break;


	default:

		WL_ERROR(("wl%d: %s: unexpected authentication sequence %d from %s\n",
			wlc->pub->unit, __FUNCTION__, auth_seq, sa));
		status = DOT11_SC_AUTH_SEQ;
		break;
	}

	if (status != DOT11_SC_SUCCESS) {
		WL_INFORM(("wl%d: %s: skip IE parse, status %u\n",
		           wlc->pub->unit, __FUNCTION__, status));
		goto send_result;
	}

parse_ies:
	/* prepare IE mgmt calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_auth_nhdlr_cb;
	upp.vsie_fn = wlc_auth_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.auth.alg = auth_alg;
	ftpparm.auth.seq = auth_seq;
	ftpparm.auth.scb = scb;
	ftpparm.auth.status = status;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	if (wlc_iem_parse_frame(wlc->iemi, bsscfg, FC_AUTH, &upp, &pparm,
	                        body, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_parse_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		/* Don't bail out, send response... */
	}
	status = ftpparm.auth.status;

send_result:

	/* send authentication response */
	if (status == DOT11_SC_SUCCESS && scb && SCB_AUTHENTICATED(scb)) {
		WL_ASSOC(("wl%d.%d %s: %s authenticated\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, sa));
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_AUTH_IND, &hdr->sa, WLC_E_STATUS_SUCCESS,
			DOT11_SC_SUCCESS, auth_alg, 0, 0);
	}

#ifdef RXCHAIN_PWRSAVE
	/* fast switch back from rxchain_pwrsave state upon authentication */
	if ((status == DOT11_SC_SUCCESS) && scb && SCB_AUTHENTICATED(scb)) {
		wlc_reset_rxchain_pwrsave_mode(ap);
	}
#endif /* RXCHAIN_PWRSAVE */

	wlc_sendauth(bsscfg, &hdr->sa, &bsscfg->BSSID, scb,
		auth_alg, auth_seq + 1, status, NULL, short_preamble);

smf_stats:
	wlc_smfstats_update(wlc, bsscfg, SMFS_TYPE_AUTH, status);
}

static void
wlc_assoc_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_assoc_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

static void wlc_ap_process_assocreq_next(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * assoc_decision, struct scb *scb,
	wlc_assoc_req_t * param);
static void wlc_ap_process_assocreq_done(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc, struct dot11_management_header *hdr, uint8 *body, uint body_len,
	struct scb *scb, wlc_assoc_req_t * param);
static void wlc_ap_process_assocreq_exit(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc, struct dot11_management_header *hdr, uint8 *body, uint body_len,
	void *rsp, uint rsp_len, struct scb *scb, wlc_assoc_req_t * param);
static void wl_smf_stats(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, wlc_assoc_req_t * param);

static void wl_smf_stats(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, wlc_assoc_req_t * param)
{
#ifdef SMF_STATS
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	bool reassoc;
	uint8 type;

	reassoc = ((ltoh16(hdr->fc) & FC_KIND_MASK) == FC_REASSOC_REQ);
	type = (reassoc ? SMFS_TYPE_REASSOC : SMFS_TYPE_ASSOC);
	wlc_smfstats_update(wlc, bsscfg, type, param->status);
#endif
}

/* respond to association and reassociation requests */
void
wlc_ap_process_assocreq(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, uint body_len,
	struct scb *scb, bool short_preamble)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	ap_bsscfg_cubby_t *ap_cfg = AP_BSSCFG_CUBBY(appvt, bsscfg);
#if defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif
	struct dot11_assoc_req *req = (struct dot11_assoc_req *) body;
	wlc_rateset_t req_rates;
	wlc_rateset_t sup_rates, ext_rates;
	uint8 req_rates_lookup[WLC_MAXRATE+1];
	bool reassoc;
	bool erp_sta;
	uint16 capability;
	uint16 status = DOT11_SC_SUCCESS;
	uint i;
	uint8 r;
	int idx;
	assoc_decision_t decision;
	wlc_assoc_req_t param_buf;
	wlc_assoc_req_t * param;
	/* Used to encapsulate data to a generate event */
	bool akm_ie_included = FALSE;
	wlc_bss_info_t *current_bss;
	uint16 type;
	opmode_cap_t opmode_cap_curr = OMC_NONE;
	opmode_cap_t opmode_cap_reqd = ap_cfg->opmode_cap_reqd;
	wlc_iem_upp_t upp;
	wlc_iem_pparm_t pparm;
	wlc_iem_ft_pparm_t ftpparm;
	uint8 *tlvs;
	uint tlvs_len;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	ASSERT(bsscfg != NULL);
	ASSERT(bsscfg->up);
	ASSERT(BSSCFG_AP(bsscfg));

	param = &param_buf;
	bzero(param, sizeof(wlc_assoc_req_t));
	param->hdr = hdr;
	param->body = body;
	param->ap = ap;
	param->bsscfg = bsscfg;
	param->body_len = body_len;
	param->scb = scb;
	param->short_preamble = short_preamble;

	/* Get the AID, assign a new one if needed */
	if (!scb->aid)
		scb->aid = wlc_bsscfg_newaid(scb->bsscfg);

	param->aid = scb->aid;

#ifdef RXCHAIN_PWRSAVE
	/* fast switch back from rxchain_pwrsave state upon association */
	wlc_reset_rxchain_pwrsave_mode(ap);
#endif /* RXCHAIN_PWRSAVE */

	current_bss = bsscfg->current_bss;

	bzero(&req_rates, sizeof(wlc_rateset_t));

	reassoc = ((ltoh16(hdr->fc) & FC_KIND_MASK) == FC_REASSOC_REQ);
	capability = ltoh16(req->capability);

	type = reassoc ? FC_REASSOC_REQ : FC_ASSOC_REQ;

	if (SCB_AUTHENTICATED(scb) && SCB_ASSOCIATED(scb) && SCB_MFP(scb)) {
		status = DOT11_SC_ASSOC_TRY_LATER;
		goto done;
	}

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_ASSOC(("wl%d: AP Scan in progress, abort association\n", wlc->pub->unit));
		status = DOT11_SC_ASSOC_BUSY_FAIL;
		if (WIN7_AND_UP_OS(wlc->pub))
			goto exit;
		else
			goto smf_stats;
	}

#ifdef BCMAUTH_PSK
	if (AP_ENAB(wlc->pub) && WSEC_TKIP_ENABLED(bsscfg->wsec) &&
		wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg))
		return;
#endif /* BCMAUTH_PSK */

	if ((reassoc && body_len < DOT11_REASSOC_REQ_FIXED_LEN) ||
	    (!reassoc && body_len < DOT11_ASSOC_REQ_FIXED_LEN)) {
		status = DOT11_SC_FAILURE;
		if (WIN7_AND_UP_OS(wlc->pub))
			goto exit;
		else
			goto smf_stats;
	}

	/* set up some locals to hold info from the (re)assoc packet */
	if (!reassoc) {
		tlvs = body + DOT11_ASSOC_REQ_FIXED_LEN;
		tlvs_len = body_len - DOT11_ASSOC_REQ_FIXED_LEN;
	} else {
		tlvs = body + DOT11_REASSOC_REQ_FIXED_LEN;
		tlvs_len = body_len - DOT11_REASSOC_REQ_FIXED_LEN;
	}

	/* Init per scb WPA_auth and wsec.
	 * They will be reinitialized when parsing the AKM IEs.
	 */
	scb->WPA_auth = WPA_AUTH_DISABLED;
	scb->wsec = 0;

	/* send up all IEs in the WLC_E_ASSOC_IND/WLC_E_REASSOC_IND event */
	param->e_data = tlvs;
	param->e_datalen = tlvs_len;

	/* SSID is first tlv */
	param->ssid = (bcm_tlv_t*)tlvs;
	if (!bcm_valid_tlv((bcm_tlv_t *)tlvs, tlvs_len)) {
		status = DOT11_SC_FAILURE;
		if (WIN7_AND_UP_OS(wlc->pub))
			goto exit;
		else
			goto smf_stats;
	}

	/* prepare IE mgmt parse calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_assoc_nhdlr_cb;
	upp.vsie_fn = wlc_assoc_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.assocreq.sup = &sup_rates;
	ftpparm.assocreq.ext = &ext_rates;
	ftpparm.assocreq.scb = scb;
	ftpparm.assocreq.status = status;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;
	pparm.ht = N_ENAB(wlc->pub);
#ifdef WL11AC
	pparm.vht = VHT_ENAB(wlc->pub);
#endif

	/* parse IEs */
	if (wlc_iem_parse_frame(wlc->iemi, bsscfg, type, &upp, &pparm,
	                        tlvs, tlvs_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_parse_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		status = ftpparm.assocreq.status;
		ASSERT(status != DOT11_SC_SUCCESS);
		if (WIN7_AND_UP_OS(wlc->pub))
			goto exit;
		else
			goto smf_stats;
	}
	status = ftpparm.assocreq.status;

	ASSERT(bsscfg == scb->bsscfg);

	/* store the advertised capability field */
	scb->cap = capability;

	param->akm_ie_included = akm_ie_included = (scb->wpaie != NULL);

	/* ===== Assoc Request Validations Start Here ===== */

	/* rates validation */
	if (wlc_combine_rateset(wlc, &sup_rates, &ext_rates, &req_rates) != BCME_OK) {
		WL_ERROR(("wl%d: could not parse the rateset from (Re)Assoc Request "
		          "packet from %s\n", wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_RATE_MISMATCH;
		goto done;
	}
	if (wlc_ht_get_phy_membership(wlc->hti) != req_rates.htphy_membership) {
		WL_ASSOC(("wl%d: mismatch Membership from (Re)Assoc Request packet from"
			  " %s\n", wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_RATE_MISMATCH;
		goto done;
	}


	/* catch the case of an already assoc. STA */
	if (SCB_ASSOCIATED(scb)) {
		/* If we are in PS mode then return to non PS mode as there is a state mismatch
		 * between the STA and the AP
		 */
		if (SCB_PS(scb))
			wlc_apps_scb_ps_off(wlc, scb, TRUE);

		/* return STA to auth state and check the (re)assoc pkt */
		wlc_scb_clearstatebit(scb, ~AUTHENTICATED);
	}

	/* check if we are authenticated w/ this particular bsscfg */
	/*
	  get the index for that bsscfg. Would be nicer to find a way to represent
	  the authentication status directly.
	*/
	idx = WLC_BSSCFG_IDX(bsscfg);
	if (idx < 0) {
		WL_ERROR(("wl%d: %s: association request for non existent bsscfg\n",
		        wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_FAIL;
		goto done;
	}

	if (!SCB_AUTHENTICATED_BSSCFG(scb, idx)) {
		WL_ASSOC(("wl%d: %s: association request for non-authenticated station\n",
		        wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_FAIL;
		goto done;
	}

	/* validate 11h */
	if (WL11H_ENAB(wlc) &&
	    (wlc_11h_get_spect(wlc->m11h) == SPECT_MNGMT_STRICT_11H) &&
	    ((capability & DOT11_CAP_SPECTRUM) == 0)) {

		/* send a failure association response to this STA */
		WL_REGULATORY(("wl%d: %s: association denied as spectrum management is required\n",
		               wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_SPECTRUM_REQUIRED;
		goto done;
	}

#ifdef BAND5G
	/* Previously detected radar and are in the
	 * process of switching to a new channel
	 */
	if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
		/* send a failure association response to this node */
		WL_REGULATORY(("wl%d: %s: association denied while in radar avoidance mode\n",
		        wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_FAIL;
		goto done;
	}
#endif /* BAND5G */

	/* get the requester's rates into a lookup table and record ERP capability */
	erp_sta = FALSE;
	bzero(req_rates_lookup, sizeof(req_rates_lookup));
	for (i = 0; i < req_rates.count; i++) {
		r = req_rates.rates[i] & RATE_MASK;
		if ((r > WLC_MAXRATE) || (rate_info[r] == 0)) {
			WL_INFORM(("%s: bad rate in rate set 0x%x\n", __FUNCTION__, r));
			continue;
		}
		req_rates_lookup[r] = r;
		if (IS_OFDM(r))
			erp_sta = TRUE;
	}

	if (erp_sta)
		opmode_cap_curr = OMC_ERP;

	/* update the scb's capability flags */
	scb->flags &= ~(SCB_NONERP | SCB_LONGSLOT | SCB_SHORTPREAMBLE);
	if (wlc->band->gmode && !erp_sta)
		scb->flags |= SCB_NONERP;
	if (wlc->band->gmode && (!(capability & DOT11_CAP_SHORTSLOT)))
		scb->flags |= SCB_LONGSLOT;
	if (capability & DOT11_CAP_SHORT)
		scb->flags |= SCB_SHORTPREAMBLE;

#ifdef WL11K_AP
	if (WL11K_ENAB(wlc->pub) && wlc_rrm_enabled(wlc->rrm_info, bsscfg)) {
		scb->flags3 &= ~SCB3_RRM;
		if (capability & DOT11_CAP_RRM)
			scb->flags3 |= SCB3_RRM;
	}
#endif /* WL11K_AP */

	/* check the required rates */
	for (i = 0; i < current_bss->rateset.count; i++) {
		/* check if the rate is required */
		r = current_bss->rateset.rates[i];
		if (r & WLC_RATE_FLAG) {
			if (req_rates_lookup[r & RATE_MASK] == 0) {
				/* a required rate was not available */
				WL_ERROR(("wl%d: %s does not support required rate %d\n",
				        wlc->pub->unit, sa, r & RATE_MASK));
				status = DOT11_SC_ASSOC_RATE_MISMATCH;
				goto done;
			}
		}
	}

	/* verify mcs basic rate settings */
	if (N_ENAB(wlc->pub)) {
		ht_cap_ie_t *ht_cap_p = NULL;

		/* find the HT cap IE, if found copy the mcs set into the requested rates */
		if (ftpparm.assocreq.ht_cap_ie != NULL) {
			ht_cap_p = wlc_read_ht_cap_ie(wlc, ftpparm.assocreq.ht_cap_ie,
			        TLV_HDR_LEN + ftpparm.assocreq.ht_cap_ie[TLV_LEN_OFF]);

			wlc_ht_update_scbstate(wlc->hti, scb, ht_cap_p, NULL, NULL);
			if (ht_cap_p != NULL) {
				bcopy(ht_cap_p->supp_mcs, req_rates.mcs, MCSSET_LEN);

				/* verify mcs basic rate settings */
				if (wlc_ht_add_ie_verify_rates(wlc->hti,
					req_rates.mcs, MCSSET_LEN) != BCME_OK) {
						/* a required rate was not available */
						WL_ERROR(("wl%d: %s not support required mcs\n",
							wlc->pub->unit, sa));
						status = DOT11_SC_ASSOC_RATE_MISMATCH;
						goto done;
				}

				opmode_cap_curr = OMC_HT;
			}
		}
#ifdef WL11AC
		if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype)) {
			vht_cap_ie_t *vht_cap_p = NULL;
			vht_cap_ie_t vht_cap;
			vht_op_ie_t *vht_op_p = NULL;
			vht_op_ie_t vht_op;

			if (ftpparm.assocreq.vht_cap_ie != NULL)
				vht_cap_p = wlc_read_vht_cap_ie(wlc->vhti,
					ftpparm.assocreq.vht_cap_ie,
				        TLV_HDR_LEN + ftpparm.assocreq.vht_cap_ie[TLV_LEN_OFF],
				        &vht_cap);
			if (ftpparm.assocreq.vht_op_ie != NULL)
				vht_op_p =  wlc_read_vht_op_ie(wlc->vhti,
					ftpparm.assocreq.vht_op_ie,
				        TLV_HDR_LEN + ftpparm.assocreq.vht_op_ie[TLV_LEN_OFF],
				        &vht_op);
			wlc_vht_update_scb_state(wlc->vhti, wlc->band->bandtype, scb, ht_cap_p,
			                vht_cap_p, vht_op_p, ftpparm.assocreq.vht_ratemask);

			if (vht_cap_p != NULL)
				opmode_cap_curr = OMC_VHT;
		}
#endif /* WL11AC */
	}

	/*
	 * Verify whether the operation capabilities of current STA
	 * acceptable for this BSS based on required setting.
	 */
	if (opmode_cap_curr < opmode_cap_reqd) {
		WL_ASSOC(("wl%d: %s: Assoc is rejected due to oper capability "
			"mode mismatch. Reqd mode=%d, client's mode=%d\n",
			wlc->pub->unit, sa, opmode_cap_reqd, opmode_cap_curr));
		status = DOT11_SC_ASSOC_RATE_MISMATCH;
		goto done;
	}

	/*
	 * If in an auth mode that wants a WPA info element, look for one.
	 * If found, check to make sure what it requests is supported.
	 */
	if ((bsscfg->WPA_auth != WPA_AUTH_DISABLED && WSEC_ENABLED(bsscfg->wsec)) ||
	    WSEC_SES_OW_ENABLED(bsscfg->wsec)) {
		/* WPA/RSN IE is parsed in the registered IE mgmt callbacks and
		 * the scb->WPA_auth and scb->wsec should have been setup by now,
		 * otherwise it signals an abnormality in the STA assoc req frame.
		 */
		if (scb->WPA_auth == WPA_AUTH_DISABLED || !WSEC_ENABLED(scb->wsec)) {
			/* check for transition mode */
			if (!WSEC_WEP_ENABLED(bsscfg->wsec) && !WSEC_SES_OW_ENABLED(bsscfg->wsec)) {
				WL_ERROR(("wl%d: %s: "
				          "deny transition mode assoc req from %s... "
				          "transition mode not enabled on AP\n",
				          wlc->pub->unit, __FUNCTION__, sa));
				status = DOT11_SC_ASSOC_FAIL;
				goto done;
			}
		}
		WL_WSEC(("wl%d: %s: %s WPA_auth 0x%x\n", wlc->pub->unit, __FUNCTION__, sa,
		         scb->WPA_auth));
	}

	/* check the capabilities.
	 * In case OW_ENABLED, allow privacy bit to be set even if !WSEC_ENABLED if
	 * there is no wpa or rsn IE in request.
	 * This covers Microsoft doing WPS association with sec bit set even when we are
	 * in open mode..
	 */
	/* by far scb->WPA_auth should be setup if the STA has sent us appropriate request */
	if ((capability & DOT11_CAP_PRIVACY) && !WSEC_ENABLED(bsscfg->wsec) &&
	    (!WSEC_SES_OW_ENABLED(bsscfg->wsec) || akm_ie_included)) {
		WL_ERROR(("wl%d: %s is requesting privacy but encryption is not enabled on the"
		        " AP. SES OW %d WPS IE %d\n", wlc->pub->unit, sa,
		        WSEC_SES_OW_ENABLED(bsscfg->wsec), akm_ie_included));
		status = DOT11_SC_ASSOC_FAIL;
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_PRUNE, &hdr->sa,
			0, WLC_E_PRUNE_ENCR_MISMATCH, 0, 0, 0);
		goto done;
	}

	/* When WEP is enabled along with the WPA we'll deny STA request,
	 * if STA attempts with WPA IE and shared key 802.11 authentication,
	 * by deauthenticating the STA...
	 */
	if ((bsscfg->WPA_auth != WPA_AUTH_DISABLED) && WSEC_ENABLED(bsscfg->wsec) &&
	    (WSEC_WEP_ENABLED(bsscfg->wsec))) {
		/*
		 * WPA with 802.11 open authentication is required, or 802.11 shared
		 * key authentication without WPA authentication attempt (no WPA IE).
		 */
		/* attempt WPA auth with 802.11 shared key authentication */
		if (scb->auth_alg == DOT11_SHARED_KEY && scb->WPA_auth != WPA_AUTH_DISABLED) {
			WL_ERROR(("wl%d: WPA auth attempt with 802.11 shared key auth from %s, "
			          "deauthenticating...\n", wlc->pub->unit, sa));
			(void)wlc_senddeauth(wlc, bsscfg, scb, &scb->ea, &bsscfg->BSSID,
			                     &bsscfg->cur_etheraddr, DOT11_RC_AUTH_INVAL);
			status = DOT11_RC_AUTH_INVAL;
			goto done;
		}
	}


	/* If not APSTA, deny association to stations unable to support 802.11b short preamble
	 * if short network.  In APSTA, we don't enforce short preamble on the AP side since it
	 * might have been set by the STA side.  We need an extra flag for enforcing short
	 * preamble independently.
	 */
	if (!APSTA_ENAB(wlc->pub) && BAND_2G(wlc->band->bandtype) &&
	    bsscfg->PLCPHdr_override == WLC_PLCP_SHORT && !(capability & DOT11_CAP_SHORT)) {
		WL_ERROR(("wl%d: %s does not support short preambles\n", wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_SHORT_REQUIRED;
		goto done;
	}
	/* deny association to stations unable to support 802.11g short slot timing
	 * if shortslot exclusive network
	 */
	if ((BAND_2G(wlc->band->bandtype)) &&
	    ap->shortslot_restrict && !(capability & DOT11_CAP_SHORTSLOT)) {
		WL_ERROR(("wl%d: %s does not support ShortSlot\n", wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_SHORTSLOT_REQUIRED;
		goto done;
	}
	/* check the max association limit */
	if (wlc_assocscb_getcnt(wlc) >= appvt->maxassoc) {
		WL_ERROR(("wl%d: %s denied association due to max association limit\n",
		          wlc->pub->unit, sa));
		status = DOT11_SC_ASSOC_BUSY_FAIL;
		goto done;
	}

#if defined(MBSS) || defined(WLP2P)
	if (wlc_bss_assocscb_getcnt(wlc, bsscfg) >= bsscfg->maxassoc) {
		WL_ERROR(("wl%d.%d %s denied association due to max BSS association "
		          "limit\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), sa));
		status = DOT11_SC_ASSOC_BUSY_FAIL;
		goto done;
	}
#endif /* MBSS || WLP2P */

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg)) {
		if (wlc_p2p_process_assocreq(wlc->p2p, scb, tlvs, tlvs_len) != BCME_OK) {
			status = DOT11_SC_ASSOC_FAIL;
			goto done;
		}
	}
#endif
	param->status = status;

	bcopy(&req_rates, &param->req_rates, sizeof(wlc_rateset_t));

	if (!WLEXTSTA_ENAB(wlc->pub)) {
		decision.assoc_approved = TRUE;
		decision.reject_reason = 0;
		bcopy(&scb->ea, &decision.da, ETHER_ADDR_LEN);
		wlc_ap_process_assocreq_next(wlc, bsscfg, &decision, scb, param);
	}


#ifdef PSTA
	/* Send WLC_E_PSTA_PRIMARY_INTF_IND event to indicate psta primary mac addr */
	if (scb && scb->psta_prim) {
		wl_psta_primary_intf_event_t psta_prim_evet;

		WL_ASSOC(("wl%d.%d scb:"MACF", psta_prim:"MACF"\n",
			wlc->pub->unit,	WLC_BSSCFG_IDX(bsscfg),
			ETHER_TO_MACF(scb->ea), ETHER_TO_MACF(scb->psta_prim->ea)));

		memcpy(&psta_prim_evet.prim_ea, &scb->psta_prim->ea, sizeof(struct ether_addr));
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_PSTA_PRIMARY_INTF_IND,
			&scb->ea, WLC_E_STATUS_SUCCESS, 0, 0,
			&psta_prim_evet, sizeof(wl_psta_primary_intf_event_t));
	}
#endif /* PSTA */

	return;

	/* ===== Assoc Request Validations End ===== */


done:
	param->status = status;
	wlc_ap_process_assocreq_done(ap, bsscfg, &decision, hdr, body, body_len, scb, param);
	goto body_free;

exit:
	wlc_ap_process_assocreq_exit(ap, bsscfg, &decision, hdr, body, body_len, NULL, 0,
		scb, param);
	goto body_free;

smf_stats:
	wl_smf_stats(ap, bsscfg, hdr, param);
	/* fall through */

body_free:
	return;
}
	/*
	 * here if association request parsing is successful...
	 */

void wlc_ap_process_assocreq_decision(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, assoc_decision_t *dc)
{
}

static void wlc_ap_process_assocreq_next(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc, struct scb *scb, wlc_assoc_req_t * param)
{
	wlc_ap_info_t *ap;
	struct dot11_management_header *hdr;
	uint8 *body;
	uint body_len;
	struct dot11_assoc_req *req;

	uint16 capability;
#if defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLMSG_WSEC)
	char dabuf[ETHER_ADDR_STR_LEN], *da = bcm_ether_ntoa(&dc->da, dabuf);
#endif
#if  defined(BCMDBG_ERR) || defined(BCMDBG) || defined(WLMSG_ASSOC)
	char sabuf[ETHER_ADDR_STR_LEN], *sa;
#endif

	WL_TRACE(("wl%d: wlc_process_assocreq_next\n", wlc->pub->unit));

	ASSERT(bsscfg != NULL);
	ASSERT(bsscfg->up);
	ASSERT(BSSCFG_AP(bsscfg));

	ap = wlc->ap;


#if  defined(BCMDBG_ERR) || defined(BCMDBG) || defined(WLMSG_ASSOC)
	sa = bcm_ether_ntoa(&param->hdr->sa, sabuf);
#endif

	if (scb == NULL) {
		WL_ERROR(("wl%d.%d %s could not find scb\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), da));
		return;
	}
	hdr = param->hdr;
	body = param->body;
	body_len = param->body_len;
	req = (struct dot11_assoc_req *) body;

	if (!dc->assoc_approved) {
		param->status = dc->reject_reason;
		WL_ERROR(("wl%d.%d %s denied association due to Win7\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), da));
		goto done;
	}

	capability = ltoh16(req->capability);

	/* WEP encryption */
	if (scb->WPA_auth == WPA_AUTH_DISABLED &&
	    WSEC_WEP_ENABLED(bsscfg->wsec))
		scb->wsec = WEP_ENABLED;

	wlc_ap_tpc_assoc_reset(wlc->tpc, scb);

	/* When a HT or VHT STA using TKIP or WEP only unicast cipher suite tries
	 * to associate, exclude HT and VHT IEs from assoc response to force
	 * the STA to operate in legacy mode.
	 */
	if (WSEC_ENABLED(bsscfg->wsec)) {
		bool keep_ht = TRUE;

		ASSERT(param->req_rates.count > 0);
		if (scb->wsec == TKIP_ENABLED &&
		    (wlc_ht_get_wsec_restriction(wlc->hti) & WLC_HT_TKIP_RESTRICT)) {
			keep_ht = FALSE;
		} else if (scb->wsec == WEP_ENABLED &&
		           (wlc_ht_get_wsec_restriction(wlc->hti) & WLC_HT_WEP_RESTRICT)) {
			keep_ht = FALSE;
		}

		if (!keep_ht) {
			if (N_ENAB(wlc->pub) && SCB_HT_CAP(scb))
				wlc_ht_update_scbstate(wlc->hti, scb, NULL, NULL, NULL);
#ifdef WL11AC
			if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype) && SCB_VHT_CAP(scb))
				wlc_vht_update_scb_state(wlc->vhti,
					wlc->band->bandtype, scb, NULL, NULL, NULL, 0);
#endif /* WL11AC */
		}
	}

	wlc_prot_g_cond_upd(wlc->prot_g, scb);
	wlc_prot_n_cond_upd(wlc->prot_n, scb);

#ifdef RADIO_PWRSAVE
	if (RADIO_PWRSAVE_ENAB(wlc->ap) && wlc_radio_pwrsave_in_power_save(wlc->ap)) {
		wlc_radio_pwrsave_exit_mode(wlc->ap);
		WL_INFORM(("We have an assoc request, going right out of the power save mode!\n"));
	}
#endif

	if (wlc->band->gmode)
		wlc_prot_g_mode_upd(wlc->prot_g, bsscfg);

	if (N_ENAB(wlc->pub))
		wlc_prot_n_mode_upd(wlc->prot_n, bsscfg);

	WL_ASSOC(("AP: Checking if WEP key needs to be inserted\n"));
	/* If Multi-SSID is enabled, and Legacy WEP is in use for this bsscfg, a "pairwise" key must
	   be created by copying the default key from the bsscfg.
	*/
	if (bsscfg->WPA_auth == WPA_AUTH_DISABLED)
		WL_ASSOC(("WPA disabled \n"));

	/* Since STA is declaring privacy w/o WPA IEs => WEP */
	if ((scb->wsec == 0) && (capability & DOT11_CAP_PRIVACY) &&
	    WSEC_WEP_ENABLED(bsscfg->wsec))
		scb->wsec = WEP_ENABLED;

	if (WSEC_WEP_ENABLED(bsscfg->wsec))
		WL_ASSOC(("WEP enabled \n"));
	if (MBSS_ENAB(wlc->pub))
		WL_ASSOC(("MBSS on \n"));
	if ((MBSS_ENAB(wlc->pub) || PSTA_ENAB(wlc->pub) || bsscfg != wlc->cfg) &&
	    bsscfg->WPA_auth == WPA_AUTH_DISABLED && WSEC_WEP_ENABLED(bsscfg->wsec)) {
		wlc_key_t *key;
		wlc_key_info_t key_info;
		wlc_key_algo_t algo;
		wlc_key_t *scb_key;
		uint8 data[WEP128_KEY_SIZE];
		size_t data_len;
		int err;

		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE, &key_info);
		algo = key_info.algo;
		if (algo == CRYPTO_ALGO_OFF)
			goto defkey_done;

		WL_ASSOC(("Def key installed \n"));
		if  (algo != CRYPTO_ALGO_WEP1 && algo != CRYPTO_ALGO_WEP128)
			goto defkey_done;

		WL_ASSOC(("wl%d: %s Inserting key \n", WLCWLUNIT(wlc), sa));
		err = wlc_key_get_data(key, data, sizeof(data), &data_len);
		if (err != BCME_OK) {
			WL_ASSOC(("wl%d: %s error %d getting default key data, key idx %d\n",
				WLCWLUNIT(wlc), sa, err, key_info.key_idx));
			goto defkey_done;
		}

		scb_key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
			WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
		ASSERT(scb_key != NULL);
		err = wlc_key_set_data(scb_key, algo, data, data_len);
		if (err != BCME_OK) {
			WL_ERROR(("wl%d.%d: Error %d inserting key for sa %s, key idx %d\n",
				WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), err, sa, key_info.key_idx));
		}
	}

defkey_done:

	/* Get the AID, assign a new one if needed */
	if (!scb->aid)
		scb->aid = wlc_bsscfg_newaid(scb->bsscfg);

	/* Based on wsec for STA, update AMPDU feature
	 * By spec, 11n device can send AMPDU only with Open or CCMP crypto
	 */
	if (N_ENAB(wlc->pub)) {
		/* scb->wsec is the specific unicast algo being used.
		 * It should be a subset of the whole bsscfg wsec
		 */
		ASSERT((bsscfg->wsec & scb->wsec) == scb->wsec);
		if (SCB_AMPDU(scb)) {
			if ((scb->wsec == WEP_ENABLED) ||
			    (scb->wsec == TKIP_ENABLED) ||
			    SCB_MFP(scb)) {
				wlc_scb_ampdu_disable(wlc, scb);
			}
			else {
				wlc_scb_ampdu_enable(wlc, scb);
			}
		}
	}

#ifdef TRAFFIC_MGMT
	if (!scb->wsec) {
		wlc_scb_trf_mgmt(wlc, bsscfg, scb);
	}
#endif
done:
	wlc_ap_process_assocreq_done(ap, bsscfg, dc, hdr, body, body_len, scb, param);
	return;
}

	/* ===== Prepare Assoc Response ===== */
static void wlc_ap_process_assocreq_done(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc, struct dot11_management_header *hdr,
	uint8 *body, uint body_len, struct scb *scb, wlc_assoc_req_t * param)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif
	struct dot11_assoc_req *req = (struct dot11_assoc_req *) body;

	wlc_rateset_t sup_rates, ext_rates;
#if 0
	struct ether_addr *reassoc_ap = NULL;
#endif
	wlc_bss_info_t *current_bss;
	struct dot11_assoc_resp *resp;
	uint8 *rsp = NULL;
	uint rsp_len = 0;
	uint8 rates;
	bool reassoc;
	uint16 listen;
	uint8 mcsallow = 0;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	uint16 type;
	void *pkt;
	wlc_rateset_t *req_rates = &param->req_rates;

	listen = ltoh16(req->listen);
	current_bss = bsscfg->current_bss;
	reassoc = ((ltoh16(hdr->fc) & FC_KIND_MASK) == FC_REASSOC_REQ);
	type = reassoc ? FC_REASSOC_RESP : FC_ASSOC_RESP;

#if 0
	if (reassoc) {
		struct dot11_reassoc_req *reassoc_req = (struct dot11_reassoc_req *) body;
		reassoc_ap = &reassoc_req->ap;
	}
#endif

	/* create the supported rates and extended supported rates elts */
	bzero(&sup_rates, sizeof(wlc_rateset_t));
	bzero(&ext_rates, sizeof(wlc_rateset_t));
	/* check for a supported rates override */
	if (wlc->sup_rates_override.count > 0)
		bcopy(&wlc->sup_rates_override, &sup_rates, sizeof(wlc_rateset_t));
	wlc_rateset_elts(wlc, bsscfg, &current_bss->rateset, &sup_rates, &ext_rates);

	/* filter rateset for the BSS supported rates */
	wlc_rate_hwrs_filter_sort_validate(req_rates, &current_bss->rateset, FALSE,
		wlc->stf->txstreams);

	rsp_len = DOT11_ASSOC_RESP_FIXED_LEN;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.assocresp.mcs = req_rates->mcs;
	ftcbparm.assocresp.scb = scb;
	ftcbparm.assocresp.status = param->status;
	ftcbparm.assocresp.sup = &sup_rates;
	ftcbparm.assocresp.ext = &ext_rates;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;
	cbparm.ht = SCB_HT_CAP(scb);
#ifdef WL11AC
	cbparm.vht = SCB_VHT_CAP(scb);
#endif

	/* calculate IEs' length */
	rsp_len += wlc_iem_calc_len(wlc->iemi, bsscfg, type, NULL, &cbparm);

	/* alloc a packet */
	if ((pkt = wlc_frame_get_mgmt(wlc, type, &hdr->sa, &bsscfg->cur_etheraddr,
	                              &bsscfg->BSSID, rsp_len, &rsp)) == NULL) {
		param->status = DOT11_SC_ASSOC_BUSY_FAIL;
		WL_ASSOC(("wl%d.%d %s %sassociattion failed rsp_len %d\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), sa, reassoc ? "re" : "", rsp_len));
		rsp_len = 0;
		if (WIN7_AND_UP_OS(wlc->pub))
			goto exit;
		else
			goto smf_stats;
	}
	ASSERT(rsp && ISALIGNED(rsp, sizeof(uint16)));
	resp = (struct dot11_assoc_resp *) rsp;

	/* fill out the association response body */
	resp->capability = DOT11_CAP_ESS;
	if (BAND_2G(wlc->band->bandtype) &&
	    bsscfg->PLCPHdr_override == WLC_PLCP_SHORT)
		resp->capability |= DOT11_CAP_SHORT;
	if (WSEC_ENABLED(bsscfg->wsec) && bsscfg->wsec_restrict)
		resp->capability |= DOT11_CAP_PRIVACY;
	if (wlc->shortslot && wlc->band->gmode)
		resp->capability |= DOT11_CAP_SHORTSLOT;

#ifdef WL11K_AP
	if (WL11K_ENAB(wlc->pub) && wlc_rrm_enabled(wlc->rrm_info, bsscfg))
		resp->capability |= DOT11_CAP_RRM;
#endif /* WL11K_AP */

	resp->capability = htol16(resp->capability);
	resp->status = htol16(param->status);
	resp->aid = htol16(param->aid);

	rsp += DOT11_ASSOC_RESP_FIXED_LEN;
	rsp_len -= DOT11_ASSOC_RESP_FIXED_LEN;

	/* write IEs in the frame */
	if (wlc_iem_build_frame(wlc->iemi, bsscfg, type, NULL, &cbparm,
	                        rsp, rsp_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* send the association response */
	wlc_queue_80211_frag(wlc, pkt, bsscfg->wlcif->qi, scb, bsscfg,
		param->short_preamble, NULL, WLC_LOWEST_SCB_RSPEC(scb));

	/* ===== Post Processing ===== */


	rsp -= DOT11_ASSOC_RESP_FIXED_LEN;
	rsp_len += DOT11_ASSOC_RESP_FIXED_LEN;

#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub) && SCB_MFP(scb) &&
	    SCB_AUTHENTICATED(scb) && SCB_ASSOCIATED(scb)) {
		wlc_mfp_start_sa_query(wlc->mfp, bsscfg, scb);
		goto smf_stats;
	}
#endif

	/* if error, we're done */
	if (param->status != DOT11_SC_SUCCESS) {
		if (WIN7_AND_UP_OS(wlc->pub))
			goto exit;
		else
			goto smf_stats;
	}


	/* update scb state */

	WL_ASSOC(("wl%d.%d %s %sassociated\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), sa,
		reassoc ? "re" : ""));

	scb->aid = param->aid;

#ifdef PROP_TXSTATUS
	if (wlc_scb_wlfc_entry_add(wlc, scb) == BCME_OK) {
		WLFC_DBGMESG(("AP: MAC-ADD for [%02x:%02x:%02x:%02x:%02x:%02x], "
			"handle: [%d], if:%d, t_idx:%d\n",
			scb->ea.octet[0],
			scb->ea.octet[1],
			scb->ea.octet[2],
			scb->ea.octet[3],
			scb->ea.octet[4],
			scb->ea.octet[5],
			scb->mac_address_handle,
			((scb->bsscfg == NULL) ? 0 : scb->bsscfg->wlcif->index),
			WLFC_MAC_DESC_GET_LOOKUP_INDEX(scb->mac_address_handle)));
	}
#endif /* PROP_TXSTATUS */

	/*
	 * scb->listen is used by the AP for timing out PS pkts,
	 * ensure pkts are held for at least one dtim period
	 */
	scb->listen = MAX(current_bss->dtim_period, listen);
	wlc_scb_setstatebit(scb, ASSOCIATED);

	/* Start beaconing if this is first STA */
	if (DYNBCN_ENAB(bsscfg) && wlc_bss_assocscb_getcnt(wlc, bsscfg) == 1) {
		wlc_bsscfg_bcn_enable(wlc, bsscfg);
	}

	scb->assoctime = wlc->pub->now;

#if !(defined(NDIS) && (NDISVER >= 0x0620))
#if 0
	/* notify other APs on the DS that this station has roamed */
	if (reassoc && bcmp((char*)&bsscfg->BSSID, reassoc_ap->octet, ETHER_ADDR_LEN))
		wlc_reassoc_notify(ap, &hdr->sa, bsscfg->wlcif);
#endif

	/* 802.11f assoc. announce pkt */
	wlc_assoc_notify(ap, &hdr->sa, bsscfg->wlcif);
#endif /* !(NDIS && (NDISVER >= 0x0620)) */

	/* copy sanitized set to scb */
#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		rates = WLC_RATES_OFDM;
	else
#endif
	rates = WLC_RATES_CCK_OFDM;
	if (BSS_N_ENAB(wlc, bsscfg) && SCB_HT_CAP(scb))
		mcsallow |= WLC_MCS_ALLOW;
	if (BSS_VHT_ENAB(wlc, bsscfg) && SCB_VHT_CAP(scb))
		mcsallow |= WLC_MCS_ALLOW_VHT;
	wlc_rateset_filter(req_rates, &scb->rateset, FALSE, rates, RATE_MASK, mcsallow);

	/* re-initialize rate info
	 * Note that this wipes out any previous rate stats on the STA. Since this
	 * being called at Association time, it does not seem like a big loss.
	 */
	wlc_scb_ratesel_init(wlc, scb);


#ifdef DWDS
	/* create dwds i/f for this scb:
	 *  a) if security is disabled;
	 *  b) if security is enabled and security type is WEP;
	 * For all other security types will create the interface
	 * once scb is authorized.
	 */
	if (DWDS_ENAB(bsscfg) && SCB_DWDS_CAP(scb) &&
		(!WSEC_ENABLED(bsscfg->wsec) || WSEC_WEP_ENABLED(bsscfg->wsec)))
		wlc_wds_create(wlc, scb, WDS_DYNAMIC);
#endif

#if defined(BCMAUTH_PSK)
	/* kick off 4 way handshaking */
	if (BCMAUTH_PSK_ENAB(wlc->pub) && (bsscfg->authenticator != NULL))
		wlc_auth_join_complete(bsscfg->authenticator, &scb->ea, TRUE);
#endif /* BCMAUTH_PSK */

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		wlc_p2p_enab_upd(wlc->p2p, bsscfg);
#endif

	/* Enable BTCX PS protection */
	wlc_enable_btc_ps_protection(wlc, bsscfg, TRUE);


exit:
	wlc_ap_process_assocreq_exit(ap, bsscfg, dc, hdr, body, body_len, rsp, rsp_len, scb, param);
	return;

smf_stats:
	wl_smf_stats(ap, bsscfg, hdr, param);
	return;

}

static void wlc_ap_process_assocreq_exit(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc, struct dot11_management_header *hdr, uint8 *body, uint body_len,
	void *rsp, uint rsp_len, struct scb *scb, wlc_assoc_req_t * param)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	bool reassoc;

	reassoc = ((ltoh16(hdr->fc) & FC_KIND_MASK) == FC_REASSOC_REQ);
	/* send WLC_E_REASSOC_IND/WLC_E_ASSOC_IND to interested App and/or non-WIN7 OS */
	wlc_bss_mac_event(wlc, bsscfg, reassoc ? WLC_E_REASSOC_IND : WLC_E_ASSOC_IND,
		&hdr->sa, WLC_E_STATUS_SUCCESS, param->status, 0, param->e_data, param->e_datalen);
	/* Suspend STA sniffing if it is associated to AP  */
	if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &hdr->sa))
		wlc_stamon_sta_sniff_enab(wlc->stamon_info, &hdr->sa, FALSE);
#ifdef WLWNM_AP
	if (WLWNM_ENAB(wlc->pub) && WNM_MAXIDLE_ENABLED(wlc_wnm_get_cap(wlc, bsscfg)))
			wlc_wnm_rx_tstamp_update(wlc, scb);
#endif /* WLWNM_AP */
#ifdef PLC_WET
	/* Add node so that we can route the frames appropriately */
	if (PLC_ENAB(wlc->pub))
		wlc_plc_node_add(wlc, NODE_TYPE_WIFI_ONLY, scb);
#endif /* PLC_WET */

	wl_smf_stats(ap, bsscfg, hdr, param);
}

#ifdef RXCHAIN_PWRSAVE
/*
 * Reset the rxchain power save related counters and modes
 */
void
wlc_reset_rxchain_pwrsave_mode(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	WL_INFORM(("Resetting the rxchain power save counters\n"));
	/* Come out of the power save mode if we are in it */
	if (appvt->rxchain_pwrsave.pwrsave.in_power_save) {
		wlc_stf_rxchain_set(wlc, appvt->rxchain_pwrsave.rxchain, TRUE);
#ifdef WL11N
		/* need to restore rx_stbc HT capability after exit rxchain_pwrsave mode */
		wlc_stf_exit_rxchain_pwrsave(wlc, appvt->rxchain_pwrsave.ht_cap_rx_stbc);
#endif /* WL11N */
	}
	wlc_reset_pwrsave_mode(ap, PWRSAVE_RXCHAIN);
}

void
wlc_disable_rxchain_pwrsave(wlc_ap_info_t *ap)
{
	wlc_disable_pwrsave(ap, PWRSAVE_RXCHAIN);

	return;
}

#ifdef WL11N
/*
 * get rx_stbc HT capability, if in rxchain_pwrsave mode, return saved rx_stbc value,
 * because rx_stbc capability may be changed when enter rxchain_pwrsave mode
 */
uint8
wlc_rxchain_pwrsave_stbc_rx_get(wlc_info_t *wlc)
{
	uint8 ht_cap_rx_stbc = wlc_ht_stbc_rx_get(wlc->hti);
	wlc_ap_info_t *ap = wlc->ap;
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	if ((appvt != NULL) && appvt->rxchain_pwrsave.pwrsave.in_power_save) {
		ht_cap_rx_stbc = appvt->rxchain_pwrsave.ht_cap_rx_stbc;
	}
	return ht_cap_rx_stbc;
}
#endif /* WL11N */
#endif /* RXCHAIN_PWRSAVE */

#ifdef RADIO_PWRSAVE
/*
 * Reset the radio power save related counters and modes
 */
static void
wlc_reset_radio_pwrsave_mode(wlc_ap_info_t *ap)
{
	uint8 dtim_period;
	uint16 beacon_period;
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bsscfg_t *bsscfg = wlc->cfg;

	if (bsscfg->associated) {
		dtim_period = bsscfg->current_bss->dtim_period;
		beacon_period = bsscfg->current_bss->beacon_period;
	} else {
		dtim_period = wlc->default_bss->dtim_period;
		beacon_period = wlc->default_bss->beacon_period;
	}

	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
	wlc_reset_pwrsave_mode(ap, PWRSAVE_RADIO);
	appvt->radio_pwrsave.pwrsave_state = 0;
	appvt->radio_pwrsave.radio_disabled = FALSE;
	appvt->radio_pwrsave.cncl_bcn = FALSE;
	switch (appvt->radio_pwrsave.level) {
	case RADIO_PWRSAVE_LOW:
		appvt->radio_pwrsave.on_time = 2*beacon_period*dtim_period/3;
		appvt->radio_pwrsave.off_time = beacon_period*dtim_period/3;
		break;
	case RADIO_PWRSAVE_MEDIUM:
		appvt->radio_pwrsave.on_time = beacon_period*dtim_period/2;
		appvt->radio_pwrsave.off_time = beacon_period*dtim_period/2;
		break;
	case RADIO_PWRSAVE_HIGH:
		appvt->radio_pwrsave.on_time = beacon_period*dtim_period/3;
		appvt->radio_pwrsave.off_time = 2*beacon_period*dtim_period/3;
		break;
	default:
		ASSERT(0);
		break;
	}
}
#endif /* RADIO_PWRSAVE */

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
/*
 * Reset power save related counters and modes
 */
void
wlc_reset_pwrsave_mode(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pwrsave_t *pwrsave = NULL;
	int enable = 0;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			enable = ap->rxchain_pwrsave_enable;
			pwrsave = &appvt->rxchain_pwrsave.pwrsave;
			if (pwrsave) {
				if (!enable)
					pwrsave->power_save_check &= ~PWRSAVE_ENB;
				else
					pwrsave->power_save_check |= PWRSAVE_ENB;
			}
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			enable = ap->radio_pwrsave_enable;
			pwrsave = &appvt->radio_pwrsave.pwrsave;
			appvt->radio_pwrsave.pwrsave_state = 0;
			if (pwrsave)
				pwrsave->power_save_check = enable;
			break;
#endif
		default:
			break;
	}

	WL_INFORM(("Resetting the rxchain power save counters\n"));
	if (pwrsave) {
		pwrsave->quiet_time_counter = 0;
		pwrsave->in_power_save = FALSE;
	}
}

#ifdef RXCHAIN_PWRSAVE
static void
wlc_disable_pwrsave(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pwrsave_t *pwrsave = NULL;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			pwrsave = &appvt->rxchain_pwrsave.pwrsave;
			if (pwrsave)
				pwrsave->power_save_check &= ~PWRSAVE_ENB;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			pwrsave = &appvt->radio_pwrsave.pwrsave;
			appvt->radio_pwrsave.pwrsave_state = 0;
			if (pwrsave)
				pwrsave->power_save_check = FALSE;
			break;
#endif
		default:
			break;
	}

	WL_INFORM(("Disabling power save mode\n"));
	if (pwrsave)
		pwrsave->in_power_save = FALSE;
}

#ifdef WDS
/* Detect if WDS or DWDS is configured */
static bool
wlc_rxchain_wds_detection(wlc_info_t *wlc)
{
	struct scb_iter scbiter;
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	int32 idx;

	FOREACH_BSS(wlc, idx, bsscfg) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if ((SCB_WDS(scb) != NULL) || (SCB_DWDS(scb)))
				return TRUE;
		}
	}
	return FALSE;
}
#endif /* WDS */
#endif /* RXCHAIN_PWRSAVE */

#endif /* RXCHAIN_PWRSAVE || RADIO_PWRSAVE */

bool
wlc_apsta_on_radar_channel(wlc_ap_info_t *ap)
{
	ASSERT(ap);
	return ((wlc_ap_info_pvt_t *)ap)->ap_sta_onradar;
}

static bool
wlc_ap_on_radarchan(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	int ap_idx;
	wlc_bsscfg_t *ap_cfg;
	bool ap_onradar = FALSE;

	if (AP_ACTIVE(wlc)) {
		FOREACH_UP_AP(wlc, ap_idx, ap_cfg) {
			wlc_bss_info_t *current_bss = ap_cfg->current_bss;

			if (wlc_radar_chanspec(wlc->cmi, current_bss->chanspec) == TRUE) {
				ap_onradar = TRUE;
				break;
			}
		}
	}

	return ap_onradar;
}

#ifdef STA
/* Check if any local AP/STA chanspec overlap with STA/local AP
 * radar chanspec.
 * NOTE: To take future possiblility of MCHAN in APSTA, have to use
 * FOREACH_UP_AP()
 *	FOREACH_AS_STA().
 */
void
wlc_ap_sta_onradar_upd(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) wlc->ap;
	bool dfs_slave_present = FALSE;

	/* Only when on radar channel we look for overlapping AP + STA.
	 * The moment we switch to non-radar we turn of dfs_slave_present.
	 */
	if (wlc_radar_chanspec(wlc->cmi, current_bss->chanspec) == TRUE)
	{
		int ap_idx;
		wlc_bsscfg_t *ap_cfg;

		FOREACH_UP_AP(wlc, ap_idx, ap_cfg) {
			int sta_idx;
			wlc_bsscfg_t *sta_cfg;
			wlc_bss_info_t *ap_bss = ap_cfg->current_bss;
			uint8 ap_ctlchan = wf_chspec_ctlchan(ap_bss->chanspec);

			FOREACH_AS_STA(wlc, sta_idx, sta_cfg) {
				wlc_bss_info_t *sta_bss = sta_cfg->current_bss;
				uint8 sta_ctlchan = wf_chspec_ctlchan(sta_bss->chanspec);

				if (ap_ctlchan == sta_ctlchan) {
					dfs_slave_present = TRUE;
					break;
				}
			}

			if (dfs_slave_present) break;
		}
	}

	appvt->ap_sta_onradar = dfs_slave_present;
}
#endif /* STA */

void
wlc_restart_ap(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	int i;
	wlc_bsscfg_t *bsscfg;
#ifdef RADAR
	uint bss_radar_flags = 0;
	bool radar_ap = FALSE;
#endif /* RADAR */

	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

#ifdef RADAR
	if (WL11H_AP_ENAB(wlc)) {
		FOREACH_AP(wlc, i, bsscfg) {
			bss_radar_flags |= (bsscfg)->flags & (WLC_BSSCFG_SRADAR_ENAB |
				WLC_BSSCFG_AP_NORADAR_CHAN |
				WLC_BSSCFG_USR_RADAR_CHAN_SELECT);

			/* Make sure these flags are are never combined together
			 * either within single ap bss or across all ap-bss.
			 */
			ASSERT(!(bss_radar_flags & (bss_radar_flags - 1)));
		}

		/* No random channel selection in any of bss_radar_flags modes */
		if (!bss_radar_flags &&
		    (wlc_channel_locale_flags_in_band(wlc->cmi, BAND_5G_INDEX) & WLC_DFS_TPC)) {
			wlc_dfs_sel_chspec(wlc->dfs, TRUE);
		}
	}
#endif /* RADAR */

	ap->pre_tbtt_us = (MBSS_ENAB(wlc->pub)) ? MBSS_PRE_TBTT_DEFAULT_us : PRE_TBTT_DEFAULT_us;

	/* Bring up any enabled AP configs which aren't up yet */
	FOREACH_AP(wlc, i, bsscfg) {
		if (bsscfg->enable) {
			uint wasup = bsscfg->up;
#ifdef RADAR
			wlc_bss_info_t *current_bss = bsscfg->current_bss;
#endif /* RADAR */

			WL_APSTA_UPDN(("wl%d: wlc_restart_ap -> wlc_bsscfg_up on bsscfg %d%s\n",
			               appvt->pub->unit, i, (bsscfg->up ? "(already up)" : "")));
			/* Clearing association state to let the beacon phyctl0
			 * and phyctl1 parameters be updated in shared memory.
			 * The phyctl0 and phyctl1 would be cleared from shared
			 * memory after the big hammer is executed.
			 * The wlc_bsscfg_up function below will update the
			 * associated state accordingly.
			 */
			bsscfg->associated = 0;
			if ((BCME_OK != wlc_bsscfg_up(wlc, bsscfg)) && wasup) {
				wlc_bsscfg_down(wlc, bsscfg);
			}
#ifdef RADAR
			if (bsscfg->up &&
				(wlc_radar_chanspec(wlc->cmi, current_bss->chanspec) == TRUE)) {
				radar_ap = TRUE;
			}
#endif /* RADAR */
		}
	}

#ifdef RADAR
	if (WL11H_AP_ENAB(wlc) && AP_ACTIVE(wlc)) {
		/* Check If radar_detect explicitly disabled
		 * OR
		 * non of the AP's on radar chanspec
		 */
		bool dfs_on = ((bss_radar_flags
				& (WLC_BSSCFG_SRADAR_ENAB | WLC_BSSCFG_AP_NORADAR_CHAN)) ||
				!radar_ap) ? OFF:ON;

		wlc_set_dfs_cacstate(wlc->dfs, dfs_on);
	}
#endif /* RADAR */

#ifdef RXCHAIN_PWRSAVE
	wlc_reset_rxchain_pwrsave_mode(ap);
#endif
#ifdef RADIO_PWRSAVE
	wlc_reset_radio_pwrsave_mode(ap);
#endif
}

void
wlc_bss_up(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bss_info_t *target_bss = bsscfg->target_bss;

	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

#ifdef WL11N
	/* Adjust target bss rateset according to target channel bandwidth */
	wlc_rateset_bw_mcs_filter(&target_bss->rateset,
		WL_BW_CAP_40MHZ(wlc->band->bw_cap)?CHSPEC_WLC_BW(target_bss->chanspec):0);
#endif /* WL11N */

	wlc_suspend_mac_and_wait(wlc);
	wlc_BSSinit(wlc, target_bss, bsscfg, WLC_BSS_START);
	wlc_enable_mac(wlc);

	/* update the AP association count */
	wlc_ap_up_upd(ap, bsscfg, TRUE);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		wlc_mcnx_bss_upd(wlc->mcnx, bsscfg, TRUE);
	}
#endif

	if (BSS_WME_ENAB(wlc, bsscfg))
		wlc_edcf_acp_apply(wlc, bsscfg, TRUE);

	wlc_led_event(wlc->ledh);

	if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band))
		wlc_full_phy_cal(wlc, bsscfg, PHY_PERICAL_UP_BSS);

	/* Indicate AP is now up */
	WL_APSTA_UPDN(("Reporting link up on config %d (AP enabled)\n",
		WLC_BSSCFG_IDX(bsscfg)));
	wlc_link(wlc, TRUE, &bsscfg->cur_etheraddr, bsscfg, 0);
}

static void
wlc_ap_up_upd(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, bool state)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	bsscfg->associated = state;

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		if (state == TRUE) {
			wlc_mchan_create_bss_chan_context(wlc, bsscfg,
			                                  bsscfg->current_bss->chanspec);
		}
		else {
			wlc_mchan_delete_bss_chan_context(wlc, bsscfg);
		}
	}
#endif /* WLMCHAN */

	wlc->aps_associated = (uint8)AP_BSS_UP_COUNT(wlc);

	wlc_ap_sta_onradar_upd(bsscfg);

	wlc_mac_bcn_promisc(wlc);

	if ((wlc->aps_associated == 0) || ((wlc->aps_associated == 1))) /* No need beyond 1 */
		wlc_bmac_enable_tbtt(wlc->hw, TBTT_AP_MASK, wlc->aps_associated ? TBTT_AP_MASK : 0);

	wlc->pub->associated = wlc->aps_associated > 0 || wlc->stas_associated > 0;

	wlc_phy_hold_upd(WLC_PI(wlc), PHY_HOLD_FOR_NOT_ASSOC,
		wlc->pub->associated ? FALSE : TRUE);

	/* Win7: Enable AP if it wasn't enabled */
	if (WIN7_AND_UP_OS(wlc->pub) && wlc->aps_associated && !AP_ENAB(wlc->pub))
		wlc->pub->_ap = 1;

	wlc->pub->associated = wlc->aps_associated > 0 || wlc->stas_associated > 0;

#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub))
		wlc_rsdb_update_active(wlc, NULL);
#endif /* WLRSDB */
}

/* known reassociation magic packets */

struct lu_reassoc_pkt {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 unknown_field[2];
	uint8 data[36];
};

static
const struct lu_reassoc_pkt lu_reassoc_template = {
	{ { 0x01, 0x60, 0x1d, 0x00, 0x01, 0x00 },
	{ 0 },
	HTON16(sizeof(struct lu_reassoc_pkt) - sizeof(struct ether_header)) },
	{ 0xaa, 0xaa, 0x03, { 0x00, 0x60, 0x1d }, HTON16(0x0001) },
	{ 0x00, 0x04 },
	"Lucent Technologies Station Announce"
};

struct csco_reassoc_pkt {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 unknown_field[4];
	struct ether_addr ether_dhost, ether_shost, a1, a2, a3;
	uint8 pad[4];
};
/* WES - I think the pad[4] at the end of the struct above should be
 * dropped, it appears to just be the ethernet padding to 64
 * bytes. This would fix the length calculation below, (no more -4).
 * It matches with the 0x22 field in the 'unknown_field' which appears
 * to be the length of the encapsulated packet starting after the snap
 * header.
 */

static
const struct csco_reassoc_pkt csco_reassoc_template = {
	{ { 0x01, 0x40, 0x96, 0xff, 0xff, 0x00 },
	{ 0 },
	HTON16(sizeof(struct csco_reassoc_pkt) - sizeof(struct ether_header) - 4) },
	{ 0xaa, 0xaa, 0x03, { 0x00, 0x40, 0x96 }, HTON16(0x0000) },
	{ 0x00, 0x22, 0x02, 0x02 },
	{ { 0x01, 0x40, 0x96, 0xff, 0xff, 0x00 } },
	{ { 0 } },
	{ { 0 } },
	{ { 0 } },
	{ { 0 } },
	{ 0x00, 0x00, 0x00, 0x00 }
};

bool
wlc_roam_check(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, struct ether_header *eh, uint len)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	struct lu_reassoc_pkt *lu = (struct lu_reassoc_pkt *) eh;
	struct csco_reassoc_pkt *csco = (struct csco_reassoc_pkt *) eh;
	struct ether_addr *sta = NULL;
	struct scb *scb;

	/* check for Lucent station announce packet */
	if (!bcmp(eh->ether_dhost, (const char*)lu_reassoc_template.eth.ether_dhost,
	          ETHER_ADDR_LEN) &&
	    len >= sizeof(struct lu_reassoc_pkt) &&
	    !bcmp((const char*)&lu->snap, (const char*)&lu_reassoc_template.snap,
	              DOT11_LLC_SNAP_HDR_LEN))
		sta = (struct ether_addr *) lu->eth.ether_shost;

	/* check for Cisco station announce packet */
	else if (!bcmp(eh->ether_dhost, (const char*)csco_reassoc_template.eth.ether_dhost,
	               ETHER_ADDR_LEN) &&
	         len >= sizeof(struct csco_reassoc_pkt) &&
	         !bcmp((const char*)&csco->snap, (const char*)&csco_reassoc_template.snap,
	               DOT11_LLC_SNAP_HDR_LEN))
		sta = &csco->a1;

	/* not a magic packet */
	else
		return (FALSE);

	/* disassociate station */
	if ((scb = wlc_scbfind(wlc, bsscfg, sta)) && SCB_ASSOCIATED(scb)) {
		WL_ERROR(("wl%d: %s roamed\n", wlc->pub->unit, bcm_ether_ntoa(sta, eabuf)));
		if (APSTA_ENAB(wlc->pub) && (scb->flags & SCB_MYAP)) {
			WL_APSTA(("wl%d: Ignoring roam report from my AP.\n", wlc->pub->unit));
			return (FALSE);
		}

		wlc_senddisassoc(wlc, bsscfg, scb, sta, &bsscfg->BSSID,
		                 &bsscfg->cur_etheraddr, DOT11_RC_NOT_AUTH);
		wlc_scb_resetstate(scb);
		wlc_scb_setstatebit(scb, AUTHENTICATED);

		wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, sta, WLC_E_STATUS_SUCCESS,
			DOT11_RC_DISASSOC_LEAVING, 0, 0, 0);
		/* Resume sniffing of this STA frames if the STA has been
	     * configured to be monitored before association.
	     */
		if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, sta))
			wlc_stamon_sta_sniff_enab(wlc->stamon_info, sta, TRUE);
	}

	return (TRUE);
}

#if !(defined(NDIS) && (NDISVER >= 0x0620))
static void
wlc_assoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	const uchar pkt80211f[] = {0x00, 0x01, 0xAF, 0x81, 0x01, 0x00};
	struct ether_header *eh;
	uint16 len = sizeof(pkt80211f);
	void *p;

	/* prepare 802.11f IAPP announce packet. This should look like a wl rx packet since it sent
	   along the same path. Some work should be done to evaluate the real need for
	   extra headroom (how much), but the alignement enforced by wlc->hwrxoff_pktget must be
	   preserved.
	*/
	if ((p = PKTGET(wlc->osh, sizeof(pkt80211f) + BCMEXTRAHDROOM + wlc->hwrxoff_pktget +
		ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED(PKTDATA(wlc->osh, p), sizeof(uint32)));
	PKTPULL(wlc->osh, p, BCMEXTRAHDROOM + wlc->hwrxoff_pktget);

	eh = (struct ether_header *) PKTDATA(wlc->osh, p);
	bcopy(&ether_bcast, eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy(sta, eh->ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(len);
	bcopy(pkt80211f, &eh[1], len);
	WL_PRPKT("802.11f assoc", PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));

		wl_sendup(wlc->wl, wlcif->wlif, p, 1);

	return;

err:
	WL_ERROR(("wl%d: %s: pktget error\n", wlc->pub->unit, __FUNCTION__));
	WLCNTINCR(wlc->pub->_cnt->rxnobuf);
	WLCNTINCR(wlcif->_cnt.rxnobuf);
	return;
}

#if 0
static void
wlc_reassoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	void *p;
	osl_t *osh;
	struct lu_reassoc_pkt *lu;
	struct csco_reassoc_pkt *csco;
	int len;
	wlc_bsscfg_t *cfg;

	osh = wlc->osh;

	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	/* prepare Lucent station announce packet */
	len = sizeof(struct lu_reassoc_pkt);
	if ((p = PKTGET(osh, len + BCMEXTRAHDROOM + wlc->hwrxoff_pktget +
		ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));
	PKTPULL(osh, p, BCMEXTRAHDROOM + wlc->hwrxoff_pktget);

	lu = (struct lu_reassoc_pkt*) PKTDATA(osh, p);
	bcopy((const char*)&lu_reassoc_template, (char*) lu, sizeof(struct lu_reassoc_pkt));
	bcopy((const char*)sta, (char*)&lu->eth.ether_shost, ETHER_ADDR_LEN);
	WL_PRPKT("lu", PKTDATA(osh, p), PKTLEN(osh, p));

		wl_sendup(wlc->wl, wlcif->wlif, p, 1);

	/* prepare Cisco station announce packets */
	len = sizeof(struct csco_reassoc_pkt);
	if ((p = PKTGET(osh, len + BCMEXTRAHDROOM + wlc->hwrxoff_pktget +
		ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));
	PKTPULL(osh, p, BCMEXTRAHDROOM + wlc->hwrxoff_pktget);

	csco = (struct csco_reassoc_pkt *) PKTDATA(osh, p);
	bcopy((const char*)&csco_reassoc_template, (char*)csco, len);
	bcopy((char*)sta, (char*)&csco->eth.ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)sta, (char*)&csco->ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)sta, (char*)&csco->a1, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a2, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a3, ETHER_ADDR_LEN);
	WL_PRPKT("csco1", PKTDATA(osh, p), PKTLEN(osh, p));

		wl_sendup(wlc->wl, wlcif->wlif, p, 1);

	len = sizeof(struct csco_reassoc_pkt);
	if ((p = PKTGET(osh, len + BCMEXTRAHDROOM + wlc->hwrxoff_pktget +
		ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));
	PKTPULL(osh, p, BCMEXTRAHDROOM + wlc->hwrxoff_pktget);

	csco = (struct csco_reassoc_pkt *) PKTDATA(osh, p);
	bcopy((const char*)&csco_reassoc_template, (char*)csco, len);
	bcopy((char*)&cfg->BSSID, (char*)&csco->eth.ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)sta, (char*)&csco->a1, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a2, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a3, ETHER_ADDR_LEN);
	WL_PRPKT("csco2", PKTDATA(osh, p), PKTLEN(osh, p));

		wl_sendup(wlc->wl, wlcif->wlif, p, 1);

	return;

err:
	WL_ERROR(("wl%d: %s: pktget error\n", wlc->pub->unit, __FUNCTION__));
	WLCNTINCR(wlc->pub->_cnt->rxnobuf);
	WLCNTINCR(wlcif->_cnt.rxnobuf);
	return;
}
#endif
#endif /* !(NDIS && (NDISVER >= 0x0620)) */

uint8
wlc_lowest_basicrate_get(wlc_bsscfg_t *cfg)
{
	uint8 i, rate = 0;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	for (i = 0; i < current_bss->rateset.count; i++) {
		if (current_bss->rateset.rates[i] & WLC_RATE_FLAG) {
			rate = current_bss->rateset.rates[i] & RATE_MASK;
			break;
		}
	}

	return rate;
}

static void
wlc_ap_probe_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;

	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

#ifdef WDS
	if (SCB_LEGACY_WDS(scb) && WLWDS_CAP(wlc))
		wlc_ap_wds_probe_complete(wlc, txs, scb);
	else
#endif
	wlc_ap_sta_probe_complete(wlc, txs, scb, pkt);
}

static int
wlc_ap_sendnulldata_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt, void *data)
{
	/* register packet callback */
	WLF2_PCB1_REG(pkt, WLF2_PCB1_STA_PRB);
	return BCME_OK;
}

static void
wlc_ap_sta_probe(wlc_ap_info_t *ap, struct scb *scb)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint8 rate_override;
#ifdef PSPRETEND
	bool  ps_pretend = SCB_PS_PRETEND(scb);
#else
	const bool ps_pretend = FALSE;
#endif

	/* If a probe is still pending, don't send another one */
	if (scb->flags & SCB_PENDING_PROBE)
		return;

	/* use the lowest basic rate */
	rate_override = wlc_lowest_basicrate_get(scb->bsscfg);

	ASSERT(VALID_RATE(wlc, rate_override));

	if (!wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea, rate_override,
		ps_pretend ? WLF_PSDONTQ : 0,
		ps_pretend ? PRIO_8021D_VO : PRIO_8021D_BE,
		wlc_ap_sendnulldata_cb, NULL))
	{
		WL_ERROR(("wl%d: wlc_ap_sta_probe: wlc_sendnulldata failed\n",
		          wlc->pub->unit));
		return;
	}

	scb->flags |= SCB_PENDING_PROBE;
}

static void
wlc_ap_sta_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb, void *pkt)
{
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	ASSERT(scb != NULL);

	scb->flags &= ~SCB_PENDING_PROBE;

#ifdef PSPRETEND
	if (SCB_PS_PRETEND_PROBING(scb)) {
		if ((txstatus & TX_STATUS_MASK) == TX_STATUS_ACK_RCV) {
			/* probe response OK - exit PS Pretend state */
			WL_PS(("wl%d.%d: received ACK to ps pretend probe "MACF" (count %d)\n",
			        wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			        ETHER_TO_MACF(scb->ea), scb->grace_attempts));

			/* Assert check that the fifo was cleared before exiting ps mode */
			if (SCB_PS_PRETEND_BLOCKED(scb)) {
				WL_ERROR(("wl%d.%d: %s: SCB_PS_PRETEND_BLOCKED, "
				          "expected to see PMQ PPS entry\n", wlc->pub->unit,
				          WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
			}
			if (!(wlc->block_datafifo & DATA_BLOCK_PS)) {
				wlc_apps_scb_ps_off(wlc, scb, FALSE);
			}
			else {
				WL_ERROR(("wl%d.%d: %s: annother PS flush in progress, "
				          "unable to resume\n", wlc->pub->unit,
				          WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
			}
		}
		else {
			++scb->grace_attempts;
			WL_PS(("wl%d.%d: no response to ps pretend probe "MACF" (count %d)\n",
			        wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			        ETHER_TO_MACF(scb->ea), scb->grace_attempts));
		}
		/* we re-probe using ps pretend probe timer if not stalled,
		* so return from here
		*/
		return;
	}
#endif /* PSPRETEND */
	/* ack indicates the sta should not be removed or we might have missed the ACK but if there
	 * was some activity after sending the probe then it indicates there is life out there in
	 * scb.
	 */
	if (((txstatus & TX_STATUS_MASK) != TX_STATUS_NO_ACK) ||
	    (wlc->pub->now - scb->used < wlc->ap->scb_activity_time) ||
	    wlc->ap->scb_activity_time == 0) {
		scb->grace_attempts = 0;
		/* update the primary PSTA also */
		if (scb->psta_prim)
			scb->psta_prim->used = wlc->pub->now;
		return;
	}

	/* If still more grace_attempts are left, then probe the STA again */
	if (++scb->grace_attempts < wlc->ap->scb_max_probe) {
		wlc->ap->reprobe_scb = TRUE;
		return;
	}

	WL_INFORM(("wl%d: wlc_ap_sta_probe_complete: no ACK from %s for Null Data\n",
		wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));

	if (SCB_AUTHENTICATED(scb)) {
		wlc_deauth_complete(wlc, SCB_BSSCFG(scb), WLC_E_STATUS_SUCCESS, &scb->ea,
			DOT11_RC_INACTIVITY, 0);
	}

	/* free the scb */
	wlc_scbfree(wlc, scb);
	WLPKTTAGSCBSET(pkt, NULL);
}

#ifdef BCMDBG
static int
wlc_dump_ap(wlc_ap_info_t *ap, struct bcmstrbuf *b)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	bcm_bprintf(b, "\n");

	bcm_bprintf(b, " shortslot_restrict %d scb_timeout %d\n",
		ap->shortslot_restrict, ap->scb_timeout);

	bcm_bprintf(b, "tbtt %d pre-tbtt-us %u. max latency %u. "
		"min threshold %u. block datafifo %d "
		"\n",
		WLCNTVAL(wlc->pub->_cnt->tbtt),
		ap->pre_tbtt_us, MBSS_PRE_TBTT_MAX_LATENCY_us,
		MBSS_PRE_TBTT_MIN_THRESH_us, wlc->block_datafifo);

	return 0;
}

#ifdef RXCHAIN_PWRSAVE
static int
wlc_dump_rxchain_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;

	wlc_dump_pwrsave(&appvt->rxchain_pwrsave.pwrsave, b);

	return 0;
}
#endif /* RXCHAIN_PWRSAVE */

#ifdef RADIO_PWRSAVE
static int
wlc_dump_radio_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;

	wlc_dump_pwrsave(&appvt->radio_pwrsave.pwrsave, b);

	return 0;
}
#endif /* RADIO_PWRSAVE */

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
static void
wlc_dump_pwrsave(wlc_pwrsave_t *pwrsave, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, " in_power_save %d\n",
		pwrsave->in_power_save);

	bcm_bprintf(b, " no: of times in power save mode %d\n",
		pwrsave->in_power_save_counter);

	bcm_bprintf(b, " power save time (in secs) %d\n",
		pwrsave->in_power_save_secs);

	return;
}
#endif /* RXCHAIN_PWRSAVE or RADIO_PWRSAVE */
#endif /* BCMDBG */

static void
wlc_ap_watchdog(void *arg)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) arg;
	wlc_ap_info_t *ap = &appvt->appub;
	wlc_info_t *wlc = appvt->wlc;
#ifdef RXCHAIN_PWRSAVE
	bool done = 0;
#endif
	int idx;
	wlc_bsscfg_t *cfg;

	/* part 1 */
	if (AP_ENAB(wlc->pub)) {
		struct scb *scb;
		struct scb_iter scbiter;

		/* before checking for stuck tx beacons make sure atleast one
		 * ap bss is up phy is not muted(for whatever reason) and beaconing.
		 */
		if (!phy_utils_ismuted((phy_info_t *)WLC_PI(wlc)) && !MBSS_ENAB4(wlc->pub) &&
		    (ap->txbcn_timeout > 0) && (AP_BSS_UP_COUNT(wlc) > 0)) {
			uint16 txbcn_snapshot;

			/* see if any beacons are transmitted since last
			 * watchdog timeout.
			 */
			txbcn_snapshot = wlc_read_shm(wlc, M_UCODE_MACSTAT +
			                              OFFSETOF(macstat_t, txbcnfrm));

			/* if no beacons are transmitted for txbcn timeout period
			 * then some thing has gone bad, do the big-hammer.
			 */
			if (txbcn_snapshot == appvt->txbcn_snapshot) {
				if (++appvt->txbcn_inactivity >= ap->txbcn_timeout) {
					WL_ERROR(("wl%d: bcn inactivity detected\n",
					          wlc->pub->unit));
					WL_ERROR(("wl%d: txbcnfrm %d prev txbcnfrm %d "
					          "txbcn inactivity %d timeout %d\n",
					          wlc->pub->unit, txbcn_snapshot,
					          appvt->txbcn_snapshot,
					          appvt->txbcn_inactivity,
					          ap->txbcn_timeout));
					appvt->txbcn_inactivity = 0;
					appvt->txbcn_snapshot = 0;
					wlc_fatal_error(wlc);
					return;
				}
			} else
				appvt->txbcn_inactivity = 0;

			/* save the txbcn counter */
			appvt->txbcn_snapshot = txbcn_snapshot;
		}

		/* deauth rate limiting - enable sending one deauth every second */
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
#ifdef RXCHAIN_PWRSAVE
			if ((ap->rxchain_pwrsave_enable == RXCHAIN_PWRSAVE_ENAB_BRCM_NONHT) &&
			    !done) {
				if (SCB_ASSOCIATED(scb) && !(scb->flags & SCB_BRCM) &&
				    (scb->flags & SCB_HTCAP)) {
					if (appvt->rxchain_pwrsave.pwrsave.in_power_save) {
						wlc_stf_rxchain_set(wlc,
							appvt->rxchain_pwrsave.rxchain, TRUE);
#ifdef WL11N
						/* need to restore rx_stbc HT capability
						 * after exit rxchain_pwrsave mode
						 */
						wlc_stf_exit_rxchain_pwrsave(wlc,
							appvt->rxchain_pwrsave.ht_cap_rx_stbc);
#endif /* WL11N */
						appvt->rxchain_pwrsave.pwrsave.in_power_save =
							FALSE;
					}
					appvt->rxchain_pwrsave.pwrsave.power_save_check &=
						~PWRSAVE_ENB;
					done = 1;
				}
			}
#endif /* RXCHAIN_PWRSAVE */
			/* clear scb deauth. sent flag so new deauth allows to be sent */
			scb->flags &= ~SCB_DEAUTH;
		}
#ifdef RXCHAIN_PWRSAVE
		if (!done) {
			if (!ap->rxchain_pwrsave_enable)
				appvt->rxchain_pwrsave.pwrsave.power_save_check &= ~PWRSAVE_ENB;
			else
				appvt->rxchain_pwrsave.pwrsave.power_save_check |= PWRSAVE_ENB;
		}
#endif
	}

#if defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND)
	if (wlc->pub->txdelay_params.period > 0)
		wlc_delay_stats_watchdog(wlc);
#endif /* defined(WLPKTDLYSTAT) && defined(WLPKTDLYSTAT_IND) */

	/* part 2 */
	if (AP_ENAB(wlc->pub)) {

		/* process age-outs only when not in scan progress */
		if (!SCAN_IN_PROGRESS(wlc->scan)) {
			/* age out ps queue packets */
			wlc_apps_psq_ageing(wlc);
			if (wlc->pub->now < wlc->pub->pending_now)
				wlc->pub->pending_now = 0;

			/* age out stas */
			if ((ap->scb_timeout &&
			(((wlc->pub->now - wlc->pub->pending_now)
			  % ap->scb_timeout) == 0)) ||
#ifdef WLP2P
			    (wlc->p2p && wlc_p2p_go_scb_timeout(wlc->p2p)) ||
#endif /* WLP2P */
			    ap->reprobe_scb) {
				wlc_ap_stas_timeout(ap);
				ap->reprobe_scb = FALSE;
			}
		} else {
			wlc->pub->pending_now++;
		}

		if (WLC_CHANIM_ENAB(wlc) && WLC_CHANIM_MODE_ACT(wlc->chanim_info))
			wlc_lq_chanim_upd_act(wlc);

#ifdef RXCHAIN_PWRSAVE
		/* Do the wl power save checks */
		if (appvt->rxchain_pwrsave.pwrsave.power_save_check & PWRSAVE_ENB)
			wlc_pwrsave_mode_check(ap, PWRSAVE_RXCHAIN);

		if (appvt->rxchain_pwrsave.pwrsave.in_power_save)
			appvt->rxchain_pwrsave.pwrsave.in_power_save_secs++;
#endif

#ifdef RADIO_PWRSAVE
		if (appvt->radio_pwrsave.pwrsave.power_save_check)
			wlc_pwrsave_mode_check(ap, PWRSAVE_RADIO);

		if (appvt->radio_pwrsave.pwrsave.in_power_save)
			appvt->radio_pwrsave.pwrsave.in_power_save_secs++;
#endif
	}

	/* Part 4 */
	FOREACH_UP_AP(wlc, idx, cfg) {
		/* update brcm_ie and our beacon */
		if (wlc_bss_update_brcm_ie(wlc, cfg)) {
			WL_APSTA_BCN(("wl%d.%d: wlc_watchdog() calls wlc_update_beacon()\n",
			              wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
			wlc_bss_update_beacon(wlc, cfg);
			wlc_bss_update_probe_resp(wlc, cfg, TRUE);
		}
	}
}

int
wlc_ap_get_maxassoc(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;

	return appvt->maxassoc;
}

void
wlc_ap_set_maxassoc(wlc_ap_info_t *ap, int val)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;

	appvt->maxassoc = val;
}

int
wlc_ap_get_maxassoc_limit(wlc_ap_info_t *ap)
{
	wlc_info_t *wlc = ((wlc_ap_info_pvt_t *)ap)->wlc;

#if defined(MAXASSOC_LIMIT)
	if (MAXASSOC_LIMIT <= wlc->pub->tunables->maxscb)
		return MAXASSOC_LIMIT;
	else
#endif /* MAXASSOC_LIMIT */
		return wlc->pub->tunables->maxscb;
}

static int
wlc_ap_ioctl(void *hdl, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_ap_info_t *ap = (wlc_ap_info_t *) hdl;
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	int val = 0, *pval;
	bool bool_val;
	int bcmerror = 0;
	struct maclist *maclist;
	wlc_bsscfg_t *bsscfg;
	struct scb_iter scbiter;
	struct scb *scb = NULL;

	/* update bsscfg pointer */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* default argument is generic integer */
	pval = (int *) arg;
	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	/* bool conversion to avoid duplication below */
	bool_val = (val != 0);

	switch (cmd) {

	case WLC_GET_SHORTSLOT_RESTRICT:
		if (AP_ENAB(wlc->pub)) {
			ASSERT(pval != NULL);
			*pval = ap->shortslot_restrict;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_SET_SHORTSLOT_RESTRICT:
		if (AP_ENAB(wlc->pub))
			ap->shortslot_restrict = bool_val;
		else
			bcmerror = BCME_NOTAP;
		break;

#ifdef BCMDBG
	case WLC_GET_IGNORE_BCNS:
		if (AP_ENAB(wlc->pub)) {
			ASSERT(pval != NULL);
			*pval = wlc->ignore_bcns;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_SET_IGNORE_BCNS:
		if (AP_ENAB(wlc->pub))
			wlc->ignore_bcns = bool_val;
		else
			bcmerror = BCME_NOTAP;
		break;
#endif /* BCMDBG */

	case WLC_GET_SCB_TIMEOUT:
		if (AP_ENAB(wlc->pub)) {
			ASSERT(pval != NULL);
			*pval = ap->scb_timeout;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_SET_SCB_TIMEOUT:
		if (AP_ENAB(wlc->pub))
			ap->scb_timeout = val;
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_GET_ASSOCLIST:
		ASSERT(arg != NULL);
		maclist = (struct maclist *) arg;
		ASSERT(maclist);

		/* returns a list of STAs associated with a specific bsscfg */
		if (len < (int)(sizeof(maclist->count) + (maclist->count * ETHER_ADDR_LEN))) {
			bcmerror = BCME_BADARG;
			break;
		}
		val = 0;
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (SCB_ASSOCIATED(scb)) {
				val++;
				if (maclist->count >= (uint)val) {
					bcopy((void*)&scb->ea, (void*)&maclist->ea[val-1],
					ETHER_ADDR_LEN);
				} else {
					bcmerror = BCME_BUFTOOSHORT;
					break;
				}
			}

		}
		if (!bcmerror)
			maclist->count = val;
		break;

	case WLC_TKIP_COUNTERMEASURES:
		if (BSSCFG_AP(bsscfg) && WSEC_TKIP_ENABLED(bsscfg->wsec))
			(void)wlc_keymgmt_tkip_set_cm(wlc->keymgmt, bsscfg, (val != 0));
		else
			bcmerror = BCME_BADARG;
		break;

#ifdef APCS
	case WLC_GET_CHANNEL_SEL:
		if (!AP_ACTIVE(wlc) && !SCAN_IN_PROGRESS(wlc->scan)) {
			ASSERT(pval != NULL);
			*pval = ap->chanspec_selected;
		} else {
			bcmerror = BCME_BADARG;
		}
		break;

#endif /* APCS */

#ifdef RADAR
	case WLC_SET_RADAR:
		bcmerror = wlc_dfs_set_radar(wlc->dfs, (int)val);
		break;

	case WLC_GET_RADAR:
		ASSERT(pval != NULL);
		*pval = (int32)wlc_dfs_get_radar(wlc->dfs);
		break;
#endif /* RADAR */

	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	return (bcmerror);
}

static int
wlc_ap_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) hdl;
	wlc_ap_info_t* ap = &appvt->appub;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

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

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {

	case IOV_GVAL(IOV_AUTHE_STA_LIST):
		/* return buf is a maclist */
		err = wlc_sta_list_get(ap, bsscfg, (uint8*)arg, len,
		                       wlc_authenticated_sta_check_cb);
		break;

	case IOV_GVAL(IOV_AUTHO_STA_LIST):
		/* return buf is a maclist */
		err = wlc_sta_list_get(ap, bsscfg, (uint8*)arg, len,
		                       wlc_authorized_sta_check_cb);
		break;

	case IOV_GVAL(IOV_WME_STA_LIST):	/* Deprecated; use IOV_STA_INFO */
		/* return buf is a maclist */
		err = wlc_sta_list_get(ap, bsscfg, (uint8*)arg, len,
		                       wlc_wme_sta_check_cb);
		break;

	case IOV_GVAL(IOV_MAXASSOC):
		*(uint32*)arg = wlc_ap_get_maxassoc(wlc->ap);
		break;

	case IOV_SVAL(IOV_MAXASSOC):
		if (int_val > wlc_ap_get_maxassoc_limit(wlc->ap)) {
			err = BCME_RANGE;
			goto exit;
		}
		wlc_ap_set_maxassoc(wlc->ap, int_val);
		break;

#if defined(MBSS) || defined(WLP2P)
	case IOV_GVAL(IOV_BSS_MAXASSOC):
		*(uint32*)arg = bsscfg->maxassoc;
		break;

	case IOV_SVAL(IOV_BSS_MAXASSOC):
		if (int_val > wlc->pub->tunables->maxscb) {
			err = BCME_RANGE;
			goto exit;
		}
		bsscfg->maxassoc = int_val;
		break;
#endif /* MBSS || WLP2P */

	case IOV_GVAL(IOV_AP_ISOLATE):
		*ret_int_ptr = (int32)bsscfg->ap_isolate;
		break;

	case IOV_GVAL(IOV_AP):
		*((uint*)arg) = BSSCFG_AP(bsscfg);
		break;

#if defined(STA) && defined(AP)
	case IOV_SVAL(IOV_AP):
		if (!APSTA_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		wlc_bsscfg_reinit(wlc, bsscfg, bool_val);
		break;
#endif /* defined(STA) && defined(AP) */

	case IOV_SVAL(IOV_AP_ISOLATE):
		bsscfg->ap_isolate = (uint8)int_val;
		break;
	case IOV_GVAL(IOV_SCB_ACTIVITY_TIME):
		*ret_int_ptr = (int32)ap->scb_activity_time;
		break;

	case IOV_SVAL(IOV_SCB_ACTIVITY_TIME):
		ap->scb_activity_time = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_CLOSEDNET):
		*ret_int_ptr = bsscfg->closednet_nobcnssid;
		break;

	case IOV_SVAL(IOV_CLOSEDNET):
		/* "closednet" control two functionalities: hide ssid in bcns
		 * and don't respond to broadcast probe requests
		 */
		bsscfg->closednet_nobcnssid = bool_val;
		bsscfg->closednet_nobcprbresp = bool_val;
		if (BSSCFG_AP(bsscfg) && bsscfg->up) {
			wlc_bss_update_beacon(wlc, bsscfg);
			if (!MBSS_ENAB(wlc->pub))
				wlc_mctrl(wlc, MCTL_CLOSED_NETWORK,
				(bool_val ? MCTL_CLOSED_NETWORK : 0));
		}
		break;

	case IOV_GVAL(IOV_BSS):
		if (p_len < (int)sizeof(int)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val >= 0) {
			bsscfg = wlc_bsscfg_find(wlc, int_val, &err);
		} /* otherwise, use the value from the wlif object */

		if (bsscfg)
			*ret_int_ptr = bsscfg->up;
		else if (err == BCME_NOTFOUND)
			*ret_int_ptr = 0;
		else
			break;
		break;

	case IOV_SVAL(IOV_BSS): {
		bool sta = FALSE;

		if (len < (int)(2 * sizeof(int))) {
			err = BCME_BUFTOOSHORT;
			break;
		}
#ifdef STA
		if (int_val2 == 3)
			sta = FALSE;
		else if (int_val2 == 2)
			sta = TRUE;
#endif
		/* On an 'up', use wlc_bsscfg_alloc() to create a bsscfg if it does not exist,
		 * but on a 'down', just find the bsscfg if it already exists
		 */
		if (int_val >= 0) {
			bsscfg = wlc_bsscfg_find(wlc, int_val, &err);
			if (int_val2 > 0) {
				if (bsscfg == NULL && err == BCME_NOTFOUND) {
					bsscfg = wlc_bsscfg_alloc(wlc, int_val, 0, NULL, !sta);
					if (bsscfg == NULL)
						err = BCME_NOMEM;
					else if ((err = wlc_bsscfg_init(wlc, bsscfg))) {
						WL_ERROR(("wl%d: wlc_bsscfg_init %s failed (%d)\n",
						          wlc->pub->unit, sta ? "STA" : "AP", err));
						wlc_bsscfg_free(wlc, bsscfg);
					}
				}
			}
#ifdef WLRSDB
			/* RSDB OVERRIDE: IOV_BSS does not follow the usual bsscfg: prefix model */
			if (RSDB_ENAB(wlc->pub) && (bsscfg != NULL))
				wlc = bsscfg->wlc;
#endif /* WLRSDB */
		}

#ifdef STA
		if (int_val2 == 3)
			break;
		else if (int_val2 == 2)
			break;
#endif
		if (err) {
			/* do not error on a 'down' of a nonexistent bsscfg */
			if (err == BCME_NOTFOUND && int_val2 == 0)
				err = 0;
			break;
		}

		if (int_val2 > 0) {
			if (bsscfg->up) {
				WL_APSTA_UPDN(("wl%d: Ignoring UP, bsscfg %d already UP\n",
					wlc->pub->unit, int_val));
				break;
			}
			if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) ||
				mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE)) {
				WL_APSTA_UPDN(("wl%d: Ignoring UP, bsscfg %d; radio off\n",
					wlc->pub->unit, int_val));
				err = BCME_RADIOOFF;
				break;
			}

			WL_APSTA_UPDN(("wl%d: BSS up cfg %d (%s) -> wlc_bsscfg_enable()\n",
				wlc->pub->unit, int_val, (BSSCFG_AP(bsscfg) ? "AP" : "STA")));
			if (BSSCFG_AP(bsscfg))
				err = wlc_bsscfg_enable(wlc, bsscfg);
#ifdef WLP2P
			else if (BSS_P2P_ENAB(wlc, bsscfg))
				err = BCME_ERROR;
#endif
#ifdef STA
			else if (BSSCFG_STA(bsscfg))
				wlc_join(wlc, bsscfg, bsscfg->SSID, bsscfg->SSID_len,
				         NULL, NULL, 0);
#endif
			if (err)
				break;
#ifdef RADAR
			if (WL11H_AP_ENAB(wlc)) {
				if (BSSCFG_AP(bsscfg) && !(BSSCFG_SRADAR_ENAB(bsscfg) ||
				      BSSCFG_AP_NORADAR_CHAN_ENAB(bsscfg)))
					wlc_set_dfs_cacstate(wlc->dfs, ON);
			}
#endif /* RADAR */
		} else {
			if (!bsscfg->enable) {
				WL_APSTA_UPDN(("wl%d: Ignoring DOWN, bsscfg %d already DISABLED\n",
					wlc->pub->unit, int_val));
				break;
			}
			WL_APSTA_UPDN(("wl%d: BSS down on %d (%s) -> wlc_bsscfg_disable()\n",
				wlc->pub->unit, int_val, (BSSCFG_AP(bsscfg) ? "AP" : "STA")));
			wlc_bsscfg_disable(wlc, bsscfg);

			/* Turn of radar_detect if none of AP's are on radar chanspec */
			if (WL11H_AP_ENAB(wlc)) {
				if (!wlc_ap_on_radarchan(wlc->ap) && !(BSSCFG_SRADAR_ENAB(bsscfg) ||
					BSSCFG_AP_NORADAR_CHAN_ENAB(bsscfg)))
					wlc_set_dfs_cacstate(wlc->dfs, OFF);
			}
		}
		break;
	}

	case IOV_GVAL(IOV_APCSCHSPEC):
		if (wlc->pub->up && AP_ENAB(wlc->pub) &&
		    !SCAN_IN_PROGRESS(wlc->scan)) {
			*ret_int_ptr = (int32)ap->chanspec_selected;
		}
		else {
			err = BCME_BADARG;
		}
		break;

#if defined(STA) /* APSTA */
	case IOV_GVAL(IOV_APSTA):
		*ret_int_ptr = APSTA_ENAB(wlc->pub);
		break;

	case IOV_SVAL(IOV_APSTA):
		/* Flagged for no set while up */
		if (bool_val == APSTA_ENAB(wlc->pub)) {
			WL_APSTA(("wl%d: No change to APSTA mode\n", wlc->pub->unit));
			break;
		}

		bsscfg = wlc_bsscfg_primary(wlc);
		if (bool_val) {
			/* Turning on APSTA, force various other items:
			 *   Global AP, cfg (wlc->cfg) STA, not IBSS.
			 *   Make beacon/probe AP config bsscfg[1].
			 *   Force off 11D.
			 */
			WL_APSTA(("wl%d: Enabling APSTA mode\n", wlc->pub->unit));
			if (bsscfg->enable)
				wlc_bsscfg_disable(wlc, bsscfg);
		}
		else {
			/* Turn off APSTA: make global AP and cfg[0] same */
			WL_APSTA(("wl%d: Disabling APSTA mode\n", wlc->pub->unit));
		}
		err = wlc_bsscfg_reinit(wlc, bsscfg, bool_val == 0);
		if (err)
			break;
		wlc->pub->_ap = TRUE;
		if (bool_val) {
			wlc->default_bss->infra = 1;
		}
		wlc->pub->_apsta = bool_val;

		/* Act similarly to WLC_SET_AP */
		wlc_ap_upd(wlc, bsscfg);
		wlc->wet = FALSE;
		wlc_radio_mpc_upd(wlc);
		break;

#endif /* APSTA */


	case IOV_GVAL(IOV_PREF_CHANSPEC):
		*ret_int_ptr = (int32)wlc->ap->pref_chanspec;
		break;

	case IOV_SVAL(IOV_PREF_CHANSPEC):
		/* Special case to clear user preferred setting.
		 * 0 - pref_chanspec means we're using Auto Channel.
		 */
		if (int_val == 0) {
			wlc->ap->pref_chanspec = 0;
			break;
		}

		if (!wlc_valid_chanspec_db(wlc->cmi, (chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		wlc->ap->pref_chanspec = (chanspec_t)int_val;
		break;

#ifdef RXCHAIN_PWRSAVE
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_ENABLE):
		*ret_int_ptr = ap->rxchain_pwrsave_enable;
		break;

	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_ENABLE):
		if (!int_val)
			appvt->rxchain_pwrsave.pwrsave.power_save_check &= ~PWRSAVE_ENB;
		else
			appvt->rxchain_pwrsave.pwrsave.power_save_check |= PWRSAVE_ENB;

		ap->rxchain_pwrsave_enable = int_val;
		if (!int_val)
			wlc_reset_rxchain_pwrsave_mode(ap);

#ifdef WLPM_BCNRX
		if (PM_BCNRX_ENAB(wlc->pub) && bool_val) {
			/* Avoid ucode interference if AP enables this power-save mode */
			wlc_pm_bcnrx_disable(wlc);
		}
#endif
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_QUIET_TIME):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.quiet_time;
		break;

	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_QUIET_TIME):
		appvt->rxchain_pwrsave.pwrsave.quiet_time = int_val;
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_PPS):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.pps_threshold;
		break;

	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_PPS):
		appvt->rxchain_pwrsave.pwrsave.pps_threshold = int_val;
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.in_power_save;
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.stas_assoc_check;
		break;
	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK):
		appvt->rxchain_pwrsave.pwrsave.stas_assoc_check = int_val;
		if (int_val && ap->rxchain_pwrsave_enable &&
		    appvt->rxchain_pwrsave.pwrsave.in_power_save &&
		    wlc_ap_stas_associated(wlc->ap)) {
			wlc_reset_rxchain_pwrsave_mode(ap);
		}
		break;
#endif /* RXCHAIN_PWRSAVE */
#ifdef RADIO_PWRSAVE
	case IOV_GVAL(IOV_RADIO_PWRSAVE_ENABLE):
		*ret_int_ptr = ap->radio_pwrsave_enable;
		break;

	case IOV_SVAL(IOV_RADIO_PWRSAVE_ENABLE):
		if (!MBSS_ENAB(wlc->pub)) {
			err = BCME_EPERM;
			WL_ERROR(("wl%d: Radio pwrsave not supported in non-mbss case yet.\n",
				wlc->pub->unit));
			break;
		}
		ap->radio_pwrsave_enable = appvt->radio_pwrsave.pwrsave.power_save_check = int_val;
		wlc_reset_radio_pwrsave_mode(ap);

#ifdef WLPM_BCNRX
		if (PM_BCNRX_ENAB(wlc->pub) && bool_val) {
			/* Avoid ucode interference if AP enables this power-save mode */
			wlc_pm_bcnrx_disable(wlc);
		}
#endif
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE_QUIET_TIME):
		*ret_int_ptr = appvt->radio_pwrsave.pwrsave.quiet_time;
		break;

	case IOV_SVAL(IOV_RADIO_PWRSAVE_QUIET_TIME):
		appvt->radio_pwrsave.pwrsave.quiet_time = int_val;
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE_PPS):
		*ret_int_ptr = appvt->radio_pwrsave.pwrsave.pps_threshold;
		break;

	case IOV_SVAL(IOV_RADIO_PWRSAVE_PPS):
		appvt->radio_pwrsave.pwrsave.pps_threshold = int_val;
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE):
		*ret_int_ptr = appvt->radio_pwrsave.pwrsave.in_power_save;
		break;
	case IOV_SVAL(IOV_RADIO_PWRSAVE_LEVEL):{
		uint8 dtim_period;
		uint16 beacon_period;

		bsscfg = wlc->cfg;

		if (bsscfg->associated) {
			dtim_period = bsscfg->current_bss->dtim_period;
			beacon_period = bsscfg->current_bss->beacon_period;
		} else {
			dtim_period = wlc->default_bss->dtim_period;
			beacon_period = wlc->default_bss->beacon_period;
		}

		if (int_val > RADIO_PWRSAVE_HIGH) {
			err = BCME_RANGE;
			goto exit;
		}

		if (dtim_period == 1) {
			err = BCME_ERROR;
			goto exit;
		}

		appvt->radio_pwrsave.level = int_val;
		switch (appvt->radio_pwrsave.level) {
		case RADIO_PWRSAVE_LOW:
			appvt->radio_pwrsave.on_time = 2*beacon_period*dtim_period/3;
			appvt->radio_pwrsave.off_time = beacon_period*dtim_period/3;
			break;
		case RADIO_PWRSAVE_MEDIUM:
			appvt->radio_pwrsave.on_time = beacon_period*dtim_period/2;
			appvt->radio_pwrsave.off_time = beacon_period*dtim_period/2;
			break;
		case RADIO_PWRSAVE_HIGH:
			appvt->radio_pwrsave.on_time = beacon_period*dtim_period/3;
			appvt->radio_pwrsave.off_time = 2*beacon_period*dtim_period/3;
			break;
		}
		break;
	}
	case IOV_GVAL(IOV_RADIO_PWRSAVE_LEVEL):
		*ret_int_ptr = appvt->radio_pwrsave.level;
		break;
	case IOV_SVAL(IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK):
		appvt->radio_pwrsave.pwrsave.stas_assoc_check = int_val;
		if (int_val && RADIO_PWRSAVE_ENAB(wlc->ap) &&
		    wlc_radio_pwrsave_in_power_save(wlc->ap) &&
		    wlc_ap_stas_associated(wlc->ap)) {
			wlc_radio_pwrsave_exit_mode(wlc->ap);
			WL_INFORM(("Going out of power save as there are associated STASs!\n"));
		}
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK):
	        *ret_int_ptr = appvt->radio_pwrsave.pwrsave.stas_assoc_check;
		break;
#endif /* RADIO_PWRSAVE */
#ifdef BCM_DCS
	case IOV_GVAL(IOV_BCMDCS):
		*ret_int_ptr = ap->dcs_enabled ? TRUE : FALSE;
		break;

	case IOV_SVAL(IOV_BCMDCS):
		ap->dcs_enabled = bool_val;
		break;

#endif /* BCM_DCS */
	case IOV_GVAL(IOV_DYNBCN):
		*ret_int_ptr = (int32)((bsscfg->flags & WLC_BSSCFG_DYNBCN) == WLC_BSSCFG_DYNBCN);
		break;
	case IOV_SVAL(IOV_DYNBCN):
		if (!BSSCFG_AP(bsscfg)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (int_val && !DYNBCN_ENAB(bsscfg)) {
			bsscfg->flags |= WLC_BSSCFG_DYNBCN;

			/* Disable beacons if no sta is associated */
			if (wlc_bss_assocscb_getcnt(wlc, bsscfg) == 0)
				wlc_bsscfg_bcn_disable(wlc, bsscfg);
		} else if (!int_val && DYNBCN_ENAB(bsscfg)) {
			bsscfg->flags &= ~WLC_BSSCFG_DYNBCN;
			wlc_bsscfg_bcn_enable(wlc, bsscfg);
		}
		break;

	case IOV_GVAL(IOV_SCB_LASTUSED): {
		uint elapsed = 0;
		uint min_val = (uint)-1;
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb)) {
				elapsed = wlc->pub->now - scb->used;
				if (elapsed < min_val)
					min_val = elapsed;
			}
		}
		*ret_int_ptr = min_val;
		break;
	}

	case IOV_GVAL(IOV_SCB_PROBE): {
		wl_scb_probe_t scb_probe;

		scb_probe.scb_timeout = ap->scb_timeout;
		scb_probe.scb_activity_time = ap->scb_activity_time;
		scb_probe.scb_max_probe = ap->scb_max_probe;

		bcopy((char *)&scb_probe, (char *)arg, sizeof(wl_scb_probe_t));
		break;
	}

	case IOV_SVAL(IOV_SCB_PROBE): {
		wl_scb_probe_t *scb_probe = (wl_scb_probe_t *)arg;

		if (!scb_probe->scb_timeout || (!scb_probe->scb_max_probe)) {
			err = BCME_BADARG;
			break;
		}

		ap->scb_timeout = scb_probe->scb_timeout;
		ap->scb_activity_time = scb_probe->scb_activity_time;
		ap->scb_max_probe = scb_probe->scb_max_probe;
		break;
	}

	case IOV_GVAL(IOV_SCB_ASSOCED): {

		bool assoced = TRUE;
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb))
				break;
		}

		if (!scb)
			assoced = FALSE;

		*ret_int_ptr = (uint32)assoced;
		break;
	}

	case IOV_SVAL(IOV_ACS_UPDATE):

		if (SCAN_IN_PROGRESS(wlc->scan)) {
			err = BCME_BUSY;
			break;
		}

		if (wlc->pub->up && (ap->chanspec_selected != 0) &&
		    (WLC_BAND_PI_RADIO_CHANSPEC != ap->chanspec_selected))
			wlc_ap_acs_update(wlc);

		if (WL11H_AP_ENAB(wlc) && AP_ACTIVE(wlc)) {
			if (wlc_radar_chanspec(wlc->cmi, ap->chanspec_selected))
				wlc_set_dfs_cacstate(wlc->dfs, ON);
			else
				wlc_set_dfs_cacstate(wlc->dfs, OFF);
		}
		break;

	case IOV_GVAL(IOV_BEACON_INFO):
	case IOV_GVAL(IOV_PROBE_RESP_INFO): {
		/* Retrieve the 802.11 beacon management packet */
		int hdr_skip = 0;
		int pkt_len = 0;
		uint8 *pkt_data = NULL;
		uint16 *template = NULL;

		if (BSSCFG_AP(bsscfg) && bsscfg->up) {
			if (MBSS_BCN_ENAB(wlc, bsscfg)) {
#if defined(MBSS)
				wlc_pkt_t pkt;

				if (IOV_ID(actionid) == IOV_BEACON_INFO) {
					wlc_spt_t *bcn_template =
						wlc_mbss_get_bcn_template(wlc, bsscfg);

					ASSERT(bcn_template != NULL);
					hdr_skip = D11_TXH_LEN_EX(wlc);
					pkt = SPT_LATEST_PKT(bcn_template);
					pkt_len = PKTLEN(wlc->osh, pkt);
					pkt_data = (uint8 *)PKTDATA(wlc->osh, pkt);
				} else {
					/* IOV_PROBE_RESP_INFO */
					if ((pkt = wlc_mbss_get_probe_template(wlc, bsscfg))) {
						hdr_skip = D11_TXH_LEN_EX(wlc);
						pkt_len = PKTLEN(wlc->osh, pkt);
						pkt_data = (uint8 *)PKTDATA(wlc->osh, pkt);
					}
				}
#endif /* MBSS */
			} else if (HWBCN_ENAB(bsscfg)) {
				uint type;
				ratespec_t rspec;

				hdr_skip = D11_PHY_HDR_LEN;
				pkt_len = wlc->pub->bcn_tmpl_len;
				template = (uint16 *)MALLOC(wlc->pub->osh, pkt_len);
				if (template == NULL) {
					WL_ERROR(("wl%d: %s: MALLOC template failed,"
						" malloced %d bytes\n", wlc->pub->unit,
						__FUNCTION__, MALLOCED(wlc->osh)));
					err = BCME_NOMEM;
					break;
				}
				pkt_data = (uint8 *) template;

				if (IOV_ID(actionid) == IOV_BEACON_INFO) {
					type = FC_BEACON;
					rspec = wlc_lowest_basic_rspec(wlc,
						&bsscfg->current_bss->rateset);
				} else {
					/* IOV_PROBE_RESP_INFO */
					type = FC_PROBE_RESP;
					rspec = (ratespec_t) 0;
				}

				wlc_bcn_prb_template(wlc, type, rspec,
				                     bsscfg, template, &pkt_len);
			}
		}

		/* Only interested in the management frame */
		pkt_data += hdr_skip;
		pkt_len -= hdr_skip;

		if (len >= (pkt_len + (int) sizeof(int_val))) {
			*ret_int_ptr = (int32) pkt_len;
			if (pkt_len > 0) {
				arg = (char *)arg + sizeof(int_val);
				bcopy(pkt_data, arg, pkt_len);
			}
		} else {
			err = BCME_BUFTOOSHORT;
		}

		if (template) {
			MFREE(wlc->pub->osh, template, pkt_len);
		}

	        break;
	}
	case IOV_GVAL(IOV_MODE_REQD):
		*ret_int_ptr = (int32)wlc_ap_get_opmode_cap_reqd(wlc, bsscfg);
		break;
	case IOV_SVAL(IOV_MODE_REQD):
		err = wlc_ap_set_opmode_cap_reqd(wlc, bsscfg, int_val);
		break;

#ifdef PSPRETEND
	case IOV_GVAL(IOV_PSPRETEND_THRESHOLD):
	{
		*ret_int_ptr = (int32)bsscfg->ps_pretend_threshold;
	}
	break;

	case IOV_SVAL(IOV_PSPRETEND_THRESHOLD):
	{
		bsscfg->ps_pretend_threshold = (uint8)int_val;
		if (WLC_TXC_ENAB(wlc)) {
			wlc_txc_inv_all(wlc->txc);
		}
	}
	break;

	case IOV_GVAL(IOV_PSPRETEND_RETRY_LIMIT):
	{
		*ret_int_ptr = (int32)bsscfg->ps_pretend_retry_limit;
	}
	break;

	case IOV_SVAL(IOV_PSPRETEND_RETRY_LIMIT):
	{
		bsscfg->ps_pretend_retry_limit = (uint8)int_val;
		if (WLC_TXC_ENAB(wlc)) {
			wlc_txc_inv_all(wlc->txc);
		}
	}
	break;
#endif /* PSPRETEND */

	case IOV_GVAL(IOV_BSS_RATESET):
		if (bsscfg)
			*ret_int_ptr = (int32)bsscfg->rateset;
		else {
			*ret_int_ptr = 0;
			err = BCME_BADARG;
		}
		break;

	case IOV_SVAL(IOV_BSS_RATESET):
		if (!bsscfg || int_val < WLC_BSSCFG_RATESET_DEFAULT ||
		            int_val > WLC_BSSCFG_RATESET_MAX)
			err = BCME_BADARG;
		else if (bsscfg->up)
			/* do not change rateset while this bss is up */
			err = BCME_NOTDOWN;
		else
			bsscfg->rateset = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_FORCE_BCN_RSPEC):
		*ret_int_ptr = (int32)(appvt->force_bcn_rspec & RATE_MASK);

		break;

	case IOV_SVAL(IOV_FORCE_BCN_RSPEC): {
		ratespec_t rspec = (ratespec_t)(int_val & RATE_MASK);

		/* escape if new value identical to current value */
		if ((rspec & RATE_MASK) == (appvt->force_bcn_rspec & RATE_MASK))
			break;

		err = wlc_force_bcn_rspec_upd(wlc, bsscfg, ap, rspec);
		break;

	}
	break;

#ifdef USBAP
	case IOV_GVAL(IOV_WLANCOEX):
		*ret_int_ptr = (int32)appvt->wlancoex;
		break;

	case IOV_SVAL(IOV_WLANCOEX): {
		if (appvt->wlancoex == bool_val)
			break;

		err = wlc_wlancoex_upd(wlc, ap, bool_val);
		break;
	}
#endif /* USBAP */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

exit:
	return err;
}

static void
wlc_ap_acs_update(wlc_info_t *wlc)
{

	WL_INFORM(("wl%d: %s: changing chanspec to %d\n",
		wlc->pub->unit, __FUNCTION__, wlc->ap->chanspec_selected));
	wlc_set_home_chanspec(wlc, wlc->ap->chanspec_selected);
	wlc_suspend_mac_and_wait(wlc);

	wlc_set_chanspec(wlc, wlc->ap->chanspec_selected);
	if (AP_ENAB(wlc->pub)) {
		wlc->bcn_rspec = wlc_lowest_basic_rspec(wlc,
			&wlc->cfg->current_bss->rateset);
		ASSERT(wlc_valid_rate(wlc, wlc->bcn_rspec,
			CHSPEC_IS2G(wlc->cfg->current_bss->chanspec) ?
			WLC_BAND_2G : WLC_BAND_5G, TRUE));
		wlc_beacon_phytxctl(wlc, wlc->bcn_rspec, wlc->chanspec);
	}

	if (wlc->pub->associated) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, FALSE);
	}
	wlc_enable_mac(wlc);
}

/*
 * Set the operational capabilities for STAs required to associate to the BSS.
 */
static int
wlc_ap_set_opmode_cap_reqd(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg,
	opmode_cap_t opmode)
{
	int err = 0;

	wlc_ap_info_pvt_t *appvt;
	ap_bsscfg_cubby_t *ap_cfg;

	ASSERT(wlc != NULL);
	ASSERT(bsscfg != NULL);

	appvt = (wlc_ap_info_pvt_t *) wlc->ap;
	ASSERT(appvt != NULL);
	ap_cfg = AP_BSSCFG_CUBBY(appvt, bsscfg);
	ASSERT(ap_cfg != NULL);

	if (opmode >= OMC_MAX) {
		err = BCME_RANGE;
		goto exit;
	}

	/* can only change setting if the BSS is down */
	if (bsscfg->up) {
		err = BCME_ASSOCIATED;
		goto exit;
	}

	/* apply setting */
	ap_cfg->opmode_cap_reqd = opmode;

exit:
	return err;
}

/*
 * Get the operational capabilities for STAs required to associate to the BSS.
 */
static opmode_cap_t
wlc_ap_get_opmode_cap_reqd(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t *appvt;
	ap_bsscfg_cubby_t *ap_cfg;

	ASSERT(wlc != NULL);
	ASSERT(bsscfg != NULL);

	appvt = (wlc_ap_info_pvt_t *) wlc->ap;
	ASSERT(appvt != NULL);
	ap_cfg = AP_BSSCFG_CUBBY(appvt, bsscfg);
	ASSERT(ap_cfg != NULL);

	/* return the setting */
	return ap_cfg->opmode_cap_reqd;
}

static int wlc_ap_bsscfg_init(void *context, wlc_bsscfg_t *cfg)
{
	wlc_ap_info_pvt_t *appvt;
	ap_bsscfg_cubby_t *ap_cfg;

	ASSERT(context != NULL);
	ASSERT(cfg != NULL);

	appvt = (wlc_ap_info_pvt_t *)context;
	ap_cfg = AP_BSSCFG_CUBBY(appvt, cfg);
	ASSERT(ap_cfg != NULL);

	/* The operational mode capabilities init */
	ap_cfg->opmode_cap_reqd = OMC_NONE;

	return 0;
}


void
wlc_ap_bsscfg_scb_cleanup(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb_iter scbiter;
	struct scb *scb;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (SCB_ASSOCIATED(scb)) {
			if (!scb->permanent) {
				wlc_scbfree(wlc, scb);
			}
		}
	}
}

void
wlc_ap_scb_cleanup(wlc_info_t *wlc)
{
	wlc_bsscfg_t *apcfg;
	int idx;

	FOREACH_UP_AP(wlc, idx, apcfg) {
		wlc_txflowcontrol_override(wlc, apcfg->wlcif->qi, OFF, TXQ_STOP_FOR_PKT_DRAIN);
		wlc_scb_update_band_for_cfg(wlc, apcfg, WLC_BAND_PI_RADIO_CHANSPEC);
	}
}

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
/*
 * Returns true if check for associated STAs is enabled
 * and there are STAs associated
 */
static bool
wlc_pwrsave_stas_associated_check(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	bool check_assoc_stas = FALSE;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			check_assoc_stas = appvt->rxchain_pwrsave.pwrsave.stas_assoc_check;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			check_assoc_stas = appvt->radio_pwrsave.pwrsave.stas_assoc_check;
			break;
#endif
		default:
			break;
	}
	return (check_assoc_stas && (wlc_ap_stas_associated(ap) > 0));
}

/*
 * At every watchdog tick we update the power save
 * data structures and see if we can go into a power
 * save mode
 */
static void
wlc_pwrsave_mode_check(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pwrsave_t *pwrsave = NULL;
	uint pkts_per_second, total_pktcount;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			pwrsave = &appvt->rxchain_pwrsave.pwrsave;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			pwrsave = &appvt->radio_pwrsave.pwrsave;
			break;
#endif
		default:
			break;
	}

#ifdef RXCHAIN_PWRSAVE
	/* Enter rxchain_pwrsave mode when there is no STA associated to AP
	 * and BSS is already up
	 */
	if ((appvt->rxchain_pwrsave.pwrsave.power_save_check & NONASSOC_PWRSAVE_ENB) &&
#ifdef WDS
		!wlc_rxchain_wds_detection(wlc) &&
#endif /* WDS */
		(type == PWRSAVE_RXCHAIN) && (wlc_ap_stas_associated(ap) == 0) &&
		!pwrsave->in_power_save && wlc->cfg->up) {
		appvt->rxchain_pwrsave.rxchain = wlc->stf->rxchain;
#ifdef WL11N
		/* need to save and disable rx_stbc HT capability
		 * before enter rxchain_pwrsave mode
		 */
		appvt->rxchain_pwrsave.ht_cap_rx_stbc = wlc_stf_enter_rxchain_pwrsave(wlc);
#endif /* WL11N */
		wlc_stf_rxchain_set(wlc, 0x1, FALSE);
		pwrsave->in_power_save = TRUE;
		pwrsave->in_power_save_counter++;
		return;
	}
#endif /* RXCHAIN_PWRSAVE */

	/* Total pkt count - forwarded packets + packets the os has given + sendup packets */
	total_pktcount =  WLPWRSAVERXFVAL(wlc) + WLPWRSAVETXFVAL(wlc);

	/* Calculate the packets per second */
	pkts_per_second = total_pktcount - pwrsave->prev_pktcount;

	/* Save the current packet count for next second */
	pwrsave->prev_pktcount = total_pktcount;

	if (pkts_per_second < pwrsave->pps_threshold) {
		/* When the packets are below the threshold we just
		 * increment our timeout counter
		 */
		if (!pwrsave->in_power_save) {
			if ((pwrsave->quiet_time_counter >= pwrsave->quiet_time) &&
			    (! wlc_pwrsave_stas_associated_check(ap, type))) {
				WL_INFORM(("Entering power save mode pps is %d\n",
					pkts_per_second));
#ifdef RXCHAIN_PWRSAVE
				if (type == PWRSAVE_RXCHAIN) {
					/* Save current configured rxchains */
					appvt->rxchain_pwrsave.rxchain = wlc->stf->rxchain;
#ifdef WL11N
					/* need to save and disable rx_stbc HT capability
					 * before enter rxchain_pwrsave mode
					 */
					appvt->rxchain_pwrsave.ht_cap_rx_stbc =
						wlc_stf_enter_rxchain_pwrsave(wlc);
#endif /* WL11N */
					wlc_stf_rxchain_set(wlc, 0x1, TRUE);
				}
#endif /* RXCHAIN_PWRSAVE */
				pwrsave->in_power_save = TRUE;
				pwrsave->in_power_save_counter++;
				return;
			}
		}
		pwrsave->quiet_time_counter++;
	} else {
		/* If we are already in the wait mode counting
		 * up then just reset the counter since
		 * packets have gone above threshold
		 */
		pwrsave->quiet_time_counter = 0;
		WL_INFORM(("Resetting quiet time\n"));
		if (pwrsave->in_power_save) {
			if (type == PWRSAVE_RXCHAIN) {
#ifdef RXCHAIN_PWRSAVE
				wlc_stf_rxchain_set(wlc, appvt->rxchain_pwrsave.rxchain, TRUE);
#ifdef WL11N
				/* need to restore rx_stbc HT capability
				 * after exit rxchain_pwrsave mode
				 */
				wlc_stf_exit_rxchain_pwrsave(wlc,
					appvt->rxchain_pwrsave.ht_cap_rx_stbc);
#endif /* WL11N */
#endif /* RXCHAIN_PWRSAVE */
			} else if (type == PWRSAVE_RADIO) {
#ifdef RADIO_PWRSAVE
				wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
				if (appvt->radio_pwrsave.radio_disabled) {
					wlc_bmac_radio_hw(wlc->hw, TRUE,
						(CHIPID(wlc->pub->sih->chip) == BCM5356_CHIP_ID) ?
						TRUE : FALSE);
					appvt->radio_pwrsave.radio_disabled = FALSE;
				}
				appvt->radio_pwrsave.pwrsave_state = 0;
				appvt->radio_pwrsave.cncl_bcn = FALSE;
#endif
			}
			WL_INFORM(("Exiting power save mode pps is %d\n", pkts_per_second));
			pwrsave->in_power_save = FALSE;
		}
	}
}
#endif /* RXCHAIN_PWRSAVE || RADIO_PWRSAVE */

#ifdef RADIO_PWRSAVE

/*
 * Routine that enables/disables the radio for the duty cycle
 */
static void
wlc_radio_pwrsave_timer(void *arg)
{
	wlc_ap_info_t *ap = (wlc_ap_info_t*)arg;
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;

	if (appvt->radio_pwrsave.radio_disabled) {
		WL_INFORM(("wl power timer OFF period end. enabling radio\n"));

		if (appvt->radio_pwrsave.level) {
			appvt->radio_pwrsave.cncl_bcn = TRUE;
		}

		wlc_bmac_radio_hw(wlc->hw, TRUE,
			(CHIPID(wlc->pub->sih->chip) == BCM5356_CHIP_ID) ?  TRUE : FALSE);
		appvt->radio_pwrsave.radio_disabled = FALSE;
		/* Re-enter power save state */
		appvt->radio_pwrsave.pwrsave_state = 2;
	} else {
		WL_INFORM(("wl power timer OFF period starts. disabling radio\n"));
		wlc_radio_pwrsave_off_time_start(ap);
		wlc_bmac_radio_hw(wlc->hw, FALSE,
			(CHIPID(wlc->pub->sih->chip) == BCM5356_CHIP_ID) ?  TRUE : FALSE);
		appvt->radio_pwrsave.radio_disabled = TRUE;
	}
}

/*
 * Start the on time of the beacon interval
 */
void
wlc_radio_pwrsave_on_time_start(wlc_ap_info_t *ap, bool dtim)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint32 on_time, state;

	/* Update quite ie only the first time we enter power save mode.
	 * If we are entering power save for the first time set the count
	 * such that quiet time starts after 3 bcn intervals.
	 *
	 * state 0 - initial state / just exited pwr save
	 * state 1 - entering pwr save - sent quiet ie with count n
	 * state 2 - re-entering pwr save - sent quiet ie with count 1
	 * state 3 - in pwr save - radio on/off
	 *
	 * ---> 0 ----> 1 ------> 3
	 *      ^       |    ^    |
	 *      |       |    |    |
	 *      |       v    |    v
	 *      +<---------- 2 <--+
	 *      |                 |
	 *      |                 |
	 *      |                 |
	 *      +<----------------+
	 */
	state = appvt->radio_pwrsave.pwrsave_state;
	if (state == 0) {
		wlc_radio_pwrsave_update_quiet_ie(ap, 3);
		/* Enter power save state */
		appvt->radio_pwrsave.tbtt_skip = 3;
		appvt->radio_pwrsave.pwrsave_state = 1;
	}

	/* We are going to start radio on/off only after counting
	 * down tbtt_skip bcn intervals.
	 */
	if (appvt->radio_pwrsave.tbtt_skip-- > 0) {
		WL_INFORM(("wl%d: tbtt skip %d\n", wlc->pub->unit,
		           appvt->radio_pwrsave.tbtt_skip));
		return;
	}

	if (!dtim)
		return;

	if (appvt->radio_pwrsave.level) {
		appvt->radio_pwrsave.cncl_bcn = FALSE;
	}

	/* Schedule the timer to turn off the radio after on_time msec */
	on_time = appvt->radio_pwrsave.on_time * DOT11_TU_TO_US;
	on_time += ap->pre_tbtt_us;

	appvt->radio_pwrsave.tbtt_skip = ((on_time / DOT11_TU_TO_US) /
	                                          wlc->cfg->current_bss->beacon_period);
	on_time /= 1000;
	/* acc for extra phy and mac suspend delays, it seems to be huge. Need
	 * to extract out the exact delays.
	 */
	on_time += RADIO_PWRSAVE_TIMER_LATENCY;

	WL_INFORM(("wl%d: adding timer to disable phy after %d ms state %d, skip %d\n",
	           wlc->pub->unit, on_time, appvt->radio_pwrsave.pwrsave_state,
	           appvt->radio_pwrsave.tbtt_skip));

	/* In case pre-tbtt intr arrived before the timer that disables the radio */
	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);

	wl_add_timer(wlc->wl, appvt->radio_pwrsave.timer, on_time, FALSE);

	/* Update bcn and probe resp to send quiet ie starting from next
	 * tbtt intr.
	 */
	wlc_radio_pwrsave_update_quiet_ie(ap, appvt->radio_pwrsave.tbtt_skip);
}

/*
 * Start the off time of the beacon interval
 */
static void
wlc_radio_pwrsave_off_time_start(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint32 off_time;

	/* Calcuate the delay after which to schedule timer to turn on
	 * the radio. Also take in to account the phy enabling latency.
	 * We have to make sure the phy is enabled by the next pre-tbtt
	 * interrupt time.
	 */
	off_time = appvt->radio_pwrsave.off_time;

	off_time *= DOT11_TU_TO_US;
	off_time -= (ap->pre_tbtt_us + PHY_DISABLE_MAX_LATENCY_us);

	/* In power save state */
	appvt->radio_pwrsave.pwrsave_state = 3;
	off_time /= 1000;

	/* acc for extra phy and mac suspend delays, it seems to be huge. Need
	 * to extract out the exact delays.
	 */
	off_time -= RADIO_PWRSAVE_TIMER_LATENCY;

	WL_INFORM(("wl%d: add timer to enable phy after %d msec state %d\n",
	           wlc->pub->unit, off_time, appvt->radio_pwrsave.pwrsave_state));

	/* Schedule the timer to turn on the radio after off_time msec */
	wl_add_timer(wlc->wl, appvt->radio_pwrsave.timer, off_time, FALSE);
}

/*
 * Check whether we are in radio power save mode
 */
int
wlc_radio_pwrsave_in_power_save(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;

	return (appvt->radio_pwrsave.pwrsave.in_power_save);
}

/*
 * Enter radio power save
 */
void
wlc_radio_pwrsave_enter_mode(wlc_info_t *wlc, bool dtim)
{
	/* If AP is in radio power save mode, we need to start the duty
	 * cycle with TBTT
	 */
	if (AP_ENAB(wlc->pub) && RADIO_PWRSAVE_ENAB(wlc->ap) &&
	    wlc_radio_pwrsave_in_power_save(wlc->ap)) {
		wlc_radio_pwrsave_on_time_start(wlc->ap, dtim);
	}
}


/*
 * Exit out of the radio power save if we are in it
 */
void
wlc_radio_pwrsave_exit_mode(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;

	appvt->radio_pwrsave.pwrsave.quiet_time_counter = 0;
	appvt->radio_pwrsave.pwrsave.in_power_save = FALSE;
	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
	if (appvt->radio_pwrsave.radio_disabled) {
		wlc_bmac_radio_hw(wlc->hw, TRUE,
			(CHIPID(wlc->pub->sih->chip) == BCM5356_CHIP_ID) ?  TRUE : FALSE);
		appvt->radio_pwrsave.radio_disabled = FALSE;
	}
	appvt->radio_pwrsave.pwrsave_state = 0;
	appvt->radio_pwrsave.cncl_bcn = FALSE;
}

/*
 * Update the beacon with quiet IE
 */
static void
wlc_radio_pwrsave_update_quiet_ie(wlc_ap_info_t *ap, uint8 count)
{
#ifdef WL11H
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint32 duration;
	wlc_bsscfg_t *cfg = wlc->cfg;
	dot11_quiet_t quiet_cmd;

	if (WL11H_ENAB(wlc))
		return;

	duration = appvt->radio_pwrsave.off_time;
	duration *= DOT11_TU_TO_US;
	duration -= (ap->pre_tbtt_us + PHY_ENABLE_MAX_LATENCY_us +
	             PHY_DISABLE_MAX_LATENCY_us);
	duration /= DOT11_TU_TO_US;

	/* Setup the quiet command */
	quiet_cmd.period = 0;
	quiet_cmd.count = count;
	quiet_cmd.duration = (uint16)duration;
	quiet_cmd.offset = appvt->radio_pwrsave.on_time % cfg->current_bss->beacon_period;
	WL_INFORM(("wl%d: quiet cmd: count %d, dur %d, offset %d\n",
	            wlc->pub->unit, quiet_cmd.count,
	            quiet_cmd.duration, quiet_cmd.offset));

	wlc_quiet_do_quiet(wlc->quiet, cfg, &quiet_cmd);
#endif /* WL11H */
}

bool
wlc_radio_pwrsave_bcm_cancelled(const wlc_ap_info_t *ap)
{
	const wlc_ap_info_pvt_t *appvt = (const wlc_ap_info_pvt_t *)ap;

	return appvt->radio_pwrsave.cncl_bcn;
}

#endif /* RADIO_PWRSAVE */

/* ======================= IE mgmt routines ===================== */
/* ============================================================== */
/* Supported Rates IE */
static uint
wlc_assoc_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	return TLV_HDR_LEN + ftcbparm->assocresp.sup->count;
}

static int
wlc_assoc_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	bcm_write_tlv(DOT11_MNG_RATES_ID, ftcbparm->assocresp.sup->rates,
		ftcbparm->assocresp.sup->count, data->buf);

	return BCME_OK;
}

/* Extended Supported Rates IE */
static uint
wlc_assoc_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->assocresp.ext->count == 0) {
		return 0;
	}

	return TLV_HDR_LEN + ftcbparm->assocresp.ext->count;
}

static int
wlc_assoc_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	bcm_write_tlv(DOT11_MNG_EXT_RATES_ID, ftcbparm->assocresp.ext->rates,
		ftcbparm->assocresp.ext->count, data->buf);

	return BCME_OK;
}

/* SSID */
static int
wlc_assoc_parse_ssid_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif

	if (data->ie == NULL || data->ie_len <= TLV_BODY_OFF) {
		WL_ASSOC(("wl%d: %s attempted association with no SSID\n",
		          wlc->pub->unit, bcm_ether_ntoa(&ftpparm->assocreq.scb->ea, eabuf)));
		ftpparm->assocreq.status = DOT11_SC_FAILURE;
		return BCME_ERROR;
	}

	/* failure if the SSID does not match any active AP config */
	if (!WLC_IS_MATCH_SSID(wlc, cfg->SSID, &data->ie[TLV_BODY_OFF],
	                       cfg->SSID_len, data->ie[TLV_LEN_OFF])) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char ssidbuf[SSID_FMT_BUF_LEN];

		wlc_format_ssid(ssidbuf, &data->ie[TLV_BODY_OFF], data->ie[TLV_LEN_OFF]);
		WL_ASSOC(("wl%d: %s attempted association with incorrect SSID ID\"%s\"\n",
		          wlc->pub->unit, bcm_ether_ntoa(&ftpparm->assocreq.scb->ea, eabuf),
		          ssidbuf));
#endif
		ftpparm->assocreq.status = DOT11_SC_ASSOC_FAIL;
		return BCME_ERROR;
	}

	ftpparm->assocreq.status = DOT11_SC_SUCCESS;
	return BCME_OK;
}


static int
wlc_assoc_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_rateset_t *sup = ftpparm->assocreq.sup;

	bzero(sup, sizeof(*sup));

	if (data->ie == NULL || data->ie_len <= TLV_BODY_OFF) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char eabuf[ETHER_ADDR_STR_LEN];
		struct scb *scb = ftpparm->assocreq.scb;
		wlc_info_t *wlc = (wlc_info_t *)ctx;

		WL_ASSOC(("wl%d: %s attempted association with no Supported Rates IE\n",
		          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
#endif
		ftpparm->assocreq.status = DOT11_SC_ASSOC_RATE_MISMATCH;
		return BCME_ERROR;
	}

#ifdef BCMDBG
	if (data->ie[TLV_LEN_OFF] > WLC_NUMRATES) {
		wlc_info_t *wlc = (wlc_info_t *)ctx;

		WL_ERROR(("wl%d: %s: IE contains too many rates, truncate\n",
		          wlc->pub->unit, __FUNCTION__));
	}
#endif

	sup->count = MIN(data->ie[TLV_LEN_OFF], WLC_NUMRATES);
	bcopy(&data->ie[TLV_BODY_OFF], sup->rates, sup->count);

	ftpparm->assocreq.status = DOT11_SC_SUCCESS;
	return BCME_OK;
}

static int
wlc_assoc_parse_ext_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_rateset_t *ext = ftpparm->assocreq.ext;

	bzero(ext, sizeof(*ext));

	if (data->ie == NULL || data->ie_len <= TLV_BODY_OFF) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char eabuf[ETHER_ADDR_STR_LEN];
		struct scb *scb = ftpparm->assocreq.scb;
		wlc_info_t *wlc = (wlc_info_t *)ctx;

		WL_ASSOC(("wl%d: %s attempted association with no Extended Supported Rates IE\n",
		          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
#endif
		return BCME_OK;
	}

#ifdef BCMDBG
	if (data->ie[TLV_LEN_OFF] > WLC_NUMRATES) {
		wlc_info_t *wlc = (wlc_info_t *)ctx;

		WL_ERROR(("wl%d: %s: IE contains too many rates, truncate\n",
		          wlc->pub->unit, __FUNCTION__));
	}
#endif

	ext->count = MIN(data->ie[TLV_LEN_OFF], WLC_NUMRATES);
	bcopy(&data->ie[TLV_BODY_OFF], ext->rates, ext->count);

	ftpparm->assocreq.status = DOT11_SC_SUCCESS;
	return BCME_OK;
}

static int
wlc_assoc_parse_wps_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WSEC_SES_OW_ENABLED(cfg->wsec)) {
		bcm_tlv_t *wpsie = (bcm_tlv_t *)data->ie;
		struct scb *scb = ftpparm->assocreq.scb;

		if (wpsie == NULL)
			return BCME_OK;

		if (wpsie->len < 5) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif
			WL_ERROR(("wl%d: wlc_assocresp: unsupported request in WPS IE from %s\n",
			          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
			ftpparm->assocreq.status = DOT11_SC_ASSOC_FAIL;
			return BCME_ERROR;
		}

		ftpparm->assocreq.wps_ie = (uint8 *)wpsie;

		return wlc_scb_save_wpa_ie(wlc, scb, wpsie);
	}

	return BCME_OK;
}

/* ============================================================== */
/* ======================= IE mgmt routines ===================== */

#ifdef PSPRETEND
bool
wlc_ap_do_pspretend_probe(wlc_info_t *wlc, struct scb *scb, uint32 elapsed_time)
{
	uint8 rate_override;
	uint32 listen_in_ms = scb->listen * wlc->default_bss->beacon_period;

	/* this increments each time we are called from the pretend probe timer,
	 * approximately once per 10ms
	 */
	scb->ps_pretend_probe++;

	/* If a probe is still pending, don't send another one */
	if (scb->flags & SCB_PENDING_PROBE) {
		return FALSE;
	}

	/* As time goes by, we probe less often - this function is called
	 * once per timer interval (10ms) per SCB in pps state, so
	 * we return early if we decide to skip
	 */
	if ((elapsed_time >= listen_in_ms) &&
#ifdef BCMDBG
	        !(wlc->block_datafifo & DATA_BLOCK_PS)) {
#else
	        TRUE) {
#endif
		/* there's no more point to sending probes, the
		 * destination has probably died. Note that the TIM
		 * is set indicating ps state, so after pretend probing
		 * has completed, there is still the beacon to awaken the STA
		 */
		scb->ps_pretend &= ~PS_PRETEND_PROBING;
		if (scb->grace_attempts) {
			/* reset grace_attempts but indicate we have done some probing */
			scb->grace_attempts = 1;
		}
		return FALSE;
	}
	else if (wlc_scb_restrict_do_probe(scb)) {
		/* probing requested  */
	}
	else if (elapsed_time > 600) {
		/* after 0.6s, only probe every 5th timer, approx 50 ms */
		if ((scb->ps_pretend_probe % 5) != 0) {
			return FALSE;
		}
	}
	else if (elapsed_time > 300) {
		/* after 0.3s, only probe every 3rd timer, approx 30ms */
		if ((scb->ps_pretend_probe % 3) != 0) {
			return FALSE;
		}
	}
	else if (elapsed_time > 150) {
		/* after 0.15s, only probe every 2nd timer, approx 20ms */
		if ((scb->ps_pretend_probe % 2) != 0) {
			return FALSE;
		}
	}
	/* else we probe each time (10ms) */

	/* If there is fifo drain in process, we do not send the probe. Partly because
	 * sending a probe might interfere with the fifo drain, but mainly because
	 * even if the data probe was successful, we cannot exit ps pretend state
	 * whilst the fifo drain was still happening. So, waiting for the fifo drain
	 * to finish first is clean, predictable and consistent.
	 * Note that when the DATA_BLOCK_PS condition clears, a probe is sent regardless.
	 */
	if (wlc->block_datafifo & DATA_BLOCK_PS) {
		WL_PS(("wl%d.%d: %s: "MACF" DATA_BLOCK_PS pending %d pkts "
			   "time since pps %dms\n",
			   wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__,
			   ETHER_TO_MACF(scb->ea), TXPKTPENDTOT(wlc), elapsed_time));
		return FALSE;
	}

	/* use the lowest basic rate */
	rate_override = wlc_lowest_basicrate_get(scb->bsscfg);
	ASSERT(VALID_RATE(wlc, rate_override));

	WL_PS(("wl%d.%d: pps probe to "MACF" time since pps is %dms\n", wlc->pub->unit,
	        WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), ETHER_TO_MACF(scb->ea), elapsed_time));

	if (!wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea, rate_override,
		WLF_PSDONTQ, PRIO_8021D_VO, wlc_ap_sendnulldata_cb, NULL))
	{
		WL_ERROR(("wl%d.%d: %s: wlc_sendnulldata failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
		return FALSE;
	}

	scb->flags |= SCB_PENDING_PROBE;

	return TRUE;
}

void
wlc_ap_pspretend_probe(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_pps_info_t* pps_info;

	ASSERT(wlc);

	pps_info = wlc->pps_info;
	pps_info->is_ps_pretend_probe_timer_running = FALSE;

	/* double check in case driver went down since timer was scheduled */
	if (wlc->pub->up && !wlc->going_down) {
		struct scb_iter scbiter;
		struct scb *scb;
		bool        ps_pretend_probe_continue = FALSE;
		uint32      tsf_low = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_PS_PRETEND_PROBING(scb)) {
				uint32 pretend_elapsed = (tsf_low - scb->ps_pretend_start)/1000;
				ps_pretend_probe_continue = TRUE;
				wlc_ap_do_pspretend_probe(wlc, scb, pretend_elapsed);
			}
			else {
				scb->ps_pretend_probe = 0;
			}
		}

		/* retstart timer as long as at least one scb is in pretend state */
		if (ps_pretend_probe_continue) {
			/* 10 milliseconds */
			wl_add_timer(wlc->wl, pps_info->ps_pretend_probe_timer,
			             10, FALSE);
			pps_info->is_ps_pretend_probe_timer_running = TRUE;
		}
	}
}
#endif /* PSPRETEND */

ratespec_t
wlc_force_bcn_rspec(wlc_info_t *wlc)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*)wlc->ap;
	return appvt->force_bcn_rspec;
}

static int
wlc_force_bcn_rspec_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_ap_info_t *ap, ratespec_t rspec)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;

	/* new bcn rspec requested */
	if (rspec) {
		uint i;
		/* check rspec is valid */
		if (!wlc_valid_rate(wlc, rspec, CHSPEC_IS2G(cfg->current_bss->chanspec) ?
		    WLC_BAND_2G : WLC_BAND_5G, TRUE))
			return BCME_BADRATESET;

		/* check rspec is basic rate */
		for (i = 0; i < cfg->current_bss->rateset.count; i++) {
			if ((cfg->current_bss->rateset.rates[i] & WLC_RATE_FLAG) &&
			    (cfg->current_bss->rateset.rates[i] & RATE_MASK) == rspec) {
#ifdef WL11N
				if (IS_OFDM(rspec)) {
					if (WLCISNPHY(wlc->band) &&
					    wlc->stf->ss_opmode == PHY_TXC1_MODE_CDD) {
						rspec |= (1 << RSPEC_TXEXP_SHIFT);
					}
					if (WLCISHTPHY(wlc->band) ||
					    WLCISACPHY(wlc->band)) {
						uint ntx = wlc_stf_txchain_get(wlc, rspec);
						uint nss = wlc_ratespec_nss(rspec);
						rspec |= ((ntx - nss) << RSPEC_TXEXP_SHIFT);
					}
				}
#endif /* WL11N */
				appvt->force_bcn_rspec = rspec;
				break;
			}
		}
		if (i == cfg->current_bss->rateset.count)
			return BCME_UNSUPPORTED;
	}
	else
		appvt->force_bcn_rspec = (ratespec_t) 0;

	/* update beacon rate */
	wlc_update_beacon(wlc);

	return BCME_OK;
}

#ifdef USBAP
bool
wlc_wlancoex_on(wlc_info_t *wlc)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) wlc->ap;
	bool wlancoex_on = FALSE;

	if (appvt) {
		wlancoex_on = appvt->wlancoex;
	}

	return wlancoex_on;
}

static int
wlc_wlancoex_upd(wlc_info_t *wlc, wlc_ap_info_t *ap, bool coex_en)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) wlc->ap;
	char *var;
	int i, err = BCME_OK;
	uint16 coexcfg = 0;
	uint16 coexgpio = 0;
	wlc_bsscfg_t *cfg;

#ifdef WLC_HIGH_ONLY
	/* wlancoex slave shmem init */
	if ((var = getvar(wlc->pub->vars, "wlancoex_s")) != NULL) {
		if (coex_en) {
			coexcfg = M_WLCX_CONFIG_EN;
			coexgpio = (1 << bcm_strtoul(var, NULL, 0));
		}
	}
#else
	/* wlancoex master shmem init */
	if ((var = getvar(wlc->pub->vars, "wlancoex_m")) != NULL) {
		if (coex_en) {
			coexcfg = M_WLCX_CONFIG_EN | M_WLCX_CONFIG_MASTER;
			coexgpio = (1 << bcm_strtoul(var, NULL, 0));
		}
	}
#endif /* WLC_HIGH_ONLY */
	else {
		WL_ERROR(("wl%d: wlancoex param error. wlancoex_m:%s wlancoex_s:%s\n",
			wlc->pub->unit,
			getvar(wlc->pub->vars, "wlancoex_m"),
			getvar(wlc->pub->vars, "wlancoex_s")));
		return BCME_BADARG;

	}

	/* setup wlancoex driver flag */
	appvt->wlancoex = coex_en;
	/* setup wlancoex ucode mode flag */
	wlc_write_shm(wlc, M_WLCX_CONFIG, coexcfg);
	/* setup wlancoex ucode gpio value */
	wlc_write_shm(wlc, M_WLCX_GPIO_CONFIG, coexgpio);

	FOREACH_BSS(wlc, i, cfg) {
		if (BSSCFG_STA(cfg)) {
			uint pm = coex_en? PM_MAX: PM_OFF;
			err = wlc_ioctl(wlc, WLC_SET_PM, (void *)&pm, sizeof(pm), cfg->wlcif);

			if (err) {
				WL_ERROR(("wl%d: wlancoex set PM error %s\n",
					wlc->pub->unit,
					wl_ifname(wlc->wl, cfg->wlcif->wlif)));
				break;
			}
		}
	}

	return err;
}
#endif /* USBAP */
