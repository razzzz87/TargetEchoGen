#include "rf.h"
#include "ui_rf.h"
#include "dachelper.h"

RF::RF(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RF)
{
    ui->setupUi(this);
    ui->SpDAC3FixedLevel->setRange(-27,0);
}

RF::~RF()
{
    delete ui;
}

void RF::on_PbDAC3TgrSetup_clicked()
{
    uint32_t trigSourceSelect = 0;
    uint32_t pwSamples = 0;
    uint32_t signalDelay= 0;
    uint32_t pulseGap = 0;
    uint32_t pulseWidth= 0;
    uint32_t triggerEnable=1;

    pulseWidth = ui->LeDAC3TgrPulseWidthSample->text().toUInt();
    pulseGap = ui->LeDAC3TgrPulseGapSample->text().toUInt();

    DacHelper::LxTriggerSetup(eETHPL1G,trigSourceSelect,pwSamples,signalDelay,pulseGap,pulseWidth,triggerEnable);
}

void RF::on_PbDAC3TgrStop_clicked()
{
    DacHelper::LxTriggerStop(eETHPL1G);
}

void RF::on_PbDAC3TgrStart_clicked()
{
    DacHelper::LxTriggerStart(eETHPL1G);
}

void RF::on_PbDAC3FixedLevelSet_clicked()
{
    if(ui->RbDAC3FixedLevel->isChecked())
    {
        uint32_t dbm = ui->SpDAC3FixedLevel->value();
        DacHelper::FixAmplSetting(eETHPL1G,dbm);
    }
}

