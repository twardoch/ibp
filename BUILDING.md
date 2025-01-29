LDFLAGS="-L/usr/local/opt/qt@5/lib" CPPFLAGS="-I/usr/local/opt/qt@5/include" PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig" cmake . \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -DCMAKE_CXX_FLAGS="-std=c++17 -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x060000 -DQ_COMPILER_STDCXX=201703L" \
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/local/opt/lcms2/lib -L/usr/local/Cellar/freeimage/3.18.0/lib" \
    -DCMAKE_INCLUDE_PATH="/usr/local/include;/usr/local/Cellar/freeimage/3.18.0/include" \
    -DCMAKE_LIBRARY_PATH="/usr/local/lib;/usr/local/Cellar/freeimage/3.18.0/lib" \
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@5;/usr/local/opt/lcms2;/usr/local/Cellar/freeimage/3.18.0" \
    -DCMAKE_SHARED_LINKER_FLAGS="-L/usr/local/Cellar/freeimage/3.18.0/lib -L/usr/local/opt/lcms2/lib -llcms2" && make -j$(sysctl -n hw.ncpu)