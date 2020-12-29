#include <Windows.h>
#include <iostream>

float delays[32]{ };
bool toggled = false;
float next_random_delay = 0.0f;
bool mouseDown;
HHOOK hookdoMouse;

using namespace std;

LRESULT __stdcall mouse_callback(int code, WPARAM wparam, LPARAM lparam) {
	MSLLHOOKSTRUCT* hook = (MSLLHOOKSTRUCT*)lparam;

	if ((hook->flags == LLMHF_INJECTED) || (hook->flags == LLMHF_LOWER_IL_INJECTED))
		return false;

	if ((hook->flags & LLMHF_INJECTED) == LLMHF_INJECTED) {
		return false;
	}

	if (wparam != WM_MOUSEMOVE) {
		if ((hook->flags == LLMHF_INJECTED) || (hook->flags == LLMHF_LOWER_IL_INJECTED))
			return false;

		switch (wparam) {
		case WM_LBUTTONDOWN:
			mouseDown = true;
			break;

		case WM_LBUTTONUP:
			mouseDown = false;
			break;
		}
	}
	return CallNextHookEx(hookdoMouse, code, wparam, lparam);
}

DWORD WINAPI hookmouse() {
	hookdoMouse = SetWindowsHookEx(WH_MOUSE_LL, &mouse_callback, NULL, 0);
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(hookdoMouse);

	return 0;
}

string gen_random(const int len) {

	string tmp_s;
	static const char alphanum[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789";

	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}

void run_menu(void) {
	while (true) {
		void* hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		WORD color; // dx
		const char* v1; // rcx

		SetConsoleTextAttribute(hConsoleOutput, 15);
		printf("\rState: ");
		color = 12;

		if (toggled)
			color = 10;

		SetConsoleTextAttribute(hConsoleOutput, color);

		v1 = "[OFF]";

		if (toggled)
			v1 = "[ON] ";

		printf(v1);
		SetConsoleTextAttribute(hConsoleOutput, 15);
		printf("\tCPS: ");
		SetConsoleTextAttribute(hConsoleOutput, 5);
		float cps = 1000.0f / next_random_delay;
		printf("%2.2f", cps);

		if (cps < 10.0f)
			printf(" ");

		Sleep(50);
	}
}


void main_autoclicker_thread(void) {
	static bool pressed = false;

	while (!GetAsyncKeyState(VK_F8)) {
		if (GetKeyState(VK_F4) & 0x8000 && !pressed) {
			pressed = true;
		}
		else if (!(GetKeyState(VK_F4) & 0x8000) && pressed) {
			toggled = !toggled;
			pressed = false;
		}

		static auto last_tickcount = 0;
		auto tick_count = GetTickCount();

		if (tick_count >= last_tickcount + 1000) {
			next_random_delay = delays[rand() % 32];
			last_tickcount = tick_count;
		}
		Sleep(10);
	}
	exit(0);
}

void main_clicking_thread(void) {
	while (true) {
		HWND minecraft = GetForegroundWindow();
		char window_title[256]{ };
		GetWindowTextA(minecraft, window_title, 128);


		if (FindWindowA("LWJGL", window_title)) {
			if (mouseDown && toggled) {
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				Sleep(next_random_delay / 2);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(next_random_delay / 2);
			}
			else {
				Sleep(200);
			}
		}
	}
}

void spam_random_title(void) {
	while (true) {
		std::string add = (gen_random(32));
		SetConsoleTitleA(add.c_str());
		Sleep(300);
	}
}

int main() {
	static auto generate_random_float = [](float min, float max) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = max - min;
		float r = random * diff;
		return min + r;
	};

	for (int i = 0; i < 32; i++)
		delays[i] = 1000.0f / generate_random_float(11.0f, 15.0f);

	void* hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hConsoleOutput, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(hConsoleOutput, &cursorInfo);
	SetConsoleTextAttribute(hConsoleOutput, 13);
	printf("Vepe - v1.0\n");
	SetConsoleTextAttribute(hConsoleOutput, 5);
	printf("Made with love by Erouax\n\n\n");
	SetConsoleTextAttribute(hConsoleOutput, 8);
	printf("Destruct: F8\nToggle: F4\n\n\n");
	void* v4 = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)main_clicking_thread, nullptr, 0, nullptr);
	CloseHandle(v4);
	void* v5 = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)main_autoclicker_thread, nullptr, 0, nullptr);
	CloseHandle(v5);
	void* v6 = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)spam_random_title, nullptr, 0, nullptr);
	CloseHandle(v6);
	void* v7 = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)&hookmouse, nullptr, 0, 0);
	CloseHandle(v7);

	run_menu();
}
