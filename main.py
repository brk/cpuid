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

class MainPage(webapp.RequestHandler):

  def get(self):
  	  path = file_path('index.html')
  	  template_values = {}
  	  self.response.out.write(template.render(path, template_values))


def main():
  application = webapp.WSGIApplication([('/', MainPage)],
                                       debug=True)
  wsgiref.handlers.CGIHandler().run(application)


if __name__ == '__main__':
  main()
