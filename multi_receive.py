#!/usr/bin/env python

from RF24 import *
from RF24Network import *
from RF24Mesh import *
import time
import datetime
import argparse
from struct import *
from influxdb import InfluxDBClient


def main(host='localhost', port=8086):
	# radio setup for RPi B Rev2: CS0=Pin 24
	radio = RF24(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ)
	network = RF24Network(radio)
	mesh = RF24Mesh(radio, network)

	mesh.setNodeID(0)
	mesh.begin()
	radio.setPALevel(RF24_PA_MAX) # Power Amplifier
        radio.setAutoAck(True)
        radio.enableDynamicPayloads()
        radio.enableAckPayload()
	radio.printDetails()

	while 1:
		mesh.update()
		mesh.DHCP()
#		while network.available():
#                    receive_payload = radio.read(radio.getDynamicPayloadSize())
#                    print('Got payload size={} value="{}"'.format(len(receive_payload), receive_payload.decode('utf-8')))


		header, payload = network.read(128)
		node = oct(header.from_node)
                timeStamp = datetime.datetime.utcnow()
		if chr(header.type) == "H":
			temperature, humidity = unpack('<fL', bytes(payload))

			print('{} Received payload temperature: {:+.2f}, humidity: {}% from node {}'.format(timeStamp, temperature, humidity, node))
			json_body = [
				{
					"measurement": "rooms",
					"tags": {
						"node": node
					},
					"time": timeStamp,
					"fields": {
						"Temperature": temperature,
						"Humidity": humidity
					}
				}
			]
			send_to_db(json_body)

		if chr(header.type) == "S":
			status = ord(payload)
			timeStamp = datetime.datetime.utcnow()

			print('{} Received status update: {} from node {}'.format(timeStamp, status, node))
			json_body = [
				{
					"measurement": "status",
					"tags": {
						"node": node
					},
					"time": timeStamp,
					"fields": {
						"Status": status
					}
				}
			]
			send_to_db(json_body)
		if chr(header.type) == "I":
			print("Received node info from node {}: {} bytes {}".format(node, len(payload), payload))
			json_body = [
			{
				"measurement": "nodeInfo",
				"tags": {
					"node": node
				},
				"time": timeStamp,
				"fields": {
					"Info": payload,
				}
			}
	        ]
			#send_to_db(json_body)

		time.sleep(.1)

	print('Exiting, no network...')


def send_to_db(json_body, host='localhost', port=8086):
	"""Instantiate a connection to the InfluxDB."""
	user 		= 'admin'
	password	= 'admin'
	dbname 		= 'letna'
	

	client = InfluxDBClient(host, port, user, password, dbname)
	
	#print("Create database: " + dbname)
	#client.create_database(dbname)

	#print("Create a retention policy")
	client.create_retention_policy('awesome_policy', '3d', 3, default=True)

	#print("Write points: {0}".format(json_body))
	response = client.write_points(json_body)


def parse_args():
    """Parse the args."""
    parser = argparse.ArgumentParser(description='example code to play with InfluxDB')
    parser.add_argument('--host', type=str, required=False,default='localhost',help='hostname of InfluxDB http API')
    parser.add_argument('--port', type=int, required=False, default=8086,help='port of InfluxDB http API')
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    main(host=args.host, port=args.port)
    
