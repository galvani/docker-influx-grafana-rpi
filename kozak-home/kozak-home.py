#!/usr/bin/env python

import sys
import subprocess
import time

def main():
    waitForNodeListener = 3;
    proc = subprocess.Popen(['python3', '-u', 'node-master.py'],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    
    print "Your app's PID is %s. Waiting for start up..." % proc.pid

    start = time.time()
    elapsed = 0

    print start
    try:
        while (elapsed < waitForNodeListener):
            nextline = proc.stdout.readline()
            if nextline != '' or proc.poll() is None:
                print "CHILD: {}".format(nextline.rstrip())

            if proc.poll()==None:
                # Process all what is necessary
                current = time.time()
                elapsed = current - start
                time.sleep(.1)
            else:
                print "process finished, restarting...\n"
                proc = subprocess.Popen(['python3', '-u', 'node-master.py'],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT)
                
            time.sleep(.2)
    finally:
        proc.terminate()
    app(proc)

def app(monitor):
    while (True):
        print ("App is running, child [{}] is running\n".format(monitor.pid))
        time.sleep(1)

main()