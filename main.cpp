#include <QApplication>
#include "GameWidget.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    GameWidget game;
    game.setWindowTitle("Thunder Zone v3.0");
    game.show();
    return app.exec();
}