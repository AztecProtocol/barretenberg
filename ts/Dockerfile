#FROM aztecprotocol/barretenberg-wasm-linux-clang:latest
FROM 278380418400.dkr.ecr.eu-west-2.amazonaws.com/barretenberg-wasm-linux-clang AS builder

FROM node:18-alpine
COPY --from=0 /usr/src/barretenberg /usr/src/barretenberg
WORKDIR /usr/src/barretenberg/ts
COPY .yarn .yarn
COPY package.json package.json
COPY yarn.lock yarn.lock
COPY .yarnrc.yml .yarnrc.yml
RUN yarn --immutable
COPY . .
RUN yarn formatting && yarn build:ts
CMD ["yarn", "test"]