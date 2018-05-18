#!/usr/bin/env python

import sys

sys.path.append('./connector')

import subprocess
import time
import argparse
import connector.mysql

def main(host, port):
    print "lol"

def parse_args():
    """Parse the args."""
    parser = argparse.ArgumentParser(description='example code to play with InfluxDB')
    parser.add_argument('--host', type=str, required=False,default='localhost',help='hostname of InfluxDB http API')
    parser.add_argument('--port', type=int, required=False, default=8086,help='port of InfluxDB http API')
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    main(host=args.host, port=args.port)
    
