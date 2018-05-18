FROM resin/rpi-raspbian:stretch
MAINTAINER Jan Kozak

RUN apt-get update
RUN apt-get install wget
# RUN apt-get install software-properties-common python
RUN wget -qO- https://dl.influxdata.com/influxdb/releases/influxdb-1.5.2_linux_armhf.tar.gz | tar xz -C /tmp
RUN cd /tmp/influxdb-1.5.2-1/ && cp -R * / && rm -fr /tmp/influxdb-1.5.2-1

RUN rm /etc/influxdb/influxdb.conf
COPY ./influxdb.conf /etc/influxdb/influxdb.conf

CMD ["influxd"] 
