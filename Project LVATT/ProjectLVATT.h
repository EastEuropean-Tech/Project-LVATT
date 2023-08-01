#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProjectLVATT.h"

class ProjectLVATT : public QMainWindow
{
    Q_OBJECT

public:
    ProjectLVATT(QWidget *parent = nullptr);
    ~ProjectLVATT();

private:
    Ui::ProjectLVATTClass ui;
};
