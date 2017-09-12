
if(UNIX)
  
  find_package(Boost ${SAPPHIRE_BOOST_VER} COMPONENTS system)
  if(Boost_FOUND)
    set(BOOST_LIBRARY_DIR ${Boost_LIBRARY_DIR})
  else()
    if (EXISTS /opt/build_libs/${SAPPHIRE_BOOST_FOLDER_NAME})
      set(Boost_INCLUDE_DIR /opt/build_libs/${SAPPHIRE_BOOST_FOLDER_NAME})
      set(BOOST_LIBRARYDIR /opt/build_libs/${SAPPHIRE_BOOST_FOLDER_NAME}/stage/lib)
    else()
      message(FATAL_ERROR "Unable to find boost ${SAPPHIRE_BOOST_VER} package!")
    endif()
  endif()
else()

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../external/${SAPPHIRE_BOOST_FOLDER_NAME})
    message(STATUS "Using boost in /libraries/external")
	  set(Boost_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external/${SAPPHIRE_BOOST_FOLDER_NAME})
    set(BOOST_LIBRARYDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external/${SAPPHIRE_BOOST_FOLDER_NAME}/lib32-msvc-14.0)
  else()
      find_package(Boost ${SAPPHIRE_BOOST_VER} COMPONENTS system)
      if(Boost_FOUND)
        set(BOOST_LIBRARY_DIR ${Boost_LIBRARY_DIR})
      elseif ((EXISTS $ENV{BOOST_ROOT_DIR}) AND (EXISTS $ENV{BOOST_LIB_DIR}))
        set(Boost_INCLUDE_DIR $ENV{BOOST_ROOT_DIR})
	      set(BOOST_LIBRARYDIR $ENV{BOOST_LIB_DIR})
      else()
        message(FATAL_ERROR "SapphireError: Unable to find boost ${SAPPHIRE_BOOST_VER} package and environment variables BOOST_ROOT_DIR and BOOST_LIB_DIR not set!")
      endif()
  endif()
endif()

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost ${SAPPHIRE_BOOST_VER} COMPONENTS log log_setup thread date_time filesystem system)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${BOOST_LIBRARYDIR})

