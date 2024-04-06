FROM --platform=arm64 ubuntu:22.04

RUN apt update && apt upgrade -y
RUN apt install gcc g++ valgrind neofetch git wget python3-pip clang-format software-properties-common locales pkg-config automake autoconf autoconf-archive libtool m4 -y

RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 17
RUN apt install libc++-17-dev libc++abi-17-dev -y

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt install -y build-essential g++-12
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-17 main"
RUN apt-get install clang-tidy-17 -y
RUN python3 -m pip install "conan>=2.2.2" cmake

# Compile gpiod
WORKDIR /opt
RUN git clone https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
RUN locale-gen "en_US.UTF-8"
WORKDIR /opt/libgpiod
RUN ./autogen.sh --enable-bindings-cxx
RUN make -j
RUN make install

# Configure conan for libhal
RUN conan remote add libhal-trunk https://libhal.jfrog.io/artifactory/api/conan/trunk-conan
RUN conan config install -sf profiles/baremetal/v2 https://github.com/libhal/conan-config.git
RUN conan profile detect --force
# Set profile based on arch x86 or arm64
# RUN if [[ -z "$arg" ]] ; then echo Argument not provided ; else echo Argument is $arg ; fi
RUN conan config install -sf profiles/armv8/linux/ -tf profiles https://github.com/libhal/conan-config.git

# Test by building demos
RUN mkdir /test_libhal
WORKDIR /test_libhal
RUN git clone https://github.com/libhal/libhal-lpc40
WORKDIR /test_libhal/libhal-lpc40
RUN conan config install -sf conan/profiles/v2 -tf profiles https://github.com/libhal/libhal-lpc40.git
RUN conan config install -tf profiles -sf conan/profiles/v1 https://github.com/libhal/arm-gnu-toolchain.git
RUN conan build demos -pr lpc4078 -pr arm-gcc-12.3 -s build_type=MinSizeRel -b missing

RUN mkdir /code
WORKDIR /code

CMD ["/bin/bash"]
