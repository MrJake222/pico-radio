# use this to tar sources

date=$(date --iso-8601=seconds)
tar --exclude=cmake-build-debug --exclude=cmake-build-release --exclude=.git -czpvf ../pico-radio-cpp-$date.tgz .
