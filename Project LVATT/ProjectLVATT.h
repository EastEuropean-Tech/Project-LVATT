#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProjectLVATT.h"

class LVATTMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    LVATTMainWindow(QWidget *parent = nullptr);
    ~LVATTMainWindow();

private:
    Ui::LVATTMainWindowClass ui;
};
