FROM 278380418400.dkr.ecr.eu-west-2.amazonaws.com/barretenberg-x86_64-linux-clang-assert as builder1
RUN test -d /usr/src/barretenberg/cpp/build || (echo "/usr/src/barretenberg/cpp/build directory not found" && exit 1)

FROM 278380418400.dkr.ecr.eu-west-2.amazonaws.com/bb.js as builder2
RUN test -d /usr/src/barretenberg/ts || (echo "/usr/src/barretenberg/ts directory not found" && exit 1)

FROM node:18-alpine

RUN apk update && apk add cmake git bash curl
COPY --from=builder1 /usr/src/barretenberg/cpp/build /usr/src/barretenberg/cpp/build
COPY --from=builder2 /usr/src/barretenberg/ts /usr/src/barretenberg/ts
WORKDIR /usr/src/barretenberg/acir_tests
COPY . .
# Run all native tests.
RUN ./run_acir_tests.sh
# Just run double_verify_proof as a sanity check as bb.js is quite slow.
RUN BB=../ts/dest/main.js ./run_acir_tests.sh double_verify_proof
