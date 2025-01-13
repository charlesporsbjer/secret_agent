FROM golang:latest AS build-stage

WORKDIR /src
COPY go.mod ./
RUN go mod download

COPY *.go ./

COPY go_cert go_cert
# lägga till fler mappar?

RUN CGO_ENABLED=0 GOOS=linux go build -o /server

FROM gcr.io/distroless/base-debian11 AS build-release-stage

WORKDIR /
COPY --from=build-stage /server /server

COPY --from=build-stage /src/go_cert go_cert

EXPOSE 8081

ENTRYPOINT [ "/server" ]


