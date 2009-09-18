#!/usr/bin/env python
#
# Copyright (c) 2009 Ben Karel. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file or at http://eschew.org/txt/bsd.txt

from __future__ import with_statement
import urllib2
import urllib
import hashlib
import hmac

global machine
machine = """{{ machine_name }}"""
privkey = """{{ private_key }}"""

def form_data(payload, private_key):
  hexmac = hmac.new(private_key, payload, hashlib.sha1).hexdigest()
  return urllib.urlencode( { 'hexsha1' : hexmac, 'payload' : payload } )

if __name__ == '__main__':
  import sys
  
  if len(sys.argv) == 1: # no argument provided, print help
    print """  Script to upload venchmark results for machine '%(machine)s'
  Usage:
    %(script)s <in file>     -- upload JSON data from given file
    %(script)s -             -- upload JSON data from stdin""" % (
      { 'script': sys.argv[0] , 'machine': machine })
    # print example_data()
    sys.exit(0)
    
  if len(sys.argv) == 2: # one argument provided: file or stdin?
    source = sys.argv[1]
    if source == '-':
      payload = sys.stdin.read()
    else:
      with open(sys.argv[1]) as f:
        payload = f.read()