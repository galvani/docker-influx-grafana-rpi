FROM jsdidierlaurent/rpi-grafana
MAINTAINER Jan Kozak

RUN grafana-cli plugins install grafana-clock-panel
RUN grafana-cli plugins install grafana-piechart-panel

CMD ["influxd"] 
