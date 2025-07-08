// #include "matfileprocessing.h"
// #include <functional>

// RadarSession::RadarSession() : sessionDoesNotEnd(false), counter(0), PRT(1.0) {
//     period = std::chrono::microseconds(static_cast<int>(PRT));
// }

// void RadarSession::runTimer() {

//     while (sessionDoesNotEnd) {
//             handleInterrupt();
//          std::this_thread::sleep_for(period);
//     }
// }

// void RadarSession::startSession(){

//     sessionDoesNotEnd = true;
//     loadData();
//         timerThread = std::thread(std::bind(&RadarSession::runTimer, this));
//         timerThread.detach();
//         // Start the motion data processing in a separate thread
//         std::thread motionDataThread(&RadarSession::processMotionData, this);
//         motionDataThread.detach();
//         // Start the platform attitude update in a separate thread
//         std::thread platformAttitudeThread(&RadarSession::updatePlatformAttitude, this); // New thread
//         platformAttitudeThread.detach();

//         // Start the pulse amplitude update in a separate thread
//         std::thread pulseAmplitudeThread(&RadarSession::updatePulseAmplitude, this); // New thread
//          pulseAmplitudeThread.detach();


// }

// void RadarSession::stopSession() {
//     sessionDoesNotEnd = false;
//     if (timerThread.joinable()) {
//         timerThread.join();
//     }
// }

// void RadarSession::handleInterrupt() {
//     // TODO: Add interrupt handling logic
//     counter++;
// }

// void RadarSession::loadData() {
//     // Import data (pseudo-code, replace with actual file reading)
//     std::ifstream input_sourceofmotiondata("input_sourceofmotiondata.txt");
//     std::ifstream input_sourceofplatformattitudedata_file("input_sourceofplatformattitudedata.txt");
//     std::ifstream buttoninfo_considerradarlookanglesGUI("buttoninfo_considerradarlookanglesGUI.txt");
//     std::ifstream input_sourceofLookangle_None0_Zero1_File2_Code3_Eth4("input_sourceofLookangle_None0_Zero1_File2_Code3_Eth4.txt");
//     std::ifstream buttoninfo_targetazfromFile("buttoninfo_targetazfromFile.txt");
//     std::ifstream buttoninfo_loadtargetcoordinatesfromGUI("buttoninfo_loadtargetcoordinatesfromGUI.txt");
//     std::ifstream input_frame("input_frame.txt");
//     std::ifstream file("Pulsewisecsvfile.csv");

//     if (buttoninfo_loadtargetcoordinatesfromGUI) {
//         std::ifstream targetcoordinates("targetcoordinates.txt");
//     }

//     // Example of setting the input_sourceofplatformattitudedata variable
//     if (input_sourceofplatformattitudedata_file.is_open()) {
//         std::getline(input_sourceofplatformattitudedata_file, input_sourceofplatformattitudedata);
//     }
//     // Example of setting the RadarLookAngleDataavlonNetwork variable
//     RadarLookAngleDataavlonNetwork = true; // Set based on your logic
// }


// void RadarSession::updatePlatformAttitude() {
// #if 0
//     while (sessionDoesNotEnd) {

//         if (input_sourceofplatformattitudedata == "1553B") {
//             RTYaw = From1553B; // just keep updating from latest packet
//         } else if (input_sourceofplatformattitudedata == "Eth") {
//             RTYaw = FromUDPPacket; // just keep updating from latest packet
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(12.5)); // 12.5ms rate
//     }
// #endif
// }


// void RadarSession::updatePulseAmplitude() {
//     while (sessionDoesNotEnd) {
//         if (RadarLookAngleDataavlonNetwork) {
//             RTGimbalAz = FromUDPPacket; // just keep updating from latest packet
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10ms rate
//     }
// }
// void RadarSession::process_file_data() {
//     for (int i = 0; i < file.RadiationOn.size(); ++i) {
//         std::vector<double> rx(LenPulse, 0.0);
//         double scalingBasic = mapdBmtofactorAsPer0dBmofDAC;

//         double InterpolatedX = 0;
//         double InterpolatedZ = 0;
//         double InterpolatedY = 0;

//         if (RadiationOn(i) == 1) {
//             if (buttoninfo_considerradarlookanglesGUI == 0) {
//                 finalinput_sourceofLookangle[i] = file.sourceofLookangle[i] & file.consider_radar_angles;
//             } else {
//                 finalinput_sourceofLookangle[i] = input_sourceofLookangle;
//             }

//             std::string source = finalinput_sourceofLookangle[i];

