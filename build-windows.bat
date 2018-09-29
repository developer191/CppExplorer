@echo off

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-IC:/Unix/include -IC:/Unix/include/cpplib -LC:/Unix/lib" ..
mingw32-make