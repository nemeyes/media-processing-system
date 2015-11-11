/*
 * Support for ISO/IEC 14496-3:2001(E) AAC ADTS audio streams.
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the MPEG TS, PS and ES tools.
 *
 * The Initial Developer of the Original Code is Amino Communications Ltd.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Amino Communications Ltd, Swavesey, Cambridge UK
 *
 * ***** END LICENSE BLOCK *****
 */

#ifndef _adts_defns
#define _adts_defns

#include "audio_defns.h"
// AAC ADTS provides audio in frames of constant time

// Flags for ``read_next_adts_frame``
//
// Specify this flag to indicate that there is no emphasis field in the ADTS
// header. Generally, MPEG-2 ADTS audio (ID=0) has no emphasis field and
// MPEG-4 (ID=1) has emphasis, but some H.264/AAC streams have MPEG-4 ADTS
// with no emphasis and in those cases, you'll need this flag.
#define ADTS_FLAG_NO_EMPHASIS    (1<<0)
// Specify this flag to indicate that there is always an emphasis field, even
// if the ID says there isn't one - included for symmetry with NO_EMPHASIS.
#define ADTS_FLAG_FORCE_EMPHASIS (1<<1)

#endif // _adts_defns

// Local Variables:
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 2
// End:
// vim: set tabstop=8 shiftwidth=2 expandtab:
