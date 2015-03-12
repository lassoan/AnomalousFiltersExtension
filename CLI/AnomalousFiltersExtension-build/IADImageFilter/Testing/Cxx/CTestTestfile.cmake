# CMake generated Testfile for 
# Source directory: /home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension/IADImageFilter/Testing/Cxx
# Build directory: /home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/IADImageFilter/Testing/Cxx
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(IADImageFilterTest "/home/antonio/Slicer-Git/Slicer-SuperBuild-Debug/Slicer-build/Slicer" "--launcher-no-splash" "--launcher-additional-settings" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/AdditionalLauncherSettings.ini" "--launch" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/IADImageFilter/Testing/Cxx/IADImageFilterTest" "--compare" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/IADImageFilter/Data/Baseline/IADImageFilterTest.nhdr" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/Testing/Temporary/IADImageFilterTest.nhdr" "ModuleEntryPoint" "--sigma" "2.5" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/IADImageFilter/Data/Input/CTHeadAxial.nhdr" "/home/antonio/Documents/CSIM-Toolkits/Slicer/CLI/AnomalousFiltersExtension-build/Testing/Temporary/IADImageFilterTest.nhdr")
set_tests_properties(IADImageFilterTest PROPERTIES  LABELS "IADImageFilter")
