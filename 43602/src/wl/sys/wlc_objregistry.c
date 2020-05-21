/*
 * WLC Object Registry API Implementation
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_objregistry.c sriramn $
 */

/**
 * @file
 * @brief
 * With the rise of RSDB (Real Simultaneous Dual Band), the need arose to support two 'wlc'
 * structures, with the requirement to share data between the two. To meet that requirement, a
 * simple 'key=value' mechanism is introduced.
 *
 * WLC Object Registry provides mechanisms to share data across WLC instances in RSDB
 * The key-value pairs (enums/void *ptrs) to be stored in the "registry" are decided at design time.
 * Even the Non_RSDB (single instance) goes thru the Registry calls to have a unified interface.
 * But the Non_RSDB functions call have dummy/place-holder implementation managed using MACROS.
 *
 * The registry stores key=value in a simple array which is index-ed by 'key'
 * The registry also maintains a reference counter, which helps the caller in freeing the
 * 'value' associated with a 'key'
 * The registry stores objects as pointers represented by "void *" and hence a NULL value
 * indicates unused key
 *
 */



#ifdef WL_OBJ_REGISTRY

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_types.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_objregistry.h>

#ifdef BCMDBG
#define WL_OBJR_DBG(x) printf x
#else
#define WL_OBJR_DBG(x)
#endif

struct wlc_obj_registry {
	int count;
	void **value;
	uint8 *ref;
};


/* WLC Object Registry Public APIs */
wlc_obj_registry_t* wlc_obj_registry_alloc(osl_t *osh, int count)
{
	wlc_obj_registry_t *objr = NULL;
	if ((objr = MALLOCZ(osh, sizeof(wlc_obj_registry_t))) == NULL) {
			WL_ERROR(("wl %s: out of memory, malloced %d bytes\n",
				__FUNCTION__, MALLOCED(osh)));
	} else {
		objr->count = count;

		if ((objr->value =
			MALLOCZ(osh, sizeof(void*) * count)) == NULL) {
			WL_ERROR(("wl %s: out of memory, malloced %d bytes\n",
				__FUNCTION__, MALLOCED(osh)));
			wlc_obj_registry_free(objr, osh);
			return NULL;
		}

		if ((objr->ref =
			MALLOCZ(osh, sizeof(uint8) * count)) == NULL) {
			WL_ERROR(("wl %s: out of memory, malloced %d bytes\n",
				__FUNCTION__, MALLOCED(osh)));
			wlc_obj_registry_free(objr, osh);
			return NULL;
		}
	}
	WL_OBJR_DBG(("WLC_OBJR CREATED : %p\n", objr));

	return objr;
}

void wlc_obj_registry_free(wlc_obj_registry_t *objr, osl_t *osh)
{
	if (objr) {
#ifdef BCMDBG
		if (objr->value && objr->ref) {
			/* Check if some stale refs are still present */
			int i = 0;
			for (i = 0; i < objr->count; i++) {
				if (objr->ref[i] || objr->value[i]) {
					WL_ERROR(("key:%d ref:%d value:%p\n",
						i, objr->ref[i], objr->value[i]));
					ASSERT(0);
				}
			}
		}
#endif /* BCMDBG */
		if (objr->value)
			MFREE(osh, objr->value, sizeof(void*) * objr->count);
		if (objr->ref)
			MFREE(osh, objr->ref, sizeof(uint8) * objr->count);
		MFREE(osh, objr, sizeof(wlc_obj_registry_t));
	}
}

int wlc_obj_registry_set(wlc_obj_registry_t *objr, wlc_obj_registry_key_t key, void *value)
{
	if (objr) {
		WL_OBJR_DBG(("%s:%d:key[%d]=value[%p]\n", __FUNCTION__, __LINE__, (int)key, value));
		if ((key < 0) || (key >= objr->count)) return BCME_RANGE;
		objr->value[(int)key] = value;
		return BCME_OK;
	}
	return BCME_ERROR;
}

void* wlc_obj_registry_get(wlc_obj_registry_t *objr, wlc_obj_registry_key_t key)
{
	void *value = NULL;
	if (objr) {
		if ((key < 0) || (key >= objr->count)) return NULL;
		value = objr->value[(int)key];
		WL_OBJR_DBG(("%s:%d:key[%d]=value[%p]\n", __FUNCTION__, __LINE__, (int)key, value));
	}
	return value;
}

int wlc_obj_registry_ref(wlc_obj_registry_t *objr, wlc_obj_registry_key_t key)
{
	int ref = 0;
	if (objr && (key >= 0) && (key < objr->count)) {
#ifdef BCMDBG
		void *value = NULL;
		value = objr->value[(int)key];
#endif
		ref = ++(objr->ref[(int)key]);
		WL_OBJR_DBG(("%s:%d: key[%d]=value[%p]/REF[%d]\n",
			__FUNCTION__, __LINE__, (int)key, value, ref));
	}
	return ref;
}

int wlc_obj_registry_unref(wlc_obj_registry_t *objr, wlc_obj_registry_key_t key)
{
	int ref = 0;
	if (objr && (key >= 0) && (key < objr->count)) {
#ifdef BCMDBG
		void *value = NULL;
		value = objr->value[(int)key];
#endif
		ref = (objr->ref[(int)key]);
		if (ref > 0) {
			ref = --(objr->ref[(int)key]);
			WL_OBJR_DBG(("%s:%d: key[%d]=value[%p]/REF[%d]\n",
				__FUNCTION__, __LINE__, (int)key, value, ref));
		}
	}
	return ref;
}
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void wlc_dump_objr(wlc_obj_registry_t *objr, struct bcmstrbuf *b)
{
	int i = 0;
	WL_OBJR_DBG(("%s: %d \n", __FUNCTION__, __LINE__));
	if (b) {
		if (objr) {
		bcm_bprintf(b, "\nDumping Object Registry\n");
		bcm_bprintf(b, "Key\tValue\t\tRef\n");
		for (i = 0; i < objr->count; i++) {
			bcm_bprintf(b, "%d\t%p\t%d\n", i, objr->value[(int)i], objr->ref[(int)i]);
		}
		} else {
		}
	} else {
		WL_OBJR_DBG(("\nkey\tvalue\tref\n"));
		for (i = 0; i < objr->count; i++) {
			WL_OBJR_DBG(("%d\t%p\t%d\n", i, objr->value[(int)i], objr->ref[(int)i]));
		}
	}
}
#endif /* BCMDBG */

#endif /* WL_OBJ_REGISTRY */