//             if (source == "None") {
//                 VisibleZoneStart[i] = -10000;
//                 VisibleZoneEnd[i] = 10000;
//             }
//             else if (source == "Code") {
//                 int targetno = 0;
//                 if (buttoninfo_loadtargetcoordinatesfromGUI == 1) {
//                     if (targetcoordinatesfromGUI.Xt.size() > 1) {
//                         for (size_t j = 0; j < targetcoordinatesfromGUI.DecidesVisibleZone.size(); ++j) {
//                             if (targetcoordinatesfromGUI.DecidesVisibleZone[j] == 1) {
//                                 targetno = j;
//                                 break;
//                             }
//                         }
//                     }
//                 }

//                 double timepassed = TNow - T0;

//                 targetcoordinatesforzone[i][0] = targetcoordinatesfromGUI.Xt[targetno] + targetcoordinatesfromGUI.vxt[targetno] * timepassed + 0.5 * axt[targetno] * pow(timepassed, 2);
//                 targetcoordinatesforzone[i][1] = targetcoordinatesfromGUI.Yt[targetno] + targetcoordinatesfromGUI.vyt[targetno] * timepassed + 0.5 * ayt[targetno] * pow(timepassed, 2);
//                 targetcoordinatesforzone[i][2] = targetcoordinatesfromGUI.Zt[targetno] + targetcoordinatesfromGUI.vzt[targetno] * timepassed + 0.5 * azt[targetno] * pow(timepassed, 2);

//                 double InterpolatedX, InterpolatedY, InterpolatedZ;
//                 if (input_sourceofmotiondata == "File") {
//                     InterpolatedX = file.X[i];
//                     InterpolatedY = file.y[i];
//                     InterpolatedZ = file.Z[i];
//                 } else if (input_sourceofmotiondata == "1553B" || input_sourceofmotiondata == "Eth") {
//                     InterpolatedX = RTX;
//                     InterpolatedY = RTY;
//                     InterpolatedZ = RTZ;
//                 }

//                 if (input_frame == "Type1") {
//                     LookAngle[i] = atan2(targetcoordinatesforzone[i][2] - InterpolatedZ, targetcoordinatesforzone[i][0] - InterpolatedX) * 180.0 / M_PI;
//                     VisibleZoneStart[i] = LookAngle[i] - 5;
//                     VisibleZoneEnd[i] = LookAngle[i] + 5;
//                 }
//             }
//             else if (source == "Zero") {
//                 LookAngle1[i] = 0;
//                 if (input_sourceofplatformattitudedata == "File") {
//                     VisibleZoneStart[i] = -file.Yaw[i] + LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = -file.Yaw[i] + LookAngle1[i] + 5;
//                 } else if (input_sourceofplatformattitudedata == "1553B" || input_sourceofplatformattitudedata == "Eth") {
//                     VisibleZoneStart[i] = -RTYaw + LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = -RTYaw + LookAngle1[i] + 5;
//                 } else if (input_sourceofplatformattitudedata == "Zero") {
//                     VisibleZoneStart[i] = LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = LookAngle1[i] + 5;
//                 }
//             }
//             else if (source == "File") {
//                 LookAngle1[i] = file.GimbalAz[i];
//                 if (input_sourceofplatformattitudedata == "File") {
//                     VisibleZoneStart[i] = -file.Yaw[i] + LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = -file.Yaw[i] + LookAngle1[i] + 5;
//                 } else if (input_sourceofplatformattitudedata == "1553B" || input_sourceofplatformattitudedata == "Eth") {
//                     VisibleZoneStart[i] = -RTYaw + LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = -RTYaw + LookAngle1[i] + 5;
//                 } else if (input_sourceofplatformattitudedata == "Zero") {
//                     VisibleZoneStart[i] = LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = LookAngle1[i] + 5;
//                 }
//             }
//             else if (source == "Eth") {
//                 LookAngle1[i] = RTGimbalAz;
//                 if (input_sourceofplatformattitudedata == "File") {
//                     VisibleZoneStart[i] = -file.Yaw[i] + LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = -file.Yaw[i] + LookAngle1[i] + 5;
//                 } else if (input_sourceofplatformattitudedata == "1553B" || input_sourceofplatformattitudedata == "Eth") {
//                     VisibleZoneStart[i] = -RTYaw + LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = -RTYaw + LookAngle1[i] + 5;
//                 } else if (input_sourceofplatformattitudedata == "Zero") {
//                     VisibleZoneStart[i] = LookAngle1[i] - 5;
//                     VisibleZoneEnd[i] = LookAngle1[i] + 5;
//                 }
//             }
//         }
//         if (std::isnan(Delay[i])) {
//             if (std::isnan(Range[i])) {
//                 if (buttoninfo_loadtargetcoordinatesfromGUI == 1) {
//                     if (std::isnan(file.Xt[i]) || std::isnan(file.Yt[i]) || std::isnan(file.Zt[i])) {
//                         double timepassed = TNow - T0;

