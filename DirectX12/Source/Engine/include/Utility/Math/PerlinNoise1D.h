#pragma once
#include <vector>
#include <random>
#include <numeric>

class PerlinNoise1D {
private:
    //順列テーブル（ハッシュ値の計算に使用）
    std::vector<int> p;

    //滑らかな補間のためのフェード関数: 6t^5 - 15t^4 + 10t^3
    double fade(double t) const {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    //線形補間 (Linear Interpolation)
    double lerp(double t, double a, double b) const {
        return a + t * (b - a);
    }

    //1次元の勾配を計算
    //ハッシュ値の最下位ビットを利用して、右上がり(x)か左上がり(-x)かをランダムに決定
    double grad(int hash, double x) const {
        return (hash & 1) == 0 ? x : -x;
    }

public:
    PerlinNoise1D(unsigned int seed = 123) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);
        
        // 配列外アクセスを防ぐため2倍に拡張
        p.insert(p.end(), p.begin(), p.end());
    }

    //1Dノイズの生成
    double noise(double x) const {
        //所属する区間の左側の整数座標を求める
        int X = static_cast<int>(std::floor(x)) & 255;

        //区間内での相対座標 (0.0 ~ 1.0)
        x -= std::floor(x);

        //フェード関数による重み付け
        double u = fade(x);

        //左右の格子点でのハッシュ値を取得
        int hash_left = p[X];
        int hash_right = p[X + 1];

        //勾配の計算 (1次元の内積)
        //左側の点からの距離は x, 右側の点からの距離は x - 1.0
        double grad_left = grad(hash_left, x);
        double grad_right = grad(hash_right, x - 1.0);

        //線形補間
        double res = lerp(u, grad_left, grad_right);

        //結果を -1.0 ~ 1.0 付近から 0.0 ~ 1.0 に正規化
        return (res + 1.0) / 2.0;
    }
};
