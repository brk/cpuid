#!/usr/bin/env python
#
# Copyright (c) 2009 Ben Karel. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file or at http://eschew.org/txt/bsd.txt

import os
import wsgiref.handlers
import sys
import cgi

from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db
from google.appengine.api import users


def file_path(filename):
	return os.path.join(os.path.dirname(__file__), filename)

def get_all_results():
	return {
		'compilers': [
			{
				'name': 'bitcc',
				'languages': ['bitc'],
				'srcdate': '2009-06-01',
				'version': '0.10',
				'repository': ''
			},
			
			{
				'name': 'foster',
				'languages': ['foster'],
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
				'language': 'bitc'
			}
		],
		'results': [
			{
				'date':'2009-08-01',
				'in_name': 'n',
				'out_name': 'walltimes',
				'compile_ms': 0.1,
				'in_keys': '[ 32, 64, 128 ]',
				'out_values': '[ 12.3, 242.6, 920.13 ]',
				'machine_name': 'godwin',
				'benchmark_name': 'bitc.benchmark.nbody',
				'fixed-parameters': '-O2',
				'geom_mean': '143.8'
			},
      {
				'date':'2009-08-02',
				'in_name': 'nprocs',
				'out_name': 'walltime_ms',
				'compile_ms': 0.2,
				'in_keys': '[ 16, 32, 64 ]',
				'out_values': '[ 123, 24.26, 9.20 ]',
				'machine_name': 'godwin',
				'benchmark_name': 'bitc.benchmark.nbody',
				'fixed-parameters': '-O2',
				'geom_mean': '43.8'
			}
		]
	}

class Compiler(db.Model):
  name       = db.StringProperty(required=True)
  languages  = db.StringListProperty(required=True)
  srcdate    = db.DateTimeProperty(required=False)
  version    = db.StringProperty(required=True)
  repository = db.LinkProperty(required=False)
  
class Machine(db.Model):
  name        = db.StringProperty(required=True)
  description = db.TextProperty(required=False)
  private_key = db.TextProperty(required=True)
  owner       = db.UserProperty(required=True)
  
class Benchmark(db.Model):
  name        = db.StringProperty(required=True)
  language    = db.StringProperty(required=True)
  repository  = db.LinkProperty(required=False)
  sourcecode  = db.TextProperty(required=False)

class BenchmarkResult(db.Model):
  date             = db.DateTimeProperty(required=True)
  # for example: n (size of problem), t (sequential timings)
  in_name          = db.StringProperty(required=True) # independent variable
  out_name         = db.StringProperty(required=True) # dependent variable
  in_keys          = db.StringListProperty(required=True)
  out_values       = db.StringListProperty(required=True)
  compile_ms       = db.FloatProperty(required=False)
  compile_command  = db.TextProperty(required=False)
  run_command      = db.TextProperty(required=False)
  machine          = db.ReferenceProperty(Machine)
  benchmark        = db.ReferenceProperty(Benchmark)
  fixed_parameters = db.TextProperty(required=True)
  owner            = db.UserProperty(required=True)

def get_venchup_contents(machine_name, privkey):
  return template.render(
      file_path('venchup.py'),
      { 'private_key' : privkey, 'machine_name': machine_name })

class MainPage(webapp.RequestHandler):
  def get(self):
    user = users.get_current_user()
    path = file_path('index.html')
    all_results = get_all_results()
    template_values = {
      'db': all_results,
      'user': user,
      'login_url': users.create_login_url(self.request.url),
      'logout_url': users.create_logout_url(self.request.url)
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
    user = users.get_current_user()
    if not user:
      self.redirect(users.create_login_url(self.request.url))
      return
      
    import re
    machine = self.request.get('machine-name')
    # allow v8-whatever
    machine_name_pattern = r'[a-zA-Z][a-zA-Z0-9-]{1,}(?:-[a-zA-Z0-9-]{3,})*'
    if not re.match(machine_name_pattern, machine):
      self.response.out.write("Invalid machine name!")
      return
      
    # make sure we don't already have a machine with that name
    q = Machine.gql('WHERE name = :name LIMIT 1', name=machine)
    if q.get():
      self.response.out.write("Error: a machine with that name already exists!")
      return
    
    from Crypto.PublicKey import RSA
    from Crypto.Util import randpool
    import cPickle
    
    rp = randpool.RandomPool()
    RSAkey = RSA.generate(256, rp.get_bytes)
    # escape newlines in pickled string
    privkey = cPickle.dumps(RSAkey, 0)
    newline_escaped_privkey = privkey.replace('\n', r'\n')
    
    # Create an entry in the data store
    dbmach = Machine(name=machine, private_key=privkey, owner=user)
    dbmach.put()
    
    self.response.out.write("""
<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" type="text/css" href="/css/venchmarks.css">
  </head>
  <body>
  <p>Your new machine named <pre>%(machine-name)s</pre> has internal key %(ds_key)s</p>
  
  Copy the following <kbd>venchup.py</kbd> script,
  or <a href="venchup.py?machine=%(machine-name)s&privkey=%(privkey)s">download it directly</a>:<br>
  <textarea cols=80 rows=15>%(venchup.py)s</textarea>
  <br>
  <br>
  <a href="/">Return to main page</a>
  </body>
</html>""" % {
  'machine-name': cgi.escape(machine),
  'venchup.py' : get_venchup_contents(machine, newline_escaped_privkey),
  'privkey': cgi.escape(newline_escaped_privkey),
  'ds_key': dbmach.key()
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

class UsersViewPage(webapp.RequestHandler):
  def get(self, user_str):
    if user_str == 'me':
      user = users.get_current_user()
    else:
      user = None
      
    if not User:
      self.response.out.write("You must be logged in to view your user page!")
    else:
      my_machines = db.GqlQuery("SELECT * FROM Machine WHERE owner = :1", user)
      path = file_path('user.html')
      template_values = {
        'user': user,
        'login_url': users.create_login_url(self.request.url),
        'logout_url': users.create_logout_url(self.request.url),
        'machines': my_machines
      }
      self.response.out.write(template.render(path, template_values))
      
def main():
  application = webapp.WSGIApplication([
                      ('/', MainPage),
                      ('/register-machine', RegisterMachinePage),
                      ('/venchup.py', VenchupPyDownload),
                      ('/users/view/(.*)', UsersViewPage)
                  ],
                                       debug=True)
  wsgiref.handlers.CGIHandler().run(application)


if __name__ == '__main__':
  main()
