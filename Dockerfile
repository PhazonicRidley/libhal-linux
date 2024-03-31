FROM --platform=arm64 ubuntu:22.04

RUN apt update && apt upgrade -y
RUN apt install gcc g++ valgrind neofetch gpiod libgpiod2 git wget python3-pip clang-format software-properties-common -y

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt install -y build-essential g++-12
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-17 main"
RUN apt-get install clang-tidy-17 -y
RUN python3 -m pip install "conan>=2.1.0" cmake
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
