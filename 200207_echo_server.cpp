// [출처] [socket / Linux] 4. 에코(echo) 서버 / 클라이언트 구현|작성자 kaka
// http://blog.naver.com/PostView.nhn?blogId=clown7942&logNo=110108561424&parentCategoryNo=&categoryNo=26&viewDate=&isShowPopularPosts=true&from=search

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // read(), write()
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h> // sockaddr_in, socket()

#define BUFSIZE 1024

void error_handling(char* message);

int main(int argc, char** argv)
{
	int serv_sock; // 유저의 접속을 받는 소켓
	int clnt_sock; // 유저가 접속시 유저에게 소켓 할당 (통신 : read, write)

	// 통신시 클라이언트에서 온 내용을 담거나, 클라이언트에게 보낼 내용을 담는 버퍼
	char message[BUFSIZE];
	int str_len;

	/*
	struct sockaddr_in {
		// address family: AF_INET, AF_INET6, AF_LOCAL

		// AF_INET : IPv4
		// AF_INET6 : IPv6
		// AF_LOCAL : Local 통신용 (UNIX)

		// PF_INET과 혼동 할 수 있는데,
		// PF_INET은 프르토콜 체계를 설정 할 때, AF_INET은 주소 체계를 설정할 때 사용한다
		// 상수 값은 사실 같은 값이지만 sa_family_t는 address family를 명시하는 변수임으로
		// AF_INET을 사용하는걸 권장한다.

		sa_family_t    sin_family;

		in_port_t      sin_port; // port 번호
		struct in_addr sin_addr; // 주소
	}
	*/

	// 서버와 클라이언트에 대한 주소체계, 주소, 포트
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	// 입력 값으로 포트번호를 입력 (안중요)
	if (argc != 2) {
		printf("Usage : &s <port>\n", argv[0]);
		exit(1);
	}

	// PF_INET : IPv4 사용, socket 함수에서는 PF_INET을 사용하는 걸 권장
	// SOCK_STREAM : TCP 사용
	// 0 : protocol 인자인데 앞의 프토로콜 체계와(3계층) 전송 체계(4계층)을 정의해서 필요가 없음
	//     거의 쓸 일 없지만, 하나의 프로토콜 체계 안에서 동일한 프로토콜이 둘 이상 존재하는 경우에 사용한다고 한다.
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);    /* 서버 소켓 생성 */

	// 소켓 생성 실패
	if (serv_sock == -1)
		error_handling("socket() error");

	// 서버 정보를 구조체에 담음
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	/* 소켓에 주소 할당 */
	if (bind(serv_sock, (structsockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error");

	// 5 : backlog - listen 에서 accept 사이에 들어오는 요청을 저장하고 있음
	if (listen(serv_sock, 5) == -1)  /* 연결 요청 대기 상태로 진입 */
		error_handling("listen() error");

	clnt_addr_size = sizeof(clnt_addr);


	/* 연결 요청 수락 */
	clnt_sock = accept(serv_sock, (struct sockaddr*) & clnt_addr, &clnt_addr_size);

	if (clnt_sock == -1)
		error_handling("accept() error");

	/* 데이터 수신 및 전송
	 * 클라이언트가 EOF를 보낼 때까지 데이터를 받아서 서버 콘솔에 한번 출력해 주고
	 * 클라이언트로 재전송해 주고 있다. */

	 // 클라이언트 입력을 대기
	while ((str_len = read(clnt_sock, message, BUFSIZE)) != 0) {
		// 클라이언트에게 다시 전달 (에코)
		write(clnt_sock, message, str_len);
		write(1, message, str_len);
	}

	close(clnt_sock);       /* 연결 종료 */
	return 0;
}



void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

