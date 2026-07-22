#include "timer.h"
#include <windows.h>

unsigned int PreTime = 0;
float DeltaTime = 0.0f;
float ElapsedTime[10] = {};
int FrameRate = 0;

void initDeltaTime() {
	PreTime = timeGetTime();
	DeltaTime = 0;
}

void setDeltaTime() {
	unsigned int curTime = timeGetTime();
	DeltaTime = (curTime - PreTime) / 1000.0f;
	PreTime = curTime;
	
}

bool timer(int number, float interval) {
	ElapsedTime[number] += DeltaTime;
	if (ElapsedTime[number] > interval) {
		ElapsedTime[number] -= interval;
		return true;
	}
	return false;
}

void resetTimer(int number) {
	ElapsedTime[number] = 0;
}

void adjustFrameRate(int frameRate)
{
	const int curTime = timeGetTime();
	float delta = curTime - PreTime;

	//1フレーム当たり何ミリ秒間隔か算出
	const float interval = 1000.0f / static_cast<float>(frameRate);

	//経過時間がインターバルより大きければ、その分休止
	if (interval > delta) {
		Sleep(interval - delta);
		delta = interval;
	}

	FrameRate = static_cast<int>(1000.0f / delta);
}

int getFrameRate()
{
	return FrameRate;
}

float getDeltaTime() {
	return DeltaTime;
}
