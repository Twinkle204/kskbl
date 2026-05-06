# Qt 模块
QT = core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += widgets
}

CONFIG += c++17

TARGET = AimTrainer
TEMPLATE = app

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD/include

# 源文件
SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/GameWidget.cpp \
    src/GameEngine.cpp \
    src/Target.cpp \
    src/TargetSpawner.cpp \
    src/ScoreManager.cpp \
    src/AudioHandler.cpp

# 头文件
HEADERS += \
    include/MainWindow.h \
    include/GameWidget.h \
    include/GameEngine.h \
    include/Target.h \
    include/TargetSpawner.h \
    include/ScoreManager.h \
    include/AudioHandler.h

# 资源文件
RESOURCES += \
    resources/resources.qrc

# 默认生成路径
win32 {
    DESTDIR = $$PWD/bin
    OBJECTS_DIR = $$PWD/build/obj
    MOC_DIR = $$PWD/build/moc
    RCC_DIR = $$PWD/build/rcc
    UI_DIR = $$PWD/build/ui
}
