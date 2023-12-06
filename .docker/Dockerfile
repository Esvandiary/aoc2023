FROM debian:latest

RUN apt update
RUN apt install -y cmake build-essential musl-dev musl musl-tools

ADD entrypoint.sh /entrypoint.sh
RUN chmod +x entrypoint.sh

VOLUME /code
WORKDIR /code
ENTRYPOINT ["/entrypoint.sh"]
