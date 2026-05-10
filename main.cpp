#include <QApplication>
#include "GameWidget.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    GameWidget game;
    game.setWindowTitle("ThunderFighter");
    game.show();
    return app.exec();
}