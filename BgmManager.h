#ifndef BGMMANAGER_H
#define BGMMANAGER_H

#include <QObject>
#include <QMediaPlayer>

class BgmManager : public QObject {
    Q_OBJECT
public:
    explicit BgmManager(QObject* parent = nullptr);
    ~BgmManager();

    void playMenu();
    void playBattle();
    void playBoss();
    void stop();
    void setMuted(bool muted);

private:
    QMediaPlayer* m_player;
    bool m_muted = false;
};

#endif