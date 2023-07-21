#FROM aztecprotocol/barretenberg-wasm-linux-clang:latest
FROM 278380418400.dkr.ecr.eu-west-2.amazonaws.com/barretenberg-wasm-linux-clang

FROM node:18-alpine

RUN apk add --no-cache make g++ python3 && apk add cmake --update-cache --repository http://dl-cdn.alpinelinux.org/alpine/edge/main

COPY --from=0 /usr/src/barretenberg /usr/src/barretenberg
WORKDIR /usr/src/barretenberg/ts
COPY .yarn .yarn
COPY package.json package.json
COPY yarn.lock yarn.lock
COPY .yarnrc.yml .yarnrc.yml
RUN yarn --immutable
COPY . .
ENV DOCKER_ENV=true
RUN yarn formatting && yarn build
CMD ["yarn", "test"]
