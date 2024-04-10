rm CMakeCache.txt
cmake . \
 -DOPTION_RECALBOX_PRODUCTION_BUILD=true \
 -DCMAKE_C_FLAGS="-mabi=lp64 -mcpu=cortex-a55" \
 -DCMAKE_CXX_FLAGS="-mabi=lp64 -mcpu=cortex-a55" \
 -DBUILD_SHARED_LIBS=ON
 make -j8

# Build with Debug
#  cmake . \
#  -DOPTION_RECALBOX_PRODUCTION_BUILD=true \
#  -DCMAKE_BUILD_TYPE=Debug \
#  -DCMAKE_C_FLAGS="-mabi=lp64 -mcpu=cortex-a55" \
#  -DCMAKE_CXX_FLAGS="-mabi=lp64 -mcpu=cortex-a55" \
#  -DBUILD_SHARED_LIBS=ON
#  make -j8