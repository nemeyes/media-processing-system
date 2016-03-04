#pragma once

#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <atlbase.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>
#include <wmcontainer.h>
#include <initguid.h>
#include <mferror.h>

#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }
#define RETURN_ON_FAIL(value) if(FAILED(value)) return value;

const IID BORROWED_IID_IMFDXGIDeviceManager = { 0xeb533d5d, 0x2db6, 0x40f8, { 0x97, 0xa9, 0x49, 0x46, 0x92, 0x01, 0x4f, 0x07 } };
const GUID BORROWED_MF_SA_D3D11_AWARE = { 0x206b4fc8, 0xfcf9, 0x4c51, { 0xaf, 0xe3, 0x97, 0x64, 0x36, 0x9e, 0x33, 0xa0 } };

