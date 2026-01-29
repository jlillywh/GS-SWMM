#ifndef SWMM_TEST_MODELS_H
#define SWMM_TEST_MODELS_H

#include <string>
#include <fstream>
#include <cstdio>

/**
 * SWMM Test Model Generator
 * 
 * This header provides utilities to generate valid SWMM model files
 * for testing purposes. It eliminates the need to manually create
 * and maintain multiple .inp files.
 */

namespace SwmmTestModels {

/**
 * Generate a complete treatment train model with all required elements
 * This is the standard model for testing the GoldSim-SWMM bridge
 * 
 * Elements created:
 * - Rain gage: RG1
 * - Subcatchment: S1
 * - Storage nodes: ST1 (bioswale), ST2 (detention), ST3 (retention)
 * - Junction: J1
 * - Outfall: J2
 * - Conduits: C1, C2, C3
 */
inline bool CreateTreatmentTrainModel(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "[TITLE]\n";
    file << "Treatment Train Test Model\n\n";

    file << "[OPTIONS]\n";
    file << "FLOW_UNITS           CFS\n";
    file << "INFILTRATION         HORTON\n";
    file << "FLOW_ROUTING         DYNWAVE\n";
    file << "START_DATE           01/01/2024\n";
    file << "START_TIME           00:00:00\n";
    file << "REPORT_START_DATE    01/01/2024\n";
    file << "REPORT_START_TIME    00:00:00\n";
    file << "END_DATE             01/01/2024\n";
    file << "END_TIME             06:00:00\n";
    file << "SWEEP_START          01/01\n";
    file << "SWEEP_END            12/31\n";
    file << "DRY_DAYS             0\n";
    file << "REPORT_STEP          00:01:00\n";
    file << "WET_STEP             00:01:00\n";
    file << "DRY_STEP             01:00:00\n";
    file << "ROUTING_STEP         0:00:30\n";
    file << "ALLOW_PONDING        NO\n";
    file << "INERTIAL_DAMPING     PARTIAL\n";
    file << "VARIABLE_STEP        0.75\n";
    file << "LENGTHENING_STEP     0\n";
    file << "MIN_SURFAREA         0\n";
    file << "NORMAL_FLOW_LIMITED  BOTH\n";
    file << "SKIP_STEADY_STATE    NO\n";
    file << "FORCE_MAIN_EQUATION  H-W\n";
    file << "LINK_OFFSETS         DEPTH\n";
    file << "MIN_SLOPE            0\n\n";

    file << "[RAINGAGES]\n";
    file << ";;               Rain      Time   Snow   Data\n";
    file << ";;Name           Type      Intrvl Catch  Source\n";
    file << "RG1              INTENSITY 1:00   1.0    TIMESERIES TS1\n\n";

    file << "[SUBCATCHMENTS]\n";
    file << ";;                                                 Total    Pcnt.             Pcnt.    Curb     Snow\n";
    file << ";;Name           Raingage         Outlet           Area     Imperv   Width   Slope    Length   Pack\n";
    file << "S1               RG1              ST1              5.0      50       500     0.5      0        \n\n";

    file << "[SUBAREAS]\n";
    file << ";;Subcatchment   N-Imperv   N-Perv     S-Imperv   S-Perv     PctZero    RouteTo    PctRouted\n";
    file << "S1               0.01       0.1        0.05       0.05       25         OUTLET\n\n";

    file << "[INFILTRATION]\n";
    file << ";;Subcatchment   MaxRate    MinRate    Decay      DryTime    MaxInfil\n";
    file << "S1               3.0        0.5        4          7          0\n\n";

    file << "[JUNCTIONS]\n";
    file << ";;               Invert     Max.       Init.      Surcharge  Ponded\n";
    file << ";;Name           Elev.      Depth      Depth      Depth      Area\n";
    file << "J1               95         5          0          0          0\n\n";

    file << "[OUTFALLS]\n";
    file << ";;               Invert     Outfall      Stage/Table      Tide\n";
    file << ";;Name           Elev.      Type         Time Series      Gate\n";
    file << "J2               90         FREE                          NO\n\n";

    file << "[STORAGE]\n";
    file << ";;               Invert   Max.     Init.    Storage    Curve                      Ponded   Evap\n";
    file << ";;Name           Elev.    Depth    Depth    Curve      Params                     Area     Frac\n";
    file << "ST1              100      8        0        FUNCTIONAL 1000    0        0        0        0\n";
    file << "ST2              98       10       0        FUNCTIONAL 2000    0        0        0        0\n";
    file << "ST3              96       12       0        FUNCTIONAL 3000    0        0        0        0\n\n";

    file << "[CONDUITS]\n";
    file << ";;               Inlet            Outlet                      Manning    Inlet      Outlet     Init.      Max.\n";
    file << ";;Name           Node             Node             Length     N          Offset     Offset     Flow       Flow\n";
    file << "C1               ST1              ST2              100        0.01       0          0          0          0\n";
    file << "C2               ST2              ST3              100        0.01       0          0          0          0\n";
    file << "C3               ST3              J2               100        0.01       0          0          0          0\n\n";

    file << "[XSECTIONS]\n";
    file << ";;Link           Shape        Geom1            Geom2      Geom3      Geom4      Barrels\n";
    file << "C1               CIRCULAR     2                0          0          0          1\n";
    file << "C2               CIRCULAR     2                0          0          0          1\n";
    file << "C3               CIRCULAR     2                0          0          0          1\n\n";

    file << "[TIMESERIES]\n";
    file << ";;Name           Date       Time       Value\n";
    file << "TS1                         0:00       0.0\n";
    file << "TS1                         1:00       0.5\n";
    file << "TS1                         2:00       1.0\n";
    file << "TS1                         3:00       0.5\n";
    file << "TS1                         4:00       0.0\n\n";

    file << "[REPORT]\n";
    file << "INPUT      NO\n";
    file << "CONTROLS   NO\n";
    file << "SUBCATCHMENTS ALL\n";
    file << "NODES ALL\n";
    file << "LINKS ALL\n\n";

    file << "[MAP]\n";
    file << "DIMENSIONS 0.000 0.000 10000.000 10000.000\n";
    file << "Units      None\n\n";

    file.close();
    return true;
}

/**
 * Generate a minimal valid SWMM model with specified number of subcatchments
 * Useful for testing subcatchment index validation
 */
inline bool CreateModelWithSubcatchments(const std::string& filename, int num_subcatchments) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "[TITLE]\n";
    file << "Test Model with " << num_subcatchments << " Subcatchments\n\n";

