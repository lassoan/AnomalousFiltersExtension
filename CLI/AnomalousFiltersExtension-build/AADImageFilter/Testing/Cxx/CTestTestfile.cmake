# CMake generated Testfile for 
# Source directory: /home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension/AADImageFilter/Testing/Cxx
# Build directory: /home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/AADImageFilter/Testing/Cxx
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(AADImageFilterTest "/home/antonio/Slicer-Git/Slicer-SuperBuild-Debug/Slicer-build/Slicer" "--launcher-no-splash" "--launcher-additional-settings" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/AdditionalLauncherSettings.ini" "--launch" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/AADImageFilter/Testing/Cxx/AADImageFilterTest" "--compare" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/AADImageFilter/Data/Baseline/AADImageFilterTest.nhdr" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/Testing/Temporary/AADImageFilterTest.nhdr" "ModuleEntryPoint" "--sigma" "2.5" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/AADImageFilter/Data/Input/CTHeadAxial.nhdr" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/Testing/Temporary/AADImageFilterTest.nhdr")
set_tests_properties(AADImageFilterTest PROPERTIES  LABELS "AADImageFilter")
