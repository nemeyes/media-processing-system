#ifndef _PTZ_SAMSUNG_DEVICE_INFO_H_
#define _PTZ_SAMSUNG_DEVICE_INFO_H_


enum PTZ_SAMSUNG_DEVICE_TYPE_T
{
	SNP_3120VH = 0,
	SNP_5200H,
	SNP_1000A,
	SNP_6200RH,
};

enum PTZ_SAMSUNG_SNP_3120VH_PROTCOL_TYPE_T
{
	SNP_3120VH_VNP,
};

enum PTZ_SAMSUNG_SNP_5200H_PROTCOL_TYPE_T
{
	SNP_5200H_SUNAPI,
};

enum PTZ_SAMSUNG_SNP_1000A_PROTCOL_TYPE_T
{
	SNP_1000A_SOCKET,
};

enum PTZ_SAMSUNG_SNP_6200RH_PROTCOL_TYPE_T
{
	SNP_6200RH_SUNAPI,
};

enum PTZ_SAMSUNG_SNP_3120VH_VNP_FW_VERISON_T
{
	SNP_3120VH_VNP_V_1_29_130107 = 0,
};

enum PTZ_SAMSUNG_SNP_5200H_SUNAPI_FW_VERISON_T
{
	SNP_5200H_SUNAPI_V_1_04_111116 = 0,
};

enum PTZ_SAMSUNG_SNP_1000A_SOCKET_FW_VERISON_T
{
	SNP_1000A_SOCKET_V_2_8_4 = 0,
};

enum PTZ_SAMSUNG_SNP_6200RH_SUNAPI_FW_VERISON_T
{
	SNP_6200RH_SUNAPI_V_1_11_131002 = 0,
};


#endif