#include "chatServer.h"

//�ִ����Ӽ���
const int MAX_SESSION_COUNT = 100;

int main()
{
	//asio �ʼ�
	boost::asio::io_service ioServ;

	//charServer Ŭ���� ����
	chatServer clsServer(ioServ);
	clsServer.init(MAX_SESSION_COUNT);
	clsServer.start();

	ioServ.run();

	std::cout << "��Ʈ��ũ ���� ����" << std::endl;
	std::cout << "�ƹ�Ű�� ������ ���α׷��� ����˴ϴ�" << std::endl;
	getchar();

	return 0;
}