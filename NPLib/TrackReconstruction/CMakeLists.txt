add_custom_command(OUTPUT NPRansacDict.cxx COMMAND ../scripts/build_dict.sh NPRansac.h NPRansacDict.cxx NPRansac.rootmap libNPTrackReconstruction.so NPTrackReconstructionLinkDef.h DEPENDS NPRansac.h)

add_custom_command(OUTPUT NPClusterDict.cxx COMMAND ../scripts/build_dict.sh NPCluster.h NPClusterDict.cxx NPCluster.rootmap libNPTrackReconstruction.so NPTrackReconstructionLinkDef.h DEPENDS NPCluster.h)

add_custom_command(OUTPUT TrackingDict.cxx COMMAND ../scripts/build_dict.sh Tracking.h TrackingDict.cxx Tracking.rootmap libNPTrackReconstruction.so NPTrackReconstructionLinkDef.h DEPENDS Tracking.h)

add_library(NPTrackReconstruction SHARED NPRansac.cxx NPCluster.cxx NPTrack.cxx Tracking.cxx NPRansacDict.cxx NPClusterDict.cxx TrackingDict.cxx)
target_link_libraries(NPTrackReconstruction ${ROOT_LIBRARIES} NPCore)

install(FILES NPRansac.h NPCluster.h NPTrack.h Tracking.h DESTINATION ${CMAKE_INCLUDE_OUTPUT_DIRECTORY})
