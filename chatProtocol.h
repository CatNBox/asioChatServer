#pragma once

//패킷 등 설정

const unsigned short PORT_NUMBER = 31400;

const int MAX_RECEIVE_BUFFER_LEN = 512;

const int MAX_NAME_LEN = 17;
const int MAX_MESSAGE_LEN = 129;

struct PACKET_HEADER
{
	short packetID;
	short packetSize;
};

//PKT_ ID정의
const short REQ_IN = 1;
const short RES_IN = 2;
const short REQ_CHAT = 6;
const short NOTICE_CHAT = 7;

struct PKT_REQ_IN : public PACKET_HEADER
{
	char szName[MAX_NAME_LEN];

	void init()
	{
		packetID = REQ_IN;
		packetSize = sizeof(PKT_REQ_IN);
		memset(szName, 0, MAX_NAME_LEN);
	}
};

struct PKT_RES_IN : public PACKET_HEADER
{
	bool isSuccess;

	void init()
	{
		packetID = RES_IN;
		packetSize = sizeof(PKT_RES_IN);
		isSuccess = false;
	}
};

struct PKT_REQ_CHAT : public PACKET_HEADER
{
	char szMessage[MAX_MESSAGE_LEN];

	void init()
	{
		packetID = REQ_CHAT;
		packetSize = sizeof(PKT_REQ_CHAT);
		memset(szMessage, 0, MAX_MESSAGE_LEN);
	}
};

struct PKT_NOTICE_CHAT : public PACKET_HEADER
{
	char szName[MAX_NAME_LEN];
	char szMessage[MAX_MESSAGE_LEN];

	void init()
	{
		packetID = NOTICE_CHAT;
		packetSize = sizeof(PKT_NOTICE_CHAT);
		memset(szName, 0, MAX_NAME_LEN);
		memset(szMessage, 0, MAX_MESSAGE_LEN);
	}
};