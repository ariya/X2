#include <QtGui/QApplication>
#include <QtCore/QProcess>
#include <QtWebKit/QWebView>

#if defined(Q_OS_SYMBIAN) && defined(ORIENTATIONLOCK)
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif // Q_OS_SYMBIAN && ORIENTATIONLOCK

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if defined(Q_OS_SYMBIAN) && defined(ORIENTATIONLOCK)
    const CAknAppUiBase::TAppUiOrientation uiOrientation = CAknAppUi::EAppUiOrientationLandscape;
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
    TRAPD(error,
        if (appUi)
            appUi->SetOrientationL(uiOrientation);
    );
    Q_UNUSED(error)
#endif // ORIENTATIONLOCK

    QWebView webView;
    webView.load(QString::fromLatin1("html/examples/kitchensink/index.html"));

#if defined(Q_OS_SYMBIAN)
    webView.showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    webView.showMaximized();
#else
    webView.show();
#endif

    return app.exec();
}
