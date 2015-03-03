cmake_minimum_required(VERSION 2.8.9)

project(AnomalousFiltersExtension)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Modules/AnomalousFilters")
set(EXTENSION_CATEGORY "Filtering.Anomalous Filters")
set(EXTENSION_CONTRIBUTORS "Antonio Carlos Senra Filho (University of Sao Paulo), Luiz Otavio Murta Junior (University of Sao Paulo)")
set(EXTENSION_DESCRIPTION "This is an example of a simple extension")
set(EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Modules/AnomalousFiltersExtension.png")
set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Modules/Screenshots/1.png")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(AADImageFilter)
add_subdirectory(IADImageFilter)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
