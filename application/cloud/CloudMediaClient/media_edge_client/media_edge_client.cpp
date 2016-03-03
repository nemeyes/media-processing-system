#include "stdafx.h"
#include "CloudMediaClient.h"
#include "CloudMediaClientDlg.h"
#include "media_edge_client.h"

ic::media_edge_client::media_edge_client(CCloudMediaClientDlg * front)
	: _front(front)
{

}

ic::media_edge_client::~media_edge_client(void)
{

}

void ic::media_edge_client::assoc_completion_callback(void)
{
	_front->EnableConnectButton(FALSE);
	_front->EnableDisconnectButton(TRUE);
}

void ic::media_edge_client::leave_completion_callback(void)
{
	_front->EnableConnectButton(TRUE);
	_front->EnableDisconnectButton(FALSE);
}