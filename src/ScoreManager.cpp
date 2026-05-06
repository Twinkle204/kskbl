#include "ScoreManager.h"

#include <QDateTime>

ScoreManager::ScoreManager(QObject *parent)
    : QObject(parent)
    , m_score(0)
    , m_hits(0)
    , m_misses(0)
    , m_roundStartTime(0)
    , m_lastSpawnTime(0.0f) {
}

ScoreManager::~ScoreManager() {
}

void ScoreManager::reset() {
    m_score = 0;
    m_hits = 0;
    m_misses = 0;
    m_hitRecords.clear();
    m_reactionTimes.clear();
    m_roundStartTime = QDateTime::currentMSecsSinceEpoch();
    m_lastSpawnTime = 0.0f;
}

int ScoreManager::recordHit(float reactionTimeMs, float mouseDistance) {
    int points = calculateHitScore(reactionTimeMs, mouseDistance);

    m_score += points;
    m_hits++;

    HitRecord record;
    record.reactionTimeMs = reactionTimeMs;
    record.mouseDistance = mouseDistance;
    record.score = points;
    record.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_hitRecords.append(record);

    if (reactionTimeMs > 0) {
        m_reactionTimes.append(reactionTimeMs);
    }

    emit scoreChanged();
    emit hitsChanged();

    return points;
}

void ScoreManager::recordMiss() {
    m_misses++;
    emit missesChanged();
    emit accuracyChanged();
}

float ScoreManager::accuracy() const {
    int total = m_hits + m_misses;
    if (total == 0) return 0.0f;
    return static_cast<float>(m_hits) / total * 100.0f;
}

float ScoreManager::averageReactionTime() const {
    if (m_reactionTimes.isEmpty()) return 0.0f;

    float sum = 0.0f;
    for (float time : m_reactionTimes) {
        sum += time;
    }
    return sum / m_reactionTimes.size();
}

float ScoreManager::totalMouseDistance() const {
    float total = 0.0f;
    for (const HitRecord &record : m_hitRecords) {
        total += record.mouseDistance;
    }
    return total;
}

int ScoreManager::flickScore() const {
    int score = 0;
    for (const HitRecord &record : m_hitRecords) {
        if (record.mouseDistance > 100.0f) {
            score += record.score;
        }
    }
    return score;
}

int ScoreManager::trackingScore() const {
    int score = 0;
    for (const HitRecord &record : m_hitRecords) {
        if (record.mouseDistance < 50.0f && record.reactionTimeMs < 200.0f) {
            score += record.score;
        }
    }
    return score;
}

int ScoreManager::clickingScore() const {
    return m_score - flickScore() - trackingScore();
}

QVariantMap ScoreManager::getGameStats() const {
    QVariantMap stats;
    stats["score"] = m_score;
    stats["hits"] = m_hits;
    stats["misses"] = m_misses;
    stats["accuracy"] = accuracy();
    stats["avgReactionTime"] = averageReactionTime();
    stats["totalMouseDistance"] = totalMouseDistance();
    stats["flickScore"] = flickScore();
    stats["trackingScore"] = trackingScore();
    stats["clickingScore"] = clickingScore();
    return stats;
}

int ScoreManager::calculateHitScore(float reactionTimeMs, float mouseDistance) {
    int baseScore = 100;

    int reactionScore = 0;
    if (reactionTimeMs < 200.0f) {
        reactionScore = 100;
    } else if (reactionTimeMs < 400.0f) {
        reactionScore = 50;
    } else if (reactionTimeMs < 600.0f) {
        reactionScore = 25;
    }

    int distanceScore = 0;
    if (mouseDistance < 50.0f) {
        distanceScore = 50;
    } else if (mouseDistance < 150.0f) {
        distanceScore = 30;
    } else if (mouseDistance > 200.0f) {
        distanceScore = 20;
    }

    return baseScore + reactionScore + distanceScore;
}
