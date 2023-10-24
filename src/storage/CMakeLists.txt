set(CMAKE_CXX_STANDARD 17)  # 或更高版本

set(SOURCES 
        disk_manager.cpp 
        buffer_pool_manager.cpp 
        ../replacer/replacer.h 
        ../replacer/lru_replacer.cpp 
)
add_library(storage STATIC ${SOURCES})
