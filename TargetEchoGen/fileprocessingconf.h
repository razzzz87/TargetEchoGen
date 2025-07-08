// #if 0
// #ifndef FILEPROCESSINGCONF_H
// #define FILEPROCESSINGCONF_H
// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <cmath>
// #include <iostream>
// #include <string>
// using namespace std;
// class DataSource {
// public:
//     virtual void import() = 0;
// };

// class MotionDataSource : public DataSource {
// public:
//     void import() override {
//         std::cout << "Importing motion data source: 1553B, File, Ethernet, None" << std::endl;
//     }
// };

// class PlatformAttitudeDataSource : public DataSource {
// public:
//     void import() override {
//         std::cout << "Importing platform attitude data source: 1553B, File, Ethernet, Zero" << std::endl;
//     }
// };

// class RadarLookAnglesGUI {
// public:
//     bool considerRadarLookAngles() {
//         std::string decision;
//         std::cout << "Consider radar look angles in GUI (Yes/No): ";
//         std::cin >> decision;
//         return (decision == "Yes");
//     }
// };

// class LookAngleDataSource : public DataSource {
// public:
//     void import() override {
//         std::cout << "Importing look angle data source: None (0), Zero (1), File (2), Code (3), Ethernet (4)" << std::endl;
//     }
// };

// class TargetAzimuth {
// public:
//     bool loadFromFile() {
//         std::string decision;
//         std::cout << "Load target azimuth from file (Yes/No): ";
//         std::cin >> decision;
//         return (decision == "Yes");
//     }
// };

// class TargetCoordinates {
// public:
//     bool loadFromGUI() {
//         std::string decision;
//         std::cout << "Load target coordinates from GUI (Yes/No): ";
//         std::cin >> decision;
//         return (decision == "Yes");
//     }
// };

// class DataProcessor {
// public:
//     DataProcessor(const string& filename, bool loadTargetCoordinatesFromGUI, bool sourceOfMotionData, bool considerRadarLookAnglesGUI, bool targetAzFromFile, int sourceOfLookangle)
//         : buttoninfo_loadtargetcoordinatesfromGUI(loadTargetCoordinatesFromGUI),
//           input_sourceofmotiondata(sourceOfMotionData),
//           buttoninfo_considerradarlookanglesGUI(considerRadarLookAnglesGUI),
//           buttoninfo_targetazfromFile(targetAzFromFile),
//           input_sourceofLookangle_None0_Zero1_File2_Code3_Eth4(sourceOfLookangle) {
//         file = importCSV(filename);
//     }

//     void process() {
//         if (buttoninfo_loadtargetcoordinatesfromGUI) {
//             targetcoordinates = importCSV("targetcoordinates.csv");
//         }

//         checkCoordinatesFromGUI();
//         checkMotionData();
//         checkTargetCoordinates();
//         checkRadarLookAngles();
//     }

// private:
//     vector<vector<string>> file;
//     vector<vector<string>> targetcoordinates;
//     vector<double> Xt, Yt, Zt, Delay, Range, X, Y, Z, TargetAz, PW, WaveformType, WaveformParam, PWLUT, WaveformtypeLUT, WaveformParamLUT;
//     vector<int> AddNoise;
//     bool buttoninfo_loadtargetcoordinatesfromGUI;
//     bool input_sourceofmotiondata;
//     bool buttoninfo_considerradarlookanglesGUI;
//     bool buttoninfo_targetazfromFile;
//     int input_sourceofLookangle_None0_Zero1_File2_Code3_Eth4;
//     bool RadiationOn = false; // Example value

//     vector<vector<string>> importCSV(const string& filename) {
//         vector<vector<string>> data;
//         ifstream file(filename);
//         string line;
//         while (getline(file, line)) {
//             vector<string> row;
//             string cell;
//             stringstream lineStream(line);
//             while (getline(lineStream, cell, ',')) {
//                 row.push_back(cell);
//             }
//             data.push_back(row);
//         }
//         return data;
//     }


