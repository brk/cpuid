#!/usr/bin/env python
#
# Copyright 2007 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#



import os
import wsgiref.handlers

from google.appengine.ext.webapp import template
from google.appengine.ext import webapp

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

class MainPage(webapp.RequestHandler):

  def get(self):
  	path = file_path('index.html')
  	all_results = get_all_results()
  	template_values = {
  		'db': all_results
	}
  	self.response.out.write(template.render(path, template_values))


def main():
  application = webapp.WSGIApplication([('/', MainPage)],
                                       debug=True)
  wsgiref.handlers.CGIHandler().run(application)


if __name__ == '__main__':
  main()
