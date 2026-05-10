#include "BgmManager.h"

BgmManager::BgmManager(QObject* parent) : QObject(parent) {
    m_player = new QMediaPlayer(this);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status) {    // 播放完自动循环
        if (status == QMediaPlayer::EndOfMedia) m_player->play();
        });
}

BgmManager::~BgmManager() {
    m_player->stop();
}

void BgmManager::playMenu() {
    m_player->stop();
    m_player->setMedia(QUrl("qrc:/res/bgm_menu.mp3"));
    m_player->setVolume(m_muted ? 0 : 100);
    m_player->play();
}

void BgmManager::playBattle() {
    m_player->stop();
    m_player->setMedia(QUrl("qrc:/res/bgm_battle.mp3"));
    m_player->setVolume(m_muted ? 0 : 100);
    m_player->play();
}

void BgmManager::playBoss() {
    m_player->stop();
    m_player->setMedia(QUrl("qrc:/res/bgm_boss.mp3"));
    m_player->setVolume(m_muted ? 0 : 100);
    m_player->play();
}

void BgmManager::stop() {
    m_player->stop();
}

void BgmManager::setMuted(bool muted) {
    m_muted = muted;
    m_player->setVolume(muted ? 0 : 100);
}