HuServer와 연결 가능한 언리얼엔진5 클라이언트로 직접 네트워크 코드에 접근하지 않고 UI에서 게임 로직 클래스에만 접근하는 사용하기 쉬운 프레임워크를 구현

LoginReq Request;
Socket->Send( Request );
같은 방식이 아니라
GameLogic->Login();
방식으로 사용한다.

구조는 Source/HuClient/HuCommon 경로의 소스를 참고한다.
