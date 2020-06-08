add_library(CardUtils card_utils.cpp)

add_library(Cribbage cribbage_core.cpp)
target_link_libraries(Cribbage PUBLIC CardUtils)

add_library(CribbagePlayer cribbage_player.cpp)
target_link_libraries(CribbagePlayer PUBLIC CardUtils)
target_link_libraries(CribbagePlayer PUBLIC Cribbage)


