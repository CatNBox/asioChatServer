#pragma once
#include "chatSession.h"

class chatServer
{
public:
	//member function
	chatServer(boost::asio::io_service& ioServ);
	~chatServer();
	void init(const int nMaxSessionCnt);
	void start();
	void closeSession(const int nSessionID);
	void processPacket(const int nSessionID, const char* pData);

	//member variable, constant

private:
	//member function
	bool postAccept();
	void handle_accept(chatSession* pSession, const boost::system::error_code& errCode);
	
	//member variable, constant
	int mSeqNum;
	bool mIsAceepting;
	boost::asio::ip::tcp::acceptor mAcceptor;
	std::vector<chatSession*> mSessionList;
	std::deque<int> mSessionQue;

};