//                         for (size_t targetno = 0; targetno < targetcoordinatesfromGUI.Xt.size(); ++targetno) {
//                             targetcoordinates[targetno][0] = targetcoordinatesfromGUI.Xt[targetno] + targetcoordinatesfromGUI.vxt[targetno] * timepassed + 0.5 * axt[targetno] * pow(timepassed, 2);
//                             targetcoordinates[targetno][1] = targetcoordinatesfromGUI.Yt[targetno] + targetcoordinatesfromGUI.vyt[targetno] * timepassed + 0.5 * ayt[targetno] * pow(timepassed, 2);
//                             targetcoordinates[targetno][2] = targetcoordinatesfromGUI.Zt[targetno] + targetcoordinatesfromGUI.vzt[targetno] * timepassed + 0.5 * azt[targetno] * pow(timepassed, 2);
//                             targetcoordinates[targetno][3] = MaptargetcoordinatesfromGUI.PowerOffset[targetno].Toscaling;
//                             targetcoordinates[targetno][4] = targetcoordinatesfromGUI.DecidesVisibleZone[targetno];
//                         }
//                     } else {
//                         scaling = scalingBasic;
//                         targetcoordinates = {{file.Xt[i], file.Yt[i], file.Zt[i], scaling, 1}};
//                     }
//                 } else {
//                     scaling = scalingBasic;
//                     targetcoordinates = {{file.Xt[i], file.Yt[i], file.Zt[i], scaling, 1}};
//                 }

//                 if (InterpolatedX == 0) {
//                     if (input_sourceofmotiondata == "File") {
//                         InterpolatedX = file.X[i];
//                         InterpolatedY = file.y[i];
//                         InterpolatedZ = file.Z[i];
//                     } else if (input_sourceofmotiondata == "1553B" || input_sourceofmotiondata == "Eth") {
//                         InterpolatedX = RTX;
//                         InterpolatedY = RTY;
//                         InterpolatedZ = RTZ;
//                     }
//                 }

//                 for (size_t targetno = 0; targetno < targetcoordinates.size(); ++targetno) {
//                     double Xt = targetcoordinates[targetno][0];
//                     double Yt = targetcoordinates[targetno][1];
//                     double Zt = targetcoordinates[targetno][2];
//                     double scaling = targetcoordinates[targetno][3];
//                     int decidesVisibleZone = targetcoordinates[targetno][4];

//                     if (finalinput_sourceofLookangle[i] == "None") {
//                         TargetOnOff[targetno] = 1;
//                     } else {
//                         double AngleOfTarget = 0;
//                         if (buttoninfo_targetazfromFile == 1) {
//                             AngleOfTarget = file.TargetAz[i];
//                         } else if (input_frame == "Type1") {
//                             AngleOfTarget = atan2(Zt - InterpolatedZ, Xt - InterpolatedX) * 180.0 / M_PI;
//                         }

//                         TargetOnOff[targetno] = (AngleOfTarget >= VisibleZoneStart[i] && AngleOfTarget <= VisibleZoneEnd[i]) ? 1 : 0;
//                     }

//                     if (TargetOnOff[targetno]) {
//                         std::vector<double> T;
//                         for (double t = 0; t < PW[i]; t += 1.0 / f_dec) {
//                             T.push_back(t);
//                         }

//                         double rangetemp = std::hypot(std::hypot(Zt - InterpolatedZ, Xt - InterpolatedX), Yt - InterpolatedY);
//                         double timedelay = 2 * rangetemp / speed_light;

//                         if (!std::isnan(timedelay)) {
//                             if (decidesVisibleZone == 1) {
//                                 PRT = MapRangeToPRTinPWLUT(rangetemp);
//                             }

//                             std::vector<std::complex<double>> rxxtemp(LenPulse, {0, 0});
//                             for (size_t j = 0; j < T.size(); ++j) {
//                                 double phase = 2 * M_PI * (-b / 2) * T[j] + M_PI * k * T[j] * T[j];
//                                 rxxtemp[timedelay * f_dec + j] = scaling * std::polar(1.0, phase);
//                             }

//                             for (size_t j = 0; j < LenPulse; ++j) {
//                                 rx[j] += rxxtemp[j];
//                             }
//                         }
//                     }
//                 }
//                 generatesignal = 0;
//             }
//         }
//         if (std::isnan(Delay[i])) {
//             if (!std::isnan(Range[i])) {
//                 double timedelay = Range[i] * 2 / speed_light;
//                 PRT = MapRangeToPRTinPWLUT(Range[i]);

