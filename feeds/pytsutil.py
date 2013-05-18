#!/usr/bin/python

import urllib2, time, hashlib, os

def mkdir(n):
    try:
        os.mkdir(n)
    except:
        pass

class HttpPoll:
    def __init__(self, url, ext="html", fetchDir="data"):
        self.url = url
        self.ext = ext
        self.fetchDir = fetchDir
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
                file = open(self.fetchDir+"/"+urlFile+time.strftime('.%Y%m%d-%H%M%S.'+self.ext), "wb")
                if self.ext == "html":
                    file.write(unicode(page.info()))
                file.write(unicode(pageText))
                file.close()
                self.hashOld = hash
                return pageText

        return None

def pollUrl(sender, url, sleepMs, fetchDir, replay, parsePage):
    if replay:
        loop = 0
        for root, dirs, files in os.walk(fetchDir):
            for name in files:
                print loop, name
                page = open(os.path.join(root, name)).read()
                parsePage(sender, page)
                loop+=1
    else:
        poll = HttpPoll("http://indexes.nikkei.co.jp/nkave/historical/nikkei_stock_average_daily_en.csv", "csv")
        while True:
            page = poll.pollHttp()
            if page: parsePage(sender, page)
            time.sleep(sleepMs)
