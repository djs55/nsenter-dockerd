FROM alpine:3.2 as base
RUN apk update && apk add build-base
COPY nsenter1.c /
RUN cc -O3 -Wall -static nsenter1.c -o /usr/bin/nsenter1

FROM scratch
COPY --from=base /usr/bin/nsenter1 /usr/bin/nsenter1
ENTRYPOINT ["/usr/bin/nsenter1"]
