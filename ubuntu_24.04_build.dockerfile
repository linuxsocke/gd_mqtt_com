FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    automake autopoint wget libtool libblkid-dev \

# Change shell to bash so the source command works
SHELL ["/bin/bash", "-c"]

RUN python3 -m venv /mesonvenv && \
    source /mesonvenv/bin/activate && pip install meson ninja

ARG PACKAGE_CONTACT
ARG OUTPUT_DIR
RUN mkdir -p $OUTPUT_DIR

RUN git clone --branch v1.5.3 https://github.com/Haivision/srt.git /workspace/thirdparty/srt

###########################
## Build srt ------- 
WORKDIR /workspace/thirdparty/srt

RUN cmake . -B ./build -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install
RUN cmake --build build --target install
RUN cp -r ./install/* /usr/local/ && ldconfig

RUN LATEST_TAG=$(git describe --tags --exact-match) && \
    REPOSITORY_NAME=$(basename $(git rev-parse --show-toplevel)) && \
    cpack -G TGZ -B ${OUTPUT_DIR} \
    -D CPACK_INSTALLED_DIRECTORIES="$(pwd)/install;." \
    -D CPACK_PACKAGE_VERSION="${LATEST_TAG}" \
    -D CPACK_PACKAGE_DESCRIPTION="${REPOSITORY_NAME}" \
    -D CPACK_PACKAGE_FILE_NAME="${REPOSITORY_NAME}-${LATEST_TAG}" \
    -D CPACK_PACKAGE_NAME="${REPOSITORY_NAME}" \
    -D CPACK_PACKAGE_CONTACT="${PACKAGE_CONTACT}"

#WORKDIR /workspace
#
#RUN git clone https://github.com/godotengine/godot-cpp.git /workspace/godot-cpp --branch godot-4.3-stable
#RUN cmake -DCMAKE_BUILD_TYPE=Release ./godot-cpp -B ./godot-cpp/build
#RUN cmake --build ./godot-cpp/build
#
#COPY ./include              /workspace/include
#COPY ./src                  /workspace/src
#COPY ./tests                /workspace/tests
#COPY ./CMakeLists.txt       /workspace/CMakeLists.txt
#
#RUN cmake ./ -B ./build \
#    -DCPP_BINDINGS_PATH=./godot-cpp \
#    -DGODOT_GDEXTENSION_DIR=./godot-cpp/gdextension/ \
#    -DGLIB_PATH=$(pwd)/thirdparty/glib/install \
#    -DSRT_PATH=$(pwd)/thirdparty/srt/install \
#    -DGSTREAMER_PATH=$(pwd)/thirdparty/gstreamer/install \
#    -DCMAKE_BUILD_TYPE=Release
#RUN cmake --build ./build
#
#RUN mv ./bin/x11 $OUTPUT_DIR

CMD ["tail", "-f", "/dev/null"]