#include "HorizontalSpinBox.h"
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QWheelEvent>
#include <QKeyEvent>

HorizontalSpinBox::HorizontalSpinBox(QWidget *parent)
    : QWidget(parent),
    m_edit(new QLineEdit(this)),
    m_btnDec(new QToolButton(this)),
    m_btnInc(new QToolButton(this)),
    m_value(0),
    m_min(0),
    m_max(100),
    m_step(1)
{
    m_btnDec->setText("<");
    m_btnInc->setText(">");
    m_btnDec->setCursor(Qt::PointingHandCursor);
    m_btnInc->setCursor(Qt::PointingHandCursor);

    m_edit->setValidator(new QIntValidator(m_min, m_max, this));
    m_edit->setAlignment(Qt::AlignCenter);
    m_edit->setText(QString::number(m_value));
    m_edit->installEventFilter(this);

    auto l = new QHBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(4);
    l->addWidget(m_btnDec);
    l->addWidget(m_edit, 1);
    l->addWidget(m_btnInc);

    connect(m_btnDec, &QToolButton::clicked, this, &HorizontalSpinBox::onDecClicked);
    connect(m_btnInc, &QToolButton::clicked, this, &HorizontalSpinBox::onIncClicked);
    connect(m_edit, &QLineEdit::editingFinished, this, &HorizontalSpinBox::onEditingFinished);
    connect(m_edit, &QLineEdit::textEdited, this, &HorizontalSpinBox::onTextEdited);

    // default minimum sizes to match your stylesheet later
    m_btnDec->setFixedSize(26, 26);
    m_btnInc->setFixedSize(26, 26);
    setFocusProxy(m_edit);
    updateButtons();
}

int HorizontalSpinBox::value() const { return m_value; }

void HorizontalSpinBox::setValue(int v)
{
    int nv = clamp(v);
    if (nv == m_value) return;
    m_value = nv;
    m_edit->setText(QString::number(m_value));
    updateButtons();
    emit valueChanged(m_value);
}

int HorizontalSpinBox::minimum() const { return m_min; }
void HorizontalSpinBox::setMinimum(int m) { m_min = m; if (m_max < m_min) m_max = m_min; m_edit->setValidator(new QIntValidator(m_min, m_max, this)); setValue(m_value); }

int HorizontalSpinBox::maximum() const { return m_max; }
void HorizontalSpinBox::setMaximum(int m) { m_max = m; if (m_min > m_max) m_min = m_max; m_edit->setValidator(new QIntValidator(m_min, m_max, this)); setValue(m_value); }

int HorizontalSpinBox::singleStep() const { return m_step; }
void HorizontalSpinBox::setSingleStep(int s) { m_step = qMax(1, s); }

void HorizontalSpinBox::onDecClicked()
{
    setValue(m_value - m_step);
}

void HorizontalSpinBox::onIncClicked()
{
    setValue(m_value + m_step);
}

void HorizontalSpinBox::onEditingFinished()
{
    bool ok = false;
    int v = m_edit->text().toInt(&ok);
    if (ok) setValue(v); else m_edit->setText(QString::number(m_value));
    emit editingFinished();
}

void HorizontalSpinBox::onTextEdited(const QString &text)
{
    Q_UNUSED(text);
    // optional live validation/feedback
}

int HorizontalSpinBox::clamp(int v) const
{
    if (v < m_min) return m_min;
    if (v > m_max) return m_max;
    return v;
}

void HorizontalSpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus()) { QWidget::wheelEvent(event); return; }
    int delta = event->angleDelta().y();
    if (delta > 0) setValue(m_value + m_step); else if (delta < 0) setValue(m_value - m_step);
    event->accept();
}

bool HorizontalSpinBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_edit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Up) { setValue(m_value + m_step); return true; }
            if (ke->key() == Qt::Key_Down) { setValue(m_value - m_step); return true; }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void HorizontalSpinBox::updateButtons()
{
    m_btnDec->setEnabled(m_value > m_min);
    m_btnInc->setEnabled(m_value < m_max);
}
