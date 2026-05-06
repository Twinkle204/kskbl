#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>

class GameWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartGame();
    void onPauseGame();
    void onResumeGame();
    void onEndGame();
    void onQuit();

private:
    void setupUi();
    void createActions();
    void createMenus();

    GameWidget *m_gameWidget;
    QAction *m_pauseAction;
    QAction *m_resumeAction;
    bool m_gameRunning;
};
