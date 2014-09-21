/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DIRECTSHOWPLAYERSERVICE_H
#define DIRECTSHOWPLAYERSERVICE_H

#include "qmediaplayer.h"
#include "qmediaresource.h"
#include "qmediaservice.h"
#include "qmediatimerange.h"

#include "directshoweventloop.h"
#include "directshowglobal.h"

#include <QtCore/qcoreevent.h>
#include <QtCore/qmutex.h>
#include <QtCore/qurl.h>
#include <QtCore/qwaitcondition.h>

class DirectShowAudioEndpointControl;
class DirectShowMetaDataControl;
class DirectShowPlayerControl;
class DirectShowVideoRendererControl;
#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
class Vmr9VideoWindowControl;
#endif

QT_BEGIN_NAMESPACE
class QMediaContent;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class DirectShowPlayerService : public QMediaService
{
    Q_OBJECT
public:
    enum StreamType
    {
        AudioStream = 0x01,
        VideoStream = 0x02
    };

    DirectShowPlayerService(QObject *parent = 0);
    ~DirectShowPlayerService();

    QMediaControl* requestControl(const char *name);
    void releaseControl(QMediaControl *control);

    void load(const QMediaContent &media, QIODevice *stream);
    void play();
    void pause();
    void stop();

    qint64 position() const;
    QMediaTimeRange availablePlaybackRanges() const;

    void seek(qint64 position);
    void setRate(qreal rate);

    int bufferStatus() const;

    void setAudioOutput(IBaseFilter *filter);
    void setVideoOutput(IBaseFilter *filter);

protected:
    void customEvent(QEvent *event);

private Q_SLOTS:
    void videoOutputChanged();

private:
    void releaseGraph();
    void updateStatus();

    int findStreamTypes(IBaseFilter *source) const;
    int findStreamType(IPin *pin) const;

    bool isConnected(IBaseFilter *filter, PIN_DIRECTION direction) const;
    IBaseFilter *getConnected(IBaseFilter *filter, PIN_DIRECTION direction) const;

    void run();

    void doSetUrlSource(QMutexLocker *locker);
    void doSetStreamSource(QMutexLocker *locker);
    void doRender(QMutexLocker *locker);
    void doFinalizeLoad(QMutexLocker *locker);
    void doSetRate(QMutexLocker *locker);
    void doSeek(QMutexLocker *locker);
    void doPlay(QMutexLocker *locker);
    void doPause(QMutexLocker *locker);
    void doStop(QMutexLocker *locker);
    void doReleaseAudioOutput(QMutexLocker *locker);
    void doReleaseVideoOutput(QMutexLocker *locker);
    void doReleaseGraph(QMutexLocker *locker);

    void graphEvent(QMutexLocker *locker);

    enum Task
    {
        Shutdown           = 0x0001,
        SetUrlSource       = 0x0002,
        SetStreamSource    = 0x0004,
        SetSource          = SetUrlSource | SetStreamSource,
        SetAudioOutput     = 0x0008,
        SetVideoOutput     = 0x0010,
        SetOutputs         = SetAudioOutput | SetVideoOutput,
        Render             = 0x0020,
        FinalizeLoad       = 0x0040,
        SetRate            = 0x0080,
        Seek               = 0x0100,
        Play               = 0x0200,
        Pause              = 0x0400,
        Stop               = 0x0800,
        ReleaseGraph       = 0x1000,
        ReleaseAudioOutput = 0x2000,
        ReleaseVideoOutput = 0x4000,
        ReleaseFilters     = ReleaseGraph | ReleaseAudioOutput | ReleaseVideoOutput
    };

    enum Event
    {
        FinalizedLoad = QEvent::User,
        Error,
        RateChange,
        Started,
        Paused,
        DurationChange,
        StatusChange,
        EndOfMedia,
        PositionChange
    };

    enum GraphStatus
    {
        NoMedia,
        Loading,
        Loaded,
        InvalidMedia
    };

    DirectShowPlayerControl *m_playerControl;
    DirectShowMetaDataControl *m_metaDataControl;
    DirectShowVideoRendererControl *m_videoRendererControl;
#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
    Vmr9VideoWindowControl *m_videoWindowControl;
#endif
    DirectShowAudioEndpointControl *m_audioEndpointControl;

    QThread *m_taskThread;
    DirectShowEventLoop *m_loop;
    int m_pendingTasks;
    int m_executingTask;
    int m_executedTasks;
    HANDLE m_taskHandle;
    HANDLE m_eventHandle;
    GraphStatus m_graphStatus;
    QMediaPlayer::Error m_error;
    QIODevice *m_stream;
    IFilterGraph2 *m_graph;
    IBaseFilter *m_source;
    IBaseFilter *m_audioOutput;
    IBaseFilter *m_videoOutput;
    int m_streamTypes;
    qreal m_rate;
    qint64 m_position;
    qint64 m_duration;
    bool m_buffering;
    bool m_seekable;
    bool m_atEnd;
    QMediaTimeRange m_playbackRange;
    QUrl m_url;
    QMediaResourceList m_resources;
    QString m_errorString;
    QMutex m_mutex;

    friend class DirectShowPlayerServiceThread;

    // UWEBKIT

    // define the prototype of the class factory entry point in a COM dll
    typedef HRESULT (STDAPICALLTYPE* FN_DLLGETCLASSOBJECT)(REFCLSID clsid, REFIID iid, void** ppv);
 
    bool uwkAddGraphFilters();         

    IBaseFilter* m_vp8DecoderFilter;
    IBaseFilter* m_vorbisDecoderFilter;
    IBaseFilter* m_webmSplitterFilter;

    static void uwkLoadCodecs();

    // if for any reason setting up video failed (including loading codec dlls, getting/setting pins, etc)
    static bool s_uwkVideoFailed;

    static HMODULE s_vp8DecoderDLL;
    static HMODULE s_vorbisDecoderDLL;
    static HMODULE s_webmSplitterDLL;

    static IClassFactory* s_vp8DecoderFactory;
    static IClassFactory* s_vorbisDecoderFactory;
    static IClassFactory* s_webmSplitterFactory;

    static FN_DLLGETCLASSOBJECT s_vp8DecoderGetClassObject;
    static FN_DLLGETCLASSOBJECT s_vorbisDecoderGetClassObject;
    static FN_DLLGETCLASSOBJECT s_webmSplitterGetClassObject;

    static CLSID s_vp8DecoderCLSID;
    static CLSID s_vorbisDecoderCLSID;
    static CLSID s_webmSplitterCLSID;  


    // END UWEBKIT
};


#endif