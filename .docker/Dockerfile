FROM mcr.microsoft.com/dotnet/sdk:8.0

RUN apt update
RUN apt install -y clang zlib1g-dev

ADD entrypoint.sh /entrypoint.sh
RUN chmod +x entrypoint.sh

VOLUME /code
WORKDIR /code
ENTRYPOINT ["/entrypoint.sh"]
