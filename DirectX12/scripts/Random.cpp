#include "Random.h"

int Random::dist(int l, int r)
{
	//乱数生成期の初期化
	static std::random_device seed_gen;
	static unsigned seed = seed_gen();
	static std::mt19937 mt(seed);

	//乱数生成
	std::uniform_int_distribution<> dist(l, r);
	return dist(mt);

}

float Random::dist(float l, float r)
{
	//乱数生成期の初期化
	static std::random_device seed_gen;
	static unsigned seed = seed_gen();
	static std::mt19937 mt(seed);

	//乱数生成
	std::uniform_real_distribution<> dist(l, r);
	return dist(mt);
}
