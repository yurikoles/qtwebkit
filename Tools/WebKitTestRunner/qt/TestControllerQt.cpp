/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 University of Szeged. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "TestController.h"

#include "PlatformWebView.h"

#include <cstdlib>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFileInfo>
#include <QLibrary>
#include <QObject>
#include <QTimer>
#include <QtGlobal>
#include <WebKit/WKStringQt.h>
#include <qquickwebview_p.h>
#include <wtf/Platform.h>
#include <wtf/text/WTFString.h>

namespace WTR {

class TestController::RunLoopQt : public QObject {
    Q_OBJECT
public:
    RunLoopQt();

    void runUntil(double timeout);
    void notifyDone();
    void runModal(PlatformWebView*);
public Q_SLOTS:
    void timerFired();
private:
    QEventLoop m_runUntilLoop;
    QEventLoop m_modalLoop;
    QTimer m_runUntilLoopTimer;
    bool m_runUntilLoopClosePending;
};

TestController::RunLoopQt::RunLoopQt()
    : m_runUntilLoopClosePending(false)
{
    m_runUntilLoopTimer.setSingleShot(true);
    QObject::connect(&m_runUntilLoopTimer, SIGNAL(timeout()), this, SLOT(timerFired()));
}

void TestController::RunLoopQt::runUntil(double timeout)
{
    ASSERT(!m_runUntilLoop.isRunning());
    if (timeout) {
        const int timeoutInMilliSecs = timeout * 1000;
        m_runUntilLoopTimer.start(timeoutInMilliSecs);
    }
    m_runUntilLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void TestController::RunLoopQt::notifyDone()
{
    if (m_modalLoop.isRunning()) {
        // Wait for the modal loop first. We only kill it if we timeout.
        m_runUntilLoopClosePending = true;
        return;
    }

    m_runUntilLoopTimer.stop();
    m_runUntilLoop.exit();
}

void TestController::RunLoopQt::timerFired()
{
    if (m_modalLoop.isRunning()) {
        m_runUntilLoopClosePending = true;
        m_modalLoop.exit();
        return;
    }

    m_runUntilLoop.exit();
}

void TestController::RunLoopQt::runModal(PlatformWebView* view)
{
    ASSERT(!m_modalLoop.isRunning());
    view->setModalEventLoop(&m_modalLoop);
    m_modalLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if (m_runUntilLoopClosePending)
        m_runUntilLoop.exit();
}

void TestController::notifyDone()
{
    m_runLoop->notifyDone();
}

void TestController::platformInitialize()
{
    m_runLoop = new RunLoopQt;
    QQuickWebView::platformInitialize();
}

WKPreferencesRef TestController::platformPreferences()
{
    return WKPageGroupGetPreferences(m_pageGroup.get());
}

void TestController::platformDestroy()
{
    delete m_runLoop;
}

void TestController::platformConfigureViewForTest(const TestInvocation&)
{
    WKPageSetApplicationNameForUserAgent(mainWebView()->page(), WKStringCreateWithUTF8CString("WebKitTestRunnerQt"));
}

void TestController::platformResetPreferencesToConsistentValues()
{
}

void TestController::setHidden(bool)
{
}

void TestController::updatePlatformSpecificTestOptionsForTest(TestOptions&, const std::string& pathOrURL) const
{
}

WKContextRef TestController::platformContext()
{
    return m_context.get();
}

void TestController::abortModal()
{
}

void TestController::platformRunUntil(bool& condition, WTF::Seconds timeout)
{
    UNUSED_PARAM(condition);
    const bool shouldTimeout = !(qgetenv("QT_WEBKIT2_DEBUG") == "1" || timeout.value() <= 0);
    m_runLoop->runUntil(shouldTimeout ? timeout.value() : 0);
}

static bool isExistingLibrary(const QString& path)
{
#if OS(WINDOWS)
    const char* librarySuffixes[] = { ".dll" };
#elif OS(MAC_OS_X)
    const char* librarySuffixes[] = { ".bundle", ".dylib", ".so" };
#elif OS(UNIX)
    const char* librarySuffixes[] = { ".so" };
#else
#error Library path suffix should be specified for this platform
#endif
    for (unsigned i = 0; i < sizeof(librarySuffixes) / sizeof(const char*); ++i) {
        if (QLibrary::isLibrary(path + librarySuffixes[i]))
            return true;
    }

    return false;
}

void TestController::initializeInjectedBundlePath()
{
    QString path = QLatin1String(getenv("WTR_INJECTEDBUNDLE_PATH"));
    if (path.isEmpty())
        path = QFileInfo(QCoreApplication::applicationDirPath() + "/../lib/libWTRInjectedBundle").absoluteFilePath();
    if (!isExistingLibrary(path))
        qFatal("Cannot find the injected bundle at %s\n", qPrintable(path));

    m_injectedBundlePath.adopt(WKStringCreateWithQString(path));
}

void TestController::initializeTestPluginDirectory()
{
    m_testPluginDirectory.adopt(WKStringCreateWithUTF8CString(qgetenv("QTWEBKIT_PLUGIN_PATH").constData()));
}

void TestController::platformInitializeContext()
{
}

void TestController::runModal(PlatformWebView* view)
{
    singleton().m_runLoop->runModal(view);
}

const char* TestController::platformLibraryPathForTesting()
{
    return 0;
}

} // namespace WTR

#include "TestControllerQt.moc"
