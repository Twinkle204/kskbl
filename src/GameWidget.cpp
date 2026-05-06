#include "GameWidget.h"
#include "GameEngine.h"
#include "ScoreManager.h"

#include <QPainter>
#include <QMessageBox>
#include <QInputDialog>
#include <QtMath>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , m_gameEngine(nullptr)
    , m_scoreManager(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_gameTimer(new QTimer(this))
    , m_gameRunning(false)
    , m_paused(false)
    , m_gameTimeRemaining(60)
    , m_lastMousePos(0, 0)
    , m_mouseCaptured(false)
    , m_score(0)
    , m_hits(0)
    , m_misses(0)
    , m_avgReactionTime(0.0f) {

    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_gameEngine = new GameEngine(this);
    m_scoreManager = new ScoreManager(this);

    connect(m_updateTimer, &QTimer::timeout, this, &GameWidget::onTimeout);
    connect(m_gameTimer, &QTimer::timeout, this, &GameWidget::updateHUD);

    m_updateTimer->setInterval(16);
    m_gameTimer->setInterval(1000);
}

GameWidget::~GameWidget() {
}

void GameWidget::startGame() {
    m_score = 0;
    m_hits = 0;
    m_misses = 0;
    m_gameTimeRemaining = 60;
    m_gameRunning = true;
    m_paused = false;
    m_mouseCaptured = true;

    m_gameEngine->reset();
    m_scoreManager->reset();

    m_updateTimer->start();
    m_gameTimer->start();

    setCursor(Qt::BlankCursor);
    grabKeyboard();
    grabMouse();

    update();
}

void GameWidget::pauseGame() {
    if (m_gameRunning && !m_paused) {
        m_paused = true;
        m_updateTimer->stop();
        m_gameTimer->stop();

        setCursor(Qt::ArrowCursor);
        releaseKeyboard();
        releaseMouse();
        update();
    }
}

void GameWidget::resumeGame() {
    if (m_gameRunning && m_paused) {
        m_paused = false;
        m_updateTimer->start();
        m_gameTimer->start();

        setCursor(Qt::BlankCursor);
        grabKeyboard();
        grabMouse();
        update();
    }
}

void GameWidget::endGame() {
    m_gameRunning = false;
    m_paused = false;
    m_updateTimer->stop();
    m_gameTimer->stop();

    setCursor(Qt::ArrowCursor);
    releaseKeyboard();
    releaseMouse();
    update();
}

bool GameWidget::isPaused() const {
    return m_paused;
}

void GameWidget::showGameStats() {
    float accuracy = (m_hits + m_misses) > 0
        ? (float)m_hits / (m_hits + m_misses) * 100.0f
        : 0.0f;

    QString stats = QString(
        "===== 游戏统计 =====\n\n"
        "得分: %1\n"
        "命中: %2\n"
        "失误: %3\n"
        "命中率: %4%\n"
        "平均反应时间: %5 ms"
    ).arg(m_score).arg(m_hits).arg(m_misses)
     .arg(accuracy, 0, 'f', 1)
     .arg(m_avgReactionTime, 0, 'f', 1);

    QMessageBox::information(this, "游戏结束", stats);
}

void GameWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_gameRunning && !m_paused) {
        drawBackground(painter);
        drawGameOver(painter);
    } else if (m_paused) {
        drawBackground(painter);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 24));
        painter.drawText(rect(), Qt::AlignCenter, "游戏暂停\n按 ESC 继续");
    } else {
        drawBackground(painter);

        m_gameEngine->render(painter);

        drawCrosshair(painter);
        drawHUD(painter);
    }
}

void GameWidget::drawCrosshair(QPainter &painter) {
    int cx = width() / 2;
    int cy = height() / 2;

    QPen crosshairPen(Qt::white, 2);
    crosshairPen.setCapStyle(Qt::RoundCap);
    painter.setPen(crosshairPen);

    painter.drawLine(cx - 15, cy, cx - 5, cy);
    painter.drawLine(cx + 5, cy, cx + 15, cy);
    painter.drawLine(cx, cy - 15, cx, cy - 5);
    painter.drawLine(cx, cy + 5, cx, cy + 15);

    QPen shadowPen(Qt::black, 2);
    shadowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(shadowPen);
    painter.setOpacity(0.5);
    painter.drawEllipse(QPoint(cx, cy), 3, 3);
    painter.setOpacity(1.0);
}

void GameWidget::drawHUD(QPainter &painter) {
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16));

    painter.drawText(20, 30, QString("得分: %1").arg(m_score));
    painter.drawText(20, 55, QString("时间: %1s").arg(m_gameTimeRemaining));

    float accuracy = (m_hits + m_misses) > 0
        ? (float)m_hits / (m_hits + m_misses) * 100.0f
        : 0.0f;
    painter.drawText(width() - 150, 30, QString("命中率: %1%").arg(accuracy, 0, 'f', 1));
}

void GameWidget::drawBackground(QPainter &painter) {
    painter.fillRect(rect(), QColor(20, 20, 30));

    int gridSize = 50;
    QPen gridPen(QColor(50, 50, 60), 1);
    painter.setPen(gridPen);

    for (int x = 0; x < width(); x += gridSize) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }
}

void GameWidget::drawGameOver(QPainter &painter) {
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 32, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, "AimTrainer");
    painter.setFont(QFont("Arial", 16));
    painter.drawText(rect().adjusted(0, 60, 0, 0), Qt::AlignCenter, "按 Enter 开始游戏");
}

void GameWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape && m_gameRunning) {
        if (m_paused) {
            resumeGame();
        } else {
            pauseGame();
        }
        return;
    }

    if (event->key() == Qt::Key_Return && !m_gameRunning && !m_paused) {
        startGame();
        return;
    }

    m_keysPressed[static_cast<Qt::Key>(event->key())] = true;
}

void GameWidget::keyReleaseEvent(QKeyEvent *event) {
    m_keysPressed[static_cast<Qt::Key>(event->key())] = false;
}

void GameWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseCaptured && m_gameRunning && !m_paused) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_gameEngine->handleMouseMove(delta.x(), delta.y());
    }
    m_lastMousePos = event->pos();
}

void GameWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_gameRunning && !m_paused) {
        bool hit = m_gameEngine->handleClick(width() / 2, height() / 2);

        if (hit) {
            m_hits++;
            int score = m_scoreManager->recordHit();
            m_score += score;
            emit hitRecorded();
        } else {
            m_misses++;
            emit missRecorded();
        }
        update();
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
}

void GameWidget::onTimeout() {
    m_gameEngine->update(16.0f / 1000.0f);

    if (m_gameTimeRemaining <= 0) {
        endGame();
        showGameStats();
        return;
    }

    update();
}

void GameWidget::updateHUD() {
    if (m_gameRunning && !m_paused) {
        m_gameTimeRemaining--;
        update();
    }
}
