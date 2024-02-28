FROM ubuntu:latest

RUN apt update && apt upgrade -y
RUN apt install gcc g++ valgrind neofetch gpiod libgpiod2 git wget python3-pip clang-format -y
# Set up libhal
RUN apt install software-properties-common -y
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt install -y build-essential g++-11
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main"
RUN apt install clang-tidy-16 -y
RUN python3 -m pip install "conan>=2.0.13" cmake
RUN conan remote add libhal-trunk https://libhal.jfrog.io/artifactory/api/conan/trunk-conan
RUN conan config install -sf profiles/baremetal https://github.com/libhal/conan-config.git
RUN conan profile detect --force
RUN conan config install -sf profiles/armv8/linux/ -tf profiles https://github.com/libhal/conan-config.git

# Test by building demos
RUN mkdir /libhal_testing
WORKDIR /libhal_testing
RUN git clone https://github.com/libhal/libhal-lpc40
WORKDIR /libhal_testing/libhal-lpc40/demos
RUN conan config install -sf conan/profiles/ -tf profiles https://github.com/libhal/libhal-armcortex.git
RUN conan config install -sf conan/profiles/ -tf profiles https://github.com/libhal/libhal-lpc40.git
RUN conan build . -pr lpc4078 -s build_type=MinSizeRel -b missing
RUN mkdir /code
WORKDIR /code

CMD ["/bin/bash"]
