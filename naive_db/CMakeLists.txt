#CmakeList.txt for command_processor library in OTUS Homework #10

find_package(Threads)

# set library name and content
add_library(naive_db
  naive_db.h naive_db.cpp
  db_operations.h db_operations.cpp
)

target_link_libraries(naive_db pthread)

