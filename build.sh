cmake . \
 -DOPTION_RECALBOX_PRODUCTION_BUILD=true \
 -DCMAKE_C_FLAGS="-mabi=lp64 -mcpu=cortex-a55" \
 -DCMAKE_CXX_FLAGS="-mabi=lp64 -mcpu=cortex-a55" \
 -DBUILD_SHARED_LIBS=ON