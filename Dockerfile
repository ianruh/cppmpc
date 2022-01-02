FROM ubuntu

RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends tzdata
# SymEngine Dependencies
RUN apt install -y cmake libgmp-dev
# cppmpc dependencies
RUN apt install -y libeigen3-dev  swig
# Dev Dependencies
RUN apt install -y python3 python3-pip python3-venv git

COPY . /cppmpc/
WORKDIR /cppmpc

RUN ./utils clean-all
RUN ./utils bootstrap
