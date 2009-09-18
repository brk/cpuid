#!/usr/bin/env python
#
# Copyright (c) 2009 Ben Karel. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.

from __future__ import with_statement
import urllib2
import urllib
import hashlib
import hmac

privkey = """{{ private_key }}"""

def form_data(payload):
  global privkey
  
  hexmac = hmac.new(privkey, payload, hashlib.sha1).hexdigest()
  return urllib.urlencode( { 'hexsha1' : hexmac, 'payload' : payload } )

if __name__ == '__main__':
  import sys
  if len(sys.argv) == 1: # no argument provided, print help
    print """Usage:
    %s <in file>     -- upload JSON data from given file
    %s -             -- upload JSON data from stdin""" % (sys.argv[0], sys.argv[0])
    print
    print "Example data:"
    print "private key: ", privkey
    payload = '{ "hello" : "world" }'
    print "payload: ", payload
    print "form data: ", form_data(payload)
    sys.exit(0)
  if len(sys.argv) == 2: # one argument provided: file or stdin?
    source = sys.argv[1]
    if source == '-':
      payload = sys.stdin.read()
    else:
      with open(sys.argv[1]) as f:
        payload = f.read()