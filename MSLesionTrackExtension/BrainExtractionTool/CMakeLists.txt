
#-----------------------------------------------------------------------------
set(MODULE_NAME BrainExtractionTool)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

# we don't want to copy if we're building in the source dir
if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)

    # list of files for which we add a copy rule
    set(INPUTS Data/Input/ref_vols/atlas.nii.gz Data/Input/ref_vols/atlas_mask.nii.gz Data/Input/ref_vols/atlas_mask_dilated.nii.gz Data/Input/ref_vols/atlas_mask_eroded.nii.gz Data/Input/ref_vols/Mini.nii.gz)

    foreach(item IN LISTS INPUTS)
        message(STATUS ${item})
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${item}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/${item}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${item}"
        )
    endforeach()
endif()

# files are only copied if a target depends on them
add_custom_target(data-target ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Data/Input/ref_vols/atlas.nii.gz" "${CMAKE_CURRENT_BINARY_DIR}/Data/Input/ref_vols/atlas_mask.nii.gz" "${CMAKE_CURRENT_BINARY_DIR}/Data/Input/ref_vols/atlas_mask_dilated.nii.gz" "${CMAKE_CURRENT_BINARY_DIR}/Data/Input/ref_vols/atlas_mask_eroded.nii.gz" "${CMAKE_CURRENT_BINARY_DIR}/Data/Input/ref_vols/Mini.nii.gz")
#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKIOImageBase
  ITKSmoothing
  ITKRegistrationCommon
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(MODULE_INCLUDE_DIRECTORIES
ref_vols
dat
  )

set(MODULE_SRCS
  )

set(MODULE_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  ADDITIONAL_SRCS ${MODULE_SRCS}
RobustStrip.cxx aux_methods.cxx   loadModel.cpp graph.cpp maxflow.cpp maxflow_main.cpp
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
