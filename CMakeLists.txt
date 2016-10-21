cmake_minimum_required(VERSION 2.8.9)

project(AnomalousFiltersExtension)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/AnomalousFilters")
set(EXTENSION_CATEGORY "Filtering.Anomalous Filters")
set(EXTENSION_CONTRIBUTORS "Antonio Carlos Senra Filho (University of Sao Paulo), Luiz Otavio Murta Junior (University of Sao Paulo)")
set(EXTENSION_DESCRIPTION "This extension aims to provide several approaches in order to apply the anomalous spatial filters on medical images.")
set(EXTENSION_ICONURL "https://www.slicer.org/slicerWiki/images/8/89/AnomalousDiffusionExtension-logo.png")
set(EXTENSION_SCREENSHOTURLS "https://www.slicer.org/slicerWiki/images/6/64/MRI_raw.png https://www.slicer.org/slicerWiki/images/e/e8/MRI_AAD.png https://www.slicer.org/slicerWiki/images/0/0d/MRI_IAD.png https://www.slicer.org/slicerWiki/images/7/7b/Aad_scalar_gui.png https://www.slicer.org/slicerWiki/images/3/39/Iad_scalar_gui.png")

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
