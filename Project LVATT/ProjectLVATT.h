#pragma once

#include <QtWidgets/QMainWindow>

#include <QtMultimedia/QAudio>
#include <QtMultimedia/QMediaPlayer.h>
#include <QtMultimedia/QAudioOutput.h>

#include "ui_ProjectLVATT.h"

class LVATTMainWindow : public QMainWindow
{
    Q_OBJECT
private:
	QMediaPlayer* player;
	QAudioOutput* audioOutput;

public:
    LVATTMainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
	{
		ui.setupUi(this);
		connect(ui.pushButton, &QPushButton::released, this, &LVATTMainWindow::PlayAudioCallback);
	}

    ~LVATTMainWindow()
	{
	}

    void PlayAudioCallback()
	{
		player = new QMediaPlayer;
		player->setSource(QUrl::fromLocalFile(".\\terror.wav"));
		player->play();
	}

private:
    Ui::LVATTMainWindowClass ui;
};
