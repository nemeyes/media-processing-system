#pragma once

extern void css_disckey(unsigned char * dkey, unsigned char * pkey);
extern void css_titlekey(unsigned char * tkey, unsigned char * dkey);
extern void css_descramble(unsigned char * sector, unsigned char * tkey);

extern unsigned char g_PlayerKeys[][6];
extern int g_nPlayerKeys;