//                 if (buttoninfo_targetazfromFile == 1) {
//                     if (file.TargetAz[i] >= VisibleZoneStart[i] && file.TargetAz[i] <= VisibleZoneEnd[i]) {
//                         generatesignal = 1;
//                     } else {
//                         generatesignal = 0;
//                         rx.assign(LenPulse, std::complex<double>(0, 0));
//                     }
//                 }
//             }
//         } else {
//             double timedelay = Delay[i] * 1e-6;
//             double rangetemp = timedelay * speed_light / 2;
//             PRT = MapRangeToPRTinPWLUT(rangetemp);

//             if (buttoninfo_targetazfromFile == 1) {
//                 if (file.TargetAz[i] >= VisibleZoneStart[i] && file.TargetAz[i] <= VisibleZoneEnd[i]) {
//                     generatesignal = 1;
//                 } else {
//                     rx.assign(LenPulse, std::complex<double>(0, 0));
//                     generatesignal = 0;
//                 }
//             }
//         }

//         if (generatesignal) {
//             std::vector<double> T;
//             for (double t = 0; t < PW[i]; t += 1.0 / f_dec) {
//                 T.push_back(t);
//             }

//             scaling = scalingBasic;
//             std::vector<std::complex<double>> rxxtemp(LenPulse, {0, 0});

//             std::string waveform = WaveformType[i];
//             double k = 0;
//             if (waveform == "LFM") {
//                 double b = WaveformParam[i];
//                 k = b / PW[i];
//                 if (!std::isnan(timedelay)) {
//                     for (size_t j = 0; j < T.size(); ++j) {
//                         double phase = 2 * M_PI * (-b / 2) * T[j] + M_PI * k * T[j] * T[j];
//                         rxxtemp[timedelay * f_dec + j] = scaling * std::polar(1.0, phase);
//                     }
//                 }
//             } else if (waveform == "SIN") {
//                 for (size_t j = 0; j < T.size(); ++j) {
//                     rxxtemp[timedelay * f_dec + j] = scaling * sin(2 * M_PI * WaveformParam[i] * T[j]);
//                 }
//             } else if (waveform == "COS") {
//                 for (size_t j = 0; j < T.size(); ++j) {
//                     rxxtemp[timedelay * f_dec + j] = scaling * cos(2 * M_PI * WaveformParam[i] * T[j]);
//                 }
//             } else if (waveform == "CONST") {
//                 for (size_t j = 0; j < T.size(); ++j) {
//                     rxxtemp[timedelay * f_dec + j] = scaling * WaveformParam[i];
//                 }
//             } else if (waveform == "GAUSSIAN") {
//                 rxxtemp = awgn(std::vector<std::complex<double>>(LenPulse, {0, 0}), WaveformParam[i]);
//             } else if (waveform == "Waveform1") {
//                 insertWaveform(rxxtemp, Waveform1, timedelay);
//             } else if (waveform == "Waveform2") {
//                 insertWaveform(rxxtemp, Waveform2, timedelay);
//             } else if (waveform == "Waveform3") {
//                 insertWaveform(rxxtemp, Waveform3, timedelay);
//             } else if (waveform == "Waveform4") {
//                 insertWaveform(rxxtemp, Waveform4, timedelay);
//             } else if (waveform == "Waveform5") {
//                 insertWaveform(rxxtemp, Waveform5, timedelay);
//             } else if (waveform == "Waveform6") {
//                 insertWaveform(rxxtemp, Waveform6, timedelay);
//             }

//             rx.assign(rxxtemp.begin(), rxxtemp.begin() + LenPulse);
//         } else {
//             rx.assign(LenPulse, std::complex<double>(0, 0));
//         }

//         if (AddNoise[i] == 1) {
//             if (std::isnan(NoisedBm[i])) {
//                 NoisedBm[i] = 0;
//             }
//             rx = awgn(rx, NoisedBm[i]);
//         }

//         if (sessionended) {
//             sessiondoesnotend = 0;
//         }
// }


// double MapRangeToPRTinPWLUT(double range, const std::map<double, double>& lut) {
//     auto upper = lut.lower_bound(range);
//     if (upper == lut.begin()) return upper->second;
//     if (upper == lut.end()) return std::prev(upper)->second;

//     auto lower = std::prev(upper);

//     double x0 = lower->first, y0 = lower->second;
//     double x1 = upper->first, y1 = upper->second;

//     // Linear interpolation
//     return y0 + (range - x0) * (y1 - y0) / (x1 - x0);
// }
