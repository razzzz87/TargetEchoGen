#include "connectionctx.h"
// --- Singleton instance ---
ConnectionHelper& ConnectionHelper::instance() {
    static ConnectionHelper s;
    return s;
}

// --- Constructor ---
ConnectionHelper::ConnectionHelper() {
    // Initialize all entries so lookups are safe
    for (iface f : { eNONE, eETHPS1G, eETHPL1G, eETH10G, eSERIAL, ePLSERIAL, ePCIe })
        m_state[f] = {};
}

// --- Getters ---
iface ConnectionHelper::activeInterface() const {
    return m_active;
}

ConnInfo ConnectionHelper::info(iface f) const {
    return m_state.value(f, {});
}

bool ConnectionHelper::isActiveConnected() const {
    return m_active != eNONE && m_state.value(m_active).connected;
}

iface ConnectionHelper::selectedInterface() const {
    return m_selected;
}

bool ConnectionHelper::isSelectedConnected() const {
    return m_selected != eNONE && m_state.value(m_selected).connected;
}

// --- Slots ---
void ConnectionHelper::setSelected(iface sel) {
    if (m_selected == sel) return;
    m_selected = sel;
    emit selectedChanged(m_selected);
}

void ConnectionHelper::setConnected(iface f, bool on, const ConnInfo& extra) {
    ConnInfo s = m_state.value(f, {});
    s.connected = on;
    if (!extra.ip.isEmpty())        s.ip = extra.ip;
    if (extra.port)                 s.port = extra.port;
    if (!extra.serialPort.isEmpty()) s.serialPort = extra.serialPort;

    m_state[f] = s;
    emit stateChanged(f, s);

    // if we just disconnected the active iface, clear active
    if (f == m_active && !on)
        setActive(eNONE);
}

void ConnectionHelper::setActive(iface f) {
    if (m_active == f) return;

    if (m_active != eNONE) {
        ConnInfo prev = m_state[m_active];
        prev.active = false;
        m_state[m_active] = prev;
        emit stateChanged(m_active, prev);
    }

    m_active = f;

    if (m_active != eNONE) {
        ConnInfo cur = m_state[m_active];
        cur.active = true;
        m_state[m_active] = cur;
        emit stateChanged(m_active, cur);
    }

    emit activeChanged(m_active);
}