    file << "[OPTIONS]\n";
    file << "FLOW_UNITS           CFS\n";
    file << "INFILTRATION         HORTON\n";
    file << "FLOW_ROUTING         DYNWAVE\n";
    file << "START_DATE           01/01/2024\n";
    file << "START_TIME           00:00:00\n";
    file << "END_DATE             01/01/2024\n";
    file << "END_TIME             01:00:00\n";
    file << "REPORT_STEP          00:01:00\n";
    file << "WET_STEP             00:01:00\n";
    file << "DRY_STEP             01:00:00\n";
    file << "ROUTING_STEP         0:00:30\n\n";

    file << "[RAINGAGES]\n";
    file << "RG1              INTENSITY 1:00   1.0    TIMESERIES TS1\n\n";

    file << "[SUBCATCHMENTS]\n";
    for (int i = 0; i < num_subcatchments; i++) {
        file << "SUB" << i << "             RG1              J2               1.0      50       100     0.5      0\n";
    }
    file << "\n";

    file << "[SUBAREAS]\n";
    for (int i = 0; i < num_subcatchments; i++) {
        file << "SUB" << i << "             0.01       0.1        0.05       0.05       25         OUTLET\n";
    }
    file << "\n";

    file << "[INFILTRATION]\n";
    for (int i = 0; i < num_subcatchments; i++) {
        file << "SUB" << i << "             3.0        0.5        4          7          0\n";
    }
    file << "\n";

    file << "[OUTFALLS]\n";
    file << "J2               90         FREE                          NO\n\n";

    file << "[TIMESERIES]\n";
    file << "TS1                         0:00       0.0\n";
    file << "TS1                         1:00       0.5\n\n";

    file.close();
    return true;
}

/**
 * Generate a model missing specific treatment train elements
 * Useful for testing element validation
 */
