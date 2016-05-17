#!/bin/bash
gcc -m64 -static-libgcc -static-libstdc++ -std=c++0x -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -pthread -O3 convert/*.cpp sync/sync_util.cpp sync/sync_event.cpp environment/*.cpp utf8/*.cpp servicemanager.cpp -o servicemanager_nix_64 -lstdc++ -lrt
gcc -m32 -static-libgcc -static-libstdc++ -std=c++0x -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -pthread -O3 convert/*.cpp sync/sync_util.cpp sync/sync_event.cpp environment/*.cpp utf8/*.cpp servicemanager.cpp -o servicemanager_nix_32 -lstdc++ -lrt
