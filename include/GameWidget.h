#pragma once

#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QLabel>
#include <QStackedWidget>

class GameEngine;
class ScoreManager;

class GameWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    void startGame();
    void pauseGame();
    void resumeGame();
    void endGame();
    bool isPaused() const;

    void showGameStats();

signals:
    void gameStarted();
    void gamePaused();
    void gameEnded(const QString &stats);
    void scoreUpdated(int score);
    void hitRecorded();
    void missRecorded();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onTimeout();
    void updateHUD();

private:
    void drawCrosshair(QPainter &painter);
    void drawHUD(QPainter &painter);
    void drawBackground(QPainter &painter);
    void drawGameOver(QPainter &painter);

    QPoint getLastMousePos() const { return m_lastMousePos; }

    GameEngine *m_gameEngine;
    ScoreManager *m_scoreManager;

    QTimer *m_updateTimer;
    QTimer *m_gameTimer;

    bool m_gameRunning;
    bool m_paused;
    int m_gameTimeRemaining;

    QPoint m_lastMousePos;
    bool m_mouseCaptured;

    int m_score;
    int m_hits;
    int m_misses;
    float m_avgReactionTime;

    QMap<Qt::Key, bool> m_keysPressed;
};
