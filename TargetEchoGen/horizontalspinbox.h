#pragma once

#include <QWidget>

class QLineEdit;
class QToolButton;

class HorizontalSpinBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)

public:
    explicit HorizontalSpinBox(QWidget *parent = nullptr);

    int value() const;
    void setValue(int v);

    int minimum() const;
    void setMinimum(int m);

    int maximum() const;
    void setMaximum(int m);

    int singleStep() const;
    void setSingleStep(int s);

signals:
    void valueChanged(int newValue);
    void editingFinished();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onDecClicked();
    void onIncClicked();
    void onEditingFinished();
    void onTextEdited(const QString &text);

private:
    void updateButtons();
    int clamp(int v) const;

    QLineEdit *m_edit;
    QToolButton *m_btnDec;
    QToolButton *m_btnInc;

    int m_value;
    int m_min;
    int m_max;
    int m_step;
};
