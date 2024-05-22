cmake_minimum_required(VERSION 1.0)

idf_component_register(
                       SRCS "miniProject.cpp"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES arduino
)

project(miniProject)
