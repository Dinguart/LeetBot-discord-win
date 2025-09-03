#pragma once
#include <dpp/dpp.h>
#include "LeetoDB.h"
#include "LeetCodeAPI.h"

#include <algorithm>
#include <string>
#include <future>
#include <tuple>

template <typename T>
std::pair<std::promise<T>, std::future<T>> make_promise() {
	std::promise<T> promise;
	std::future<T> future = promise.get_future();
	return { std::move(promise), std::move(future) };
}

template<typename T>
struct AwaitableFuture {
    std::future<T> fut;

    bool await_ready() const noexcept {
        return fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> handle) {
        std::thread([this, handle]() mutable {
            fut.wait();
            handle.resume();
            }).detach();
    }

    T await_resume() {
        return fut.get();
    }
};


dpp::task<void> handle_registry_commands(dpp::cluster& bot, const dpp::slashcommand_t& event, LeetoDB& db);