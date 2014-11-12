cmake_minimum_required(VERSION 3.1)
PROJECT(udf_example)

# Path for MySQL include directory
INCLUDE_DIRECTORIES("C:/Users/James/Source/BazaarRepos/mysql-server/mysql-5.5/include")
INCLUDE_DIRECTORIES("C:/Users/James/Source/BazaarRepos/mysql-server/mysql-5.5/sql")
INCLUDE_DIRECTORIES("C:/Users/James/Source/BazaarRepos/mysql-server/mysql-5.5/regex")
INCLUDE_DIRECTORIES("C:/Users/James/Source/BazaarRepos/mysql-server/mysql-5.5/")

ADD_DEFINITIONS("-DHAVE_DLOPEN")
ADD_LIBRARY(udf_example MODULE udf_example.c udf_example.def)
TARGET_LINK_LIBRARIES(udf_example wsock32)
