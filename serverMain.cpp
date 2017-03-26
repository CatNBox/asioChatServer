#include "chatServer.h"

//최대접속숫자
const int MAX_SESSION_COUNT = 100;

int main()
{
	//asio 필수
	boost::asio::io_service ioServ;

	//charServer 클래스 생성
	chatServer clsServer(ioServ);
	clsServer.init(MAX_SESSION_COUNT);
	clsServer.start();

	ioServ.run();

	std::cout << "네트워크 접속 종료" << std::endl;
	std::cout << "아무키나 누르면 프로그램이 종료됩니다" << std::endl;
	getchar();

	return 0;
}