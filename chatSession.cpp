#include "chatSession.h"
#include "chatServer.h" //헤더에 클래스 선언하고, cpp에 해당 클래스의 헤더를 인클루드함으로써 순환참조 회피

//세션 생성자
chatSession::chatSession(int sessionID, boost::asio::io_service & ioServ, chatServer * pServer)
	:mSocket(ioServ)		//asio의 소켓은 io_service를 갖고 생성되어야한다
	,mSessionID(sessionID)	//세션ID는 평범한 int이기때문에 함수내에서 초기화하는 것도 가능.
	,mServer(pServer)		//chatServer 클래스포인트이므로 함수내에서 초기화 가능.
{
}

chatSession::~chatSession()
{
	while (mSendDataQue.empty() == false)
	{
		delete[] mSendDataQue.front();
		mSendDataQue.pop_front();
	}
}

void chatSession::init()
{
	mPacketBufMarker = 0;
}

int chatSession::getSessionID()
{
	return mSessionID;
}

void chatSession::setName(const char * pName)
{
	mName = pName;
}

const char * chatSession::getName()
{
	return mName.c_str();
}

boost::asio::ip::tcp::socket & chatSession::getSocket()
{
	return mSocket;
}

void chatSession::postReceive()
{
	mSocket.async_read_some(
		boost::asio::buffer(mRecvBuf),						//클라이언트에서 받은 내용 mRecvBuf에
		boost::bind(&chatSession::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void chatSession::postSend(const bool isInstantly, const int dataSize, char * pData)
{
	char* pSendData = nullptr;

	if (isInstantly == false)
	{
		pSendData = new char[dataSize];
		memcpy(pSendData, pData, dataSize);

		mSendDataQue.push_back(pSendData);
	}
	else
	{
		pSendData = pData;
	}

	if (isInstantly == false && mSendDataQue.size() > 1)
	{
		return;
	}

	//asio 표준 전송함수 : 소켓, 버퍼, 핸들러 를 인자를 갖음
	//핸들러 - 쓰기가 완료된 후 호출되는 함수, 핸들러는 반드시 error_code, bytes_transferred를 인자로 갖어야한다.
	boost::asio::async_write(mSocket,
		boost::asio::buffer(pSendData, dataSize),		
		boost::bind(&chatSession::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void chatSession::handle_receive(const boost::system::error_code & errCode, size_t bytes_transferred)
{
	if (errCode)
	{
		if (errCode == boost::asio::error::eof)
		{
			std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
		}
		else
		{
			std::cout << "error No : " << errCode.value() << " error Message: " << errCode.message() << std::endl;
		}

		mServer->closeSession(mSessionID);
	}
	else
	{
		//mRecvBuf에 받은 내용 mPacketBuf에 복사
		//array.data() - array 첫번째 요소의 주소를 반환
		memcpy(&mPacketBuf[mPacketBufMarker], mRecvBuf.data(), bytes_transferred);
		int nPacketData = mPacketBufMarker + bytes_transferred;
		int nReadData = 0;

		//데이터 길이가 0보다 크면
		while (nPacketData > 0)
		{
			if (nPacketData < sizeof(PACKET_HEADER))
			{
				break;
			}

			PACKET_HEADER* pHeader = (PACKET_HEADER*)&mPacketBuf[nReadData];

			if (pHeader->packetSize <= nPacketData)
			{
				mServer->processPacket(mSessionID, &mPacketBuf[nReadData]);

				nPacketData -= pHeader->packetSize;
				nReadData += pHeader->packetSize;
			}
			else
			{
				break;
			}
		}

		if (nPacketData > 0)
		{
			char tempBuf[MAX_RECEIVE_BUFFER_LEN] = { 0, };
			memcpy(&tempBuf[0], &mPacketBuf[nReadData], nPacketData);
			memcpy(&mPacketBuf[0], &tempBuf[0], nPacketData);
		}

		mPacketBufMarker = nPacketData;

		postReceive();
	}
}

void chatSession::handle_write(const boost::system::error_code & errCode, size_t bytes_transferred)
{
	//전송 완료 후 호출되는 함수로
	//전송한 데이터를 큐에서 삭제하고 큐가 비어있지 않다면 다음 데이터를 패킷화하여 postSend를 호출
	delete[] mSendDataQue.front();
	mSendDataQue.pop_front();

	if (mSendDataQue.empty() == false)
	{
		char* pData = mSendDataQue.front();
		PACKET_HEADER* pHeader = (PACKET_HEADER*)pData;
		postSend(true, pHeader->packetSize, pData);
	}
}
