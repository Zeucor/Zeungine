#!/bin/bash

cd cmake/Dependencies


echo " -- Starting zegndeps Debug Configure"
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON

echo " -- Starting zegndeps Debug Build"
cmake --build build

echo " -- Starting zegndeps Debug Package"
cpack --config build/CPackConfig.cmake -C Debug


echo " -- Starting zegndeps Release Configure"
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON

echo " -- Starting zegndeps Release Build"
cmake --build build

echo " -- Starting zegndeps Release Package"
cpack --config build/CPackConfig.cmake -C Release



cd ../Headers


echo " -- Starting zeungine Headers Configure"
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON

echo " -- Starting zeungine Headers Package"
cpack --config build/CPackConfig.cmake -C Release


cd ../..


echo " -- Starting zeungine Debug Configure"
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON

echo " -- Starting zeungine Debug Build"
cmake --build build

echo " -- Starting zeungine Debug Package"
cpack --config build/CPackConfig.cmake -C Debug


echo " -- Starting zeungine Release Configure"
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON

echo " -- Starting zeungine Release Build"
cmake --build build

echo " -- Starting zeungine Release Package"
cpack --config build/CPackConfig.cmake -C Release


ls -lah releases