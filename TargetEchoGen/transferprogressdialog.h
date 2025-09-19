#ifndef TRANSFERPROGRESSDIALOG_H
#define TRANSFERPROGRESSDIALOG_H
#include <qobject.h>
#include <QDialog>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include "log.h"
class TransferProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit TransferProgressDialog(QWidget* parent = nullptr)
        : QDialog(parent), canceled(false)
    {
        setWindowTitle("File Transfer Progress");
        setModal(true);

        label = new QLabel("Transferring file...", this);
        progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);

        cancelButton = new QPushButton("Cancel", this);
        cancelButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #d9534f;"       // Bootstrap-style red
            "  color: white;"
            "  border: none;"
            "  padding: 6px 12px;"
            "  border-radius: 4px;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "  background-color: #c9302c;"       // Darker red on hover
            "}"
            "QPushButton:pressed {"
            "  background-color: #ac2925;"       // Even darker when pressed
            "}"
            );

        connect(cancelButton, &QPushButton::clicked, this, &TransferProgressDialog::onCancelClicked);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(label);
        layout->addWidget(progressBar);
        layout->addWidget(cancelButton);
        setLayout(layout);
        resize(300, 120);
    }

    void updateProgress(int percent) {
        progressBar->setValue(percent);
    }

    bool wasCanceled() const {
        LOG_DEBUG("Cancel clicked\n");
        return canceled;
    }

signals:
    void cancelRequested();

private slots:
    void onCancelClicked() {
        canceled = true;
        LOG_INFO("Cancel button clicked");
        emit cancelRequested();
    }

private:
    QLabel* label;
    QProgressBar* progressBar;
    QPushButton* cancelButton;
    bool canceled;
};


#endif // TRANSFERPROGRESSDIALOG_H
