/*
    This file is part of the OfiLabs X2 project.

    Copyright (C) 2010 Helder Correia <helder.pereira.correia@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QWidget>

#include <cmath>

typedef double (*transformation)(double);

const int NUMBER_OF_COLORS = 256;

int g_plasmaWidth = 640;
int g_plasmaHeight = 360;
bool g_fullScreen = false;

QVector<QRgb> g_palette(NUMBER_OF_COLORS);
int *g_pattern = 0;

qreal g_alpha = 20;
qreal g_alphaAdjust = 0.15;
qreal g_beta = 100;
qreal g_betaAdjust = 0.015;

int g_redComponent = 0;
int g_greenComponent = 255;
int g_blueComponent = 0;

int g_redComponentChangeFactor = 2;
int g_greenComponentChangeFactor = -2;
int g_blueComponentChangeFactor = 2;

int g_timerInterval = 40;

transformation g_baseFunction = sin;

void setUp()
{
    int *f = new int[qMax(g_plasmaWidth, g_plasmaHeight)];

    for (int x = 0; x < g_plasmaWidth; ++x)
        f[x] = qRound(g_alpha * g_baseFunction(x * g_alphaAdjust) + g_beta * cos(x * g_betaAdjust));

    QRgb rgb;
    int r = g_redComponent, g = g_greenComponent, b = g_blueComponent;
    for (int i = 0; i < NUMBER_OF_COLORS / 2; ++i) {
        rgb = QColor(r, g, b).rgb();
        g_palette[i] = g_palette[NUMBER_OF_COLORS-i-1] = rgb;
        r += g_redComponentChangeFactor;
        g += g_greenComponentChangeFactor;
        b += g_blueComponentChangeFactor;
    }

    for (int y = 0; y < g_plasmaHeight; ++y)
        for (int x = 0; x < g_plasmaWidth; ++x)
            g_pattern[y * g_plasmaWidth + x] = abs(f[x] + f[y]) % (NUMBER_OF_COLORS - 1);

    delete[] f;
}

void setRedAsBase()
{
    g_redComponent = 255; g_redComponentChangeFactor = -2;
    g_greenComponent = 0; g_greenComponentChangeFactor = 2;
    g_blueComponent = 0; g_blueComponentChangeFactor = 2;
    setUp();
}

void setGreenAsBase()
{
    g_redComponent = 0; g_redComponentChangeFactor = 2;
    g_greenComponent = 255; g_greenComponentChangeFactor = -2;
    g_blueComponent = 0; g_blueComponentChangeFactor = 2;
    setUp();
}

void setBlueAsBase()
{
    g_redComponent = 0; g_redComponentChangeFactor = 2;
    g_greenComponent = 0; g_greenComponentChangeFactor = 2;
    g_blueComponent = 255; g_blueComponentChangeFactor = -2;
    setUp();
}

void setParameters(qreal alpha, qreal alphaAdjust, qreal beta, qreal betaAdjust)
{
    g_alpha = alpha; g_alphaAdjust = alphaAdjust;
    g_beta = beta; g_betaAdjust = betaAdjust;
    setUp();
}

void setBaseFunction(transformation function)
{
    g_baseFunction = function;
    setUp();
}

class PlasmaEffect : public QWidget
{
    Q_OBJECT

public:
    PlasmaEffect() : QWidget()
    {
        setAttribute(Qt::WA_StaticContents, true);
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);

        connect(&m_animationTimer, SIGNAL(timeout()), SLOT(update()));
        m_animationTimer.start(g_timerInterval);
    }

protected:
    virtual void keyPressEvent(QKeyEvent *event)
    {
        switch (event->key()) {
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ShiftModifier)
                g_betaAdjust -= 0.0008;
            else
                g_alphaAdjust -= 0.002;
            setUp();
            break;
        case Qt::Key_Right:
            if (event->modifiers() & Qt::ShiftModifier)
                g_betaAdjust += 0.0008;
            else
                g_alphaAdjust += 0.002;
            setUp();
            break;
        case Qt::Key_Up:
            if (event->modifiers() & Qt::ShiftModifier)
                g_beta += 8;
            else
                g_alpha += 2;
            setUp();
            break;
        case Qt::Key_Down:
            if (event->modifiers() & Qt::ShiftModifier)
                g_beta -= 8;
            else
                g_alpha -= 2;
            setUp();
            break;
        case Qt::Key_R:
            setRedAsBase();
            break;
        case Qt::Key_G:
            setGreenAsBase();
            break;
        case Qt::Key_B:
            setBlueAsBase();
            break;
        case Qt::Key_S:
            setBaseFunction(sin);
            break;
        case Qt::Key_T:
            setBaseFunction(tan);
            break;
#if !defined(Q_WS_MAEMO_5) && !defined(Q_WS_S60)
        case Qt::Key_F:
            if (g_fullScreen) {
                showNormal();
                g_fullScreen = false;
            } else {
                showFullScreen();
                g_fullScreen = true;
            }
            break;
#endif
        case Qt::Key_1:
            setParameters(0, -0.046, -4, 0.0086);
            break;
        case Qt::Key_2:
            setParameters(-16, -0.012, 4, -0.0266);
            break;
        case Qt::Key_3:
            setParameters(-58, -0.002, 4, -0.1146);
            break;
        case Qt::Key_4:
            setParameters(-58, -0.002, -4, -0.0138);
            break;
        case Qt::Key_5:
            setParameters(4, 0.102, 12, -0.0686);
            break;
        case Qt::Key_6:
            setParameters(-2, 0.244, 1780, -0.0007);
            break;
        case Qt::Key_7:
            setParameters(-34, 0.092, 1780, -0.0007);
            break;
        case Qt::Key_8:
            setParameters(-130, 0.008, -12, -0.043);
            break;
        case Qt::Key_9:
            setParameters(-38, 0.002, 12, -0.0662);
            break;
        case Qt::Key_0:
            setParameters(20, 0.15, 100, 0.015);
            break;
        case Qt::Key_Escape:
            QApplication::quit();
            break;
        default:
            QWidget::keyPressEvent(event);
        }
    }

    virtual void paintEvent(QPaintEvent *)
    {
        for (int i = 0; i < NUMBER_OF_COLORS - 1; ++i)
            g_palette[i] = g_palette[i + 1];
        g_palette[NUMBER_OF_COLORS - 1] = g_palette[0];

	uchar *scanLine;
        for (int line = 0; line < g_plasmaHeight; ++line) {
            scanLine = m_image.scanLine(line);
            for (int offset = 0; offset < g_plasmaWidth; ++offset)
		scanLine[offset] = g_palette[g_pattern[line * g_plasmaWidth + offset]];
	}

        m_image.setColorTable(g_palette);

        QPainter painter(this);
        painter.drawImage(m_image.rect(), m_image, m_image.rect());
    }

    virtual void resizeEvent(QResizeEvent *event)
    {
        g_plasmaWidth = event->size().width();
        g_plasmaHeight = event->size().height();
        m_image = QImage(g_plasmaWidth, g_plasmaHeight, QImage::Format_Indexed8);
        if (g_pattern)
            delete g_pattern;
        g_pattern = new int[g_plasmaWidth*g_plasmaHeight];

        setUp();
    }

private:
    QImage m_image;
    QTimer m_animationTimer;
};

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    PlasmaEffect plasma;
#if !defined(Q_WS_MAEMO_5) && !defined(Q_WS_S60)
    plasma.resize(g_plasmaWidth, g_plasmaHeight);
    plasma.show();
#else
    plasma.showMaximized();
#endif
    return application.exec();
}

#include "plasmaeffect.moc"

