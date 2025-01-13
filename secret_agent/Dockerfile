FROM golang:latest AS build-stage

WORKDIR /src
COPY go.mod ./
<<<<<<< HEAD:mqtt-docker/Dockerfile
COPY go.sum ./
RUN go mod download

=======
RUN go mod download
>>>>>>> patriks:secret_agent/Dockerfile

COPY *.go ./

COPY go_cert go_cert
# lägga till fler mappar?

RUN CGO_ENABLED=0 GOOS=linux go build -o /server

FROM gcr.io/distroless/base-debian11 AS build-release-stage

WORKDIR /
<<<<<<< HEAD:mqtt-docker/Dockerfile
COPY cert cert
COPY names.txt names.txt
COPY --from=build-stage /server /server

EXPOSE 9191
EXPOSE 8883
=======
COPY --from=build-stage /server /server

COPY --from=build-stage /src/go_cert go_cert

EXPOSE 8081
>>>>>>> patriks:secret_agent/Dockerfile

ENTRYPOINT [ "/server" ]


