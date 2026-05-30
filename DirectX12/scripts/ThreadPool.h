#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool
{
public:
	//コンストラクタで立ち上げるスレッド数を指定
	ThreadPool(size_t numThreads);

private:
    std::vector<std::thread> mWorkers;          // ワーカースレッドの配列
    std::queue<std::function<void()>> mTasks;   // 実行待ちタスクのキュー

    std::mutex mQueueMutex;                     // キュー操作の排他制御用ミューテックス
    std::condition_variable mCondition;         // スレッドの待機・起床制御用条件変数
    bool mStop;                                 // プール停止フラグ
};