//     bool containsNaN(const vector<double>& vec) {
//         for (double v : vec) {
//                 if (isnan(v)) {
//                 return true;
//             }
//         }
//         return false;
//     }

//     void checkCoordinatesFromGUI() {
//         if (buttoninfo_loadtargetcoordinatesfromGUI) {
//             if (!Xt.empty() || !Yt.empty() || !Zt.empty()) {
//                 cout << "WARNING: Coordinates are from GUI, however Valid Entries of Coordinates found in file which won't be used" << endl;
//             }
//         }
//     }

//     void checkMotionData() {
//         if (!input_sourceofmotiondata) {
//             if (containsNaN(Delay)) {
//                 if (containsNaN(Range)) {
//                     vector<int> nanIndices;
//                     for (size_t i = 0; i < Range.size(); ++i) {
//                         if (isnan(Range[i]) && isnan(Delay[i])) {
//                             nanIndices.push_back(i);
//                         }
//                     }
//                     if (checkRadiationOn(nanIndices, RadiationOn)) {
//                         cout << "ERROR: NaN Entry of Xt/Yt/Zt found in file" << endl;
//                     }
//                 }
//             }
//         }

//         if (input_sourceofmotiondata) {
//             if (containsNaN(Delay)) {
//                 if (containsNaN(Range)) {
//                     if (containsNaN(X) || containsNaN(Y) || containsNaN(Z)) {
//                         vector<int> nanIndices;
//                         for (size_t i = 0; i < Range.size(); ++i) {
//                             if (isnan(Range[i]) && isnan(Delay[i]) && (isnan(X[i]) || isnan(Y[i]) || isnan(Z[i]))) {
//                                 nanIndices.push_back(i);
//                             }
//                         }
//                         if (checkRadiationOn(nanIndices, RadiationOn)) {
//                             cout << "ERROR: NaN Entry of Xt/Yt/Zt found in file" << endl;
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     bool checkRadiationOn(const vector<int>& indices, bool radiationOn) {
//         for (int idx : indices) {
//                 if (radiationOn) {
//                 return true;
//             }
//         }
//         return false;
//     }

//     void checkTargetCoordinates() {
//         if (!buttoninfo_loadtargetcoordinatesfromGUI && input_sourceofmotiondata) {
//             if (containsNaN(Delay)) {
//                 if (containsNaN(Range)) {
//                     if (containsNaN(Xt) || containsNaN(Yt) || containsNaN(Zt)) {
//                         vector<int> nanIndices;
//                         for (size_t i = 0; i < Range.size(); ++i) {
//                             if (isnan(Range[i]) && isnan(Delay[i]) && (isnan(Xt[i]) || isnan(Yt[i]) || isnan(Zt[i]))) {
//                                 nanIndices.push_back(i);
//                             }
//                         }
//                         if (checkRadiationOn(nanIndices, RadiationOn)) {
//                             cout << "ERROR: NaN Entry of Xt/Yt/Zt found in file" << endl;
//                         }

//                     }
//                 }
//             }
//         }

//         if (buttoninfo_loadtargetcoordinatesfromGUI) {
//             if (Xt.empty()) {
//                 cout << "ERROR: Enter Target Coordinates in GUI" << endl;
//             }
//         }
//     }

//     void checkRadarLookAngles() {
//         if (buttoninfo_considerradarlookanglesGUI) {
//             if (!input_sourceofmotiondata) {
//                 if (buttoninfo_targetazfromFile) {
//                     if (input_sourceofLookangle_None0_Zero1_File2_Code3_Eth4 == 0) {
//                         cout << "ERROR: Source of radar look angle have to be specified" << endl;
//                     }
//                     if (containsNaN(TargetAz)) {
//                         vector<int> nanIndices;
//                         for (size_t i = 0; i < TargetAz.size(); ++i) {
//                             if (isnan(TargetAz[i])) {
//                                 nanIndices.push_back(i);
//                             }
//                         }

//                         //if (any_of(nanIndices.begin(), nanIndices.end(), [&](inturn RadiationOn; })) {
//                         //    cout << "ERROR: NaN Entry of Target Az found in file" << endl;
//                         //}
//                     }
//                 }
//             }
//         }
//     }
//     void checkCompulsoryParameters() {
//         if (containsNaN(RadiationOn)) {
//             cout << "WARNING: Few Rows have NaN in RadiationOn, they will be considered as 0" << endl;
//         }

//         if (containsNaN(AddNoise)) {
//             cout << "WARNING: Few Rows have NaN in AddNoise, they will be considered as 0" << endl;
//         }

//         vector<int> indicesForAddNoise;
//         for (size_t i = 0; i < AddNoise.size(); ++i) {
//             if (AddNoise[i] == 1) {
//                 indicesForAddNoise.push_back(i);
//             }
//         }

//         for (int idx : indicesForAddNoise) {
//             if (isnan(NoisedBm[idx])) {
//                 cout << "WARNING: Few Rows where noise is to be added have NaN in NoisedB, they will be considered as 0" << endl;
//                 // NoisedBm[idx] = 0; // Uncomment this line if you want to set NaN values to 0
//             }
//         }

//         vector<int> indicesForRadiationOn;
//         for (size_t i = 0; i < RadiationOn.size(); ++i) {
//             if (RadiationOn[i] == 1) {
//                 indicesForRadiationOn.push_back(i);
//             }
//         }

//         for (int idx : indicesForRadiationOn) {
//             if (isnan(PW[idx]) || isnan(WaveformType[idx]) || isnan(WaveformParam[idx])) {
//                 cout << "WARNING: Few rows have PW or Waveform Param as NaN, they will be taken from LUT" << endl;
//                 if (PWLUT.empty() || WaveformtypeLUT.empty() || WaveformParamLUT.empty()) {
//                     cout << "ERROR: One of the LUTs not loaded" << endl;
//                 }
//             }
//         }
//     }
// };



// // #if 0
// // int main() {
// //     DataProcessor processor("Pulsewisecsvfile.csv", true, false, true, true, 0);
// //     processor.process();
// //     return 0;
// // }

// // #if 0
// // int main() {
// //     MotionDataSource motionData;
// //     motionData.import();

// //     PlatformAttitudeDataSource platformAttitudeData;
// //     platformAttitudeData.import();

// //     RadarLookAnglesGUI radarLookAngles;
// //     bool considerRadar = radarLookAngles.considerRadarLookAngles();

// //     if (considerRadar) {
// //         LookAngleDataSource lookAngleData;
// //         lookAngleData.import();
// //     }

// //     TargetAzimuth targetAzimuth;
// //     bool targetAzFromFile = targetAzimuth.loadFromFile();

// //     TargetCoordinates targetCoordinates;
// //     bool loadTargetCoordinatesFromGUI = targetCoordinates.loadFromGUI();

// //     if (considerRadar) {
// //         std::cout << "Radar look angles will be considered." << std::endl;
// //     } else {
// //         std::cout << "Radar look angles will not be considered." << std::endl;
// //     }

// //     if (targetAzFromFile) {
// //         std::cout << "Target azimuth will be loaded from file." << std::endl;
// //     } else {
// //         std::cout << "Target azimuth will not be loaded from file." << std::endl;
// //     }


// //     if (loadTargetCoordinatesFromGUI) {
// //         std::cout << "Target coordinates will be loaded from GUI." << std::endl;
// //         targetAzFromFile = false; // Ensure file is not considered if GUI is used
// //         } else if (targetAzFromFile) {
// //         std::cout << "Target azimuth will be loaded from file." << std::endl;
// //     } else {
// //         std::cout << "Target coordinates will not be loaded from GUI or file." << std::endl;
// //     }
// //     return 0;
// // }

// // class FileProcessingConf
// // {
// // public:
// //     FileProcessingConf();
// // };
// // #endif
// #endif // FILEPROCESSINGCONF_H
