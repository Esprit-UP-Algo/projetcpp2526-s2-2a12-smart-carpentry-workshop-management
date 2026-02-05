#include <QApplication>
#include <QPalette>
#include <QColor>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Force a light palette to override system theme
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#F3EFE0"));
    palette.setColor(QPalette::WindowText, QColor("#4D362D"));
    palette.setColor(QPalette::Base, QColor("#FFFFFF"));
    palette.setColor(QPalette::AlternateBase, QColor("#FAFAF8"));
    palette.setColor(QPalette::Text, QColor("#4D362D"));
    palette.setColor(QPalette::Button, QColor("#FFFFFF"));
    palette.setColor(QPalette::ButtonText, QColor("#4D362D"));
    palette.setColor(QPalette::BrightText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Highlight, QColor("#C29B6D"));
    palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    a.setPalette(palette);
    
    // Use Fusion style to have consistent cross-platform appearance
    a.setStyle("Fusion");

    MainWindow w;
    w.show();

    return a.exec();
}
