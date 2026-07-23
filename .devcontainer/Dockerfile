FROM ghcr.io/userver-framework/ubuntu-24.04-userver-base:latest

RUN apt install -y pycodestyle sudo

RUN git clone https://github.com/userver-framework/userver \
  && cd userver \
  && ./scripts/docker/setup-dev.sh \
  && BUILD_OPTIONS="-DUSERVER_FEATURE_GRPC_CHANNELZ=1" ./scripts/build_and_install_all.sh \
  && cd .. \
  && rm -rf userver/

EXPOSE 8080-8100
