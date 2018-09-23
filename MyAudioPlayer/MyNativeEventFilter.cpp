#include "MyNativeEventFilter.h"
#include <qdebug.h>
#include <qstring.h>
#include "MyAudioPlayer.h"

void MyNativeEventFilter::setMainWidget(MyAudioPlayer *w) {
	m_w = w;
}

bool MyNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
	if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
		MSG * pMsg = reinterpret_cast<MSG *>(message);
		if (pMsg->message == WM_POWERBROADCAST) {
			if (pMsg->wParam == PBT_APMSUSPEND) {
				if (m_w)
					m_w->pause();
			}
		}
		else if (pMsg->message == WM_QUERYENDSESSION) {
			if(m_w)
				m_w->quit();
		}
	}
	return false;
}