#!/usr/bin/env python
#
# Copyright (c) 2009 Ben Karel. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file or at http://eschew.org/txt/bsd.txt

import os
import wsgiref.handlers
import sys
import cgi

from google.appengine.ext.webapp import template
from google.appengine.ext import webapp
from google.appengine.api import users

def file_path(filename):
	return os.path.join(os.path.dirname(__file__), filename)

def get_all_results():
	return {
		'compilers': [
			{
				'name': 'bitcc',
				'languages': 'bitc',
				'srcdate': '2009-06-01',
				'version': '0.10',
				'repository': ''
			},
			
			{
				'name': 'foster',
				'languages': 'foster',
				'srcdate': '2009-06-10',
				'version': '20060610',
				'repository': ''
			}
		],
		'machines': [
			{
				'name': 'godwin',
				'specs': "very nice!"
			}
		],
		'benchmarks': [
			{
				'name': 'nbody',
				'language': 'bitc',
				'src': '?',
				'repository': ''
			}
		],
		'results': [
			{
				'date':'2009-08-01',
				'in-name': 'n',
				'out-name': 'walltimes',
				'compile-time': 0.1,
				'in-array': '[ 32, 64, 128 ]',
				'out-array': '[ 12.3, 242.6, 920.13 ]',
				'compile-command': '',
				'run-command': '',
				'machine-id': '?',
				'benchmark-id': '?',
				'fixed-parameters': '-O2',
				'arith-mean': '143.8'
			}
		]
	}

def get_venchup_contents(machine_name, privkey):
  return template.render(
      file_path('venchup.py'),
      { 'private_key' : privkey, 'machine_name': machine_name })

class MainPage(webapp.RequestHandler):
  
  def get(self):
  	path = file_path('index.html')
  	all_results = get_all_results()
  	template_values = {
  		'db': all_results
	  }
  	self.response.out.write(template.render(path, template_values))

class VenchupPyDownload(webapp.RequestHandler):
  def get(self):
    machine = self.request.get('machine')
    privkey = self.request.get('privkey')
    if not machine or not privkey:
      self.response.out.write("Error: must provide machine name and private key!")
      return
      
    self.response.headers["Content-Type"] = 'text/plain'
    self.response.out.write(get_venchup_contents(machine, privkey))

class RegisterMachinePage(webapp.RequestHandler):
  def post(self):
    import re
    machine = self.request.get('machine-name')
    # allow v8-whatever
    machine_name_pattern = r'[a-zA-Z][a-zA-Z0-9-]{1,}(?:-[a-zA-Z0-9-]{3,})*'
    if not re.match(machine_name_pattern, machine):
      self.response.out.write("Invalid machine name!")
      return
    
    from Crypto.PublicKey import RSA
    from Crypto.Util import randpool
    import cPickle
    
    rp = randpool.RandomPool()
    RSAkey = RSA.generate(256, rp.get_bytes)
    # escape newlines in pickled string
    privkey = cPickle.dumps(RSAkey, 0).replace('\n', r'\n')
    #style="border: 2px dotted #333;"
    
    self.response.out.write("""
<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" type="text/css" href="/css/venchmarks.css">
  </head>
  <body>
  <p>Your new machine name: <pre>%(machine-name)s</pre></p>
  
  Copy the following <kbd>venchup.py</kbd> script,
  or <a href="venchup.py?machine=%(machine-name)s&privkey=%(privkey)s">download it directly</a>:<br>
  <textarea cols=80 rows=15>%(venchup.py)s</textarea>
  </body>
</html>""" % {
  'machine-name': cgi.escape(machine),
  'venchup.py' : get_venchup_contents(machine, privkey),
  'privkey': cgi.escape(privkey)
  })
    
  def get(self):
    user = users.get_current_user()
    if not user:
      self.redirect(users.create_login_url(self.request.url))
    else: # user logged in
      self.response.out.write("""
<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" type="text/css" href="/css/venchmarks.css">
  </head>
  <body>
    <form action="%s" method="post"
      <div>
      <span>Machine Name:</span><br>
      <span style="font-size: 90%%; color: #999;">
        Must be at least two groups of letters and numbers,
        connected with dashes.
      </span><br>
      <span>Example: <span class="monospace">myproject-mymachine</span></span><br>
      <input type="text" class="monospace" value="" name="machine-name">
        <input type="submit" value="Register!">
      </div>
  </body>
</html>
      """ % self.request.url)

def main():
  application = webapp.WSGIApplication([
                      ('/', MainPage),
                      ('/register-machine', RegisterMachinePage),
                      ('/venchup.py', VenchupPyDownload)
                  ],
                                       debug=True)
  wsgiref.handlers.CGIHandler().run(application)


if __name__ == '__main__':
  main()
