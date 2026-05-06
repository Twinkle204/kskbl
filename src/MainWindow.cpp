#include "MainWindow.h"
#include "GameWidget.h"

#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_gameWidget(nullptr)
    , m_gameRunning(false) {

    setupUi();
    createActions();
    createMenus();

    setWindowTitle("AimTrainer - 瞄准训练");
    resize(1280, 720);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    m_gameWidget = new GameWidget(this);
    setCentralWidget(m_gameWidget);

    statusBar()->showMessage("准备就绪");
}

void MainWindow::createActions() {
    m_pauseAction = new QAction(tr("暂停 (&P)"), this);
    m_pauseAction->setShortcut(Qt::Key_Escape);
    connect(m_pauseAction, &QAction::triggered, this, &MainWindow::onPauseGame);

    m_resumeAction = new QAction(tr("继续 (&R)"), this);
    m_resumeAction->setEnabled(false);
    connect(m_resumeAction, &QAction::triggered, this, &MainWindow::onResumeGame);
}

void MainWindow::createMenus() {
    QMenu *gameMenu = menuBar()->addMenu(tr("游戏 (&G)"));

    QAction *startAction = new QAction(tr("开始游戏 (&S)"), this);
    startAction->setShortcut(Qt::Key_Return);
    connect(startAction, &QAction::triggered, this, &MainWindow::onStartGame);
    gameMenu->addAction(startAction);

    gameMenu->addAction(m_pauseAction);
    gameMenu->addAction(m_resumeAction);

    gameMenu->addSeparator();

    QAction *endAction = new QAction(tr("结束游戏 (&E)"), this);
    connect(endAction, &QAction::triggered, this, &MainWindow::onEndGame);
    gameMenu->addAction(endAction);

    gameMenu->addSeparator();

    QAction *quitAction = new QAction(tr("退出 (&Q)"), this);
    quitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);
    gameMenu->addAction(quitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("帮助 (&H)"));
    QAction *aboutAction = new QAction(tr("关于 (&A)"), this);
    connect(aboutAction, &QAction::triggered, []() {
        QMessageBox::information(nullptr, "关于",
            "AimTrainer v1.0\n一款 AimLab 风格的瞄准训练游戏\n基于 Qt 开发");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::onStartGame() {
    if (!m_gameRunning) {
        m_gameWidget->startGame();
        m_gameRunning = true;
        m_pauseAction->setEnabled(true);
        m_resumeAction->setEnabled(false);
        statusBar()->showMessage("游戏进行中...");
    }
}

void MainWindow::onPauseGame() {
    if (m_gameRunning) {
        m_gameWidget->pauseGame();
        m_gameRunning = false;
        m_pauseAction->setEnabled(false);
        m_resumeAction->setEnabled(true);
        statusBar()->showMessage("游戏已暂停");
    }
}

void MainWindow::onResumeGame() {
    if (!m_gameRunning) {
        m_gameWidget->resumeGame();
        m_gameRunning = true;
        m_pauseAction->setEnabled(true);
        m_resumeAction->setEnabled(false);
        statusBar()->showMessage("游戏进行中...");
    }
}

void MainWindow::onEndGame() {
    if (m_gameRunning || m_gameWidget->isPaused()) {
        m_gameWidget->endGame();
        m_gameRunning = false;
        m_pauseAction->setEnabled(false);
        m_resumeAction->setEnabled(false);
        statusBar()->showMessage("游戏结束");

        m_gameWidget->showGameStats();
    }
}

void MainWindow::onQuit() {
    if (m_gameRunning) {
        m_gameWidget->endGame();
    }
    close();
}
