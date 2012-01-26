SET(oregonframework_STAT_SRCS
   Policies/ObjectLifeTime.cpp
#   Utilities/EventProcessor.cpp
)

include_directories(
  ${ACE_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src/server/shared  
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids
  ${CMAKE_SOURCE_DIR}/src/server/framework
)

add_library(oregonframework STATIC ${oregonframework_STAT_SRCS})
