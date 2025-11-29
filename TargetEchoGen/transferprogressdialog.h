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
        progressBar->setStyleSheet(
            "QProgressBar {"
            "    min-height: 30px;"
            "    max-height: 50px;"
            "}"
            );
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        cancelButton = new QPushButton("Cancel", this);
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
