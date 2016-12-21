FROM debian:latest
RUN apt-get -y update
RUN apt-get install -y --force-yes build-essential