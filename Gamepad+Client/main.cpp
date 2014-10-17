#define  _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
//#include <stdio.h>
//#include <WinSock2.h>
#include <string>
#include <iostream>
#include <Xinput.h>

//#include <tchar.h>
//#include <strsafe.h>
#include <math.h>
#include <sstream>

//#define IP "192.168.178.30"
#define IP "192.168.43.220" 
#define PORT 5005

using namespace std;

float addDeadzone(float value, float deadvalue){
	if (abs(value) < deadvalue) return 0;
	if (value > 0) return (value - deadvalue) / (1 - deadvalue);
	return (value + deadvalue) / (1 - deadvalue);
}

string toString(float arg){
	std::ostringstream ss;
	ss << arg;
	return ss.str();
}

int main(){
	//Controller finden
	int controllerId = -1;

	for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		cout << XInputGetState(i, &state) << endl;

		if (XInputGetState(i, &state) == ERROR_SUCCESS)
			controllerId = i;
	}

	if (controllerId != -1){
		//Controller initialisieren
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		//Verbindung zum PI herstellen
		// Initialise WinSock  -> WSAStartup(gewünschte Version, ...)
		WORD wVersionRequested = MAKEWORD(1, 1);
		WSADATA wsaData;
		int wsaret = WSAStartup(wVersionRequested, &wsaData);
		if (wsaret){
			printf("ERROR on initialising SOCKET");
			//return 0;
		}

		// create Socket
		SOCKET conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);;
		if (conn == INVALID_SOCKET){
			printf("ERROR on creating SOCKET");
			//return 0;
		}

		// connect to server
		struct sockaddr_in server;
		server.sin_addr.s_addr = inet_addr(IP);
		server.sin_family = AF_INET;
		server.sin_port = htons(PORT);
		if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
		{
			closesocket(conn);
			printf("Connection failed");
			//return 0;
		}

		while (true){
			//Gamepad Werte ermitteln
			XInputGetState(controllerId, &state);
			float leftStick_X = addDeadzone((float)state.Gamepad.sThumbLX / 32767, 0.15);
			float leftStick_Y = addDeadzone((float)state.Gamepad.sThumbLY / 32767, 0.15);
			float rightStick_X = addDeadzone((float)state.Gamepad.sThumbRX / 32767, 0.15);
			float rightStick_Y = addDeadzone((float)state.Gamepad.sThumbRY / 32767, 0.15);
			float leftTrigger = (float)state.Gamepad.bLeftTrigger / 255;
			float rightTrigger = (float)state.Gamepad.bRightTrigger / 255;

			string sendM = toString(leftStick_X) + "|" +  
				toString(leftStick_Y) + "|" + 
				toString(rightStick_X) + "|" + 
				toString(rightStick_Y) + "|" + 
				toString(leftTrigger) + "|" + 
				toString(rightTrigger) + "|";

			const char * buffer = sendM.c_str();
			send(conn, buffer, 100, 0);

			Sleep(100);
		}

	}
	else {
		cout << "No Controller found!" << endl;
	}
}