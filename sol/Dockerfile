FROM 278380418400.dkr.ecr.us-east-2.amazonaws.com/barretenberg-x86_64-linux-clang-assert

FROM ubuntu:focal as builder
RUN apt-get update && apt install -y git curl

COPY --from=0 /usr/src/barretenberg/cpp/build/bin /usr/src/barretenberg/cpp/build/bin
COPY --from=0 /usr/src/barretenberg/cpp/srs_db /usr/src/barretenberg/cpp/srs_db
WORKDIR /usr/src/barretenberg/sol
RUN git init
COPY . .

RUN ls /usr/src/barretenberg/cpp/build/bin

RUN ./scripts/install_foundry.sh
ENV PATH="/usr/src/barretenberg/sol/.foundry/bin:${PATH}"

RUN forge install --no-commit \
  https://github.com/foundry-rs/forge-std \
  https://github.com/openzeppelin/openzeppelin-contracts

RUN cd ../cpp/srs_db && ./download_ignition.sh 3 && cd ../../sol

RUN ./scripts/init.sh

RUN forge test --no-match-contract TestBase