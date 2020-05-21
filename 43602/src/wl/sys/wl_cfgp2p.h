/*
 * Linux cfgp2p driver
 *
 * Copyright (C) 2014, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: wl_cfgp2p.h 469293 2014-04-10 02:59:40Z $
 */
#ifndef _wl_cfgp2p_h_
#define _wl_cfgp2p_h_
#include <proto/802.11.h>
#include <proto/p2p.h>

/* dword align allocation */
#ifdef SROM12
#define WLC_IOCTL_MAXLEN    10000
#else
#define WLC_IOCTL_MAXLEN    8192
#endif /* SROM12 */

#if !defined(WLC_HIGH_ONLY)
#if defined(WLC_HIGH) && !defined(WLC_LOW)
#define WLC_HIGH_ONLY
#endif
#endif /* #if !defined(WLC_HIGH_ONLY) */

struct bcm_cfg80211;
extern u32 wl_cfgp2p_dbg_level;

typedef struct wifi_p2p_ie wifi_wfd_ie_t;
/* Enumeration of the usages of the BSSCFGs used by the P2P Library.  Do not
 * confuse this with a bsscfg index.  This value is an index into the
 * saved_ie[] array of structures which in turn contains a bsscfg index field.
 */
typedef enum {
	P2PAPI_BSSCFG_PRIMARY, /* maps to driver's primary bsscfg */
	P2PAPI_BSSCFG_DEVICE, /* maps to driver's P2P device discovery bsscfg */
	P2PAPI_BSSCFG_CONNECTION, /* maps to driver's P2P connection bsscfg */
	P2PAPI_BSSCFG_MAX
} p2p_bsscfg_type_t;

/* vendor ies max buffer length for probe response or beacon */
#define VNDR_IES_MAX_BUF_LEN	1400
/* normal vendor ies buffer length */
#define VNDR_IES_BUF_LEN 		512

/* Structure to hold all saved P2P and WPS IEs for a BSSCFG */
struct p2p_saved_ie {
	u8  p2p_probe_req_ie[VNDR_IES_BUF_LEN];
	u8  p2p_probe_res_ie[VNDR_IES_MAX_BUF_LEN];
	u8  p2p_assoc_req_ie[VNDR_IES_BUF_LEN];
	u8  p2p_assoc_res_ie[VNDR_IES_BUF_LEN];
	u8  p2p_beacon_ie[VNDR_IES_MAX_BUF_LEN];
	u32 p2p_probe_req_ie_len;
	u32 p2p_probe_res_ie_len;
	u32 p2p_assoc_req_ie_len;
	u32 p2p_assoc_res_ie_len;
	u32 p2p_beacon_ie_len;
};

struct p2p_bss {
	u32 bssidx;
	struct net_device *dev;
	struct p2p_saved_ie saved_ie;
	void *private_data;
};

struct p2p_info {
	bool on;    /* p2p on/off switch */
	bool scan;
	int16 search_state;
	bool vif_created;
	s8 vir_ifname[IFNAMSIZ];
	unsigned long status;
	struct ether_addr dev_addr;
	struct ether_addr int_addr;
	struct p2p_bss bss[P2PAPI_BSSCFG_MAX];
	struct timer_list listen_timer;
	wl_p2p_sched_t noa;
	wl_p2p_ops_t ops;
	wlc_ssid_t ssid;
};

#define MAX_VNDR_IE_NUMBER	5

struct parsed_vndr_ie_info {
	char *ie_ptr;
	u32 ie_len;	/* total length including id & length field */
	vndr_ie_t vndrie;
};

struct parsed_vndr_ies {
	u32 count;
	struct parsed_vndr_ie_info ie_info[MAX_VNDR_IE_NUMBER];
};

