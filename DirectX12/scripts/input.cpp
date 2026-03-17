#include "input.h"
#include <array>
#include <windows.h>

static std::array<BYTE, NUM_KEYS> isKeyPressed_current = {};
static std::array<BYTE, NUM_KEYS> isKeyPressed_prev = {};

void updateInput() {
	//現在のキーの状態を前の状態にコピー
	isKeyPressed_prev = isKeyPressed_current;
	//全てのキーの状態を更新
	GetKeyboardState(isKeyPressed_current.data());
}

bool isKeyJustPressed(int key) {
	bool isPressedCurrent = (isKeyPressed_current[key] & 0x80) != 0; //現在の状態でキーが押されているか
	bool isPressedPrev = (isKeyPressed_prev[key] & 0x80) != 0; //前の状態でキーが押されていたか
	return isPressedCurrent && !isPressedPrev; //現在押されていて、前は押されていなかった場合にtrue
}

bool isKeyJustReleased(int key) {
	bool isPressedCurrent = (isKeyPressed_current[key] & 0x80) != 0; //現在の状態でキーが押されているか
	bool isPressedPrev = (isKeyPressed_prev[key] & 0x80) != 0; //前の状態でキーが押されていたか
	return !isPressedCurrent && isPressedPrev; //現在押されていなくて、前は押されていた場合にtrue
}
