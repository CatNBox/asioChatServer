#include "chatSession.h"
#include "chatServer.h" //����� Ŭ���� �����ϰ�, cpp�� �ش� Ŭ������ ����� ��Ŭ��������ν� ��ȯ���� ȸ��

//���� ������
chatSession::chatSession(int sessionID, boost::asio::io_service & ioServ, chatServer * pServer)
	:mSocket(ioServ)		//asio�� ������ io_service�� ���� �����Ǿ���Ѵ�
	,mSessionID(sessionID)	//����ID�� ����� int�̱⶧���� �Լ������� �ʱ�ȭ�ϴ� �͵� ����.
	,mServer(pServer)		//chatServer Ŭ��������Ʈ�̹Ƿ� �Լ������� �ʱ�ȭ ����.
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
		boost::asio::buffer(mRecvBuf),						//Ŭ���̾�Ʈ���� ���� ���� mRecvBuf��
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

	//asio ǥ�� �����Լ� : ����, ����, �ڵ鷯 �� ���ڸ� ����
	//�ڵ鷯 - ���Ⱑ �Ϸ�� �� ȣ��Ǵ� �Լ�, �ڵ鷯�� �ݵ�� error_code, bytes_transferred�� ���ڷ� ������Ѵ�.
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
			std::cout << "Ŭ���̾�Ʈ�� ������ ���������ϴ�" << std::endl;
		}
		else
		{
			std::cout << "error No : " << errCode.value() << " error Message: " << errCode.message() << std::endl;
		}

		mServer->closeSession(mSessionID);
	}
	else
	{
		//mRecvBuf�� ���� ���� mPacketBuf�� ����
		//array.data() - array ù��° ����� �ּҸ� ��ȯ
		memcpy(&mPacketBuf[mPacketBufMarker], mRecvBuf.data(), bytes_transferred);
		int nPacketData = mPacketBufMarker + bytes_transferred;
		int nReadData = 0;

		//������ ���̰� 0���� ũ��
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
	//���� �Ϸ� �� ȣ��Ǵ� �Լ���
	//������ �����͸� ť���� �����ϰ� ť�� ������� �ʴٸ� ���� �����͸� ��Ŷȭ�Ͽ� postSend�� ȣ��
	delete[] mSendDataQue.front();
	mSendDataQue.pop_front();

	if (mSendDataQue.empty() == false)
	{
		char* pData = mSendDataQue.front();
		PACKET_HEADER* pHeader = (PACKET_HEADER*)pData;
		postSend(true, pHeader->packetSize, pData);
	}
}
