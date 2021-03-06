list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )
##
# check for Scotch, use if there
##
find_package( Scotch )
##
# c/c++ std
##
include( CheckSTD )
##
# for demangle, perhaps others
##
include( CheckBoostDep )
find_package( LIBRT )

set( GITDEP "${CMAKE_SOURCE_DIR}/git-dep" )
##
# grab include directories for git-dep
##
if( EXISTS ${GITDEP} )
##
# get the dirs that are in the git-dep folder
##
file( GLOB DEPFOLDERLIST ${GITDEP}/* )
foreach( DEPFOLDER ${DEPFOLDERLIST} )
    message( STATUS "Checking: ${DEPFOLDER}" )
    if( IS_DIRECTORY ${DEPFOLDER} )
        message( STATUS "Found: ${DEPFOLDER}" )
        include_directories( ${DEPFOLDER}/include )
        link_directories( ${DEPFOLDER}/lib )
    endif( IS_DIRECTORY ${DEPFOLDER} )
endforeach( DEPFOLDER ${DEPFOLDERLIST} )

endif( EXISTS ${GITDEP} )

include_directories( ${CMAKE_SCOTCH_INCS} )

set( APP pisim )

add_executable( ${APP} "${APP}.cpp" )

target_link_libraries( ${APP} 
                       raft
                       cmdargs
                       ${CMAKE_SCOTCH_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_RT_LIBS} )

