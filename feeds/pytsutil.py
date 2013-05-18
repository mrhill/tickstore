#!/usr/bin/python

import urllib2, time, hashlib, os

class HttpPoll:
    def __init__(self, url, ext="html"):
        self.url = url
        self.ext = ext
        self.hashOld = 0
        self.pollCount = 0

    def pollHttp(self):
        self.pollCount += 1
        code = 0
        try:
            page = urllib2.urlopen(self.url)
        except urllib2.HTTPError, e:
            code = e.code

        print self.pollCount, time.strftime('%Y%m%d-%H%M%S'), self.url, code

        if code == 0:
            pageText = page.read().decode('utf-8', errors="ignore")
            hash = hashlib.sha1(pageText).digest()
            if hash != self.hashOld:
                (urlPath, urlFile) = os.path.split(self.url)
                file = open("data/"+urlFile+time.strftime('.%Y%m%d-%H%M%S.'+self.ext), "wb")
                if self.ext == "html":
                    file.write(unicode(page.info()))
                file.write(unicode(pageText))
                file.close()
                self.hashOld = hash
                return pageText

        return None