inline bool CreateModelMissingElement(const std::string& filename, const std::string& missing_element) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "[TITLE]\n";
    file << "Test Model Missing " << missing_element << "\n\n";

    file << "[OPTIONS]\n";
    file << "FLOW_UNITS           CFS\n";
    file << "INFILTRATION         HORTON\n";
    file << "FLOW_ROUTING         DYNWAVE\n";
    file << "START_DATE           01/01/2024\n";
    file << "START_TIME           00:00:00\n";
    file << "END_DATE             01/01/2024\n";
    file << "END_TIME             01:00:00\n";
    file << "REPORT_STEP          00:01:00\n";
    file << "WET_STEP             00:01:00\n";
    file << "DRY_STEP             01:00:00\n";
    file << "ROUTING_STEP         0:00:30\n\n";

    file << "[RAINGAGES]\n";
    file << "RG1              INTENSITY 1:00   1.0    TIMESERIES TS1\n\n";

    if (missing_element != "S1") {
        file << "[SUBCATCHMENTS]\n";
        file << "S1               RG1              ST1              5.0      50       500     0.5      0\n\n";
        file << "[SUBAREAS]\n";
        file << "S1               0.01       0.1        0.05       0.05       25         OUTLET\n\n";
        file << "[INFILTRATION]\n";
        file << "S1               3.0        0.5        4          7          0\n\n";
    }

    file << "[JUNCTIONS]\n";
    file << "J1               95         5          0          0          0\n\n";

    file << "[OUTFALLS]\n";
    if (missing_element != "J2") {
        file << "J2               90         FREE                          NO\n";
    }
    file << "\n";

    file << "[STORAGE]\n";
    if (missing_element != "ST1") {
        file << "ST1              100      8        0        FUNCTIONAL 1000    0        0        0        0\n";
    }
    if (missing_element != "ST2") {
        file << "ST2              98       10       0        FUNCTIONAL 2000    0        0        0        0\n";
    }
    if (missing_element != "ST3") {
        file << "ST3              96       12       0        FUNCTIONAL 3000    0        0        0        0\n";
    }
    file << "\n";

    file << "[CONDUITS]\n";
    if (missing_element != "C1") {
        file << "C1               ST1              ST2              100        0.01       0          0          0          0\n";
    }
    if (missing_element != "C2") {
        file << "C2               ST2              ST3              100        0.01       0          0          0          0\n";
    }
    if (missing_element != "C3") {
        file << "C3               ST3              J2               100        0.01       0          0          0          0\n";
    }
    file << "\n";

    file << "[XSECTIONS]\n";
    if (missing_element != "C1") {
        file << "C1               CIRCULAR     2                0          0          0          1\n";
    }
    if (missing_element != "C2") {
        file << "C2               CIRCULAR     2                0          0          0          1\n";
    }
    if (missing_element != "C3") {
        file << "C3               CIRCULAR     2                0          0          0          1\n";
    }
    file << "\n";

    file << "[TIMESERIES]\n";
    file << "TS1                         0:00       0.0\n";
    file << "TS1                         1:00       0.5\n\n";

    file.close();
    return true;
}

/**
 * Test fixture class for managing SWMM test files
 * Automatically creates and cleans up test files
 */
class TestFixture {
private:
    std::string model_file;
    std::string report_file;
    std::string output_file;
    bool cleanup_on_destroy;

public:
    TestFixture(const std::string& base_name = "test_model", bool auto_cleanup = true)
        : model_file(base_name + ".inp"),
          report_file(base_name + ".rpt"),
          output_file(base_name + ".out"),
          cleanup_on_destroy(auto_cleanup) {
    }

    ~TestFixture() {
        if (cleanup_on_destroy) {
            Cleanup();
        }
    }

    const char* GetModelPath() const { return model_file.c_str(); }
    const char* GetReportPath() const { return report_file.c_str(); }
    const char* GetOutputPath() const { return output_file.c_str(); }

    bool CreateTreatmentTrain() {
        return CreateTreatmentTrainModel(model_file);
    }

    bool CreateWithSubcatchments(int count) {
        return CreateModelWithSubcatchments(model_file, count);
    }

    bool CreateMissingElement(const std::string& element) {
        return CreateModelMissingElement(model_file, element);
    }

    void Cleanup() {
        std::remove(model_file.c_str());
        std::remove(report_file.c_str());
        std::remove(output_file.c_str());
    }

    void DisableAutoCleanup() {
        cleanup_on_destroy = false;
    }
};

} // namespace SwmmTestModels

#endif // SWMM_TEST_MODELS_H
