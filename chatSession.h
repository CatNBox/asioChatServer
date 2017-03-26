#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include "chatProtocol.h"

class chatServer;

class chatSession
{
public:
	//¸â¹öÇÔ¼ö
	chatSession(int sessionID, boost::asio::io_service& ioServ, chatServer* pServer);
	~chatSession();

	void init();

	int getSessionID();
	void setName(const char* pName);
	const char* getName();
	boost::asio::ip::tcp::socket& getSocket();

	void postReceive();
	void postSend(const bool isInstantly, const int dataSize, char* pData);

	//¸â¹öº¯¼ö, »ó¼ö

private:
	//¸â¹öÇÔ¼ö
	void handle_receive(const boost::system::error_code& errCode, size_t bytes_transferred);
	void handle_write(const boost::system::error_code& errCode, size_t bytes_transferred);

	//¸â¹öº¯¼ö, »ó¼ö
	int mSessionID;
	std::string mName;
	boost::asio::ip::tcp::socket mSocket;

	chatServer* mServer;

	std::array<char, MAX_RECEIVE_BUFFER_LEN> mRecvBuf;
	int mPacketBufMarker;
	char mPacketBuf[MAX_RECEIVE_BUFFER_LEN * 2];
	std::deque<char*> mSendDataQue;
};