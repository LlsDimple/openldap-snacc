/* Copyright 2004 IBM Corporation
 * All rights reserved.
 * Redisribution and use in source and binary forms, with or without
 * modification, are permitted only as  authorizd by the OpenLADP
 * Public License.
 */
/* ACKNOWLEDGEMENTS
 * This work originally developed by Sang Seok Lim
 * 2004/06/18	03:20:00	slim@OpenLDAP.org
 */


#include "asn-gser.h"

/*
 * VisibleString is not necessary to be translated to UTF8
 * RFC 3641
 * StringValue	    = dquote *SafeUTF8Character dquote
 * SafeUTFCharacter = %x00-21 / %x23-7f /
 *                  = dquote dquote /
 *                  = %xc0-DF %x80-BF /
 *                  = %xE0-EF 2(%x80-BF) /
 *                  = %xF0-E7 3(%x80-BF) /
 */
#define GEncVisibleStringContent GEncAsnOctsContent
#define GDecVisibleStringContent GDecAsnOctsContent
