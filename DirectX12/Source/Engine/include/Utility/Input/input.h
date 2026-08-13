#pragma once
static constexpr int NUM_KEYS = 256; //キーの数

void updateInput();

//キーが押された瞬間を判定
bool isKeyJustPressed(int key);
//キーが押され続けている間を判定
bool isKeyPressed(int key);
//キーが離された瞬間を判定
bool isKeyJustReleased(int key);
