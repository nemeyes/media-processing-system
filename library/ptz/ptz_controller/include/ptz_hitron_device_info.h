#ifndef _PTZ_HITRON_DEVICE_INFO_H_
#define _PTZ_HITRON_DEVICE_INFO_H_

enum PTZ_HITRON_DEVICE_TYPE_T
{
	NFX_12053B1 = 0,
	HEV0118H,
};

enum PTZ_HITRON_NFX_12053B1_PROTCOL_TYPE_T
{
	NFX_12053B1_ONVIF,
};

enum PTZ_HITRON_HEV0118H_PROTCOL_TYPE_T
{
	HEV0118H_ONVIF,
	HEV0118H_PELCO_D,
};

enum PTZ_HITRON_NFX_12053B1_ONVIF_FW_VERISON_T
{
	NFX_12053B1_ONVIF_V_1_0_4 = 0,
	NFX_12053B1_ONVIF_V_1_2_0_X1_RELEASE,
	NFX_12053B1_ONVIF_V_1_2_3_X1_RELEASE,
	NFX_12053B1_ONVIF_V_1_3_4_X1_RELEASE,
	NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE,
};

enum PTZ_HITRON_HEV0118H_ONVIF_FW_VERISON_T
{
	HEV0118H_ONVIF_V_2_9_0 = 0,
};

enum PTZ_HITRON_HEV0118H_PELCO_D_FW_VERISON_T
{
	HEV0118H_PELCO_D_V_3_0_0 = 0,
};

#endif