/* dongle status */
enum wl_cfgp2p_status {
	WLP2P_STATUS_DISCOVERY_ON = 0,
	WLP2P_STATUS_SEARCH_ENABLED,
	WLP2P_STATUS_IF_ADDING,
	WLP2P_STATUS_IF_DELETING,
	WLP2P_STATUS_IF_CHANGING,
	WLP2P_STATUS_IF_CHANGED,
	WLP2P_STATUS_LISTEN_EXPIRED,
	WLP2P_STATUS_ACTION_TX_COMPLETED,
	WLP2P_STATUS_ACTION_TX_NOACK,
	WLP2P_STATUS_SCANNING,
	WLP2P_STATUS_GO_NEG_PHASE,
	WLP2P_STATUS_DISC_IN_PROGRESS
};


#define wl_to_p2p_bss_ndev(cfg, type)		((cfg)->p2p->bss[type].dev)
#define wl_to_p2p_bss_bssidx(cfg, type)		((cfg)->p2p->bss[type].bssidx)
#define wl_to_p2p_bss_saved_ie(cfg, type)	((cfg)->p2p->bss[type].saved_ie)
#define wl_to_p2p_bss_private(cfg, type)		((cfg)->p2p->bss[type].private_data)
#define wl_to_p2p_bss(cfg, type)			((cfg)->p2p->bss[type])
#define wl_get_p2p_status(cfg, stat) ((!(cfg)->p2p_supported) ? 0 : \
		test_bit(WLP2P_STATUS_ ## stat, &(cfg)->p2p->status))
#define wl_set_p2p_status(cfg, stat) ((!(cfg)->p2p_supported) ? 0 : \
		set_bit(WLP2P_STATUS_ ## stat, &(cfg)->p2p->status))
#define wl_clr_p2p_status(cfg, stat) ((!(cfg)->p2p_supported) ? 0 : \
		clear_bit(WLP2P_STATUS_ ## stat, &(cfg)->p2p->status))
#define wl_chg_p2p_status(cfg, stat) ((!(cfg)->p2p_supported) ? 0 : \
	change_bit(WLP2P_STATUS_ ## stat, &(cfg)->p2p->status))
#define p2p_on(cfg) ((cfg)->p2p->on)
#define p2p_scan(cfg) ((cfg)->p2p->scan)
#define p2p_is_on(cfg) ((cfg)->p2p && (cfg)->p2p->on)

#define WL_CFGP2P_DBG_NONE    (0)
#define WL_CFGP2P_DBG_ERROR   (1 << 0)
#define WL_CFGP2P_DBG_INFO    (1 << 0)
#define WL_CFGP2P_DBG_TRACE   (1 << 0)
#define WL_CFGP2P_DBG_ACTION  (1 << 0)
#define WL_CFGP2P_DBG_DBG     (1 << 0)

#ifdef CUSTOMER_HW4
#define WL_CFGP2P_DBG_ERROR_TEXT    "CFGP2P-INFO2) "
#else
#define WL_CFGP2P_DBG_ERROR_TEXT    "CFGP2P-ERROR) "
#endif
#define WL_CFGP2P_DBG_INFO_TEXT     "CFGP2P-INFO) "
#define WL_CFGP2P_DBG_TRACE_TEXT    "CFGP2P-TRACE) "
#define WL_CFGP2P_DBG_ACTION_TEXT   "CFGP2P-ACTION) "
#define WL_CFGP2P_DBG_DBG_TEXT      "CFGP2P-DBG) "


#define CFGP2P_ERR(args) \
	do { \
		if (wl_cfgp2p_dbg_level & WL_CFGP2P_DBG_ERROR) { \
			printk(KERN_INFO WL_CFGP2P_DBG_ERROR_TEXT "%s : ", __func__); \
			printk args; \
		} \
	} while (0)
#define	CFGP2P_INFO(args) \
	do { \
		if (wl_cfgp2p_dbg_level & WL_CFGP2P_DBG_INFO) { \
			printk(KERN_INFO WL_CFGP2P_DBG_INFO_TEXT "%s : ", __func__); \
			printk args; \
		} \
	} while (0)

#define	CFGP2P_TRACE(args) \
	do { \
		if (wl_cfgp2p_dbg_level & WL_CFGP2P_DBG_TRACE) { \
			printk(KERN_DEBUG WL_CFGP2P_DBG_TRACE_TEXT "%s :", __func__); \
			printk args; \
		} \
	} while (0)

#define	CFGP2P_DBG(args) \
	do { \
		if (wl_cfgp2p_dbg_level & WL_CFGP2P_DBG_DBG) { \
			printk(KERN_DEBUG WL_CFGP2P_DBG_DBG_TEXT "%s :", __func__);	\
			printk args; \
		} \
	} while (0)

#define	CFGP2P_ACTION(args) \
	do { \
		if (wl_cfgp2p_dbg_level & WL_CFGP2P_DBG_ACTION) { \
			printk(KERN_DEBUG WL_CFGP2P_DBG_ACTION_TEXT "%s :", __func__); \
			printk args; \
		} \
	} while (0)

#define INIT_TIMER(timer, func, duration, extra_delay) \
	do { \
		init_timer(timer); \
		timer->function = func; \
		timer->expires = jiffies + msecs_to_jiffies(duration + extra_delay); \
		timer->data = (unsigned long) cfg; \
		add_timer(timer); \
	} while (0);

#if !defined(WL_CFG80211_P2P_DEV_IF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
#define WL_CFG80211_P2P_DEV_IF
#endif /* !WL_CFG80211_P2P_DEV_IF && (LINUX_VERSION >= VERSION(3, 8, 0)) */

#if defined(WL_ENABLE_P2P_IF) && (defined(WL_CFG80211_P2P_DEV_IF) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)))
#error Disable 'WL_ENABLE_P2P_IF', if 'WL_CFG80211_P2P_DEV_IF' is enabled \
	or kernel version is 3.8.0 or above
#endif /* WL_ENABLE_P2P_IF && (WL_CFG80211_P2P_DEV_IF || (LINUX_VERSION >= VERSION(3, 8, 0))) */

#if !defined(WLP2P) && (defined(WL_ENABLE_P2P_IF) || defined(WL_CFG80211_P2P_DEV_IF))
#error WLP2P not defined
#endif /* !WLP2P && (WL_ENABLE_P2P_IF || WL_CFG80211_P2P_DEV_IF) */

#if defined(WL_CFG80211_P2P_DEV_IF)
#define bcm_struct_cfgdev	struct wireless_dev
#else
#define bcm_struct_cfgdev	struct net_device
#endif /* WL_CFG80211_P2P_DEV_IF */

extern void
wl_cfgp2p_listen_expired(unsigned long data);
extern bool
wl_cfgp2p_is_pub_action(void *frame, u32 frame_len);
extern bool
wl_cfgp2p_is_p2p_action(void *frame, u32 frame_len);
extern bool
wl_cfgp2p_is_gas_action(void *frame, u32 frame_len);
extern void
wl_cfgp2p_print_actframe(bool tx, void *frame, u32 frame_len, u32 channel);
extern s32
wl_cfgp2p_init_priv(struct bcm_cfg80211 *cfg);
extern void
wl_cfgp2p_deinit_priv(struct bcm_cfg80211 *cfg);
extern s32
wl_cfgp2p_set_firm_p2p(struct bcm_cfg80211 *cfg);
extern s32
wl_cfgp2p_set_p2p_mode(struct bcm_cfg80211 *cfg, u8 mode,
            u32 channel, u16 listen_ms, int bssidx);
extern s32
wl_cfgp2p_ifadd(struct bcm_cfg80211 *cfg, struct ether_addr *mac, u8 if_type,
            chanspec_t chspec);
extern s32
wl_cfgp2p_ifdisable(struct bcm_cfg80211 *cfg, struct ether_addr *mac);
extern s32
wl_cfgp2p_ifdel(struct bcm_cfg80211 *cfg, struct ether_addr *mac);
extern s32
wl_cfgp2p_ifchange(struct bcm_cfg80211 *cfg, struct ether_addr *mac, u8 if_type, chanspec_t chspec);

extern s32
wl_cfgp2p_ifidx(struct bcm_cfg80211 *cfg, struct ether_addr *mac, s32 *index);

extern s32
wl_cfgp2p_init_discovery(struct bcm_cfg80211 *cfg);
extern s32
wl_cfgp2p_enable_discovery(struct bcm_cfg80211 *cfg, struct net_device *dev, const u8 *ie,
	u32 ie_len);
extern s32
wl_cfgp2p_disable_discovery(struct bcm_cfg80211 *cfg);
extern s32
wl_cfgp2p_escan(struct bcm_cfg80211 *cfg, struct net_device *dev, u16 active, u32 num_chans,
	u16 *channels,
	s32 search_state, u16 action, u32 bssidx, struct ether_addr *tx_dst_addr);

extern s32
wl_cfgp2p_act_frm_search(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	s32 bssidx, s32 channel, struct ether_addr *tx_dst_addr);

extern wpa_ie_fixed_t *
wl_cfgp2p_find_wpaie(u8 *parse, u32 len);

extern wpa_ie_fixed_t *
wl_cfgp2p_find_wpsie(u8 *parse, u32 len);

extern wifi_p2p_ie_t *
wl_cfgp2p_find_p2pie(u8 *parse, u32 len);

extern wifi_wfd_ie_t *
wl_cfgp2p_find_wfdie(u8 *parse, u32 len);
extern s32
wl_cfgp2p_set_management_ie(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 bssidx,
            s32 pktflag, const u8 *vndr_ie, u32 vndr_ie_len);
extern s32
wl_cfgp2p_clear_management_ie(struct bcm_cfg80211 *cfg, s32 bssidx);

extern s32
wl_cfgp2p_find_idx(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 *index);
extern struct net_device *
wl_cfgp2p_find_ndev(struct bcm_cfg80211 *cfg, s32 bssidx);
extern s32
wl_cfgp2p_find_type(struct bcm_cfg80211 *cfg, s32 bssidx, s32 *type);


extern s32
wl_cfgp2p_listen_complete(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
extern s32
wl_cfgp2p_discover_listen(struct bcm_cfg80211 *cfg, s32 channel, u32 duration_ms);

extern s32
wl_cfgp2p_discover_enable_search(struct bcm_cfg80211 *cfg, u8 enable);

extern s32
wl_cfgp2p_action_tx_complete(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);

extern s32
wl_cfgp2p_tx_action_frame(struct bcm_cfg80211 *cfg, struct net_device *dev,
	wl_af_params_t *af_params, s32 bssidx);

extern void
wl_cfgp2p_generate_bss_mac(struct ether_addr *primary_addr, struct ether_addr *out_dev_addr,
            struct ether_addr *out_int_addr);

extern void
wl_cfg80211_change_ifaddr(u8* buf, struct ether_addr *p2p_int_addr, u8 element_id);
extern bool
wl_cfgp2p_bss_isup(struct net_device *ndev, int bsscfg_idx);

extern s32
wl_cfgp2p_bss(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 bsscfg_idx, s32 up);


extern s32
wl_cfgp2p_supported(struct bcm_cfg80211 *cfg, struct net_device *ndev);

extern s32
wl_cfgp2p_down(struct bcm_cfg80211 *cfg);

extern s32
wl_cfgp2p_set_p2p_noa(struct bcm_cfg80211 *cfg, struct net_device *ndev, char* buf, int len);

extern s32
wl_cfgp2p_get_p2p_noa(struct bcm_cfg80211 *cfg, struct net_device *ndev, char* buf, int len);

extern s32
wl_cfgp2p_set_p2p_ps(struct bcm_cfg80211 *cfg, struct net_device *ndev, char* buf, int len);

extern u8 *
wl_cfgp2p_retreive_p2pattrib(void *buf, u8 element_id);

extern u8*
wl_cfgp2p_find_attrib_in_all_p2p_Ies(u8 *parse, u32 len, u32 attrib);

extern u8 *
wl_cfgp2p_retreive_p2p_dev_addr(wl_bss_info_t *bi, u32 bi_length);

extern s32
wl_cfgp2p_register_ndev(struct bcm_cfg80211 *cfg);

extern s32
wl_cfgp2p_unregister_ndev(struct bcm_cfg80211 *cfg);

extern bool
wl_cfgp2p_is_ifops(const struct net_device_ops *if_ops);

#if defined(WL_CFG80211_P2P_DEV_IF)
extern struct wireless_dev *
wl_cfgp2p_add_p2p_disc_if(struct bcm_cfg80211 *cfg);

extern int
wl_cfgp2p_start_p2p_device(struct wiphy *wiphy, struct wireless_dev *wdev);

extern void
wl_cfgp2p_stop_p2p_device(struct wiphy *wiphy, struct wireless_dev *wdev);

extern int
wl_cfgp2p_del_p2p_disc_if(struct wireless_dev *wdev, struct bcm_cfg80211 *cfg);
#endif /* WL_CFG80211_P2P_DEV_IF */

#if defined(WLC_HIGH_ONLY)
extern int
wl_cfgp2p_priv_ioctl(struct net_device *net, struct ifreq *ifr, int cmd);
#endif

/* WiFi Direct */
#define SOCIAL_CHAN_1 1
#define SOCIAL_CHAN_2 6
#define SOCIAL_CHAN_3 11
#define IS_P2P_SOCIAL_CHANNEL(channel) ((channel == SOCIAL_CHAN_1) || \
					(channel == SOCIAL_CHAN_2) || \
					(channel == SOCIAL_CHAN_3))
#define SOCIAL_CHAN_CNT 3
#define AF_PEER_SEARCH_CNT 2
#define WL_P2P_WILDCARD_SSID "DIRECT-"
#define WL_P2P_WILDCARD_SSID_LEN 7
#define WL_P2P_INTERFACE_PREFIX "p2p"
#define WL_P2P_TEMP_CHAN 11

/* If the provision discovery is for JOIN operations,
 * or the device discoverablity frame is destined to GO
 * then we need not do an internal scan to find GO.
 */
#define IS_ACTPUB_WITHOUT_GROUP_ID(p2p_ie, len) \
	(wl_cfgp2p_retreive_p2pattrib(p2p_ie, P2P_SEID_GROUP_ID) == NULL)

#define IS_GAS_REQ(frame, len) (wl_cfgp2p_is_gas_action(frame, len) && \
					((frame->action == P2PSD_ACTION_ID_GAS_IREQ) || \
					(frame->action == P2PSD_ACTION_ID_GAS_CREQ)))

#define IS_P2P_PUB_ACT_RSP_SUBTYPE(subtype) ((subtype == P2P_PAF_GON_RSP) || \
							((subtype == P2P_PAF_GON_CONF) || \
							(subtype == P2P_PAF_INVITE_RSP) || \
							(subtype == P2P_PAF_PROVDIS_RSP)))
#define IS_P2P_SOCIAL(ch) ((ch == SOCIAL_CHAN_1) || (ch == SOCIAL_CHAN_2) || (ch == SOCIAL_CHAN_3))
#define IS_P2P_SSID(ssid, len) (!memcmp(ssid, WL_P2P_WILDCARD_SSID, WL_P2P_WILDCARD_SSID_LEN) && \
					(len == WL_P2P_WILDCARD_SSID_LEN))
#endif				/* _wl_cfgp2p_h_ */
