#pragma once

namespace AnalyticsUploadRetryPolicy {

constexpr int defaultMaximumAttempts() { return 3; }

bool shouldRetry(int completedAttempts,
                 bool retryable,
                 int maximumAttempts = defaultMaximumAttempts());

int retryDelayMs(int completedAttempts,
                 int baseDelayMs = 1000,
                 int maximumDelayMs = 30000);

} // namespace AnalyticsUploadRetryPolicy
