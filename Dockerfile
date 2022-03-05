FROM alpine:latest

RUN apk add --no-cache git gcc musl-dev && \
    git clone https://github.com/bucanero/ps3iso-utils.git && \
    find ps3iso-utils -type f -name "*.c" -exec \
    sh -c 'OFILE=`basename "{}" ".c"` && gcc "{}" -o /usr/local/bin/"$OFILE"' \; && \
    rm -rf ps3iso-utils && \
    apk del git gcc musl-dev

WORKDIR /tmp
