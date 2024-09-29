#include <iostream>
#include <chrono> // For time-related functions
#include <thread> // For multi-threading
#include <mutex>  // For locking mechanism in thread-safe code
#include <shared_mutex> // For shared locks
#include <algorithm> // For std::min

using namespace std; 

class DistributedRateLimiter {
private:
    int tokensPerSecond_;         // Rate limit: Tokens added per second
    int burstSize_;               // Maximum burst size
    int currentTokens_;           // Current available tokens
    chrono::steady_clock::time_point lastUpdate_;  // Last update time for token refill
    shared_mutex mutex_;          // Mutex to ensure thread-safety

public:
    // Constructor to initialize the rate limiter
    DistributedRateLimiter(int tokensPerSecond, int burstSize)
        : tokensPerSecond_(tokensPerSecond),
        burstSize_(burstSize),
        currentTokens_(burstSize),
        lastUpdate_(chrono::steady_clock::now()) {}

    // Function to check if a request can be allowed
    bool allowRequest() {
        if (tokensPerSecond_ == 0) {
            return false;  // If no tokens allowed per second, always deny
        }

        lock_guard<shared_mutex> lock(mutex_); // Lock to ensure thread safety

        auto now = chrono::steady_clock::now(); // Get current time
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastUpdate_).count(); // Calculate elapsed time
        lastUpdate_ = now; // Update the last update time

        // Ensure type consistency with static_cast<int>
        currentTokens_ = min(burstSize_, currentTokens_ + static_cast<int>(elapsed * tokensPerSecond_ / 1000));

        // Allow request if there are available tokens
        if (currentTokens_ >= 1) {
            --currentTokens_; // Decrease the token count
            return true;
        }
        return false; // Deny request if no tokens available
    }
};

int main() {
    DistributedRateLimiter limiter(5, 10);  // 5 requests per second, burst size of 10

    // Simulate 20 requests with a 100ms interval
    for (int i = 0; i < 20; ++i) {
        if (limiter.allowRequest()) {
            cout << "Request allowed" << endl;
        }
        else {
            cout << "Request denied" << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(100)); // Sleep for 100 milliseconds
    }

    return 0;
}
