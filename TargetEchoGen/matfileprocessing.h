#ifndef MATFILEPROCESSING_H
#define MATFILEPROCESSING_H

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cmath>

class RadarSession {
public:
    RadarSession();
    void startSession();
    void stopSession();
    void runTimer();

private:
    void handleInterrupt();
    void loadData();
    void processMotionData();
    void updatePlatformAttitude();
    void updatePulseAmplitude();
    void process_file_data();


    const double speed_light = 299792458;
    std::atomic<bool> sessionDoesNotEnd;
    std::atomic<int> counter;
    double PRT;
    double RTX, RTY, RTZ;
    double RTYaw;

    std::string input_sourceofplatformattitudedata; // Defined here
    bool RadarLookAngleDataavlonNetwork; // Assuming this is a boolean

    std::chrono::microseconds period;
    std::thread timerThread;
};

#endif // MATFILEPROCESSING_H
