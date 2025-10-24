#ifndef CONNECTIONCTX_H
#define CONNECTIONCTX_H
#include <QObject>
#include "Utils.h"

class ConnectionHelper : public QObject {
    Q_OBJECT
public:
    static ConnectionHelper& instance() {
        static ConnectionHelper s; return s;
    }

    // --- getters ---
    iface activeInterface() const { return m_active; }
    ConnInfo info(iface f) const  { return m_state.value(f, {}); }
    bool isActiveConnected() const { return m_active != eNONE && m_state.value(m_active).connected; }

    iface selectedInterface() const { return m_selected; }
    bool  isSelectedConnected() const {
        return m_selected != eNONE && m_state.value(m_selected).connected;
    }
public slots:
    void setSelected(iface sel)
    {
        if (m_selected == sel) return;
        m_selected = sel;
        emit selectedChanged(m_selected);
    }
    // Mark interface connected/disconnected (does not change active unless you want to)
    void setConnected(iface f, bool on, const ConnInfo& extra = {})
    {
        ConnInfo s = m_state.value(f, {});
        s.connected = on;
        if (!extra.ip.isEmpty())   s.ip = extra.ip;
        if (extra.port)            s.port = extra.port;
        if (!extra.serialPort.isEmpty()) s.serialPort = extra.serialPort;
        m_state[f] = s;
        emit stateChanged(f, s);
        // if we just disconnected the active iface, clear active
        if (f == m_active && !on) setActive(eNONE);
    }

    // Set exactly one active interface (enforced)
    void setActive(iface f)
    {
        if (m_active == f) return;
        if (m_active != eNONE)
        {
            ConnInfo prev = m_state[m_active];
            prev.active = false;
            m_state[m_active] = prev;
            emit stateChanged(m_active, prev);
        }
        m_active = f;
        if (m_active != eNONE)
        {
            ConnInfo cur = m_state[m_active];
            cur.active = true;
            m_state[m_active] = cur;
            emit stateChanged(m_active, cur);
        }
        emit activeChanged(m_active);
    }

signals:
    void stateChanged(iface which, ConnInfo state); // fires for any iface update
    void activeChanged(iface active);               // fires when active changes
    void selectedChanged(iface selected);

private:
    ConnectionHelper() {
        // init all entries so lookups are safe
        for (iface f : {eNONE, eETHPS1G, eETHPL1G, eETH10G, eSERIAL, ePLSERIAL, ePCIe})
            m_state[f] = {};
    }
    Q_DISABLE_COPY(ConnectionHelper)

    QMap<iface, ConnInfo> m_state;
    iface m_active = eNONE;
    iface m_selected = eNONE;

};

#endif // CONNECTIONCTX_H
