/*
 * BCM PSTA protocol processing
 * shared code for PSTA protocol processing in dhd & wl
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: bcm_psta.h 469476 2014-04-15 12:39:01Z vel $
 */
#ifndef _bcm_psta_h_
#define _bcm_psta_h_

typedef int32 (*bcm_psta_dhcp_proc_cb)(void *psta_cb, void *cfg_cb,
                  void **p, uint8 *uh, uint8 *dhcph, uint16 dhcplen,
                  uint16 port, uint8 **shost, bool tx, bool is_bcmc);

#define BCM_EA_CMP(e1, e2) \
	(!((((uint16 *)(e1))[0] ^ ((uint16 *)(e2))[0]) | \
	   (((uint16 *)(e1))[1] ^ ((uint16 *)(e2))[1]) | \
	   (((uint16 *)(e1))[2] ^ ((uint16 *)(e2))[2])))


#define	BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, ea) \
do { \
	ASSERT(BCM_EA_CMP((cli_mac), (ea))); \
	*((uint8 *)(ea)) = *((uint8 *)(mod_mac)); \
} while (0)

#define	BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, ea) \
do { \
	ASSERT(BCM_EA_CMP((mod_mac), (ea))); \
	*((uint8 *)(ea)) = *((uint8 *)(cli_mac)); \
} while (0)

#define	BCM_PSTA_IS_ALIAS(mod_mac, ea)	BCM_EA_CMP((mod_mac), (ea))

int32 bcm_psta_ether_type(osl_t *osh, uint8 *eh, uint16 *et, uint16 *ip_off,
	bool *is_1x);
int32 bcm_psta_arp_proc(uint8 *ah, uint8 *cli_mac, uint8 *mod_mac, bool tx);
int32 bcm_psta_udp_proc(void *psta, void *cfg, void **p, uint8 *ih, uint8 *uh,
                  uint8 **shost, bool tx, bool is_bcmc, bcm_psta_dhcp_proc_cb psta_dhcp_proc_cb);
int32 bcm_psta_dhcp_proc(uint16 port, uint8 *uh, uint8 *dhcph, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc);
int32 bcm_psta_icmp6_proc(uint8 *ih, uint8 *icmph, uint16 icmplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx);
#endif /* _bcm_psta_h_ */