#include "chatServer.h"

chatServer::chatServer(boost::asio::io_service & ioServ)
	:mAcceptor(ioServ, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
	, mIsAceepting(false)
{
}

chatServer::~chatServer()
{
	for (size_t i = 0; i < mSessionList.size(); i++)
	{
		if (mSessionList[i]->getSocket().is_open())
		{
			mSessionList[i]->getSocket().close();
		}

		delete mSessionList[i];
	}
}

void chatServer::init(const int nMaxSessionCnt)
{
	//세션풀 생성
	for (int i = 0; i < nMaxSessionCnt; i++)
	{
		chatSession* pSession = new chatSession(i, mAcceptor.get_io_service(), this);
		mSessionList.push_back(pSession);
		mSessionQue.push_back(i);
	}
}

void chatServer::start()
{
	std::cout << "서버 시작......." << std::endl;
	postAccept();
}

void chatServer::closeSession(const int nSessionID)
{
	std::cout << "클라이언트 접속 종료. 세션 ID : " << nSessionID << std::endl;

	mSessionList[nSessionID]->getSocket().close();
	mSessionQue.push_back(nSessionID);

	if (mIsAceepting == false)
	{
		postAccept();
	}
}

void chatServer::processPacket(const int nSessionID, const char * pData)
{
	PACKET_HEADER* pHeader = (PACKET_HEADER*)pData;

	switch (pHeader->packetID)
		{
		case REQ_IN:	//로그인
		{
			PKT_REQ_IN* pPacket = (PKT_REQ_IN*)pData;
			mSessionList[nSessionID]->setName(pPacket->szName);

			std::cout << "클라이언트 로그인 성공 Name : " << mSessionList[nSessionID]->getName() << std::endl;

			PKT_RES_IN sendPkt;	//로그인 답장
			sendPkt.init();
			sendPkt.isSuccess = true;

			mSessionList[nSessionID]->postSend(false, sendPkt.packetSize, (char*)&sendPkt);
		}
		break;
	case REQ_CHAT:		//채팅메세지
		{
			PKT_REQ_CHAT* pPacket = (PKT_REQ_CHAT*)pData;

			PKT_NOTICE_CHAT sendPkt;
			sendPkt.init();
			strncpy_s(sendPkt.szName, MAX_NAME_LEN, mSessionList[nSessionID]->getName(), MAX_NAME_LEN - 1);
			strncpy_s(sendPkt.szMessage, MAX_MESSAGE_LEN, pPacket->szMessage, MAX_MESSAGE_LEN - 1);

			size_t nTotSessionCnt = mSessionList.size();

			//각 세션에 받은 메세지 전달
			for (size_t i = 0; i < nTotSessionCnt; i++)
			{
				if (mSessionList[i]->getSocket().is_open())
				{
					mSessionList[i]->postSend(false, sendPkt.packetSize, (char*)&sendPkt);
				}
			}
		}
		break;
	}

	return;
}

bool chatServer::postAccept()
{
	if (mSessionQue.empty())
	{
		std::cout << "세션 모두 사용중. 빈 세션이 없습니다." << std::endl;
		mIsAceepting = false;
		return false;
	}

	mIsAceepting = true;
	int nSessionID = mSessionQue.front();

	mSessionQue.pop_front();

	mAcceptor.async_accept(mSessionList[nSessionID]->getSocket(),
		boost::bind(&chatServer::handle_accept,
			this,
			mSessionList[nSessionID],
			boost::asio::placeholders::error)
	);

	return true;
}

void chatServer::handle_accept(chatSession * pSession, const boost::system::error_code & errCode)
{
	if (!errCode)
	{
		std::cout << "클라이언트 접속 성공. SessionID : " << pSession->getSessionID() << std::endl;

		pSession->init();
		pSession->postReceive();

		postAccept();
	}
	else if (errCode == boost::asio::error::eof)
	{
		std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
	}
	else
	{
		std::cout << "error Mo : " << errCode.value() << " error Message : " << errCode.message() << std::endl;
	}
}
