# Windows only until I bother to port the C++ code required.
# Also requires python-magic to be setup which is a pain on windows :(

import urllib2
import subprocess
import magic
import os

mime = magic.Magic(magic_file="C:\\Python27\\share\\file\\magic.mgc") # dirty hack also for now. Hard to install python-magic on win32
content = urllib2.urlopen("http://www.audio-surf.com/as/asradio/game_asradiolist5.php").read()
stuff = content.split("-:*x-")
for url in stuff:
    if url.find("http") != -1:
        if url.find("cgr") != -1:
            name = url.split("/")[-1]
            print "Downloading: " + name
            outputFile = open(name, "wb")
            cbrFile = urllib2.urlopen(url)
            outputFile.write(cbrFile.read())
            outputFile.close()
            subprocess.call(["convert-cgr.exe", name])
            name = os.path.splitext(name)[0] + ".ogg"
            fileData = mime.from_file(name)
            if fileData.find("Ogg") != -1:
                # It's really an ogg, do nothing.
                print "File actually is an Ogg!"
                print "Created " + name
            elif fileData.find("ID3") != -1:
                #It's actually an mp3, rename the punk.
                print "File actually is an MP3!"
                oldName = name
                name = os.path.splitext(name)[0] + ".mp3"
                os.rename(oldName, name)
                print "Created " + name
            os.remove(os.path.splitext(name)[0] + ".cgr")
print "Finished downloading all available Audiosurf Radio songs!"
# print content