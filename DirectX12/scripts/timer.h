#pragma once
# pragma comment(lib, "winmm.lib")
#define deltaTime getDeltaTime()

void initDeltaTime();
void setDeltaTime();
float getDeltaTime();
bool timer(int number, float interval);
void resetTimer(int number);
