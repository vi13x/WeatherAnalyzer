FROM golang:1.24.5-alpine AS builder
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download
COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -o weather-bot .

FROM alpine:3.20
RUN apk --no-cache add ca-certificates
WORKDIR /root/
COPY --from=builder /app/weather-bot .
ENV WEATHER_TOKEN="8318514118:AAHM_yFNXIRGUp5DWtVIfbuWS1qNdfzbTfU"
CMD ["./weather-bot"]
