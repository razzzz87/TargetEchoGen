#ifndef CONNECTIONCTX_H
#define CONNECTIONCTX_H
#include <QObject>
#include "Utils.h"

class ConnectionHelper : public QObject {
    Q_OBJECT
public:
    static ConnectionHelper& instance();

    // --- getters ---
    iface activeInterface() const;
    ConnInfo info(iface f) const;
    bool isActiveConnected() const;

    iface selectedInterface() const;
    bool isSelectedConnected() const;

public slots:
    void setSelected(iface sel);
    void setConnected(iface f, bool on, const ConnInfo& extra = {});
    void setActive(iface f);

signals:
    void stateChanged(iface which, ConnInfo state);
    void activeChanged(iface active);
    void selectedChanged(iface selected);

private:
    ConnectionHelper(); // private constructor for singleton
    Q_DISABLE_COPY(ConnectionHelper)

    QMap<iface, ConnInfo> m_state;
    iface m_active = eNONE;
    iface m_selected = eNONE;
};

#endif // CONNECTIONCTX_H
